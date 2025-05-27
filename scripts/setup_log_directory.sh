#!/bin/bash

# 设置日志目录的脚本
# 用于统一管理通知文件的位置

LOG_DIR="/home/bxi/M2_SDK/log"

echo "Setting up unified log directory at $LOG_DIR"

# 创建日志目录（如果不存在）
mkdir -p "$LOG_DIR"

# 设置合适的权限
chmod 755 "$LOG_DIR"

# 清理旧的临时文件
echo "Cleaning up old temporary files..."
rm -f /tmp/asr_output.txt
rm -f /tmp/tts_speaking

# 确保日志目录中的文件有正确的权限
if [ -f "$LOG_DIR/asr_output.txt" ]; then
    chmod 644 "$LOG_DIR/asr_output.txt"
fi

if [ -f "$LOG_DIR/tts_speaking" ]; then
    chmod 644 "$LOG_DIR/tts_speaking"
fi

if [ -f "$LOG_DIR/asr_log.txt" ]; then
    chmod 644 "$LOG_DIR/asr_log.txt"
fi

echo "Log directory setup completed!"
echo "All notification files will now be stored in: $LOG_DIR"
echo "Files managed:"
echo "  - asr_output.txt    (ASR recognition results)"
echo "  - asr_log.txt       (ASR program logs)"
echo "  - tts_speaking      (TTS status control file)"
