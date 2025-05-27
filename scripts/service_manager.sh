#!/bin/bash

# ASR和Chat服务管理脚本

show_help() {
    echo "用法: $0 {start|stop|restart|status|logs|install|uninstall|debug|test|check}"
    echo ""
    echo "命令说明:"
    echo "  start      - 启动所有服务"
    echo "  stop       - 停止所有服务"
    echo "  restart    - 重启所有服务"
    echo "  status     - 查看服务状态"
    echo "  logs       - 查看服务日志"
    echo "  install    - 安装自启动服务"
    echo "  uninstall  - 卸载自启动服务"
    echo "  debug      - 调试模式：检查依赖和环境"
    echo "  test       - 手动测试ASR程序"
    echo "  check      - 快速检查服务状态和健康度"
}

check_sudo() {
    if [ "$EUID" -ne 0 ]; then
        echo "此操作需要root权限，请使用sudo运行"
        exit 1
    fi
}

case "$1" in
    start)
        check_sudo
        echo "启动服务..."
        systemctl start asr-service.service
        systemctl start chat-service.service
        echo "服务启动完成"
        ;;
    stop)
        check_sudo
        echo "停止服务..."
        systemctl stop chat-service.service
        systemctl stop asr-service.service
        echo "服务停止完成"
        ;;
    restart)
        check_sudo
        echo "重启服务..."
        systemctl stop chat-service.service
        systemctl stop asr-service.service
        sleep 2
        systemctl start asr-service.service
        systemctl start chat-service.service
        echo "服务重启完成"
        ;;
    status)
        echo "=== ASR服务状态 ==="
        systemctl status asr-service.service --no-pager
        echo ""
        echo "=== Chat服务状态 ==="
        systemctl status chat-service.service --no-pager
        ;;
    logs)
        echo "查看服务日志 (按Ctrl+C退出)..."
        echo "ASR服务日志："
        sudo journalctl -u asr-service.service -n 20 --no-pager
        echo ""
        echo "Chat服务日志："
        sudo journalctl -u chat-service.service -n 20 --no-pager
        echo ""
        echo "实时日志 (按Ctrl+C退出)："
        sudo journalctl -u asr-service.service -u chat-service.service -f
        ;;
    install)
        check_sudo
        /home/bxi/M2_SDK/scripts/install_autostart.sh
        ;;
    uninstall)
        check_sudo
        /home/bxi/M2_SDK/scripts/uninstall_autostart.sh
        ;;
    debug)
        echo "=== 调试模式：检查依赖和环境 ==="
        /home/bxi/M2_SDK/scripts/check_dependencies.sh
        echo -e "\n=== 检查服务状态 ==="
        systemctl status asr-service.service --no-pager -l
        echo -e "\n=== 最近的错误日志 ==="
        journalctl -u asr-service.service --no-pager -l -n 20
        ;;
    test)
        echo "=== 手动测试ASR程序 ==="
        echo "停止服务以避免冲突..."
        sudo systemctl stop asr-service.service 2>/dev/null || true
        sudo systemctl stop chat-service.service 2>/dev/null || true
        
        echo "使用包装脚本测试..."
        /home/bxi/M2_SDK/scripts/asr_wrapper.sh &
        TEST_PID=$!
        
        echo "ASR程序已启动 (PID: $TEST_PID)"
        echo "按Enter键停止测试..."
        read
        
        echo "停止测试程序..."
        kill $TEST_PID 2>/dev/null || true
        echo "测试完成"
        ;;
    check)
        echo "=== 快速服务健康检查 ==="
        /home/bxi/M2_SDK/scripts/check_services.sh
        ;;
    *)
        show_help
        exit 1
        ;;
esac
