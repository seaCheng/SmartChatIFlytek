#ifndef	VCS_API_H
#define VCS_API_H

#include "msp_errors.h"
#include "qivw.h"
#ifdef VCS_API_EXPORTS 
#define VCS_API __declspec(dllexport)
#else
#define VCS_API
#endif

#ifndef IN
#define	IN
#endif

#ifndef OUT
#define	OUT
#endif

#ifndef INOUT
#define	INOUT
#endif

#ifdef __cplusplus
extern "C" {
#endif 
	
	// 数据格式
	typedef enum RECORDFILE_OUTFORMAT
	{
		RECORDFILE_OUTFORMAT_WAV = 0,		// WAV数据
		RECORDFILE_OUTFORMAT_PCM,			// PCM数据
	}RECORDOUTFORMAT;

#define FRAME_LEN	640 //16k采样率的16bit音频，一帧的大小为640B, 时长20ms
#define	BUFFER_SIZE	4096
#define HINTS_SIZE  100

	/**
	*  @brief 音频质量回调
	*
	*  @param[in]		fEffectiveSpeechDuration	有效时长（秒）
	*  @param[in]		fClipPercentage				截幅比例
	*  @param[in]		fSnrEst						信噪比
	*  @param[in]		fSpeechAvgEnergy			平均能量
	*  @param[in]       cut_off_or_not              是否截断
	*  @param[in]		pvUserdata					用户数据，回调时原样返回
	*
	*  @return 
	*
	*  @detail 
	*/
	typedef void(*QUALITYCHECKCALLBACK)(float fEffectiveSpeechDuration, float fClipPercentage, float fSnrEst, float fSpeechAvgEnergy, long cut_off_or_not,
		 void* pvUserdata);

	/**
	*  @brief 初始化声纹采集设备
	*
	*  @param[in]		enRecordOutFormat			指定输出格式WAV或PCM，值参考RECORDOUTFORMAT
	*  @param[in]		pvUserdata					上述回调函数用户传入参数
	*
	*  @return 0x00, 成功 -1, 失败。
	*
	*  @detail 调用其他接口前，必段调用此接口进行初始化
	*/

	VCS_API int	VCS_Init(IN RECORDOUTFORMAT enRecordOutFormat, IN QUALITYCHECKCALLBACK pFunQualityCheckCallBack, IN void* pvUserdata);


	/**
	*  @brief 开始采集声纹
	*
	*  @return 0x00, 成功 -1, 失败。
	*
	*  @detail
	*/
	VCS_API int	VCS_StartRecord();

	/**
	*  @brief 暂停采集声纹
	*
	*  @return 0x00, 成功 -1, 失败。
	*
	*  @detail
	*/
	VCS_API int	VCS_PauseRecord();

	/**
	*  @brief 暂停后恢复采集声纹
	*
	*  @return 0x00, 成功 -1, 失败。
	*
	*  @detail
	*/
	VCS_API int	VCS_RestoreRecord();

	/**
	*  @brief 停止采集声纹
	*
	*  @return 0x00, 成功 -1, 失败。
	*
	*  @detail
	*/
	VCS_API int	VCS_StopRecord();


	/**
	*  @brief 清空采集数据
	*
	*  @return 0x00, 成功 -1, 失败。
	*
	*  @detail
	*/
	VCS_API int	VCS_ClearRecordData();

	/**
	*  @brief 获取声纹录制结果数据返回大小
	*
	*  @param[out]		pnOutSize	声纹录制结果数据返回大小，单位BYTE
	*
	*  @return 0x00, 成功 -1, 失败。
	*
	*/
	VCS_API int	VCS_GetVoiceDataSize(OUT int* pnOutSize);

	/**
	*  @brief 获取声纹录制结果数据
	*
	*  @param[in]		paszBuffer	装载声纹录制结果数据内存，外部创建
	*  @param[in]		nSize		装载声纹录制结果数据内存大小
	*  @param[out]		pnOutSize	实际返回声纹录制数据大小
	*
	*  @return 0x00, 成功 -1, 失败。
	*
	*/
	VCS_API int	VCS_GetVoiceData(IN char* paszBuffer, IN const int nSize, OUT int* pnOutSize);

    
	/**
	*  @brief 保存声纹录制结果数据
	*
	*  @param[in]		paszFilePath	保存的录音文件路劲全称，外部创建
	*
	*  @return 0x00, 成功 -1, 失败。
	*
	*/
	VCS_API void VCS_SaveCache(IN const char* paszFilePath);

	VCS_API void VCS_Login();

	VCS_API void VCS_Uninitial();

	VCS_API void VCS_WakeUp(const char* session_id, char * audioVec, int len);

	VCS_API int VCS_cb_ivw_msg_proc(const char* sessionID, int msg, int param1, int param2, const void* info, void* userData);

	VCS_API void VCS_SparkInitial();
	VCS_API void VCS_SparkPost(const char* paszFilePath);

	VCS_API void VCS_playPcm(char * pcmData, int len);

	VCS_API void VCS_text_to_speech(char * pText, int len);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !VCS_API_H
