//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author
/*

This file is the for using the RubberBand stretcher libraty as a standalone WebAssembly worker.
It contains the main function for setup and javascript and c++ functions to interact with the main thread through the limited system of javascript workers.
The resulting 'RubberBand.js' file can be added to any .html document and all functions accessed through the Module object.

*/

#include <stdio.h>
#include <emscripten.h>
#include <emscripten/val.h>
#include "../Rubberband/rubberband/RubberBandStretcher.h"
#define VAR_TO_JS(var) (emscripten::val(var).as_handle())
#define VEC_TO_JS(vec) (emscripten::val::array(vec).as_handle())
#define VAR_FROM_JS(var) (emscripten::val::take_ownership(var))
struct RubberbandLogger : public RubberBand::RubberBandStretcher::Logger
{
    /// Receive a log message with no numeric values.
    void log(const char* aMsg)
    {
        printf("RubberBand | %s\n", aMsg);
    }
    /// Receive a log message and one accompanying numeric value.
    void log(const char* aMsg, double aValue)
    {
        printf("RubberBand | %s | %f\n", aMsg, aValue);
    }
    /// Receive a log message and two accompanying numeric values.
    void log(const char* aMsg, double aValue, double anotherValue)
    {
        printf("RubberBand | %s | %f | %f\n", aMsg, aValue, anotherValue);
    }
};

EM_JS(emscripten::EM_VAL, get_channel_from_buffer, (emscripten::EM_VAL index), {
    return Emval.toHandle(global_audio_buffer[Emval.toValue(index)]);
}
var global_audio_buffer = {};
);
extern"C" EMSCRIPTEN_KEEPALIVE void jsRubberbandAudio(emscripten::EM_VAL aSampleRate, emscripten::EM_VAL aChannelNum)
{
#if _RELEASE
    RubberBand::RubberBandStretcher::setDefaultDebugLevel(0);
#else
    RubberBand::RubberBandStretcher::setDefaultDebugLevel(1);
#endif
    size_t sampleRate = VAR_FROM_JS(aSampleRate).as<size_t>();
    size_t channelNum = VAR_FROM_JS(aChannelNum).as<size_t>();
    std::vector<std::vector<float>> channelArrays;
    std::vector<float*> channelStarts;
    size_t numSamples = -1;
    for(int i = 0; i < channelNum; i++)
    {
        channelArrays.push_back(emscripten::vecFromJSArray<float>(VAR_FROM_JS(get_channel_from_buffer(VAR_TO_JS(i)))));
        channelStarts.push_back(channelArrays[channelArrays.size() - 1].data());
        numSamples = std::min(numSamples, channelArrays[channelArrays.size() - 1].size());
    }
    for(int i = 4; i > 3; i--)
    {
        printf("Stretching audio %i...\n", i);
        RubberBand::RubberBandStretcher stretcher(sampleRate, channelNum, std::make_shared<RubberbandLogger>(), RubberBand::RubberBandStretcher::Option::OptionThreadingNever | RubberBand::RubberBandStretcher::Option::OptionWindowShort | RubberBand::RubberBandStretcher::Option::OptionEngineFaster);
        stretcher.setTimeRatio(1 / (i * 0.1));
        stretcher.study(channelStarts.data(), numSamples, true);
        std::vector<std::vector<float>> answer;
        std::vector<float*> answerPointers;
        for(int j = 0; j < channelNum; j++)
        {
            std::vector<float> newVec;
            answer.push_back(newVec);
            answerPointers.push_back(answer[answer.size() - 1].data());
        }
        for(size_t queueStart = 0; queueStart < numSamples; queueStart += stretcher.getSamplesRequired())
        {
            size_t queue = numSamples - queueStart;
            size_t numProcess = stretcher.getSamplesRequired() < queue ? stretcher.getSamplesRequired() : queue;
            stretcher.process(channelStarts.data(), numProcess, queue < stretcher.getSamplesRequired());
            size_t avail = stretcher.available();
            for(int ch = 0; ch < channelNum; ch++)
            {
                channelStarts[ch] += numProcess;
                answerPointers[ch] = &answer[ch].back() + 1;
                answer[ch].resize(answer[ch].size() + avail);
            }
            stretcher.retrieve(answerPointers.data(), avail);
        }
        std::vector<emscripten::val> output;
        for(int j = 0; j < channelNum; j++)
        {
            output.push_back(emscripten::val::array(answer[j]));
        }
        EM_ASM({
            let input = Emval.toValue($0);
            //for(let i = 0; i < input.length; i++) {
            //    input[i] = Emval.toValue(input[i]);
            //}
            //global_audio_blobs[$2] = Module.audioDataArrayToBlob(input, $1);
            postMessage([Module.audioDataArrayToBlob(input, $1), $2]);
        }, VEC_TO_JS(output), sampleRate, i - 1);
    }
    //printf("Done stretching audio!\n");
}

extern"C" EMSCRIPTEN_KEEPALIVE emscripten::EM_VAL jsRubberbandRealtimeAudio(emscripten::EM_VAL someSampleFrames, emscripten::EM_VAL aSampleRate, emscripten::EM_VAL aChannelNum, emscripten::EM_VAL aPlaybackRate)
{
#if _RELEASE
    RubberBand::RubberBandStretcher::setDefaultDebugLevel(0);
#else
    RubberBand::RubberBandStretcher::setDefaultDebugLevel(1);
#endif
    std::vector<emscripten::val> intermediateBuffer = emscripten::vecFromJSArray<emscripten::val>(VAR_FROM_JS(someSampleFrames));
    size_t sampleRate = VAR_FROM_JS(aSampleRate).as<size_t>();
    size_t channelNum = VAR_FROM_JS(aChannelNum).as<size_t>();
    double playbackRate = VAR_FROM_JS(aPlaybackRate).as<double>();
    std::vector<std::vector<float>> sampleBuffer;
    std::vector<float*> channelPoiners;
    size_t numSamples = -1;
    for(int i = 0; i < channelNum; i++)
    {
        sampleBuffer.push_back(emscripten::vecFromJSArray<float>(intermediateBuffer[i]));
        channelPoiners.push_back(sampleBuffer[sampleBuffer.size() - 1].data());
        numSamples = std::min(numSamples, sampleBuffer[sampleBuffer.size() - 1].size());
    }
    static RubberBand::RubberBandStretcher stretcher(sampleRate, channelNum, std::make_shared<RubberbandLogger>(), RubberBand::RubberBandStretcher::Option::OptionProcessRealTime | RubberBand::RubberBandStretcher::Option::OptionWindowShort);
    stretcher.process(channelPoiners.data(), numSamples, false);
    size_t avail = stretcher.available();
    std::vector<std::vector<float>> answer;
    std::vector<float*> answerPointers;

    for(int j = 0; j < channelNum; j++)
    {
        std::vector<float> newVec;
        newVec.resize(avail);
        answer.push_back(newVec);
        answerPointers.push_back(answer[answer.size() - 1].data());
    }
    stretcher.retrieve(answerPointers.data(), avail);
    intermediateBuffer.clear();
    for(int j = 0; j < channelNum; j++)
    {
        intermediateBuffer.push_back(emscripten::val::array(answer[j]));
    }
    return VEC_TO_JS(intermediateBuffer);
}

int main()
{
    return 0;
}