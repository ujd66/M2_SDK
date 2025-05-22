#common makefile header

DIR_INC = ../../include
DIR_BIN = ../../bin
DIR_LIB = ../../libs
VPATH = /home/bxi/M2_SDK/实时转写SDK/samples/iat_online_record_sample

TARGET	= asr_offline_record_sample
BIN_TARGET = $(DIR_BIN)/$(TARGET)

CROSS_COMPILE = 
CFLAGS = -g -Wall -I. -I/home/bxi/M2_SDK/实时转写SDK/include -I$(DIR_INC) -I/home/bxi/M2_SDK/实时转写SDK/samples/iat_online_record_sample

ifdef LINUX64
LDFLAGS := -L/home/bxi/M2_SDK/实时转写SDK/libs/x64 -L$(DIR_LIB)/x64
endif
ifdef LINUX86
LDFLAGS := -L/home/bxi/M2_SDK/实时转写SDK/libs/x86 -L$(DIR_LIB)/x86
endif
ifdef LINUX_ARM64
LDFLAGS := -L/home/bxi/M2_SDK/实时转写SDK/libs/arm64 -L$(DIR_LIB)/arm64
endif

LDFLAGS += -lmsc -lrt -ldl -lpthread -lasound -lstdc++ -lcjson -Wl,-rpath,/home/bxi/M2_SDK/实时转写SDK/libs/x64

# OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))) # This line automatically includes all .c files.
# Since we added speech_recognizer.c and linuxrec.c, and removed record.c,
# this line should still work correctly if record.c is removed.
# If record.c is not removed, it will be compiled.
OBJECTS := $(patsubst %.c,%.o,$(filter-out record.c,$(wildcard *.c))) # Explicitly exclude record.c
OBJECTS += speech_recognizer.o linuxrec.o # Add speech_recognizer.o and linuxrec.o

$(BIN_TARGET) : $(OBJECTS)
	$(CROSS_COMPILE)g++ $(CFLAGS) $^ -o $@ $(LDFLAGS) # Changed gcc to g++

%.o : %.c
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $< -o $@
clean:
	@rm -f *.o $(BIN_TARGET)

.PHONY:clean

#common makefile foot
