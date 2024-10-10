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

var canvas = null;
var context;
var bgColor = 0;
var decorColor = 0;
function render(time){
    let width = canvas.width;
    let height = canvas.height;
    let size = (width < height ? width : height) * .4;
    let sizeMult = (Math.sin(time * .0005) * .5) + .5;
    let alphaMult = (1 - sizeMult) + .5;

    context.clearRect(0, 0, width, height);
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
        canvas.style.zIndex = 2;
    }
    else if(msg.data.cmd == 'hide'){
        canvas.width = msg.data.width;
        canvas.height = msg.data.height;
        canvas.style.zIndex = 0;
    }
}