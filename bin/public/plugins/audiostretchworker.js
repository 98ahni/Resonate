var Module = {};
importScripts('timestretch.js');

onmessage = async function (msg)
{
    const channelDataArray = msg.data[0];
    const samples = msg.data[1];
    const ind = msg.data[2];
    var outblob = null;
    //if(ind == 5) outblob = Module.audioBufferToBlob(Module.stretch(audioEmptyBuff, audioBuffer, 2));
    //else
    outblob = audioBufferToBlob(stretch(channelDataArray, samples, 1 / (ind * 0.1)));
    console.log('Streched blob nr ' + ind);
    postMessage([outblob, ind]);
}