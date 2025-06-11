# 设置systemd服务开机自启动指南

## 准备工作

1. 确保您已经创建了以下文件：
   - `/home/bxi/M2_SDK/asr_service.service`
   - `/home/bxi/M2_SDK/chat_service.service`
   - `/home/bxi/M2_SDK/start_system.sh`

2. 给启动脚本添加执行权限：
   ```bash
   chmod +x /home/bxi/M2_SDK/start_system.sh
   ```

## 安装systemd服务

1. 将服务文件复制到systemd目录：
   ```bash
   sudo cp /home/bxi/M2_SDK/asr_service.service /etc/systemd/system/
   sudo cp /home/bxi/M2_SDK/chat_service.service /etc/systemd/system/
   ```

2. 重新加载systemd配置：
   ```bash
   sudo systemctl daemon-reload
   ```

3. 启用服务以实现开机自启动：
   ```bash
   sudo systemctl enable asr_service.service
   sudo systemctl enable chat_service.service
   ```

4. 立即启动服务（如果需要）：
   ```bash
   sudo systemctl start asr_service.service
   # chat_service会自动启动，因为它依赖于asr_service
   ```

## 管理服务

- 检查服务状态：
  ```bash
  sudo systemctl status asr_service.service
  sudo systemctl status chat_service.service
  ```

- 停止服务：
  ```bash
  sudo systemctl stop chat_service.service
  sudo systemctl stop asr_service.service
  ```

- 重启服务：
  ```bash
  sudo systemctl restart asr_service.service
  # chat_service会自动重启
  ```

- 查看服务日志：
  ```bash
  journalctl -u asr_service.service -f
  journalctl -u chat_service.service -f
  ```

## 注意事项

1. 如果程序需要特殊权限（如访问麦克风设备），可能需要在服务文件中添加适当的权限设置。

2. 确保服务文件中的用户名（User=bxi）与您的实际用户名匹配。

3. 如果您的程序依赖于特定的环境变量，可以在服务文件的[Service]部分添加：
   ```
   Environment=变量名=值
   ```

4. 如果需要在启动前运行初始化脚本，可以修改服务文件中的ExecStart：
   ```
   ExecStart=/home/bxi/M2_SDK/start_system.sh
   ```
   而不是直接启动程序。