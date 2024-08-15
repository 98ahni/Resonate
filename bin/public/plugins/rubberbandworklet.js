//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

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