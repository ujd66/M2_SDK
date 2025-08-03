#!/bin/bash

# =================================================================
# M2_SDK 智能语音交互系统 - 日志目录统一管理脚本
# =================================================================
# 文件名: 123.sh (建议重命名为 setup_log_directory.sh)
# 作用: 统一管理和初始化语音系统的日志文件目录
# 用途: 确保 ASR、LLM、TTS 三个模块能够通过统一的日志目录进行通信
# =================================================================

# --- 配置变量 (Configuration Variables) ---
# 定义统一的日志目录路径
# 注意: 这里仍然使用了硬编码的 /home/bxi/ 路径，在实际使用时需要修改为当前用户路径
LOG_DIR="/home/bxi/M2_SDK/log"

echo "Setting up unified log directory at $LOG_DIR"

# --- 目录创建和权限设置 (Directory Creation and Permission Setup) ---
# 创建日志目录（如果不存在的话）
# -p 参数: 如果父目录不存在，会自动创建父目录
mkdir -p "$LOG_DIR"

# 设置目录权限为 755
# 755 表示: 所有者(读写执行)，组用户(读执行)，其他用户(读执行)
chmod 755 "$LOG_DIR"

# --- 清理旧文件 (Cleanup Old Files) ---
# 清理系统中可能存在的旧版本临时文件
# 这些文件可能是之前版本在 /tmp/ 目录下创建的，现在需要统一迁移到 log/ 目录
echo "Cleaning up old temporary files..."
rm -f /tmp/asr_output.txt    # 删除旧的 ASR 输出文件
rm -f /tmp/tts_speaking      # 删除旧的 TTS 状态文件

# --- 文件权限设置 (File Permission Setup) ---
# 为日志目录中可能已存在的文件设置正确的权限
# 644 表示: 所有者(读写)，组用户(只读)，其他用户(只读)

# ASR 识别结果文件 - 存储语音识别的文字结果
if [ -f "$LOG_DIR/asr_output.txt" ]; then
    chmod 644 "$LOG_DIR/asr_output.txt"
fi

# TTS 状态控制文件 - 用于协调 TTS 和 ASR 模块，防止录音时播放语音造成干扰
if [ -f "$LOG_DIR/tts_speaking" ]; then
    chmod 644 "$LOG_DIR/tts_speaking"
fi

# ASR 程序日志文件 - 记录 ASR 模块的运行状态和调试信息
if [ -f "$LOG_DIR/asr_log.txt" ]; then
    chmod 644 "$LOG_DIR/asr_log.txt"
fi

# --- 完成提示 (Completion Messages) ---
echo "Log directory setup completed!"
echo "All notification files will now be stored in: $LOG_DIR"
echo "Files managed:"
echo "  - asr_output.txt    (ASR recognition results)    # ASR 语音识别结果文件"
echo "  - asr_log.txt       (ASR program logs)           # ASR 程序运行日志"
echo "  - tts_speaking      (TTS status control file)    # TTS 状态控制文件，用于模块间协调"

# =================================================================
# 脚本用途说明:
# 
# 1. **模块间通信**: M2_SDK 系统中，ASR、LLM、TTS 三个模块需要通过文件
#    进行状态同步和数据传递，这个脚本确保所有模块使用统一的文件位置。
#
# 2. **版本迁移**: 从早期版本的 /tmp/ 目录迁移到专门的 log/ 目录，
#    提高了文件管理的规范性和可维护性。
#
# 3. **权限管理**: 确保所有相关文件有正确的读写权限，避免权限问题
#    导致的模块间通信失败。
#
# 4. **系统初始化**: 作为系统启动前的准备工作，确保运行环境正确配置。
# =================================================================