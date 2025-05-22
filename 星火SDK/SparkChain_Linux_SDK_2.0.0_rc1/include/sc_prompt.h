#ifndef SC_PROMPT_API_H
#define SC_PROMPT_API_H
#include "sc_type.h"
namespace SparkChain {

class SPARKCHAIN_API Prompt {
public:
    explicit Prompt(const char* input);
    ~Prompt();
    Prompt(const Prompt&) = delete;
    Prompt operator=(const Prompt&) = delete;

    void fill(const char* key, const char* value);
    const char* getTemplate();
    const char* format();
    void loadPrompt(const char* filePath);
    void save(const char* filePath);
private:
    class PromptImpl;
    PromptImpl* pImpl;
};

}
#endif
