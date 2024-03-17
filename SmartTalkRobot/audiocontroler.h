#ifndef AUDIOCONTROLER_H
#define AUDIOCONTROLER_H

#include <functional>
#include "vocieai-vcs-api.h"


// Define WAVE File Header
struct WavFileHead
{
    char RIFFNAME[4];
    int nRIFFLength;
    char WAVNAME[4];
    char FMTNAME[4];
    int nFMTLength;
    short nAudioFormat;
    short nChannleNumber;
    int nSampleRate;
    int nBytesPerSecond;
    short nBytesPerSample;
    short   nBitsPerSample;
    char   DATANAME[4];
    int  nDataLength;
};

class AudioControlerPrivate;

class AudioControler
{
public:
    AudioControler();
    ~AudioControler();

    void setDebug(bool bDebug);

    bool initial(IN RECORDOUTFORMAT enRecordOutFormat, IN QUALITYCHECKCALLBACK pFunQualityCheckCallBack, IN void* pvUserdata);

    void readDataOut(char* data, int maxSize, int* pnOutSize);
    int getcachDataSize();
    bool started() const;
    
    void start();

    void stop();

    void pause();

    void restore();

    void clearData();

    //保存缓存数据，并存储到文件
    void saveChache(const char* paszFilePath);

    //写wav文件头，采样位宽默认为16bit
    WavFileHead writeWavHeader(int sample_rate, int channels, uint32_t dataLength);
    
    void startPlay(char *pcmData, int len);

private:
    AudioControlerPrivate* d;
};


#endif // AUDIOCONTROLER_H