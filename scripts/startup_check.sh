#!/bin/bash

# 系统启动后自动检查脚本
# 等待系统完全启动后检查服务状态

echo "=== 系统启动后服务自检 ==="
echo "启动时间: $(date)"

# 等待系统完全启动
echo "等待系统完全启动... (30秒)"
sleep 30

echo ""
echo "开始检查服务状态..."

# 检查ASR服务
ASR_STATUS=$(systemctl is-active asr-service.service)
if [ "$ASR_STATUS" = "active" ]; then
    echo "✅ ASR服务启动成功"
else
    echo "❌ ASR服务启动失败，状态: $ASR_STATUS"
    echo "尝试重启ASR服务..."
    systemctl restart asr-service.service
    sleep 5
    ASR_STATUS=$(systemctl is-active asr-service.service)
    if [ "$ASR_STATUS" = "active" ]; then
        echo "✅ ASR服务重启成功"
    else
        echo "❌ ASR服务重启失败"
    fi
fi

# 检查Chat服务
CHAT_STATUS=$(systemctl is-active chat-service.service)
if [ "$CHAT_STATUS" = "active" ]; then
    echo "✅ Chat服务启动成功"
else
    echo "❌ Chat服务启动失败，状态: $CHAT_STATUS"
    echo "尝试重启Chat服务..."
    systemctl restart chat-service.service
    sleep 5
    CHAT_STATUS=$(systemctl is-active chat-service.service)
    if [ "$CHAT_STATUS" = "active" ]; then
        echo "✅ Chat服务重启成功"
    else
        echo "❌ Chat服务重启失败"
    fi
fi

# 记录到日志文件
LOG_FILE="/home/bxi/M2_SDK/log/startup_check.log"
mkdir -p /home/bxi/M2_SDK/log
echo "$(date): ASR=$ASR_STATUS, Chat=$CHAT_STATUS" >> "$LOG_FILE"

echo ""
echo "自检完成，详细状态："
/home/bxi/M2_SDK/scripts/check_services.sh

echo ""
echo "系统已准备就绪！"
