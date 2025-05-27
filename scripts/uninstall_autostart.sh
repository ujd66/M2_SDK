#!/bin/bash

# 卸载ASR和Chat服务的自启动脚本

set -e

echo "开始卸载ASR和Chat服务的自启动配置..."

# 检查是否以root权限运行
if [ "$EUID" -ne 0 ]; then
    echo "请以root权限运行此脚本 (使用sudo)"
    exit 1
fi

# 停止服务
echo "停止服务..."
systemctl stop asr-service.service 2>/dev/null || echo "ASR服务未运行"
systemctl stop chat-service.service 2>/dev/null || echo "Chat服务未运行"

# 禁用服务
echo "禁用服务..."
systemctl disable asr-service.service 2>/dev/null || echo "ASR服务未启用"
systemctl disable chat-service.service 2>/dev/null || echo "Chat服务未启用"

# 删除服务文件
echo "删除服务文件..."
rm -f /etc/systemd/system/asr-service.service
rm -f /etc/systemd/system/chat-service.service

# 重新加载systemd配置
echo "重新加载systemd配置..."
systemctl daemon-reload
systemctl reset-failed

echo ""
echo "服务卸载完成！"
echo "服务将不再自动启动。"
