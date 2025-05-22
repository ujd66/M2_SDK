#ifndef SC_LLMCHAIN_API_H
#define SC_LLMCHAIN_API_H

#include "sc_llm.h"
#include "sc_prompt.h"
#include "sc_type.h"
#include <map>
#include <string>

namespace SparkChain {

using LLMChainCallbacks = LLMCallbacks;
using LLMChainSyncOutut = LLMSyncOutput;

class SPARKCHAIN_API LLMChain {
public:
    LLMChain(Prompt* prompt, LLM* llm);
    LLMChain(Prompt* prompt);
    LLMChain(const LLMChain&) = delete;
    LLMChain operator=(const LLMChain&) = delete;
    
    ~LLMChain();
    void registerLLMChainCallbacks(LLMChainCallbacks* cbs);
    
    static LLM* getLLM(LLMConfig* llmConfig);                      
    static Prompt* promptTemplate(const char* tmpl);

    LLMChainSyncOutut* run(const std::map<std::string, std::string>& input = {});
    int arun(const std::map<std::string, std::string>& input = {}, void *usrTag = nullptr);

private:
    class LLMChainImpl;
    LLMChainImpl* pImpl;
};

}
#endif