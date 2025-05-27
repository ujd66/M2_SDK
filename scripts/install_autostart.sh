#!/bin/bash

# 安装ASR和Chat服务的自启动脚本
# 适用于Ubuntu 22.04系统

set -e

echo "开始安装ASR和Chat服务的自启动配置..."

# 检查是否以root权限运行
if [ "$EUID" -ne 0 ]; then
    echo "请以root权限运行此脚本 (使用sudo)"
    exit 1
fi

# 服务文件路径
SCRIPT_DIR="/home/bxi/M2_SDK/scripts"
ASR_SERVICE="$SCRIPT_DIR/asr-service.service"
CHAT_SERVICE="$SCRIPT_DIR/chat-service.service"

# 检查服务文件是否存在
if [ ! -f "$ASR_SERVICE" ]; then
    echo "错误: ASR服务文件不存在: $ASR_SERVICE"
    exit 1
fi

if [ ! -f "$CHAT_SERVICE" ]; then
    echo "错误: Chat服务文件不存在: $CHAT_SERVICE"
    exit 1
fi

# 检查可执行文件是否存在
ASR_EXEC="/home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample"
CHAT_EXEC="/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat_demo"

if [ ! -f "$ASR_EXEC" ]; then
    echo "错误: ASR可执行文件不存在: $ASR_EXEC"
    exit 1
fi

if [ ! -x "$ASR_EXEC" ]; then
    echo "警告: ASR文件没有执行权限，正在设置..."
    chmod +x "$ASR_EXEC"
fi

if [ ! -f "$CHAT_EXEC" ]; then
    echo "错误: Chat可执行文件不存在: $CHAT_EXEC"
    exit 1
fi

if [ ! -x "$CHAT_EXEC" ]; then
    echo "警告: Chat文件没有执行权限，正在设置..."
    chmod +x "$CHAT_EXEC"
fi

# 创建日志目录
echo "创建日志目录..."
mkdir -p /home/bxi/M2_SDK/log
chown bxi:bxi /home/bxi/M2_SDK/log

# 复制服务文件到systemd目录
echo "安装服务文件..."
cp "$ASR_SERVICE" /etc/systemd/system/
cp "$CHAT_SERVICE" /etc/systemd/system/

# 重新加载systemd配置
echo "重新加载systemd配置..."
systemctl daemon-reload

# 启用服务
echo "启用ASR服务..."
systemctl enable asr-service.service

echo "启用Chat服务..."
systemctl enable chat-service.service

# 检查服务状态
echo ""
echo "服务安装完成！"
echo ""
echo "可用的管理命令："
echo "1. 启动服务:"
echo "   sudo systemctl start asr-service"
echo "   sudo systemctl start chat-service"
echo ""
echo "2. 停止服务:"
echo "   sudo systemctl stop asr-service"
echo "   sudo systemctl stop chat-service"
echo ""
echo "3. 查看服务状态:"
echo "   sudo systemctl status asr-service"
echo "   sudo systemctl status chat-service"
echo ""
echo "4. 查看服务日志:"
echo "   sudo journalctl -u asr-service -f"
echo "   sudo journalctl -u chat-service -f"
echo ""
echo "5. 禁用自启动:"
echo "   sudo systemctl disable asr-service"
echo "   sudo systemctl disable chat-service"
echo ""
echo "现在重启系统后，服务将自动启动。"
echo "要立即测试，可以运行: sudo systemctl start asr-service && sudo systemctl start chat-service"
