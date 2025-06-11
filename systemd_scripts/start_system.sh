#!/bin/bash

# 创建日志目录（如果不存在）
mkdir -p /home/bxi/M2_SDK/log

# 初始化日志文件
touch /home/bxi/M2_SDK/log/asr_log.txt
touch /home/bxi/M2_SDK/log/asr_output.txt
touch /home/bxi/M2_SDK/log/tts_speaking

# 设置权限
chmod 666 /home/bxi/M2_SDK/log/asr_log.txt
chmod 666 /home/bxi/M2_SDK/log/asr_output.txt
chmod 666 /home/bxi/M2_SDK/log/tts_speaking

# 启动ASR服务
/home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample &

# 等待5秒确保ASR服务已启动
sleep 5

# 启动对话服务
/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat_demo