#!/bin/bash

echo "=== 安装/更新C++运行时库和兼容性库 ==="

# 检查是否以root权限运行
if [ "$EUID" -ne 0 ]; then
    echo "请以root权限运行此脚本 (使用sudo)"
    exit 1
fi

echo "1. 更新包列表..."
apt update

echo "2. 安装基础C++运行时库..."
apt install -y \
    libstdc++6 \
    libc6 \
    libgcc-s1 \
    build-essential \
    g++ \
    gcc

echo "3. 安装多架构兼容性库..."
# 启用i386架构 (32位兼容)
dpkg --add-architecture i386
apt update

# 安装32位兼容库
apt install -y \
    lib32stdc++6 \
    lib32gcc-s1 \
    libc6:i386 \
    libstdc++6:i386

echo "4. 安装额外的开发库..."
apt install -y \
    libc6-dev \
    libstdc++-11-dev \
    g++-11 \
    gcc-11

echo "5. 创建库链接..."
# 确保库链接正确
ldconfig

echo "6. 检查安装结果..."
echo "已安装的C++相关包:"
dpkg -l | grep -E "(libstdc|libgcc|libc6)" | grep "^ii"

echo -e "\n7. 检查库文件:"
ls -la /usr/lib/x86_64-linux-gnu/libstdc++.so*
ls -la /usr/lib/x86_64-linux-gnu/libgcc_s.so*

echo -e "\n=== C++运行时库安装完成 ==="
echo "请运行检查脚本验证: /home/bxi/M2_SDK/scripts/check_dependencies.sh"
