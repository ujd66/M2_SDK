#!/bin/bash

echo "=== 自启动解决方案最终验证 ==="
echo "验证时间: $(date)"
echo ""

# 检查服务状态
echo "1. 检查服务当前状态..."
ASR_STATUS=$(systemctl is-active asr-service.service 2>/dev/null || echo "not-found")
CHAT_STATUS=$(systemctl is-active chat-service.service 2>/dev/null || echo "not-found")

if [ "$ASR_STATUS" = "active" ] && [ "$CHAT_STATUS" = "active" ]; then
    echo "   ✅ 两个服务都在正常运行"
else
    echo "   ❌ 服务状态异常: ASR=$ASR_STATUS, Chat=$CHAT_STATUS"
fi

# 检查自启动配置
echo ""
echo "2. 检查自启动配置..."
ASR_ENABLED=$(systemctl is-enabled asr-service.service 2>/dev/null || echo "not-found")
CHAT_ENABLED=$(systemctl is-enabled chat-service.service 2>/dev/null || echo "not-found")

if [ "$ASR_ENABLED" = "enabled" ] && [ "$CHAT_ENABLED" = "enabled" ]; then
    echo "   ✅ 自启动配置正确"
else
    echo "   ❌ 自启动配置异常: ASR=$ASR_ENABLED, Chat=$CHAT_ENABLED"
fi

# 检查文件权限
echo ""
echo "3. 检查关键文件权限..."
FILES_TO_CHECK=(
    "/home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample"
    "/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat_demo"
    "/home/bxi/M2_SDK/scripts/asr_wrapper.sh"
    "/home/bxi/M2_SDK/scripts/service_manager.sh"
)

ALL_FILES_OK=true
for file in "${FILES_TO_CHECK[@]}"; do
    if [ -f "$file" ] && [ -x "$file" ]; then
        echo "   ✅ $file"
    else
        echo "   ❌ $file (不存在或无执行权限)"
        ALL_FILES_OK=false
    fi
done

# 检查库文件
echo ""
echo "4. 检查库文件..."
LIBS_TO_CHECK=(
    "/home/bxi/M2_SDK/offline_mic/libs/x64/libmsc.so"
    "/usr/lib/x86_64-linux-gnu/libstdc++.so.6"
)

for lib in "${LIBS_TO_CHECK[@]}"; do
    if [ -f "$lib" ]; then
        echo "   ✅ $lib"
    else
        echo "   ❌ $lib (不存在)"
        ALL_FILES_OK=false
    fi
done

# 检查日志目录
echo ""
echo "5. 检查日志目录..."
if [ -d "/home/bxi/M2_SDK/log" ] && [ -w "/home/bxi/M2_SDK/log" ]; then
    echo "   ✅ 日志目录存在且可写"
else
    echo "   ❌ 日志目录不存在或无权限"
    ALL_FILES_OK=false
fi

# 最终结果
echo ""
echo "=== 验证结果 ==="
if [ "$ASR_STATUS" = "active" ] && [ "$CHAT_STATUS" = "active" ] && \
   [ "$ASR_ENABLED" = "enabled" ] && [ "$CHAT_ENABLED" = "enabled" ] && \
   [ "$ALL_FILES_OK" = "true" ]; then
    echo "🎉 验证通过！自启动解决方案配置完成！"
    echo ""
    echo "✅ 所有服务正常运行"
    echo "✅ 自启动配置正确"
    echo "✅ 文件权限正确"
    echo "✅ 依赖库存在"
    echo ""
    echo "现在您可以重启系统验证自启动功能。"
    echo ""
    echo "常用管理命令:"
    echo "  - 查看状态: /home/bxi/M2_SDK/scripts/service_manager.sh check"
    echo "  - 重启服务: sudo /home/bxi/M2_SDK/scripts/service_manager.sh restart"
    echo "  - 查看日志: /home/bxi/M2_SDK/scripts/service_manager.sh logs"
else
    echo "❌ 验证失败！请检查上述问题后重新配置。"
    echo ""
    echo "建议的修复步骤:"
    if [ "$ASR_STATUS" != "active" ] || [ "$CHAT_STATUS" != "active" ]; then
        echo "  1. 重启服务: sudo /home/bxi/M2_SDK/scripts/service_manager.sh restart"
    fi
    if [ "$ASR_ENABLED" != "enabled" ] || [ "$CHAT_ENABLED" != "enabled" ]; then
        echo "  2. 重新安装自启动: sudo /home/bxi/M2_SDK/scripts/install_autostart.sh"
    fi
    if [ "$ALL_FILES_OK" != "true" ]; then
        echo "  3. 检查文件权限: chmod +x /home/bxi/M2_SDK/scripts/*.sh"
    fi
fi

echo ""
