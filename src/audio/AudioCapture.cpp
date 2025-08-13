#include <windows.h>
#undef max
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <comdef.h>
#include <iostream>
#include <cmath>
#include <algorithm> // for std::max

#include "audio/AudioCapture.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "avrt.lib")

AudioCapture::AudioCapture(VUBuffer &buffer, int sampleRate, int fftSize)
    : vuBuffer(buffer),
      sampleRate(sampleRate),
      fftSize(fftSize),
      frequencyVUMeter(fftSize),
      running(false),
      sampleBuffer(fftSize, 0.0f),
      sampleWritePos(0)
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
                std::vector<float> vuLevels(numChannels, 0.0f);

                for (UINT32 i = 0; i < numFramesAvailable; ++i)
                {
                    for (int ch = 0; ch < numChannels; ++ch)
                    {
                        float sample = samples[i * numChannels + ch];
                        vuLevels[ch] = std::max(vuLevels[ch], std::abs(sample));
                    }

                    // Convert to mono by averaging channels
                    float sum = 0.0f;
                    for (int ch = 0; ch < numChannels; ++ch)
                        sum += samples[i * numChannels + ch];
                    float monoSample = sum / numChannels;

                    // Write sample to circular buffer
                    sampleBuffer[sampleWritePos] = monoSample;
                    sampleWritePos = (sampleWritePos + 1) % fftSize;

                    // When buffer is full (writePos back to 0), process FFT
                    if (sampleWritePos == 0)
                    {
                        /* std::cout << "Sample rate: " << sampleRate;
                        std::cout << "fftSize: " << fftSize;
                        std::cout << "sampleBuffer: " << sampleBuffer.data(); */
                        frequencyVUMeter.processAudio(sampleBuffer.data(), fftSize, sampleRate);
                    }
                }
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    vuBuffer.setChannelVU(ch, vuLevels[ch]);
                }

                /* // --- NEW: Process frequency spectrum ---
                // Mixdown to mono (optional)
                std::vector<float> monoBuffer(numFramesAvailable);
                for (UINT32 i = 0; i < numFramesAvailable; ++i)
                {
                    float sum = 0.0f;
                    for (int ch = 0; ch < numChannels; ++ch)
                    {
                        sum += samples[i * numChannels + ch];
                    }
                    monoBuffer[i] = sum / numChannels; // average channels
                }

                frequencyVumeter.update(monoBuffer.data(), monoBuffer.size()); */
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
