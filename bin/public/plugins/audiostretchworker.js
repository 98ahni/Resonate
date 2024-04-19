var Module = {};
importScripts('timestretch.js');

onmessage = async function (msg)
{
    const channelDataArray = msg.data[0];
    const samples = msg.data[1];
    const sampleRate = msg.data[2];
    const isSafari = msg.data[3];
    for(var ind = 8; ind > 3; ind -= 2){
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