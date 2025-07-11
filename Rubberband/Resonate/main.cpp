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
#ifndef _RELEASE
    /// Receive a log message with no numeric values.
    void log(const char* aMsg)
    {
        EM_ASM(console.log('RubberBand | ' + Emval.toValue($0)), VAR_TO_JS(std::string(aMsg)));
        //printf("RubberBand | %s\n", aMsg);
    }
    /// Receive a log message and one accompanying numeric value.
    void log(const char* aMsg, double aValue)
    {
        EM_ASM(console.log('RubberBand | ' + Emval.toValue($0) + ' | ' + Emval.toValue($1)), VAR_TO_JS(std::string(aMsg)), VAR_TO_JS(aValue));
        //printf("RubberBand | %s | %f\n", aMsg, aValue);
    }
    /// Receive a log message and two accompanying numeric values.
    void log(const char* aMsg, double aValue, double anotherValue)
    {
        EM_ASM(console.log('RubberBand | ' + Emval.toValue($0) + ' | ' + Emval.toValue($1) + ' | ' + Emval.toValue($2)), VAR_TO_JS(std::string(aMsg)), VAR_TO_JS(aValue), VAR_TO_JS(anotherValue));
        //printf("RubberBand | %s | %f | %f\n", aMsg, aValue, anotherValue);
    }
#else
    void log(const char* aMsg) { }
    void log(const char* aMsg, double aValue) { }
    void log(const char* aMsg, double aValue, double anotherValue) { }
#endif
};

EM_JS(emscripten::EM_VAL, get_channel_from_buffer, (emscripten::EM_VAL index), {
    return Emval.toHandle(global_audio_buffer[Emval.toValue(index)]);
}
var global_audio_buffer = {};
);

size_t sampleRate;
size_t channelNum;
size_t stretchTo;
std::vector<std::vector<float>> channelArrays = {};
std::vector<float*> channelStarts = {};
size_t numSamples;
RubberBand::RubberBandStretcher* stretcher;
std::vector<std::vector<float>> answer = {};
std::vector<float*> answerPointers = {};

bool RubberBandLoop();
size_t queueStart;
bool hasStarted;

extern"C" EMSCRIPTEN_KEEPALIVE void jsRubberbandAudio(emscripten::EM_VAL aSampleRate, emscripten::EM_VAL aChannelNum, emscripten::EM_VAL aStretchTo)
{
#if _RELEASE
    RubberBand::RubberBandStretcher::setDefaultDebugLevel(0);
#else
    RubberBand::RubberBandStretcher::setDefaultDebugLevel(1);
#endif
    sampleRate = VAR_FROM_JS(aSampleRate).as<size_t>();
    channelNum = VAR_FROM_JS(aChannelNum).as<size_t>();
    stretchTo = VAR_FROM_JS(aStretchTo).as<size_t>();
    channelArrays.clear();
    channelStarts.clear();
    answer.clear();
    answerPointers.clear();
    numSamples = -1;
    queueStart = 0;
    hasStarted = false;
    for(int i = 0; i < channelNum; i++)
    {
        emscripten::val floatArrayObject = VAR_FROM_JS(get_channel_from_buffer(VAR_TO_JS(i)));
        unsigned int length = floatArrayObject["length"].as<unsigned int>();
        std::vector<float> floatArray;
        floatArray.resize(length);
        auto memory = emscripten::val::module_property("HEAPU8")["buffer"];
        auto memoryView = floatArrayObject["constructor"].new_(memory, reinterpret_cast<uintptr_t>(floatArray.data()), length);
        memoryView.call<void>("set", floatArrayObject);
        channelArrays.push_back(floatArray);
        //std::string arr = VAR_FROM_JS(get_channel_from_buffer(VAR_TO_JS(i))).as<std::string>();
        //channelArrays.push_back(std::vector<float>());
        //channelArrays.back().assign((float*)arr.data(), ((float*)arr.data()) + arr.size() / 4);
        //channelArrays.push_back(emscripten::vecFromJSArray<float>(VAR_FROM_JS(get_channel_from_buffer(VAR_TO_JS(i)))));
        channelStarts.push_back(channelArrays[channelArrays.size() - 1].data());
        numSamples = std::min(numSamples, channelArrays[channelArrays.size() - 1].size());
    }
#ifdef _DEBUG
    printf("Stretching audio %i...\n", stretchTo);
    EM_ASM(console.log('RubberBand | START'));
#endif
    stretcher = new RubberBand::RubberBandStretcher(sampleRate, channelNum, std::make_shared<RubberbandLogger>(),
        RubberBand::RubberBandStretcher::Option::OptionThreadingNever |
        RubberBand::RubberBandStretcher::Option::OptionProcessRealTime |
        RubberBand::RubberBandStretcher::Option::OptionWindowShort |
        RubberBand::RubberBandStretcher::Option::OptionEngineFaster);
    stretcher->setTimeRatio(1 / (stretchTo * 0.1));
    for(int j = 0; j < channelNum; j++)
    {
        std::vector<float> newVec;
        answer.push_back(newVec);
        answerPointers.push_back(answer[answer.size() - 1].data());
    }
#ifdef _DEBUG
    EM_ASM(console.log('RubberBand | INIT COMPLETE'));
#endif
    while(RubberBandLoop());
}

bool RubberBandLoop()
{
    if(!stretcher) return false;
    queueStart += stretcher->getSamplesRequired();
    if(!hasStarted)
    {
#ifdef _DEBUG
        EM_ASM(console.log('RubberBand | LOOP BEGIN'));
#endif
        hasStarted = true;
    }
    if(queueStart < numSamples)
    {
        size_t queue = numSamples - queueStart;
        size_t numProcess = stretcher->getSamplesRequired() < queue ? stretcher->getSamplesRequired() : queue;
        stretcher->process(channelStarts.data(), numProcess, queue <= stretcher->getSamplesRequired());
        size_t avail = stretcher->available();
        for(int ch = 0; ch < channelNum; ch++)
        {
            channelStarts[ch] += numProcess;
            size_t answerCurrSize = answer[ch].size();
            answer[ch].resize(answer[ch].size() + avail);
            answerPointers[ch] = (&answer[ch].front()) + answerCurrSize;
        }
        stretcher->retrieve(answerPointers.data(), avail);
        //EM_ASM(console.log('RubberBand | PROCESS : ' + $0 + ' : ' + $1), queueStart, numSamples);
        return true;
    }
    else
    {
#ifdef _DEBUG
        EM_ASM(console.log('RubberBand | DONE STRETCHING'));
#endif
        //delete stretcher;
        //stretcher = nullptr;
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
        }, VEC_TO_JS(output), sampleRate, stretchTo);
        return false;
    }
    return false;
}

int main()
{
    //emscripten_set_main_loop(RubberBandLoop, 0, false);
    return 0;
}