// mylib.cpp : 定义 DLL 应用程序的导出函数。
//
#include "vocieai-vcs-api.h"
#include "Audiocontroler.h"

#include "msp_cmn.h"
#include "msp_errors.h"
#include "msp_cmn.h"
#include "qivw.h"
#include "qisr.h"

#include "qtts.h"

#include "spark-api.h"
#include "spark/sparkchain.h"
#include "spark/sc_llm.h"
#include "Encoding.h"

#include <iostream>
#include <string>

#include <thread>
#include <chrono>

#include <vector>


#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define RESET "\033[0m"

AudioControler g_AudioCtrl;
char szDevSerialNo[512];

std::vector<char> sysPcmData;

using namespace SparkChain;
using namespace std;

// result cache
std::string final_result = "";
VCS_API int	VCS_Init(IN RECORDOUTFORMAT enRecordOutFormat, IN QUALITYCHECKCALLBACK pFunQualityCheckCallBack, IN void* pvUserdata)
{   
	int nRet = 0;
	g_AudioCtrl.initial(enRecordOutFormat, pFunQualityCheckCallBack, pvUserdata);
	return nRet;
}

VCS_API int	VCS_StartRecord()
{  
	int nRet = 0;
	g_AudioCtrl.start();
	return nRet;
}

VCS_API int	VCS_PauseRecord()
{
	int nRet = 0;
	g_AudioCtrl.pause();
	return nRet;
}

VCS_API int	VCS_RestoreRecord()
{
	int nRet = 0;
	g_AudioCtrl.restore();
	return nRet;
}

VCS_API int	VCS_StopRecord()
{
	int nRet = 0;
	g_AudioCtrl.stop();
	return nRet;
}

VCS_API int	VCS_ClearRecordData()
{
	int nRet = 0;
	g_AudioCtrl.clearData();
	return nRet;
}

VCS_API int	VCS_GetVoiceDataSize(OUT int* pnOutSize)
{
	int nRet = 0;
	if (pnOutSize)
	{
		*pnOutSize = g_AudioCtrl.getcachDataSize();
	}
	return nRet;
}

VCS_API int	VCS_GetVoiceData(IN char* paszBuffer, IN const int nSize, OUT int* pnOutSize)
{
	int nRet = 0;
	g_AudioCtrl.readDataOut(paszBuffer, nSize, pnOutSize);
	return nRet;
}

VCS_API void VCS_SaveCache(IN const char* paszFilePath)
{
	g_AudioCtrl.saveChache(paszFilePath);
}

VCS_API void VCS_Uninitial()
{
	MSPLogout(); //退出登录
	SparkChain::unInit();
}
VCS_API void VCS_Login()
{
	int         ret = MSP_SUCCESS;
	const char* lgi_param = "appid = 50fe981b, work_dir = .";
	int aud_src = 0;
	/* 用户登录 */
	ret = MSPLogin(nullptr, nullptr, lgi_param); //第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://www.xfyun.cn注册获取
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed, error code: %d.\n", ret);
		
	}
}

VCS_API void VCS_WakeUp(const char* session_id, char* audioVec, int len)
{
	//const char* session_id = NULL;
	int err_code = MSP_SUCCESS;
	int audio_stat = MSP_AUDIO_SAMPLE_CONTINUE;
	char sse_hints[128];

	err_code = QIVWAudioWrite(session_id, (const void*)audioVec, len, audio_stat);
	if (MSP_SUCCESS != err_code)
	{
		printf("QIVWAudioWrite failed! error code:%d\n", err_code);
		_snprintf(sse_hints, sizeof(sse_hints), "QIVWAudioWrite errorCode=%d", err_code);
		return;
	}
}


