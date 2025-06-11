# M2_SDK Systemd服务脚本

本目录包含用于设置M2_SDK系统开机自启动的systemd服务脚本。

## 文件说明

1. `asr_service.service` - ASR语音识别服务的systemd配置文件
2. `chat_service.service` - 星火大模型对话服务的systemd配置文件
3. `m2_sdk_service.service` - 单一服务文件（使用启动脚本同时启动两个程序）
4. `start_system.sh` - 系统启动脚本，用于初始化环境并启动服务
5. `systemd_setup_guide.md` - 详细的安装和配置指南

## 使用方法

您可以选择以下两种方式之一来设置系统开机自启动：

### 方式一：使用单一服务文件

1. 给启动脚本添加执行权限：
   ```bash
   chmod +x /home/bxi/M2_SDK/systemd_scripts/start_system.sh
   ```

2. 将服务文件复制到systemd目录：
   ```bash
   sudo cp /home/bxi/M2_SDK/systemd_scripts/m2_sdk_service.service /etc/systemd/system/
   ```

3. 重新加载systemd配置并启用服务：
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable m2_sdk_service.service
   ```

### 方式二：使用分离的服务文件

1. 将两个服务文件复制到systemd目录：
   ```bash
   sudo cp /home/bxi/M2_SDK/systemd_scripts/asr_service.service /etc/systemd/system/
   sudo cp /home/bxi/M2_SDK/systemd_scripts/chat_service.service /etc/systemd/system/
   ```

2. 重新加载systemd配置并启用服务：
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable asr_service.service
   sudo systemctl enable chat_service.service
   ```

更详细的说明请参考 `systemd_setup_guide.md` 文件。