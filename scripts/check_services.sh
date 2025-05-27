#!/bin/bash

echo "=== ASR和Chat服务状态检查 ==="
echo "时间: $(date)"
echo ""

echo "1. ASR服务状态:"
systemctl is-active asr-service.service
if systemctl is-active asr-service.service | grep -q "active"; then
    echo "  ✅ ASR服务运行正常"
else
    echo "  ❌ ASR服务未运行"
fi

echo ""
echo "2. Chat服务状态:"
systemctl is-active chat-service.service
if systemctl is-active chat-service.service | grep -q "active"; then
    echo "  ✅ Chat服务运行正常"
else
    echo "  ❌ Chat服务未运行"
fi

echo ""
echo "3. 进程信息:"
ps aux | grep -E "(asr_offline_record_sample|chat_demo)" | grep -v grep

echo ""
echo "4. 最近的ASR日志 (最新5条):"
journalctl -u asr-service.service --no-pager -n 5 -o cat

echo ""
echo "5. 最近的Chat日志 (最新5条):"
journalctl -u chat-service.service --no-pager -n 5 -o cat

echo ""
echo "6. 系统资源使用:"
echo "  内存使用:"
free -h | head -2
echo "  CPU负载:"
uptime
