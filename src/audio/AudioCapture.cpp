#include <windows.h>
#undef max
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <comdef.h>
#include <iostream>
#include <cmath>
#include <algorithm> // for std::max
#include <cstring>

#include "audio/AudioCapture.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "avrt.lib")

AudioCapture::AudioCapture(VUBuffer &buffer, int sampleRate, int fftSize, float decayTimeSeconds)
    : vuBuffer(buffer),
      sampleRate(sampleRate),
      fftSize(fftSize),
            frequencyVUMeter(fftSize, decayTimeSeconds),
      running(false),
      sampleBuffer(fftSize, 0.0f),
      sampleWritePos(0),
      vuLevels(2, 0.0f) // Default to stereo, will resize in captureLoop
{
}

AudioCapture::~AudioCapture()
{
    stop();
}

void AudioCapture::start()
{
    if (!running)
    {
        running = true;
        captureThread = std::thread(&AudioCapture::captureLoop, this);
    }
}

void AudioCapture::stop()
{
    running = false;
    if (captureThread.joinable())
    {
        captureThread.join();
    }
}

void AudioCapture::captureLoop()
{
    HRESULT hr;
    CoInitialize(nullptr);

    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    IMMDevice *defaultDevice = nullptr;
    IAudioClient *audioClient = nullptr;
    IAudioCaptureClient *captureClient = nullptr;

    WAVEFORMATEX *waveFormat = nullptr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void **)&deviceEnumerator);
    if (FAILED(hr))
        goto cleanup;

    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    if (FAILED(hr))
        goto cleanup;

    hr = defaultDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&audioClient);
    if (FAILED(hr))
        goto cleanup;

    hr = audioClient->GetMixFormat(&waveFormat);
    if (FAILED(hr))
        goto cleanup;

    // Use shared mode
    hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,
                                 0, 0, waveFormat, nullptr);
    if (FAILED(hr))
        goto cleanup;

    hr = audioClient->GetService(__uuidof(IAudioCaptureClient), (void **)&captureClient);
    if (FAILED(hr))
        goto cleanup;

    hr = audioClient->Start();
    if (FAILED(hr))
        goto cleanup;

    const int numChannels = waveFormat->nChannels;
    const int captureSampleRate = waveFormat->nSamplesPerSec != 0
                                      ? static_cast<int>(waveFormat->nSamplesPerSec)
                                      : sampleRate.load();
    sampleRate.store(captureSampleRate);
    vuLevels.resize(numChannels, 0.0f); // Ensure correct size for current device
    sampleBuffer.resize(static_cast<size_t>(fftSize) * numChannels);
    const size_t fftHopSize = static_cast<size_t>(fftSize / 4);
    const size_t framesToKeep = static_cast<size_t>(fftSize) - fftHopSize;

    while (running)
    {
        UINT32 packetLength = 0;
        hr = captureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr))
            break;

        while (packetLength > 0)
        {
            BYTE *data;
            UINT32 numFramesAvailable;
            DWORD flags;

            hr = captureClient->GetBuffer(&data, &numFramesAvailable, &flags, nullptr, nullptr);
            if (FAILED(hr))
                break;

            if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
            {
                float *samples = (float *)data;
                std::fill(vuLevels.begin(), vuLevels.end(), 0.0f); // Reset buffer

                for (UINT32 i = 0; i < numFramesAvailable; ++i)
                {
                    for (int ch = 0; ch < numChannels; ++ch)
                    {
                        float sample = samples[i * numChannels + ch];
                        vuLevels[ch] = std::max(vuLevels[ch], std::abs(sample));
                    }

                    for (int ch = 0; ch < numChannels; ++ch)
                        sampleBuffer[sampleWritePos * numChannels + ch] = samples[i * numChannels + ch];

                    ++sampleWritePos;

                    if (sampleWritePos == static_cast<size_t>(fftSize))
                    {
                        frequencyVUMeter.processAudio(sampleBuffer.data(), fftSize, captureSampleRate, numChannels);
                        std::memmove(sampleBuffer.data(), sampleBuffer.data() + fftHopSize * numChannels,
                                     framesToKeep * numChannels * sizeof(float));
                        sampleWritePos = framesToKeep;
                    }
                }
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    vuBuffer.setChannelVU(ch, vuLevels[ch]);
                }
            }
            else
            {
                // If silent, optionally decay VUs
                // Could implement decay logic here if desired
            }

            hr = captureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr))
                break;

            hr = captureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr))
                break;
        }

        Sleep(5);
    }

    audioClient->Stop();

cleanup:
    if (waveFormat)
        CoTaskMemFree(waveFormat);
    if (captureClient)
        captureClient->Release();
    if (audioClient)
        audioClient->Release();
    if (defaultDevice)
        defaultDevice->Release();
    if (deviceEnumerator)
        deviceEnumerator->Release();
    CoUninitialize();
}
