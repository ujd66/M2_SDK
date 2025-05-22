    export LD_LIBRARY_PATH=/home/bxi/M2_SDK/实时转写SDK/libs/x64/:/home/bxi/M2_SDK/offline_mic/libs/x64/:/home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/libs/:$LD_LIBRARY_PATH && offline_mic/bin/asr_offline_record_sample | 星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat_demo
    export LD_LIBRARY_PATH=/home/bxi/M2_SDK/实时转写SDK/libs/x64:$LD_LIBRARY_PATH && /home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample
  
  
  编译：cd /home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat && ./build.sh
  cd /home/bxi/M2_SDK/offline_mic/samples/asr_offline_record_sample/ && ./64bit_make.sh
  
  运行
  /home/bxi/M2_SDK/星火SDK/SparkChain_Linux_SDK_2.0.0_rc1/llm/online_llm/chat/chat_demo
  /home/bxi/M2_SDK/offline_mic/bin/asr_offline_record_sample
  
    
