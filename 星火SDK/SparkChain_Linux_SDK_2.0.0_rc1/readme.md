# README

## 一、Sample使用

1. 使用VisualStudioCode导入Sample项目

2. 在要体验的功能对应的cpp文件中配置好APPID三元组。注意，在使用能力前需要先获得该能力的使用授权！

3. 如果使用的是RTASR或RAASR能力，还需要额外配置RTASRAPIKEY或RAASRAPIKEY，该值可从控制台查看。具体如下：

   | KEY         | 查看方法                                                    |
   | ----------- | ----------------------------------------------------------- |
   | RTASRAPIKEY | https://console.xfyun.cn/services/rta 中查看APIKey的值      |
   | RAASRAPIKEY | https://console.xfyun.cn/services/lfasr 中查看SecretKey的值 |

4. 填写完信息后，先把sample导入到Linux系统目录中(该目录要求有读写权限)，先执行./build编译出可执行脚本，然后再执行./run即可运行，最后根据界面提示输入对应的指令体验效果。

   **注意：SDK是在linux x86_64平台上构建的，构建的gcc和g++版本是7.5.0。因此运行时最低要求gcc和g++版本至少是7.5.0以上。**

## 二、Sample结构

.
├── ai
│   ├── asr(语音听写)
│   ├── itrans(在线翻译)
│   ├── raasr(语音转写)
│   ├── rtasr(实时语音转写)
│   └── tts(在线合成)
├── llm
│   ├── offline_llm(敬请期待)
│   └── online_llm
│       ├── bma(大模型识别)
│       ├── chat(文本交互)
│       ├── embedding(文本向量化)
│       ├── imageUnderstanding(图片理解)
│       ├── personateTTS(超拟人合成)
│       └── tti(图片生成)
├── rag(敬请期待)
└── agent(敬请期待)

## 三、常见问题

