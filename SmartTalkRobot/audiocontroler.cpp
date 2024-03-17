#include "Audiocontroler.h"
#include "rtaudio/Rtaudio.h"
#include<string.h>

#include <thread>

#include <iostream>
#include <fstream> // 包含头文件 fstream

#ifdef _WIN32
#define gDeviceName "Y05B"
#else
  #define gDeviceName "Y05B"
#endif


typedef int16_t MY_TYPE;
#define FORMAT RTAUDIO_SINT16
#define SCALE  32767.0

static int input(void */*outputBuffer*/, void *inputBuffer, unsigned int nBufferSize, double streamTime, RtAudioStreamStatus status, void *userdata);
static int output(void*/*outputBuffer*/, void* inputBuffer, unsigned int nBufferSize, double streamTime, RtAudioStreamStatus status, void* userdata);

using namespace std;
class AudioControlerPrivate
{
public:
    AudioControlerPrivate(){
        audioBuffer = new char[16000 * 2 * 60];
    }

    void clearData()
    {
        audioCache.clear();
        outReadCount = 0;
    }

    int getcachDataSize()
    {
        return audioCache.size() - outReadCount;
    }

    void setPlayData(char* data, int len)
    {   
        outPlayTotal = len;
        outAudioBuffer = new char[len];
        memset(outAudioBuffer, '\0', len);
        memcpy(outAudioBuffer, data, len);
    }
    void readDataOut(char* data, int maxSize, int* pnOutSize)
    {   
        if (!pnOutSize || !data)
        {   
            std::cout << "readDataOut 0" << std::endl;
            return;
        }
        
        if (!data || maxSize < 1)
        {   
            std::cout << "readDataOut 1" << std::endl;
            *pnOutSize = 0;
            return;
        }
        const int data_size = audioCache.size() - outReadCount;

        if (data_size <= 0) 
        {   
            //std::cout << "readDataOut 2 " << audioCache.size()<< std::endl;
            *pnOutSize = 0;
            return;
        }

        const int read_size = (data_size >= maxSize) ? maxSize : data_size;
        memcpy(data, audioCache.data() + outReadCount, read_size);
        outReadCount += read_size;

        *pnOutSize = read_size;
        return;
    }
public:
    std::function<void(const char *data, unsigned int bytes)> callback;

    QUALITYCHECKCALLBACK			pFunQualityCheckCallBack;
    void* pvUserdata;

    RtAudio* outAudio = nullptr;
    unsigned int outBufferFrames;
    RtAudio::StreamParameters outParams;
    RtAudio::StreamOptions outOptions;

    bool started = false;
    RtAudio * audio = nullptr;
    unsigned int bufferFrames;
    RtAudio::StreamParameters iParams;
    RtAudio::StreamOptions options;
    std::vector<char> audioCache;
    char* audioBuffer;

    char* outAudioBuffer = nullptr;
    
    int bufferCounter = 0;

    int outReadCount{ 0 };

    int outPlayTotal = {0};
    int outPlayReadCount{ 0 };

    RECORDOUTFORMAT eForm = RECORDFILE_OUTFORMAT_WAV;
};

AudioControler::AudioControler()
    : d(new AudioControlerPrivate())
{
    //d->callback = callback;
    #ifdef _WIN32
     d->audio = new RtAudio(RtAudio::Api::WINDOWS_WASAPI);
     d->outAudio = new RtAudio(RtAudio::Api::WINDOWS_WASAPI);
    #else 
     d->audio = new RtAudio(RtAudio::Api::LINUX_PULSE);
     d->outAudio = new RtAudio(RtAudio::Api::LINUX_PULSE);
    #endif
}

void AudioControler::readDataOut(char* data, int maxSize, int* pnOutSize)
{
    d->readDataOut(data, maxSize, pnOutSize);
}

int AudioControler::getcachDataSize()
{
    return d->getcachDataSize();
}

bool AudioControler::initial(IN RECORDOUTFORMAT enRecordOutFormat, IN QUALITYCHECKCALLBACK funQualityCheckCallBack, IN void* vUserdata)
{
    d->pFunQualityCheckCallBack = funQualityCheckCallBack;
    //d->pFunRecordProgressCallBack = funRecordProgressCallBack;
    d->pvUserdata = vUserdata;
    d->eForm = enRecordOutFormat;
    return true;
}

