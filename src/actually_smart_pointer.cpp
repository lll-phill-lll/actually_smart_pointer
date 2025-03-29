
#include "actually_smart_pointer.hpp"
#include "llama.h"

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <typeinfo>

namespace asp {

#ifndef DEFAULT_MODEL_PATH
#define DEFAULT_MODEL_PATH "models/deepseek-coder-6.7b-instruct.Q4_K_M.gguf"
#endif

// === LLM singleton ===
class LLM {
public:
    static LLM& instance() {
        static LLM inst;
        return inst;
    }

    std::string ask_with_context(const std::string& question, const std::string& history) {
        ensure_initialized();
        std::string full_prompt = "Previous interactions:\n" + history + "Q: " + question + "\nA:";
        return ask_raw(full_prompt);
    }

private:
    LLM() : model(nullptr), ctx(nullptr), vocab(nullptr), sampler(nullptr) {}

    void ensure_initialized() {
        if (model) return;

        ggml_backend_load_all();

        llama_model_params model_params = llama_model_default_params();
        model_params.n_gpu_layers = 99;
        model = llama_model_load_from_file(DEFAULT_MODEL_PATH, model_params);
        vocab = llama_model_get_vocab(model);

        llama_context_params ctx_params = llama_context_default_params();
        ctx_params.n_ctx = 512;
        ctx_params.n_batch = 512;
        ctx_params.no_perf = true;
        ctx = llama_init_from_model(model, ctx_params);

        auto sparams = llama_sampler_chain_default_params();
        sparams.no_perf = true;
        sampler = llama_sampler_chain_init(sparams);
        llama_sampler_chain_add(sampler, llama_sampler_init_greedy());
    }

    std::string ask_raw(const std::string& prompt) {
        const int n_prompt = -llama_tokenize(vocab, prompt.c_str(), prompt.size(), NULL, 0, true, true);
        std::vector<llama_token> prompt_tokens(n_prompt);
        if (llama_tokenize(vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), n_prompt, true, true) < 0) {
            return "error";
        }

        llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
        if (llama_decode(ctx, batch)) return "error";

        std::string result;
        for (int i = 0; i < 20; ++i) {
            llama_token token = llama_sampler_sample(sampler, ctx, -1);
            if (llama_vocab_is_eog(vocab, token)) break;

            char buf[128];
            int n = llama_token_to_piece(vocab, token, buf, sizeof(buf), 0, true);
            if (n <= 0) break;
            result.append(buf, n);

            llama_batch token_batch = llama_batch_get_one(&token, 1);
            llama_decode(ctx, token_batch);
        }

        return result;
    }

    llama_model* model;
    llama_context* ctx;
    const llama_vocab* vocab;
    llama_sampler* sampler;
};

// === control_block ===

template <typename T>
control_block<T>::control_block(T* p, const std::string& type)
    : ptr(p), type_name(type) {}

template <typename T>
control_block<T>::~control_block() {
    delete ptr;
}

// === actually_smart_pointer ===

template <typename T>
actually_smart_pointer<T>::actually_smart_pointer(T* ptr)
    : ctrl_(new control_block<T>(ptr, typeid(T).name())), history_() {}

template <typename T>
actually_smart_pointer<T>::actually_smart_pointer(const actually_smart_pointer& other)
    : ctrl_(other.ctrl_), history_(other.history_) {
    std::string question = "Object of type '" + ctrl_->type_name + "' was copied. Allow it? Answer yes or no.";
    std::string reply = LLM::instance().ask_with_context(question, history_);
    history_ += "Q: " + question + "\n";
    history_ += "A: " + reply + "\n";

    if (reply.find("yes") == std::string::npos) {
        fprintf(stderr, "[LLM] Copy rejected by model. Object may be shared without permission.\n");
    }
}

template <typename T>
actually_smart_pointer<T>& actually_smart_pointer<T>::operator=(const actually_smart_pointer& other) {
    if (this != &other) {
        std::string question = "Pointer assignment occurred for type '" + ctrl_->type_name + "'. Allow? yes/no";
        std::string reply = LLM::instance().ask_with_context(question, history_);
        history_ += "Q: " + question + "\n";
        history_ += "A: " + reply + "\n";

        ctrl_ = other.ctrl_;
        history_ = other.history_;
    }
    return *this;
}

template <typename T>
actually_smart_pointer<T>::actually_smart_pointer(actually_smart_pointer&& other) noexcept
    : ctrl_(other.ctrl_), history_(std::move(other.history_)) {
    other.ctrl_ = nullptr;
}

template <typename T>
actually_smart_pointer<T>& actually_smart_pointer<T>::operator=(actually_smart_pointer&& other) noexcept {
    if (this != &other) {
        ctrl_ = other.ctrl_;
        history_ = std::move(other.history_);
        other.ctrl_ = nullptr;
    }
    return *this;
}

template <typename T>
actually_smart_pointer<T>::~actually_smart_pointer() {
    if (ctrl_) {
        std::string question = "Object of type '" + ctrl_->type_name + "' was released. Should it be deleted? Answer yes or no.";
        std::string reply = LLM::instance().ask_with_context(question, history_);
        history_ += "Q: " + question + "\n";
        history_ += "A: " + reply + "\n";

        if (reply.find("yes") != std::string::npos) {
            delete ctrl_;
        }
    }
}

template <typename T>
T* actually_smart_pointer<T>::get() const { return ctrl_->ptr; }

template <typename T>
T& actually_smart_pointer<T>::operator*() const { return *(ctrl_->ptr); }

template <typename T>
T* actually_smart_pointer<T>::operator->() const { return ctrl_->ptr; }

// Explicit instantiations
template class actually_smart_pointer<int>;
template class actually_smart_pointer<std::string>;

} // namespace asp

