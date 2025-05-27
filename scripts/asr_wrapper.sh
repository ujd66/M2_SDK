#!/bin/bash

# ASR程序启动包装脚本
echo "Starting ASR service wrapper..."

# 设置环境变量
export HOME=/home/bxi

# 设置库路径，包含两个可能的libmsc.so位置
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:/home/bxi/M2_SDK/offline_mic/libs/x64:/home/bxi/M2_SDK/实时转写SDK/libs/x64:/home/bxi/M2_SDK/offline_mic/bin:$LD_LIBRARY_PATH"

# 预加载系统C++库来解决符号冲突
export LD_PRELOAD="/usr/lib/x86_64-linux-gnu/libstdc++.so.6"

# 确保日志目录存在
mkdir -p /home/bxi/M2_SDK/log

# 清理可能存在的控制文件
rm -f /home/bxi/M2_SDK/log/tts_speaking 2>/dev/null
rm -f /home/bxi/M2_SDK/log/asr_output.txt 2>/dev/null

# 切换到程序目录
cd /home/bxi/M2_SDK/offline_mic/bin

# 检查必要的库文件
if [ ! -f "/home/bxi/M2_SDK/offline_mic/libs/x64/libmsc.so" ] && [ ! -f "/home/bxi/M2_SDK/实时转写SDK/libs/x64/libmsc.so" ]; then
    echo "ERROR: libmsc.so not found in either location:"
    echo "  /home/bxi/M2_SDK/offline_mic/libs/x64/libmsc.so"
    echo "  /home/bxi/M2_SDK/实时转写SDK/libs/x64/libmsc.so"
    exit 1
fi

if [ ! -f "./asr_offline_record_sample" ]; then
    echo "ERROR: asr_offline_record_sample not found in current directory"
    exit 1
fi

# 检查可执行文件权限
if [ ! -x "./asr_offline_record_sample" ]; then
    echo "ERROR: asr_offline_record_sample is not executable"
    exit 1
fi

# 输出调试信息
echo "Environment setup:"
echo "  HOME=$HOME"
echo "  LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
echo "  LD_PRELOAD=$LD_PRELOAD"
echo "  PWD=$(pwd)"

# 检查库依赖 (可选，注释掉以减少启动时间)
# echo "Checking library dependencies..."
# ldd ./asr_offline_record_sample

echo "Launching ASR program..."

# 启动ASR程序
exec ./asr_offline_record_sample
