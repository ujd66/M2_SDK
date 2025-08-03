# Makefile for compiling the ASR (Automatic Speech Recognition) sample.

# --- 1. 目录变量定义 (Directory Variable Definitions) ---
# 定义头文件、可执行文件和库文件的相对路径
DIR_INC = ../../include  # 头文件目录
DIR_BIN = ../../bin      # 编译后可执行文件的输出目录
DIR_LIB = ../../libs     # 库文件目录


# --- 2. 目标文件定义 (Target Definition) ---
# 定义最终生成的可执行文件名
TARGET	= asr_offline_record_sample
# 组合输出目录和文件名，构成完整的目标文件路径
BIN_TARGET = $(DIR_BIN)/$(TARGET)

# --- 3. 编译器和编译选项 (Compiler and Compilation Flags) ---
# 交叉编译工具链前缀 (默认为空，用于本地编译)
CROSS_COMPILE = 
# C编译器选项
# -g: 生成调试信息
# -Wall: 开启所有常用警告
# -I: 指定头文件搜索路径
CFLAGS = -g -Wall -I. -I/home/bxi/M2_SDK/实时转写SDK/include -I$(DIR_INC) 

# --- 4. 链接器选项 (Linker Flags) ---
# 根据不同的系统架构 (64位, 32位, ARM64) 设置不同的库文件搜索路径
# `make` 命令执行时，可以通过 `make LINUX64=1` 来选择对应的编译环境
ifdef LINUX64
LDFLAGS := -L/home/bxi/M2_SDK/实时转写SDK/libs/x64 -L$(DIR_LIB)/x64
endif
ifdef LINUX86
LDFLAGS := -L/home/bxi/M2_SDK/实时转写SDK/libs/x86 -L$(DIR_LIB)/x86
endif
ifdef LINUX_ARM64
LDFLAGS := -L/home/bxi/M2_SDK/实时转写SDK/libs/arm64 -L$(DIR_LIB)/arm64
endif

# --- 5. 链接库 (Linked Libraries) ---
# 添加需要链接的库
# -lmsc: 科大讯飞语音SDK核心库
# -lrt: 实时库
# -ldl: 动态链接加载库
# -lpthread: POSIX线程库
# -lasound: ALSA音频库
# -lstdc++: C++标准库
# -lcjson: cJSON库，用于处理JSON
# -Wl,-rpath,...: 设置运行时库搜索路径，确保程序能找到动态库
LDFLAGS += -lmsc -lrt -ldl -lpthread -lasound -lstdc++ -lcjson -Wl,-rpath,/home/bxi/M2_SDK/实时转写SDK/libs/x64

# --- 6. 源文件和目标文件 (Source and Object Files) ---
# OBJECTS 变量定义了所有需要编译的 .o 目标文件

# 这是一个被注释掉的旧方法，它会自动包含当前目录下所有的 .c 文件。
# OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))) # This line automatically includes all .c files.
# # Since we added speech_recognizer.c and linuxrec.c, and removed record.c,
# # this line should still work correctly if record.c is removed.
# # If record.c is not removed, it will be compiled.

# `wildcard *.c` 找到当前目录下所有.c文件
# `filter-out record.c, ...` 从列表中排除 record.c
# `patsubst %.c,%.o, ...` 将所有 .c 后缀替换为 .o
OBJECTS := $(patsubst %.c,%.o,$(filter-out record.c,$(wildcard *.c))) # 明确排除 record.c
# 手动添加额外的目标文件
OBJECTS += speech_recognizer.o linuxrec.o # Add speech_recognizer.o and linuxrec.o

# --- 7. 构建规则 (Build Rules) ---
# 定义如何生成最终的可执行文件
# `$(BIN_TARGET)`: 目标文件 (e.g., ../../bin/asr_offline_record_sample)
# `$(OBJECTS)`: 依赖的所有 .o 文件
# `$(CROSS_COMPILE)g++ ...`: 编译和链接命令
# `$^`: 代表所有的依赖文件 (即 $(OBJECTS))
# `$@`: 代表目标文件 (即 $(BIN_TARGET))
$(BIN_TARGET) : $(OBJECTS)
	$(CROSS_COMPILE)g++ $(CFLAGS) $^ -o $@ $(LDFLAGS) # Changed gcc to g++

# --- 8. 模式规则 (Pattern Rule) ---
# 定义如何从 .c 源文件生成 .o 目标文件
# `%.o: %.c`: 匹配所有 .o 文件，并找到对应的 .c 文件作为依赖
# `$<`: 代表第一个依赖文件 (即对应的 .c 文件)
%.o : %.c
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@

# --- 9. 清理规则 (Clean Rule) ---
# `clean` 是一个伪目标 (phony target)，因为它不对应一个真实的文件名
# `make clean` 命令会执行这里的指令
clean:
	@rm -f *.o $(BIN_TARGET) # 删除所有 .o 文件和最终生成的可执行文件

# .PHONY 将 `clean` 明确声明为伪目标，防止存在名为 'clean' 的文件时产生冲突
.PHONY:clean

#common makefile foot