int VCS_cb_ivw_msg_proc(const char* sessionID, int msg, int param1, int param2, const void* info, void* userData)
{   
	int* gIstatus = (int*)userData;
	if (*gIstatus == 2)
	{
		return 0;
	}

	std::cout << "VCS_cb_ivw_msg_proc id: " << std::this_thread::get_id() << std::endl;

	if (MSP_IVW_MSG_ERROR == msg) //唤醒出错消息
	{
		printf("\n\nMSP_IVW_MSG_ERROR errCode = %d\n\n", param1);
	}
	else if (MSP_IVW_MSG_WAKEUP == msg) //唤醒成功消息
	{
		printf("被唤醒了");
		

		//VCS_playPcmFile("./wav/answer.pcm");
		VCS_playPcm(sysPcmData.data(), sysPcmData.size());
		VCS_ClearRecordData();
		//std::this_thread::sleep_for(std::chrono::milliseconds(200));
		const char* asrSession_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = gb2312";
		int				errcode = MSP_SUCCESS;
		const char* asrSession_id = QISRSessionBegin(NULL, asrSession_begin_params, &errcode); //听写不需要语法，第一个参数为NULL
		if (MSP_SUCCESS != errcode)
		{
			printf("\nQISRSessionBegin failed! error code:%d\n", errcode);
			return 0;
		}
		unsigned int	total_len = 0;
		int				aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;		//音频状态
		int				ep_stat = MSP_EP_LOOKING_FOR_SPEECH;		//端点检测
		int				rec_stat = MSP_REC_STATUS_SUCCESS;			//识别状态
		char			rec_result[BUFFER_SIZE] = { NULL };
		char			hints[HINTS_SIZE] = { NULL };

		//int				errcode = MSP_SUCCESS;
		*gIstatus = 2;
		int pcm_count = 0;
		int iRsum = 0;
		int pcm_size = 0;
		std::shared_ptr<char> channelData;

		while (1)
		{
			channelData.reset(new char[1 * FRAME_LEN * 10]);
			
			int iSize = 0;
			VCS_GetVoiceDataSize(&iSize);
			if (iSize < 1 * FRAME_LEN * 10)
			{
				continue;
			}

			VCS_GetVoiceData(channelData.get(), 1 * FRAME_LEN * 10, &iRsum);
			unsigned int len = 10 * FRAME_LEN; // 每次写入200ms音频(16k，16bit)：1帧音频20ms，10帧=200ms。16k采样率的16位音频，一帧的大小为640Byte
			int ret = 0;

			aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
			if (0 == pcm_count)
				aud_stat = MSP_AUDIO_SAMPLE_FIRST;

			printf(">");
			ret = QISRAudioWrite(asrSession_id, (const void*)channelData.get(), 1 * FRAME_LEN * 10, aud_stat, &ep_stat, &rec_stat);
			if (MSP_SUCCESS != ret)
			{
				printf("\nQISRAudioWrite failed! error code:%d\n", ret);
				VCS_ClearRecordData();
				*gIstatus = 1;
				return 0;
			}

			pcm_count += (long)len;

			if (MSP_REC_STATUS_SUCCESS == rec_stat) //已经有部分听写结果
			{
				const char* rslt = QISRGetResult(asrSession_id, &rec_stat, 0, &errcode);
				if (MSP_SUCCESS != errcode)
				{
					printf("\nQISRGetResult failed! error code: %d\n", errcode);


					VCS_ClearRecordData();
					*gIstatus = 1;
					return 0;
				}
				if (NULL != rslt)
				{
					unsigned int rslt_len = strlen(rslt);
					total_len += rslt_len;
					if (total_len >= BUFFER_SIZE)
					{
						printf("\nno enough buffer for rec_result !\n");


						VCS_ClearRecordData();
						*gIstatus = 1;
						return 0;
					}
					strncat(rec_result, rslt, rslt_len);
				}
			}

			if (MSP_EP_AFTER_SPEECH == ep_stat)
				break;
			//std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		
		VCS_ClearRecordData();
		errcode = QISRAudioWrite(asrSession_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
		if (MSP_SUCCESS != errcode)
		{
			printf("\nQISRAudioWrite failed! error code:%d \n", errcode);
			
			*gIstatus = 1;
			return 0;
		}

		while (MSP_REC_STATUS_COMPLETE != rec_stat)
		{
			const char* rslt = QISRGetResult(asrSession_id, &rec_stat, 0, &errcode);
			if (MSP_SUCCESS != errcode)
			{
				printf("\nQISRGetResult failed, error code: %d\n", errcode);
				
				*gIstatus = 1;
				return 0;
			}
			if (NULL != rslt)
			{
				unsigned int rslt_len = strlen(rslt);
				total_len += rslt_len;
				if (total_len >= BUFFER_SIZE)
				{
					printf("\nno enough buffer for rec_result !\n");
					*gIstatus = 1;
					return 0;
				}
				strncat(rec_result, rslt, rslt_len);
			}
			//std::this_thread::sleep_for(std::chrono::milliseconds(150));
		}
		printf("\n语音听写结束\n");
		printf("=============================================================\n");
		printf("语句为：%s\n", rec_result);
		printf("=============================================================\n");

		QISRSessionEnd(asrSession_id, hints);

		VCS_SparkPost(rec_result);
		VCS_ClearRecordData();
		
		*gIstatus = 1;
	}
	return 0;
}

VCS_API void VCS_initialSysPcm()
{
	
	
	FILE* pFile;
	long lSize;
	char* buffer;
	size_t result;

	pFile = fopen("./wav/answer.pcm", "rb");
	if (pFile == NULL) { fputs("File error", stderr); return; }

	// obtain file size:
	fseek(pFile, 0, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);

	sysPcmData.resize(lSize);

	
	// copy the file into the buffer:
	result = fread(sysPcmData.data(), 1, lSize, pFile);
	if (result != lSize) { fputs("Reading error", stderr); return; }

	/* the whole file is now loaded in the memory buffer. */

	// terminate
	fclose(pFile);

}
VCS_API void VCS_SparkInitial()
{
	// 全局初始化
	SparkChainConfig* config = SparkChainConfig::builder();
	config->appID("50fe981b")        // 你的appid
		->apiKey("9bf75f09c13d617c350f861f7e298a8f")        // 你的apikey
		->apiSecret("MTQ4Mjg0NGExNWEwNzg2YTY2MzJiYWY3"); // 你的apisecret
	int ret = SparkChain::init(config);

	VCS_initialSysPcm();
}


void VCS_SparkPost(const char* paszFilePath)
{
	wcout << L"\n######### 同步调用 #########:" << paszFilePath << endl;
	// 配置大模型参数
	LLMConfig* llmConfig = LLMConfig::builder();
	llmConfig->domain("generalv3.5");
	llmConfig->url("ws(s)://spark-api.xf-yun.com/v3.5/chat");

	Memory* window_memory = Memory::WindowMemory(5);
	LLM* syncllm = LLM::create(llmConfig, window_memory);

	// Memory* token_memory = Memory::TokenMemory(500);
	// LLM *syncllm = LLM::create(llmConfig,token_memory);

	int i = 0;
	const char* input = paszFilePath;
	while (i++ < 1)
	{
		// 同步请求
		std::string str = paszFilePath;
		LLMSyncOutput* result = syncllm->run(Encoding::GBK2Utf8(str).c_str());
		if (result->getErrCode() != 0)
		{
			printf(RED "\nsyncOutput: %d:%s\n\n" RESET, result->getErrCode(), result->getErrMsg());
			//continue;
		}
		else
		{
			std::string ws;
			std::string s = result->getContent();
			Encoding::Utf82GBK(s, ws);
			cout << "提问:" << str << "\n回复:" << ws << "    回复结束" << endl;

			VCS_text_to_speech((char *)ws.c_str(), ws.length());
			//const char* CStr = result->getContent();
			//printf(GREEN "\n回复: %s:%s\n" RESET, result->getRole(), result->getContent());
		}
	}
	// 垃圾回收
	if (syncllm != nullptr)
	{
		LLM::destroy(syncllm);
	}
}


void VCS_playPcmFile(char* sFile)
{
	g_AudioCtrl.startPlayPcm(sFile);
}
void VCS_playPcm(char* pcmData, int len)
{
	g_AudioCtrl.startPlay(pcmData, len);
}

VCS_API void VCS_text_to_speech(char* pText, int len)
{   
	int          ret = -1;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
	const char* session_begin_params = "voice_name = xiaoyan, text_encoding = gb2312, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2";
    
	const char* sessionID = NULL;
	unsigned int audio_len = 0;

	/* 开始合成 */
	sessionID = QTTSSessionBegin(session_begin_params, &ret);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionBegin failed, error code: %d.\n", ret);
		return;
	}
	ret = QTTSTextPut(sessionID, pText, (unsigned int)len, NULL);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSTextPut failed, error code: %d.\n", ret);
		QTTSSessionEnd(sessionID, "TextPutError");
		
		return;
	}
	printf("正在合成 ...\n");
	std::vector<char> textAudioData;

	while (1)
	{
		/* 获取合成音频 */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret)
			break;
		if (NULL != data)
		{   
			std::vector<char> cache;
			cache.resize(audio_len); //short类型
			memcpy(cache.data(), data, audio_len);
			textAudioData.insert(textAudioData.end(), cache.begin(), cache.end());

			//fwrite(data, audio_len, 1, fp);
			//wav_hdr.data_size += audio_len; //计算data_size大小
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;
		printf(">");
		
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
	printf("\n");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSAudioGet failed, error code: %d.\n", ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
		
		return;
	}

	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionEnd failed, error code: %d.\n", ret);
	}

	VCS_playPcm(textAudioData.data(), textAudioData.size());
}