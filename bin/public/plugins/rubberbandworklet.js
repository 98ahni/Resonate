var Module = {};
//import '/timestretch.js'
//import '/RubberBand.js'

class RubberbandProcessor extends AudioWorkletProcessor {
    constructor(...args) {
        super(...args);
        this.playbackRate = 1;
        this.port.onmessage = (e) => {
            this.playbackRate = e;
        };
    }
    process(inputs, outputs, parameters) {
        outputs = Emval.toValue(Module._jsRubberbandRealtimeAudio(Emval.toHandle(inputs), Emval.toHandle(sampleRate), Emval.toHandle(inputs.length), Emval.toHandle(this.playbackRate)));
        console.log(JSON.parse(parameters));
        return true;
    }
}
  
registerProcessor("rubberband-processor", RubberbandProcessor);