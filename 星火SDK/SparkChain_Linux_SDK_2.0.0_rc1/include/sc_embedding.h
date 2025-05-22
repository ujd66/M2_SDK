#pragma once

#include "sc_type.h"
#include <vector>
#include <string>


namespace SparkChain {

class EmbeddingOutput {
public:
    /**
     * @brief 获取float32数组结果
     */
    std::vector<float> getResultVector();

    /**
     * @brief 获取网络返回raw结果, base64编码格式
     */
    std::string getRaw();

    /**
     * @brief 获取错误码
     */
    int getErrCode();

    /**
     * @brief 获取服务返回错误信息
     */
    std::string getErrMsg();

    /**
     * @brief 获取服务返回sid
     */
    std::string getSid();

public:
    void setSid(const std::string& sid);

    void setResultVector(const std::vector<float>& rv);

    void setRaw(const std::string& raw);

    void setErrCode(int errCode);

    void setErrMsg(const std::string& errMsg);

private:
    std::string         mRaw;
    std::string         mSid;
    std::string         mErrMsg;
    int                 mErrCode{-1};
    std::vector<float>  mResultVector;
};


class SPARKCHAIN_API Embedding {
public:
    static EmbeddingOutput embedding(const std::string& input, const std::string& domain, const std::string& uid = {});
};

}