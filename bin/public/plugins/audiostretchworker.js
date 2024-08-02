var Module = {};
importScripts('timestretch.js', 'RubberBand.js');

onmessage = async function (msg)
{
    const useLegacy = msg.data[0];
    const channelDataArray = msg.data[1];
    const samples = msg.data[2];
    const sampleRate = msg.data[3];
    const isSafari = msg.data[4];
    if(useLegacy){
        for(var ind = 9; ind > 3; ind--){
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
    else{
        global_audio_buffer = channelDataArray;
        Module._jsRubberbandAudio(Emval.toHandle(sampleRate), Emval.toHandle(channelDataArray.length));
    }
}
