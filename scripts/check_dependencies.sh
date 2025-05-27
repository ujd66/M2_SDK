#!/bin/bash

echo "=== 检查ASR程序依赖和C++运行时库 ==="

echo "1. 检查ASR可执行文件依赖:"
if [ -f "/home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample" ]; then
    ldd /home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample
else
    echo "错误: ASR可执行文件不存在"
fi

echo -e "\n2. 检查libmsc.so依赖:"
if [ -f "/home/bxi/M2_SDK/offline_mic/libs/x64/libmsc.so" ]; then
    ldd /home/bxi/M2_SDK/offline_mic/libs/x64/libmsc.so
else
    echo "错误: libmsc.so不存在"
fi

echo -e "\n3. 检查系统C++库版本:"
ls -la /usr/lib/x86_64-linux-gnu/libstdc++.so* 2>/dev/null || echo "libstdc++库文件不存在"

echo -e "\n4. 检查系统libgcc版本:"
ls -la /usr/lib/x86_64-linux-gnu/libgcc_s.so* 2>/dev/null || echo "libgcc库文件不存在"

echo -e "\n5. 检查C++运行时符号 (检查系统库是否包含所需符号):"
if [ -f "/usr/lib/x86_64-linux-gnu/libstdc++.so.6" ]; then
    echo "查找 cxxabi 相关符号:"
    objdump -T /usr/lib/x86_64-linux-gnu/libstdc++.so.6 | grep -i "cxxabi.*class_type_info" | head -5
else
    echo "libstdc++.so.6 不存在"
fi

echo -e "\n6. 检查libmsc.so需要的符号:"
if [ -f "/home/bxi/M2_SDK/offline_mic/libs/x64/libmsc.so" ]; then
    echo "查找未定义的 cxxabi 符号:"
    objdump -T /home/bxi/M2_SDK/offline_mic/libs/x64/libmsc.so | grep -i "cxxabi.*class_type_info" | head -5
else
    echo "libmsc.so 不存在"
fi

echo -e "\n7. 检查当前LD_LIBRARY_PATH:"
echo "LD_LIBRARY_PATH = $LD_LIBRARY_PATH"

echo -e "\n8. 检查gcc版本:"
gcc --version | head -1

echo -e "\n9. 检查已安装的C++相关包:"
dpkg -l | grep -E "(libstdc|libgcc|build-essential)" | grep "^ii"

echo -e "\n=== 检查完成 ==="
