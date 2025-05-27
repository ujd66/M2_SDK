# 🚀 快速部署指南

## 一键安装脚本

创建并运行以下脚本以快速部署整个系统：

```bash
#!/bin/bash
# M2_SDK 一键部署脚本

echo "🚀 开始部署 M2_SDK 智能语音交互系统..."

# 1. 检查系统依赖
echo "📋 检查系统依赖..."
if ! command -v gcc &> /dev/null; then
    echo "❌ 缺少 gcc 编译器，正在安装..."
    sudo apt-get update && sudo apt-get install -y build-essential
fi

if ! ldconfig -p | grep -q libcjson; then
    echo "❌ 缺少 libcjson 库，正在安装..."
    sudo apt-get install -y libcjson1 libcjson-dev
fi

if ! ldconfig -p | grep -q libasound; then
    echo "❌ 缺少 ALSA 库，正在安装..."
    sudo apt-get install -y libasound2-dev
fi

# 2. 设置环境变量
echo "🔧 设置环境变量..."
export LD_LIBRARY_PATH=/home/bxi/M2_SDK/实时转写SDK/libs/x64:/home/bxi/M2_SDK/offline_mic/libs/x64:/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/libs/:$LD_LIBRARY_PATH

# 3. 初始化日志目录
echo "📁 初始化日志目录..."
/home/bxi/M2_SDK/scripts/setup_log_directory.sh

# 4. 编译项目
echo "🔨 编译 ASR 模块..."
cd /home/bxi/M2_SDK/offline_mic/samples/asr_offline_record_sample/
./64bit_make.sh

echo "🔨 编译星火SDK模块..."
cd /home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat
./build.sh

# 5. 创建启动脚本
echo "📝 创建启动脚本..."
cat > /home/bxi/M2_SDK/start_system.sh << 'EOF'
#!/bin/bash
# M2_SDK 系统启动脚本

# 设置库路径
export LD_LIBRARY_PATH=/home/bxi/M2_SDK/实时转写SDK/libs/x64:/home/bxi/M2_SDK/offline_mic/libs/x64:/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/libs/:$LD_LIBRARY_PATH

echo "🎙️ 启动智能语音交互系统..."
echo "💡 提示：说出唤醒词开始对话，说'再见'结束对话"

# 启动管道连接模式
/home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample | /home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat_demo
EOF

chmod +x /home/bxi/M2_SDK/start_system.sh

echo "✅ 部署完成！"
echo ""
echo "🎯 使用方法："
echo "  启动系统: /home/bxi/M2_SDK/start_system.sh"
echo "  查看日志: tail -f /home/bxi/M2_SDK/log/asr_log.txt"
echo "  监控输出: tail -f /home/bxi/M2_SDK/log/asr_output.txt"
echo ""
echo "🎤 现在可以开始使用语音交互系统了！"
```

## 系统监控脚本

```bash
#!/bin/bash
# 系统监控脚本

echo "📊 M2_SDK 系统状态监控"
echo "========================"

# 检查进程状态
echo "🔍 进程状态:"
if pgrep -f "asr_offline_record_sample" > /dev/null; then
    echo "  ✅ ASR程序正在运行"
else
    echo "  ❌ ASR程序未运行"
fi

if pgrep -f "chat_demo" > /dev/null; then
    echo "  ✅ 对话程序正在运行"
else
    echo "  ❌ 对话程序未运行"
fi

# 检查日志文件
echo ""
echo "📁 日志文件状态:"
LOG_DIR="/home/bxi/M2_SDK/log"

if [ -f "$LOG_DIR/asr_log.txt" ]; then
    size=$(du -h "$LOG_DIR/asr_log.txt" | cut -f1)
    echo "  📄 ASR日志: $size"
else
    echo "  ❌ ASR日志文件不存在"
fi

if [ -f "$LOG_DIR/asr_output.txt" ]; then
    echo "  📄 ASR输出文件存在"
else
    echo "  ❌ ASR输出文件不存在"
fi

if [ -f "$LOG_DIR/tts_speaking" ]; then
    echo "  🔊 TTS正在播放"
else
    echo "  🔇 TTS空闲"
fi

# 检查设备状态
echo ""
echo "🎤 设备状态:"
if [ -e "/dev/wheeltec_mic" ]; then
    echo "  ✅ 麦克风设备可用"
else
    echo "  ❌ 麦克风设备不可用"
fi

# 资源使用情况
echo ""
echo "💻 资源使用:"
echo "  CPU: $(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | cut -d'%' -f1)%"
echo "  内存: $(free -h | awk 'NR==2{printf "%.1f%%\n", $3*100/$2}')"

echo ""
echo "⏰ 最后更新: $(date)"
```

## 常用维护命令

```bash
# 重启系统
pkill -f "asr_offline_record_sample"
pkill -f "chat_demo"
/home/bxi/M2_SDK/start_system.sh

# 清理日志
echo "" > /home/bxi/M2_SDK/log/asr_log.txt

# 检查库依赖
ldd /home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample

# 测试麦克风
arecord -l

# 查看系统资源
htop
```
