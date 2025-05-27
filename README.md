    # M2_SDK - 智能语音交互系统

## 📋 项目概述

M2_SDK是一个集成的智能语音交互系统，结合了语音识别(ASR)、大语言模型(LLM)和语音合成(TTS)技术，为"精灵"人形机器人提供完整的语音对话能力。

### 🎯 主要功能
- **离线语音唤醒** - 支持唤醒词检测和角度定位
- **实时语音识别** - 基于科大讯飞SDK的在线语音转文字
- **智能对话** - 集成星火大模型，提供自然语言交互
- **语音合成** - 高质量的文字转语音输出
- **多轮对话** - 支持连续对话模式

### 🏗️ 系统架构

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   语音唤醒      │    │   语音识别       │    │   大语言模型     │
│  (Wakeup)      │───▶│    (ASR)        │───▶│    (LLM)        │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
┌─────────────────┐    ┌─────────────────┐             │
│   音频输出      │◀───│   语音合成       │◀────────────┘
│  (Speaker)     │    │    (TTS)        │
└─────────────────┘    └─────────────────┘
```

## 📁 项目结构

```
M2_SDK/
├── README.md                      # 项目说明文档
├── 69a4ef4ease                    # 应用ID配置
├── log/                           # 统一日志和通信文件目录
│   ├── asr_output.txt            # ASR识别结果
│   ├── asr_log.txt               # ASR运行日志
│   ├── tts_speaking              # TTS状态控制文件
│   └── file_location_changes.md  # 文件路径变更记录
├── scripts/                       # 工具脚本
│   └── setup_log_directory.sh    # 日志目录初始化脚本
├── 实时转写SDK/                    # 科大讯飞语音SDK
│   ├── bin/                      # 可执行文件
│   ├── include/                  # 头文件
│   ├── libs/                     # 动态库
│   └── doc/                      # 文档
├── 星火SDK/                       # 星火大模型SDK
│   └── SparkChain_Linux_SDK_2.0.0_rc1/
│       ├── include/              # SDK头文件
│       ├── libs/                 # SDK库文件
│       └── llm/online_llm/chat/  # 对话程序
└── offline_mic/                   # 麦克风处理
    ├── bin/                      # 编译后的可执行文件
    ├── samples/                  # 源代码样例
    ├── include/                  # 头文件
    └── libs/                     # 库文件
```

## 🚀 快速开始

### 系统要求
- **操作系统**: Ubuntu 22.04+ 
- **硬件**: 支持的麦克风阵列设备
- **依赖库**: ALSA, pthread, libcjson

### 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential libasound2-dev libcjson1 libcjson-dev

```

### 编译项目

1. **编译ASR模块**
```bash
cd offline_mic/samples/asr_offline_record_sample/
./64bit_make.sh
```

2. **编译星火SDK对话模块**
```bash
cd 星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat
./build.sh
```

### 初始化环境

```bash
# 设置库路径和初始化日志目录
scripts/setup_log_directory.sh
```

## 🎮 运行系统

### 方式一：独立运行

1. **启动ASR程序**
```bash
offline_mic/bin/asr_offline_record_sample
```

2. **启动对话程序**
```bash
星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat_demo
```



## 🔧 配置说明

### API密钥配置
在相关源文件中配置您的API凭据：
- **科大讯飞**: `appid = ······`
- **星火大模型**: 在`chat.cpp`中设置APPID、APIKEY、APISECRET

### 人物设定
系统内置"精灵"人形机器人人设，可在`chat.cpp`中的`PERSONA_SYSTEM_CONTENT`修改：
```cpp
const char* PERSONA_SYSTEM_CONTENT = "你是上海半醒科技有限公司开发的人形机器人，名字叫做"精灵"，为客户提供服务，语言要简洁明了，不啰嗦。";
```

## 💬 使用流程

1. **启动系统** - 运行上述命令启动ASR和LLM程序
2. **等待唤醒** - 系统显示"待唤醒，请用唤醒词进行唤醒！"
3. **语音唤醒** - 说出唤醒词，系统显示"唤醒成功！角度: [X]"
4. **自动问候** - 系统自动输出"你好精灵，请介绍一下你自己"
5. **开始对话** - 系统进入语音识别模式，可以开始对话
6. **多轮交互** - 支持连续对话，直到说"再见"或100秒无活动
7. **重新等待** - 返回唤醒等待状态

## 📊 系统监控

### 日志文件
- **ASR日志**: `log/asr_log.txt`
- **通信文件**: `log/asr_output.txt`
- **状态控制**: `log/tts_speaking`

### 实时监控
```bash
# 监控ASR日志
tail -f log/asr_log.txt

# 监控ASR输出
tail -f log/asr_output.txt
```

## 🛠️ 故障排除

### 常见问题

1. **库文件缺失错误**
```bash
error while loading shared libraries: libcjson.so.1: cannot open shared object file
```
**解决方案**: 安装libcjson库或设置LD_LIBRARY_PATH

2. **设备权限问题**
```bash
open /dev/wheeltec_mic is failed
```
**解决方案**: 检查设备权限和驱动安装

3. **API认证失败**
```bash
MSPLogin failed, Error code XXX
```
**解决方案**: 检查API密钥配置和网络连接

### 调试模式
编译时添加调试信息：
```bash
gcc -g -DDEBUG -o program source.c
```


## 🔄 更新日志

### v1.2.0 (2025-05-27)
- ✅ 统一通知文件路径到`log/`目录
- ✅ 优化TTS和ASR的协调机制
- ✅ 添加自动问候功能
- ✅ 改进日志管理和监控

### v1.1.0
- ✅ 集成星火大模型SDK
- ✅ 实现多轮对话功能
- ✅ 添加TTS语音合成

### v1.0.0
- ✅ 基础ASR功能实现
- ✅ 麦克风阵列支持
- ✅ 语音唤醒功能

## 📄 许可证

本项目采用MIT许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 常用维护命令

```bash
# 重启系统
pkill -f "asr_offline_record_sample"
pkill -f "chat_demo"
/home/bxi/M2_SDK/start_system.sh

# 清理日志
echo "" > /home/bxi/M2_SDK/log/asr_log.txt

# 检查库依赖
ldd /home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample

# 测试麦克风
arecord -l
---

*最后更新: 2025年5月27日*
  
    