AudioControler::~AudioControler()
{
    if (nullptr != d)
    {
        delete d;
        d = nullptr;
    }
}

WavFileHead AudioControler:: writeWavHeader(int sample_rate,
                                                int channels,
                                                uint32_t dataLength) {
    //wav文件有44字节的wav头，所以要写44字节的wav头
    int bits=16;

    int s = sizeof(WavFileHead);
    WavFileHead wh;
    memset(&wh, 0, s);

    memcpy(wh.RIFFNAME, "RIFF", 4);
    memcpy(wh.WAVNAME, "WAVE", 4);
    memcpy(wh.FMTNAME, "fmt ", 4);
    memcpy(wh.DATANAME, "data", 4);

    wh.nRIFFLength = dataLength + 36;
    wh.nFMTLength = 16;
    wh.nAudioFormat = 0x01;
    wh.nChannleNumber = channels;
    wh.nSampleRate = sample_rate;
    wh.nBytesPerSecond = (bits / 8) * channels * sample_rate;
    wh.nBytesPerSample = (bits / 8) * channels;
    wh.nBitsPerSample = bits;
    wh.nDataLength = dataLength;

    return wh;
}


bool AudioControler::started() const
{
    return d->started;
}

void AudioControler::startPlay(char* pcmData, int len)
{
    std::cout << "start play...." << std::endl;
    d->setPlayData(pcmData, len);
    
    if (nullptr == d->outAudio)
    {
        std::cout << "start play return null...." << std::endl;
        return;
    }
    
    d->outAudio->showWarnings(true);
    d->outBufferFrames = 512;
    d->outParams.nChannels = 1;
    d->outParams.firstChannel = 0;
    d->outParams.deviceId = -1;
    d->outParams.deviceId = d->outAudio->getDefaultOutputDevice();

    //std::cout << "getDefaultInputDevice " << d->audio->getDefaultInputDevice()<< std::endl << std::endl;

    for (auto i = 0; i < d->outAudio->getDeviceCount(); i++)
    {
        auto dev = d->outAudio->getDeviceInfo(d->audio->getDeviceIds().at(i));
        if (dev.name.find(gDeviceName, 0) != string::npos)
        {
            d->iParams.deviceId = dev.ID;
            std::cout << "" << d->outAudio->getApiName(d->outAudio->getCurrentApi()) << std::endl;

            std::cout << "device name: " << dev.name <<
                " inputChannels " << dev.outputChannels << " isDefaultInput " << dev.isDefaultOutput
                << " isDefaultOutput " << dev.isDefaultOutput << " duplexChannels " << dev.duplexChannels
                << " outputChannels " << dev.outputChannels << " cur sampleRate " << dev.currentSampleRate
                << " ID " << dev.ID << " native format " << dev.nativeFormats
                << " preferance fs " << dev.preferredSampleRate << std::endl;
            auto sampleBits = dev.nativeFormats;
            std::cout << "sample bit" << sampleBits << std::endl;
            std::cout << std::endl << std::endl;
            auto sampleRates = dev.sampleRates;

            std::cout << "support sampleRates: ";
            for (auto fs : sampleRates)
            {
                std::cout << fs << " ";
            }
            std::cout << std::endl << std::endl;

            //break;
        }
    }

    if (d->outParams.deviceId == -1)
    {
        std::cout << "don't find the device!";
        return;
    }

    
    if (!d->outAudio->isStreamOpen())
        d->outAudio->openStream(&d->outParams,nullptr, FORMAT, 16000, &d->outBufferFrames, output, d, nullptr);

    if (!d->outAudio->isStreamRunning())
        d->outAudio->startStream();

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (d->outAudio->isStreamRunning() == false)
        {   
            cout << "play finished.....";
            break;
        }
    }
}

