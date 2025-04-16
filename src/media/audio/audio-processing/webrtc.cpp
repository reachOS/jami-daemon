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

#include "webrtc.h"
#include "logger.h"

namespace jami {

inline size_t
webrtcFrameSize(AudioFormat format)
{
    return (size_t) (webrtc::AudioProcessing::kChunkSizeMs * format.sample_rate / 1000);
}

constexpr int webrtcNoError = webrtc::AudioProcessing::kNoError;

WebRTCAudioProcessor::WebRTCAudioProcessor(AudioFormat format, unsigned /* frameSize */)
    : AudioProcessor(format.withSampleFormat(AV_SAMPLE_FMT_FLTP), webrtcFrameSize(format))
{
    JAMI_LOG("[webrtc-ap] WebRTCAudioProcessor, frame size = {:d} (={:d} ms), channels = {:d}",
             frameSize_,
             frameDurationMs_,
             format_.nb_channels);

    config->high_pass_filter.enabled = true;
    apm->ApplyConfig(*config);

    webrtc::StreamConfig streamConfig((int) format_.sample_rate, (int) format_.nb_channels);
    webrtc::ProcessingConfig pconfig = {
        streamConfig, /* input stream */
        streamConfig, /* output stream */
        streamConfig, /* reverse input stream */
        streamConfig, /* reverse output stream */
    };

    if (apm->Initialize(pconfig) != webrtcNoError) {
        JAMI_ERROR("[webrtc-ap] Error initialising audio processing module");
    }
}

void
WebRTCAudioProcessor::enableNoiseSuppression(bool enabled)
{
    config->noise_suppression.enabled = enabled;
    config->noise_suppression.level = webrtc::AudioProcessing::Config::NoiseSuppression::Level::kVeryHigh;
    apm->ApplyConfig(*config);
}

void
WebRTCAudioProcessor::enableAutomaticGainControl(bool enabled)
{
    config->gain_controller2.enabled = enabled;
    config->gain_controller2.adaptive_digital.enabled = enabled;
    apm->ApplyConfig(*config);
}

void
WebRTCAudioProcessor::enableEchoCancel(bool enabled)
{
    config->echo_canceller.enabled = enabled;
    config->echo_canceller.enforce_high_pass_filtering = enabled;
    apm->ApplyConfig(*config);
}

void WebRTCAudioProcessor::setDelayOffsetMs(int delayOffset)
{
    // process deinterleaved float recorded data
    // delay = (t_render - t_analyze) + (t_process - t_capture)
    if (apm->set_stream_delay_ms(delayOffset) != webrtcNoError) {
        JAMI_ERR("[webrtc-ap] set_stream_delay_ms failed");
    }

}

void
WebRTCAudioProcessor::enableVoiceActivityDetection(bool _enabled)
{
    // TODO implement VAD again, webrtc removed it.
}

std::shared_ptr<AudioFrame>
WebRTCAudioProcessor::getProcessed()
{
    if (tidyQueues()) {
        return {};
    }

    auto playback = playbackQueue_.dequeue();
    auto record = recordQueue_.dequeue();
    if (!playback || !record) {
        return {};
    }
    webrtc::StreamConfig sc((int) format_.sample_rate, (int) format_.nb_channels);

    // process reverse in place
    float** playData = (float**) playback->pointer()->extended_data;
    if (apm->ProcessReverseStream(playData, sc, sc, playData) != webrtcNoError) {
        JAMI_ERR("[webrtc-ap] ProcessReverseStream failed");
    }

    apm->set_stream_analog_level(analogLevel_);

    // process in place
    float** recData = (float**) record->pointer()->extended_data;
    if (apm->ProcessStream(recData, sc, sc, recData) != webrtcNoError) {
        JAMI_ERR("[webrtc-ap] ProcessStream failed");
    }

    analogLevel_ = apm->recommended_stream_analog_level();
    // TODO implement VAD again, webrtc removed it.
    record->has_voice = false;
    // record->has_voice = vad->VoiceActivity(record.data(), record.num_samples(), record.sample_rate_hz()) == webrtc::Vad::Activity::kActive;
    return record;
}

} // namespace jami
