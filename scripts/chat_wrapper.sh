#!/bin/bash

# Chat程序启动包装脚本
echo "Starting Chat service wrapper..."

# 设置环境变量
export HOME=/home/bxi

# 设置库路径，包含SparkChain和libmsc.so所需的路径
export LD_LIBRARY_PATH="/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/libs:/home/bxi/M2_SDK/offline_mic/bin:/home/bxi/M2_SDK/offline_mic/libs/x64:/home/bxi/M2_SDK/实时转写SDK/libs/x64:/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"

# 预加载系统C++库来解决符号冲突
export LD_PRELOAD="/usr/lib/x86_64-linux-gnu/libstdc++.so.6"

# 确保日志目录存在
mkdir -p /home/bxi/M2_SDK/log

# 切换到程序目录
cd /home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat

# 检查必要的库文件
if [ ! -f "/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/libs/libSparkChain.so" ]; then
    echo "ERROR: libSparkChain.so not found"
    exit 1
fi

if [ ! -f "/home/bxi/M2_SDK/offline_mic/bin/libmsc.so" ]; then
    echo "ERROR: libmsc.so not found in offline_mic/bin"
    exit 1
fi

if [ ! -f "./chat_demo" ]; then
    echo "ERROR: chat_demo not found in current directory"
    exit 1
fi

# 检查可执行文件权限
if [ ! -x "./chat_demo" ]; then
    echo "ERROR: chat_demo is not executable"
    exit 1
fi

# 输出调试信息
echo "Environment setup:"
echo "  HOME=$HOME"
echo "  LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo "  LD_PRELOAD=$LD_PRELOAD"
echo "  PWD=$(pwd)"

echo "Launching Chat program..."

# 启动Chat程序
exec ./chat_demo
