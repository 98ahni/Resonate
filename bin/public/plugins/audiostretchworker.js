var Module = {};
importScripts('timestretch.js', 'RubberBand.js', 'VexWarp/main.js', 'paulstretch.js');

onmessage = async function (msg)
{
    const engine = msg.data[0];
    const channelDataArray = msg.data[1];
    const samples = msg.data[2];
    const sampleRate = msg.data[3];
    const isSafari = msg.data[4];
    console.time('Audio stretch completed in');
    console.log('Stretching audio using ' + engine + ' engine...');
    if(engine === 'Legacy' || engine === 'LegacyHybrid'){
        for(var ind = (engine === 'Legacy' ? 9 : 4); ind > 3; ind--){
            var outblob = null;
            //if(ind == 5) outblob = Module.audioBufferToBlob(Module.stretch(audioEmptyBuff, audioBuffer, 2));
            //else
            console.log(isSafari ? 'Browser is Safari' : 'Browser is not Safari');
            outblob = Module.audioDataArrayToBlob(Module.stretch(channelDataArray, samples, 1 / (ind * 0.1), (isSafari ? sampleRate : sampleRate / 2)), sampleRate);
            console.log('Streched blob nr ' + ind);
            postMessage([outblob, ind]);
        }
        //outblob = undefined;      // Memory management
    }
    else if(engine === 'RubberBand'){
        global_audio_buffer = channelDataArray;
        Module._jsRubberbandAudio(Emval.toHandle(sampleRate), Emval.toHandle(channelDataArray.length));
    }
    else if(engine === 'VexWarp' || engine === 'VexWarpHybrid'){
        for(var ind = (engine === 'VexWarp' ? 9 : 4); ind > 3; ind--){
            const VexWarp = new Module.VexWarpStretch({vocode:false, stftBins:8192, stftHop:1/2, stretchFactor:1 / (ind * 0.1), sampleRate:sampleRate});
            var output = [];
            for(var ch = 0; ch < channelDataArray.length; ch++){
                VexWarp.setBuffer(channelDataArray[ch], sampleRate);
                VexWarp.stretch();
                output[ch] = VexWarp.getStretchedBuffer();
            }
            outblob = Module.audioDataArrayToBlob(output, sampleRate);
            postMessage([outblob, ind]);
        }
    }
    else if(engine === 'PaulStretch'){
        const bufferSize = 4096;
        const batchSize = 4 / 4096;
        const paulStretch = new PaulStretch(channelDataArray.length, 1 / (ind * 0.1), bufferSize * batchSize);
        paulStretch.write(channelDataArray);
        var outblocks = [];
        var output = [];
        for(var ch = 0; ch < channelDataArray.length; ch++){
            outblocks.push(new Float32Array(bufferSize));
            output.push([]);
        }
        for(var ind = 9; ind > 3; ind--){
            console.log('Stretching audio nr ' + ind);
            paulStretch.setRatio(1 / (ind * 0.1));
            while(paulStretch.writeQueueLength() !== 0){
                while ((paulStretch.readQueueLength() < (bufferSize)) 
                    && (paulStretch.process() !== 0)) { paulStretch.readQueueLength(); }
                if(paulStretch.read(outblocks) === null){ break; }
                for(var ch = 0; ch < channelDataArray.length; ch++){
                    output[ch] = output[ch].concat(Array.from(outblocks[ch]));
                }
            }
            outblob = Module.audioDataArrayToBlob(output, sampleRate);
            postMessage([outblob, ind]);
        }
    }
    console.log('Audio stretching completed using ' + engine);
    console.timeEnd('Audio stretch completed in');
}
