// testSdk.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include <chrono>
#include <string>

#include "vocieai-vcs-api.h"
#include "spark-api.h"


int gIstatus = 0; //0 未开启录音 1 正在录音 2

int main()
{   
    
    std::cout << "*********************操作说明*************************" << std::endl;
    std::cout << "****************输入字母b 开始录音********************" << std::endl;
    std::cout << "****************输入字母p 暂停录音********************" << std::endl;
    std::cout << "****************输入字母r 恢复录音********************" << std::endl;
    std::cout << "****************输入字母e 结束录音********************" << std::endl;
    std::cout << "****************输入字母q 退出程序********************" << std::endl;
    std::cout << "******************************************************" << std::endl;
    
    std::cout << "main id: " << std::this_thread::get_id() << std::endl;
    VCS_Login();
    VCS_SparkInitial();
    
    char a;
    std::cin >> a;

    bool bQuit = false;
    bool bStart = false;
    std::shared_ptr<char> channelData;
    channelData.reset(new char[1 * FRAME_LEN * 10]);

    std::thread m_readPcmDataThread;
    int sum = 0;

    int err_code = MSP_SUCCESS;
    int audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
    char sse_hints[128];
    const char* ssb_param = "ivw_threshold=0:1450,sst=wakeup,ivw_res_path =fo|res/ivw/wakeupresource.jet";
    const char* wakeUpsession_id = NULL;
    wakeUpsession_id = QIVWSessionBegin(nullptr, ssb_param, &err_code);
    
    if (err_code != MSP_SUCCESS)
    {
        printf("QIVWSessionBegin failed! error code:%d\n", err_code);
        return 0;
    }

    err_code = QIVWRegisterNotify(wakeUpsession_id, VCS_cb_ivw_msg_proc, &gIstatus);
    if (err_code != MSP_SUCCESS)
    {
        _snprintf(sse_hints, sizeof(sse_hints), "QIVWRegisterNotify errorCode=%d", err_code);
        printf("QIVWRegisterNotify failed! error code:%d\n", err_code);
        return 0;
    }

    
    m_readPcmDataThread = std::thread([&]()
        {   
            std::cout << "readPcmDataThread id: " << std::this_thread::get_id() << std::endl;
            while (!bQuit)
            {   
                if (!bStart)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                    continue;
                }
                int iRsum = 0;
                if (gIstatus == 2)
                {   
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    continue;
                }

                VCS_GetVoiceDataSize(&iRsum);
                if (iRsum < 1 * FRAME_LEN * 10)
                {
                    continue;
                }
                VCS_GetVoiceData(channelData.get(), 1 * FRAME_LEN * 10, &iRsum);
                //sum = sum + iRsum;
                if (iRsum >= 1 * FRAME_LEN * 10)
                {
                    VCS_WakeUp(wakeUpsession_id, channelData.get(), 1 * FRAME_LEN * 10);
                }
                //std::cout << "size is:"<< sum<<std::endl;
                
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                channelData.reset(new char[1 * FRAME_LEN * 10]);
            }

            QIVWSessionEnd(wakeUpsession_id, sse_hints);
        });

    m_readPcmDataThread.detach();
    
    
    
    while (!bQuit)
    {
        switch (a)
        {
        case 'b':
        {
            VCS_StartRecord();
            bStart = true;
            break;
        }

        case 'p':
        {
            VCS_PauseRecord();
            bStart = false;
            break;
        }
        case 'e':
        {
            VCS_StopRecord();
            
            //VCS_SaveCache("./test.wav");
            bStart = false;
            break;
        }
        case 'r':
        {
            VCS_RestoreRecord();
            bStart = true;
            break;
        }

        case 'q':
        {
            bStart = false;
            bQuit = true;
            break;
        }
        case 'a':
        {   
            int iRsum = 0;
            VCS_GetVoiceDataSize(&iRsum);
            
            std::shared_ptr<char> channelData;
            channelData.reset(new char[iRsum]);

            int iRead;
            VCS_GetVoiceData(channelData.get(), iRsum, &iRead);
            VCS_playPcm(channelData.get(), iRsum);

            VCS_ClearRecordData();
            break;
        }
                
        default:
            break;
        }

        std::cin >> a;
    }
    
    // 全局逆初始
    VCS_Uninitial();
}