void AudioControler::start()
{   
    std::cout << "start ...." << std::endl;
    
    if (d->started)
    {   
        std::cout << "already started return...." << std::endl;
        return;
    }
    if (nullptr == d->audio)
    {   
        std::cout << "start return null...." << std::endl;
        return;
    }
    d->clearData();
    d->audio->showWarnings(true);
    d->bufferFrames = 4800;
    d->iParams.nChannels = 1;
    d->iParams.firstChannel = 0;
	d->iParams.deviceId = -1;
    //d->iParams.deviceId = d->audio->getDefaultInputDevice();
    
	//std::cout << "getDefaultInputDevice " << d->audio->getDefaultInputDevice()<< std::endl << std::endl;


    for (auto i = 0; i < d->audio->getDeviceCount(); i++)
    {
        auto dev = d->audio->getDeviceInfo(d->audio->getDeviceIds().at(i));
        if (dev.name.find(gDeviceName, 0) != string::npos)
        {
            d->iParams.deviceId = dev.ID;
            std::cout << "" << d->audio->getApiName( d->audio->getCurrentApi()) << std::endl;
          
            std::cout << "device name: " << dev.name <<
                " inputChannels " << dev.inputChannels << " isDefaultInput " << dev.isDefaultInput
                << " isDefaultOutput " << dev.isDefaultOutput << " duplexChannels " << dev.duplexChannels
                << " outputChannels " << dev.outputChannels << " cur sampleRate " << dev.currentSampleRate
                << " ID " << dev.ID << " native format " << dev.nativeFormats
                << " preferance fs " << dev.preferredSampleRate << std::endl;
		   auto sampleBits = dev.nativeFormats;
			std::cout<<"sample bit" << sampleBits << std::endl;
			std::cout<< std::endl<< std::endl;
            auto sampleRates = dev.sampleRates;
			
            std::cout << "support sampleRates: ";
            for (auto fs : sampleRates)
            {
                std::cout << fs << " ";
            }
            std::cout << std::endl << std::endl;

            //break;
        }

    }


    if (d->iParams.deviceId == -1)
    {   
        std::cout <<"don't find the device!";
        return;
    }
    if (!d->audio->isStreamOpen())
        d->audio->openStream(NULL, &d->iParams, RTAUDIO_SINT16, 16000, &d->bufferFrames, &input, d, nullptr);
        
    if (!d->audio->isStreamRunning())
        d->audio->startStream();

        d->started = true;
}

void AudioControler::stop()
{
    if (d->audio->isStreamRunning())
        d->audio->stopStream();
    
    if (d->audio->isStreamOpen())
        d->audio->closeStream();
    
    d->started = false;
}

void AudioControler::pause()
{
    if (d->audio->isStreamRunning())
        d->audio->stopStream();

    d->started = false;
}

void AudioControler::clearData()
{
    d->clearData();
}

void AudioControler::restore()
{
    if (!d->audio->isStreamRunning())
        d->audio->startStream();

    d->started = true;

}

void AudioControler::saveChache(const char* paszFilePath){
    std::ofstream fs(paszFilePath, std::ios::out | std::ios::binary);
    if (fs)
    {
        WavFileHead head = writeWavHeader(16000, 1, d->audioCache.size());
        fs.write((char*)&head, sizeof(WavFileHead));
        fs.write((char*)(d->audioCache.data()), d->audioCache.size());
        fs.flush();
        fs.close();
    }
    else {
        std::cerr << "Failed to open or create a new file!" << std::endl;
    }
}


int input(void * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userdata)
{
	
    AudioControlerPrivate* d = (AudioControlerPrivate*)userdata;    
    std::vector<char> cache;
    
    cache.resize(nBufferFrames * 2); //short类型
    memcpy(cache.data(), inputBuffer, nBufferFrames * 2);
    d->audioCache.insert(d->audioCache.end(), cache.begin(), cache.end());
    
    return 0;
}


int output(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void* userdata)
{

    AudioControlerPrivate* d = (AudioControlerPrivate*)userdata;

    if (d->outPlayTotal < nBufferFrames * 1 * sizeof(MY_TYPE) + d->outPlayReadCount)
    {   
        memcpy(outputBuffer, d->outAudioBuffer + d->outPlayReadCount, d->outPlayTotal - d->outPlayReadCount);       
        memset((char*)(outputBuffer), 0, nBufferFrames * 1 * sizeof(MY_TYPE) - d->outPlayTotal + d->outPlayReadCount);

        delete d->outAudioBuffer;
        d->outAudioBuffer = nullptr;
        d->outPlayReadCount = 0;
        d->outPlayTotal = 0;
        return 1;
    }
    else
    {
        memcpy(outputBuffer, d->outAudioBuffer + d->outPlayReadCount, nBufferFrames * 1 * sizeof(MY_TYPE));
        d->outPlayReadCount = d->outPlayReadCount + nBufferFrames * 1 * sizeof(MY_TYPE);
        return 0;
    }

    return 0;
}
