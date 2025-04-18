/*
 *  Copyright (C) 2004-2025 Savoir-faire Linux Inc.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CORE_LAYER_H_
#define CORE_LAYER_H_

#include "audio/audiolayer.h"
#include <AudioToolbox/AudioToolbox.h>

#define checkErr(err) \
    if (err) { \
        JAMI_ERR("CoreAudio Error: %ld", static_cast<long>(err)); \
    }

/**
 * @file  CoreLayer.h
 * @brief Main iOS sound class. Manages the data transfers between the application and the hardware.
 */

namespace jami {

class RingBuffer;

class CoreLayer : public AudioLayer
{
public:
    CoreLayer(const AudioPreference& pref);
    ~CoreLayer();

    /**
     * Scan the sound card available on the system
     * @return std::vector<std::string> The vector containing the string description of the card
     */
    virtual std::vector<std::string> getCaptureDeviceList() const;
    virtual std::vector<std::string> getPlaybackDeviceList() const;

    virtual int getAudioDeviceIndex(const std::string& name, AudioDeviceType type) const;
    virtual std::string getAudioDeviceName(int index, AudioDeviceType type) const;

    /**
     * Get the index of the audio card for capture
     * @return int The index of the card used for capture
     */
    virtual int getIndexCapture() const { return indexIn_; }

    /**
     * Get the index of the audio card for playback
     * @return int The index of the card used for playback
     */
    virtual int getIndexPlayback() const { return indexOut_; }

    /**
     * Get the index of the audio card for ringtone (could be differnet from playback)
     * @return int The index of the card used for ringtone
     */
    virtual int getIndexRingtone() const { return indexRing_; }

    /**
     * Configure the AudioUnit
     */
    bool initAudioLayerIO(AudioDeviceType stream);
    void setupOutputBus();
    void setupInputBus();
    void bindCallbacks();

    int initAudioStreams(AudioUnit* audioUnit);

    /**
     * Start the capture stream and prepare the playback stream.
     * The playback starts accordingly to its threshold
     * CoreAudio Library API
     */

    virtual void startStream(AudioDeviceType stream = AudioDeviceType::ALL);

    void destroyAudioLayer();

    /**
     * Stop the playback and capture streams.
     * Drops the pending frames and put the capture and playback handles to PREPARED state
     * CoreAudio Library API
     */
    virtual void stopStream(AudioDeviceType stream = AudioDeviceType::ALL);

private:
    NON_COPYABLE(CoreLayer);

    void initAudioFormat();

    static OSStatus outputCallback(void* inRefCon,
                                   AudioUnitRenderActionFlags* ioActionFlags,
                                   const AudioTimeStamp* inTimeStamp,
                                   UInt32 inBusNumber,
                                   UInt32 inNumberFrames,
                                   AudioBufferList* ioData);

    void write(AudioUnitRenderActionFlags* ioActionFlags,
               const AudioTimeStamp* inTimeStamp,
               UInt32 inBusNumber,
               UInt32 inNumberFrames,
               AudioBufferList* ioData);

    static OSStatus inputCallback(void* inRefCon,
                                  AudioUnitRenderActionFlags* ioActionFlags,
                                  const AudioTimeStamp* inTimeStamp,
                                  UInt32 inBusNumber,
                                  UInt32 inNumberFrames,
                                  AudioBufferList* ioData);

    void read(AudioUnitRenderActionFlags* ioActionFlags,
              const AudioTimeStamp* inTimeStamp,
              UInt32 inBusNumber,
              UInt32 inNumberFrames,
              AudioBufferList* ioData);

    virtual void updatePreference(AudioPreference& pref, int index, AudioDeviceType type);

    /**
     * Number of audio cards on which capture stream has been opened
     */
    int indexIn_;

    /**
     * Number of audio cards on which playback stream has been opened
     */
    int indexOut_;

    /**
     * Number of audio cards on which ringtone stream has been opened
     */
    int indexRing_;

    AudioUnit ioUnit_;

    Float64 inSampleRate_;
    UInt32 inChannelsPerFrame_;
    Float64 outSampleRate_;
    UInt32 outChannelsPerFrame_;

    std::condition_variable readyCv_ {};
    dispatch_queue_t audioConfigurationQueue;
};

} // namespace jami

#endif // CORE_LAYER_H_
