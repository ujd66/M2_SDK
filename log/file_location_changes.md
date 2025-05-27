# 通知文件路径统一化更改记录

## 更改目的
将两个主要程序（ASR程序和星火SDK程序）生成的通知类文件统一放置在 `/home/bxi/M2_SDK/log/` 目录下，以便于管理和监控。

## 涉及的文件类型
1. **asr_output.txt** - ASR识别结果输出文件
2. **asr_log.txt** - ASR程序日志文件（stderr重定向）
3. **tts_speaking** - TTS播放状态控制文件

## 更改的文件

### 1. main.c (ASR程序)
**文件路径:** `/home/bxi/M2_SDK/offline_mic/samples/asr_offline_record_sample/main.c`

**内容:**
- /home/bxi/M2_SDK/log/tts_speaking`
- /home/bxi/M2_SDK/log/asr_output.txt`
- /home/bxi/M2_SDK/log/asr_log.txt`

**涉及的函数/位置:**
- `on_result()` - TTS状态检查和ASR结果输出
- `com_wakeup()` - TTS状态监控
- `main()` - 启动清理、日志重定向、会话循环中的TTS等待、程序结束清理

### 2. chat.cpp (星火SDK程序)
**文件路径:** `/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat.cpp`

**内容:**
- /home/bxi/M2_SDK/log/asr_output.txt`（读取ASR结果）
- /home/bxi/M2_SDK/log/tts_speaking`（TTS状态控制）

**涉及的函数/位置:**
- `SparkCallbacks::onLLMResult()` - TTS状态控制文件的创建和删除
- `run_Generation_Async()` - ASR输出文件的读取

## 新增文件

### 3. setup_log_directory.sh
**文件路径:** `/home/bxi/M2_SDK/scripts/setup_log_directory.sh`

**功能:**
- 创建统一的日志目录
- 设置正确的文件权限
- 清理旧的临时文件
- 提供设置状态报告

## 目录结构
```
/home/bxi/M2_SDK/log/
├── asr_output.txt    # ASR识别结果（ASR→LLM通信）
├── asr_log.txt       # ASR程序运行日志
└── tts_speaking      # TTS播放状态控制文件（LLM→ASR通信）
```

## 程序间通信流程
1. **ASR → LLM:** ASR将识别结果写入 `asr_output.txt`，LLM程序读取此文件
2. **LLM → ASR:** LLM开始TTS时创建 `tts_speaking` 文件，ASR检测到此文件时暂停监听
3. **LLM → ASR:** LLM完成TTS后删除 `tts_speaking` 文件，ASR恢复监听


## 注意事项
1. 确保 `/home/bxi/M2_SDK/log/` 目录有适当的读写权限
2. 定期清理日志文件以避免磁盘空间不足
3. 如果修改了用户主目录，需要相应调整路径

## 验证方法
运行设置脚本确保环境正确：
```bash
/home/bxi/M2_SDK/scripts/setup_log_directory.sh
```

检查目录权限：
```bash
ls -la /home/bxi/M2_SDK/log/
```
