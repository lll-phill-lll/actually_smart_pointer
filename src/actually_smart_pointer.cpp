#include "actually_smart_pointer.hpp"
#include "llama.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <mutex>
#include <typeinfo>

namespace asp {

static std::mutex llm_mutex;
static llama_model * model = nullptr;
static llama_context * ctx = nullptr;
static const llama_vocab * vocab = nullptr;
static llama_sampler * sampler = nullptr;

void ensure_llm_initialized(const std::string& model_path = "../models/deepseek-coder-6.7b-instruct.Q4_K_M.gguf") {
    if (model != nullptr) return;

    ggml_backend_load_all();

    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 99;
    model = llama_model_load_from_file(model_path.c_str(), model_params);
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

std::string ask_llm(const std::string& question) {
    std::lock_guard<std::mutex> lock(llm_mutex);
    ensure_llm_initialized();

    const int n_prompt = -llama_tokenize(vocab, question.c_str(), question.size(), NULL, 0, true, true);
    std::vector<llama_token> prompt_tokens(n_prompt);
    if (llama_tokenize(vocab, question.c_str(), question.size(), prompt_tokens.data(), n_prompt, true, true) < 0) {
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

// ===== control_block =====

template <typename T>
control_block<T>::control_block(T* p, const std::string& type)
    : ptr(p), ref_count(1), type_name(type) {}

template <typename T>
control_block<T>::~control_block() {
    delete ptr;
}

template <typename T>
void control_block<T>::retain() {
    ++ref_count;
}

template <typename T>
void control_block<T>::release() {
    if (--ref_count == 0) {
        std::string reply = ask_llm("Object of type '" + type_name + "' was released. Should it be deleted? Answer yes or no.");
        if (reply.find("yes") != std::string::npos) {
            delete this;
        }
    }
}

// ===== actually_smart_pointer =====

template <typename T>
actually_smart_pointer<T>::actually_smart_pointer(T* ptr)
    : ctrl_(new control_block<T>(ptr, typeid(T).name())) {}

template <typename T>
actually_smart_pointer<T>::actually_smart_pointer(const actually_smart_pointer& other)
    : ctrl_(other.ctrl_) {
    std::string reply = ask_llm("Object of type '" + ctrl_->type_name + "' was copied. Allow it? Answer yes or no.");
    if (reply.find("yes") != std::string::npos) {
        ctrl_->retain();
    } else {
        fprintf(stderr, "[LLM] copy rejected by model, not increasing ref count.\n");
    }
}

template <typename T>
actually_smart_pointer<T>& actually_smart_pointer<T>::operator=(const actually_smart_pointer& other) {
    if (this != &other) {
        ctrl_->release();
        ctrl_ = other.ctrl_;
        ctrl_->retain();
    }
    return *this;
}

template <typename T>
actually_smart_pointer<T>::~actually_smart_pointer() {
    ctrl_->release();
}

template <typename T>
T* actually_smart_pointer<T>::get() const { return ctrl_->ptr; }

template <typename T>
T& actually_smart_pointer<T>::operator*() const { return *(ctrl_->ptr); }

template <typename T>
T* actually_smart_pointer<T>::operator->() const { return ctrl_->ptr; }

template class actually_smart_pointer<int>;
template class actually_smart_pointer<std::string>;

} // namespace asp

