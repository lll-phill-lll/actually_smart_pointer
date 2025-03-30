#include "actually_smart_pointer.hpp"
#include "llama.h"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <utility>

namespace asp {

#ifndef DEFAULT_MODEL_PATH
#define DEFAULT_MODEL_PATH "models/deepseek-coder-6.7b-instruct.Q4_K_M.gguf"
#endif

// === Logging Macro ===
#ifdef LLM_DEBUG_LOG_ENABLED
    #define LOG(this_ptr) (std::clog << "[LLM][" << (void*)(this_ptr) << "] ")
#else
    #define LOG(this_ptr) if (false) std::clog
#endif

#define TRUE_STR "true"
#define FALSE_STR "false"

// === LLM singleton ===
class LLM {
public:
    static LLM& instance() {
        static LLM inst;
        return inst;
    }

    std::string ask_with_context(const std::string& question, const std::string& history) {
        ensure_initialized();
        std::string full_prompt = history + "Q: " + question + "()\nA:";

        LOG(this) << "Prompt:\n" << full_prompt << std::endl;

        std::string reply = ask_raw(full_prompt);

        LOG(this) << "Raw Response:\n" << reply << std::endl;

        // Clean up: extract first non-empty trimmed line
        std::istringstream stream(reply);
        std::string cleaned;
        for (std::string line; std::getline(stream, line); ) {
            for (char& c : line) if (c == '\r') c = '\0'; // remove carriage returns
            if (!line.empty()) {
                cleaned = line;
                break;
            }
        }

        // Trim whitespace
        cleaned.erase(0, cleaned.find_first_not_of(" \t\n\r"));
        cleaned.erase(cleaned.find_last_not_of(" \t\n\r") + 1);

        LOG(this) << "Parsed Response: \"" << cleaned << "\"" << std::endl;

        if (cleaned == TRUE_STR) return TRUE_STR;
        return FALSE_STR;
    }

private:
    LLM() : model(nullptr), ctx(nullptr), vocab(nullptr), sampler(nullptr) {}

    void ensure_initialized() {
        if (model) return;

        llama_log_set([](ggml_log_level, const char *, void *) {}, nullptr);
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

    std::string ask_raw(const std::string& prompt, int max_tokens = 2) {
        llama_kv_self_clear(ctx);
        const int n_prompt = -llama_tokenize(vocab, prompt.c_str(), prompt.size(), NULL, 0, true, true);
        std::vector<llama_token> prompt_tokens(n_prompt);
        if (llama_tokenize(vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), n_prompt, true, true) < 0) {
            return "error";
        }

        llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
        if (llama_decode(ctx, batch)) {
            return "error";
        }

        std::string result;

        for (int i = 0; i < max_tokens; ++i) {
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
control_block<T>::control_block(T* p)
    : ptr(p) {
    history = "You act as a custom shared_ptr implementation responsible for managing memory via reference counting. "
              "I will report the names of methods called on the smart pointer. "
              "Based on the sequence of operations, respond only with \"true\" if the underlying memory should be deleted, or \"false\" if it should not. "
              "Your responses must be strictly limited to either \"true\" or \"false\", with no additional commentary.\n";
}

template <typename T>
control_block<T>::~control_block() {
    delete ptr;
}

template <typename T>
void control_block<T>::append_interaction(const std::string& method, const std::string& reply) {
    history += "Q: " + method + "()\n";
    history += "A: " + reply + "\n";
}

// === actually_smart_pointer ===

template <typename T>
actually_smart_pointer<T>::actually_smart_pointer(T* ptr)
    : ctrl_(new control_block<T>(ptr)) {
    LOG(this) << "actually_smart_pointer::constructor" << std::endl;
}

template <typename T>
actually_smart_pointer<T>::actually_smart_pointer(const actually_smart_pointer& other)
    : ctrl_(other.ctrl_) {
    LOG(this) << "actually_smart_pointer::copy_constructor" << std::endl;

    std::string reply = LLM::instance().ask_with_context("copy", ctrl_->history);
    ctrl_->append_interaction("copy", reply);
}

template <typename T>
actually_smart_pointer<T>& actually_smart_pointer<T>::operator=(const actually_smart_pointer& other) {
    LOG(this) << "actually_smart_pointer::copy_assignment" << std::endl;

    if (this != &other) {
        std::string reply = LLM::instance().ask_with_context("assign", ctrl_->history);
        ctrl_->append_interaction("assign", reply);
        ctrl_ = other.ctrl_;
    }

    return *this;
}

template <typename T>
actually_smart_pointer<T>::actually_smart_pointer(actually_smart_pointer&& other) noexcept
    : ctrl_(other.ctrl_) {
    LOG(this) << "actually_smart_pointer::move_constructor" << std::endl;
    other.ctrl_ = nullptr;
}

template <typename T>
actually_smart_pointer<T>& actually_smart_pointer<T>::operator=(actually_smart_pointer&& other) noexcept {
    LOG(this) << "actually_smart_pointer::move_assignment" << std::endl;

    if (this != &other) {
        ctrl_ = other.ctrl_;
        other.ctrl_ = nullptr;
    }

    return *this;
}

template <typename T>
actually_smart_pointer<T>::~actually_smart_pointer() {
    if (ctrl_) {
        LOG(this) << "actually_smart_pointer::destructor" << std::endl;

        std::string reply = LLM::instance().ask_with_context("release", ctrl_->history);
        ctrl_->append_interaction("release", reply);

        if (reply == TRUE_STR) {
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
