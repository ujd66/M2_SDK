# ASR和Chat服务自启动配置

本文档描述如何在Ubuntu 系统上配置ASR离线录音程序和Chat对话程序的自启动。

## 文件说明

- `asr-service.service` - ASR服务的systemd配置文件
- `chat-service.service` - Chat服务的systemd配置文件  
- `install_autostart.sh` - 安装自启动服务的脚本
- `uninstall_autostart.sh` - 卸载自启动服务的脚本
- `service_manager.sh` - 服务管理脚本（推荐使用）

## 快速开始

### 1. 安装自启动服务

```bash
sudo /home/bxi/M2_SDK/scripts/install_autostart.sh
```

### 2. 立即启动服务（测试用）

```bash
sudo /home/bxi/M2_SDK/scripts/service_manager.sh start
```

### 3. 查看服务状态

```bash
/home/bxi/M2_SDK/scripts/service_manager.sh status
```

### 4. 快速健康检查

```bash
/home/bxi/M2_SDK/scripts/service_manager.sh check
```

### 5. 重启系统验证
重启后程序会自动启动！

## 详细使用说明

### 服务管理命令

```bash
# 使用服务管理脚本（推荐）
sudo /home/bxi/M2_SDK/scripts/service_manager.sh {start|stop|restart|status|logs|install|uninstall|debug|test|check}
```

### 手动systemctl命令

```bash
# 启动服务
sudo systemctl start asr-service
sudo systemctl start chat-service

# 停止服务
sudo systemctl stop asr-service
sudo systemctl stop chat-service

# 查看状态
sudo systemctl status asr-service
sudo systemctl status chat-service

# 查看日志
sudo journalctl -u asr-service -f
sudo journalctl -u chat-service -f
```

## 服务特性

### ASR服务 (asr-service)
- **启动顺序**: 在网络和音频服务之后启动
- **重启策略**: 自动重启，失败后等待3秒重试
- **日志输出**: 重定向到systemd journal
- **工作目录**: `/home/bxi/M2_SDK/offline_mic/bin`
- **依赖库**: 自动设置LD_LIBRARY_PATH

### Chat服务 (chat-service)
- **启动顺序**: 在ASR服务启动后10秒启动
- **重启策略**: 自动重启，失败后等待5秒重试
- **依赖关系**: 依赖ASR服务，ASR服务停止时Chat服务也会停止
- **工作目录**: `/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat`

## 故障排除

### 1. 服务启动失败

```bash
# 查看详细错误信息
sudo journalctl -u asr-service -n 50
sudo journalctl -u chat-service -n 50
```

### 2. 权限问题

确保可执行文件有执行权限：
```bash
chmod +x /home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample
chmod +x /home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat_demo
```

### 3. 库文件问题

检查动态库是否存在：
```bash
ldd /home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample
ldd /home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat_demo
```

### 4. 卸载服务

如果需要移除自启动配置：
```bash
sudo /home/bxi/M2_SDK/scripts/uninstall_autostart.sh
```

## 日志文件位置

- **systemd日志**: 使用 `journalctl` 查看
- **应用日志**: `/home/bxi/M2_SDK/log/` 目录下
- **ASR日志**: `/home/bxi/M2_SDK/log/asr_log.txt`

## 开机启动流程

1. 系统启动
2. 网络和音频服务就绪
3. ASR服务启动
4. 等待10秒
5. Chat服务启动
6. 两个程序开始正常工作

## 注意事项

- 服务以 `bxi` 用户身份运行，确保相关文件权限正确
- ASR服务必须先于Chat服务启动
- 如果某个服务崩溃，systemd会自动重启它
- 重启系统后服务会自动启动
