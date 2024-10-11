//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

const default_console_log = console.log;
const default_console_warn = console.warn;
const default_console_error = console.error;
(()=>{
    self.onerror = function (event, source, lineno, colno, error) {
        console.error("onerror: " + event.name + ": " + event.message + 
        "\n\t/bin/public/" + source.split('/').slice(-1) + ":" + lineno + ":" + colno);
        console.error(error.stack);
          //if(window.matchMedia('(any-pointer: coarse)').matches) // Enable if alert should be for touch only.
        alert("OnError: \n" + event.name + ": " + event.message);
    };
    fetch("/console", {
        method: "POST",
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({type: 'log', data: ['Log server open']})
    }).then(
    /*resolve*/(response)=>{
        if(response.status == 405){
            return;
        }
        console.log = (...data) => {
            fetch("/console", {
                method: "POST",
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({type: 'log', data})
            }).then(
            /*resolve*/()=>{}, 
            /*reject*/()=>{
                // No server, restore functions
                console.log = default_console_log;
                console.warn = default_console_warn;
                console.error = default_console_error;
            });
            default_console_log.apply(console, data);
        };
        console.warn = (...data) => {
            fetch("/console", {
                method: "POST",
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({type: 'warn', data})
            }).then(
            /*resolve*/()=>{}, 
            /*reject*/()=>{
                // No server, restore functions
                console.log = default_console_log;
                console.warn = default_console_warn;
                console.error = default_console_error;
            });
            default_console_warn.apply(console, data);
        };
        console.error = (...data) => {
            fetch("/console", {
                method: "POST",
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({type: 'error', data})
            }).then(
            /*resolve*/()=>{}, 
            /*reject*/()=>{
                // No server, restore functions
                console.log = default_console_log;
                console.warn = default_console_warn;
                console.error = default_console_error;
            });
            default_console_error.apply(console, data);
        };
    });
})();
function remap(value, l1, h1, l2, h2) {
    return l2 + (h2 - l2) * (value - l1) / (h1 - l1);
}

const animMaxDuration = 250;

var canvas = null;
var context;
var bgColor = 0;
var decorColor = 0;
var isShowing = false;
var animIn = false;
var animOut = false;
var animStartTime = 0;
function render(time){
    if(!isShowing){
        requestAnimationFrame(render);
        return;
    }
    let width = canvas.width;
    let height = canvas.height;
    let size = (width < height ? width : height) * .4;
    let sizeMult = ((Math.sin(time * .001) + 1) * .25) + .5;
    let alphaMult = (1 - sizeMult) + .5;

    context.clearRect(0, 0, width, height);
    if(animIn){
        if(animStartTime == 0){animStartTime = time;}
        let duration = time - animStartTime;
        if(duration >= animMaxDuration){
            duration = animMaxDuration;
            animIn = false;
            animStartTime = 0;
        }
        sizeMult = remap(Math.sqrt(Math.sqrt(duration)), 0, Math.sqrt(Math.sqrt(animMaxDuration)), 5, sizeMult);
        alphaMult = remap(duration, 0, animMaxDuration, 0, alphaMult);
        context.globalAlpha = remap(duration, 0, animMaxDuration, 0, 1);
    }
    if(animOut){
        if(animStartTime == 0){animStartTime = time; console.log("animOut start");}
        let duration = time - animStartTime;
        if(duration >= animMaxDuration){
            duration = animMaxDuration;
            animOut = false;
            isShowing = false;
            animStartTime = 0;
            console.log("animOut end");
            this.postMessage({cmd: 'resetZ'});
        }
        sizeMult = remap(duration * duration * duration, 0, animMaxDuration * animMaxDuration * animMaxDuration, sizeMult, 5);
        alphaMult = remap(duration, 0, animMaxDuration, alphaMult, 0);
        context.globalAlpha = remap(duration, 0, animMaxDuration, 1, 0);
    }

    context.fillStyle = `rgba(
        ${bgColor&0xff},
        ${(bgColor>>>8)&0xff},
        ${(bgColor>>>16)&0xff},
        ${(bgColor>>>24)/0xff}
    )`;
    context.fillRect(0, 0, width, height);
    
    context.fillStyle = "white";
    context.fillStyle = `rgba(
        ${decorColor&0xff},
        ${(decorColor>>>8)&0xff},
        ${(decorColor>>>16)&0xff},
        1
    )`;
    context.globalAlpha = .1 * alphaMult;
    context.beginPath();
    context.arc(width * .5, height * .5, size * sizeMult, 0, Math.PI * 2, true);
    context.fill();
    context.globalAlpha = .3 * alphaMult;
    context.beginPath();
    context.arc(width * .5, height * .5, size * .8 * sizeMult, 0, Math.PI * 2, true);
    context.fill();
    context.globalAlpha = .5 * alphaMult;
    context.beginPath();
    context.arc(width * .5, height * .5, size * .6 * sizeMult, 0, Math.PI * 2, true);
    context.fill();
    context.globalAlpha = .7 * alphaMult;
    context.beginPath();
    context.arc(width * .5, height * .5, size * .4 * sizeMult, 0, Math.PI * 2, true);
    context.fill();
    context.globalAlpha = .9 * alphaMult;
    context.beginPath();
    context.arc(width * .5, height * .5, size * .2 * sizeMult, 0, Math.PI * 2, true);
    context.fill();
    context.globalAlpha = 1;
    requestAnimationFrame(render);
}

onmessage = async function (msg)
{
    if(msg.data.cmd == 'init'){
        canvas = msg.data.canvas;
        context = canvas.getContext('2d');
        requestAnimationFrame(render);
    }
    else if(msg.data.cmd == 'show'){
        bgColor = msg.data.bgColor;
        decorColor = msg.data.decorColor;
        canvas.width = msg.data.width;
        canvas.height = msg.data.height;
        animIn = msg.data.animIn;
        isShowing = true;
    }
    else if(msg.data.cmd == 'hide'){
        canvas.width = msg.data.width;
        canvas.height = msg.data.height;
        animOut = true;
    }
}