# ASR和Chat程序自启动解决方案

## 🎉 问题已解决！

您的Ubuntu 22.04系统现在已配置完成，两个程序将在开机时自动启动：
- ASR离线录音程序 (`asr_offline_record_sample`)
- Chat对话程序 (`chat_demo`)

## 🚀 核心解决方案

1. **C++运行时库兼容性问题**: 通过预加载系统libstdc++库解决
2. **服务启动顺序**: ASR服务先启动，Chat服务延迟10秒启动
3. **自动重启机制**: 服务崩溃时自动重启
4. **环境配置**: 正确设置LD_LIBRARY_PATH和LD_PRELOAD

## 📋 快速命令参考

```bash
# 查看服务状态
/home/bxi/M2_SDK/scripts/service_manager.sh check

# 启动/停止/重启服务
sudo /home/bxi/M2_SDK/scripts/service_manager.sh {start|stop|restart}

# 查看详细日志
/home/bxi/M2_SDK/scripts/service_manager.sh logs

# 验证配置
/home/bxi/M2_SDK/scripts/final_verification.sh

# 调试问题
/home/bxi/M2_SDK/scripts/service_manager.sh debug
```

## 🔧 服务管理

- **启用自启动**: `sudo systemctl enable asr-service chat-service`
- **禁用自启动**: `sudo systemctl disable asr-service chat-service`
- **查看系统日志**: `sudo journalctl -u asr-service -u chat-service -f`

## 📁 重要文件位置

- **服务配置**: `/etc/systemd/system/asr-service.service`
- **包装脚本**: `/home/bxi/M2_SDK/scripts/asr_wrapper.sh`
- **管理脚本**: `/home/bxi/M2_SDK/scripts/service_manager.sh`
- **应用日志**: `/home/bxi/M2_SDK/log/`

## ⚡ 开机启动流程

1. 系统启动完成
2. 网络服务就绪
3. ASR服务自动启动
4. 10秒后Chat服务启动
5. 两个程序开始正常工作

## 🛡️ 故障自愈

- 程序崩溃时自动重启
- 服务依赖关系管理
- 环境变量自动配置
- 日志记录和监控

现在您可以**重启系统**来验证自启动功能！重启后两个程序会自动运行。
