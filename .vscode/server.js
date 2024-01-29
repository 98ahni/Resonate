var http = require('http');
var fs = require('fs');
const { exec } = require('node:child_process')

const PORT=8080;
let compileErr = false;

//exec('cd /workspaces/Resonate/');
exec('sudo su root');
//exec('emcmake cmake', function(output, err){
//    if(err){
//        console.log('error:', err);
//        compileErr = true;
//    }
//    console.log(output);
//});
// run the `ls` command using exec
exec('emcc \"/workspaces/Resonate/Source/main.cpp\" -D\"EMSCRIPTEN=1\" -D\"__EMSCRIPTEN__=1\" -pedantic -x c++-header -I\"/workspaces/Resonate/\" -I\"/workspaces/Resonate/imgui/\" -I\"/workspaces/Resonate/imgui/backends/\" -g -x c++ -D\"NO_FREETYPE\" -D\"DEBUG\" -D\"_DEBUG\" -D\"_DEBUG_\" -O0 -std=c++20 -o /workspaces/Resonate/bin/Resonate.html --bind -O0 --shell-file \"/workspaces/Resonate/.vscode/imgui_shell.html\" -g3', (err, output) => {
    // once the command has completed, the callback function is called
    if (err) {
        // log and return if we encounter an error
        console.error("could not execute command: ", err);
        compileErr = true;
        //return
    }
    // log the output received from the command
    console.log("Output: \n", output)
});

if(!compileErr){
fs.readFile('bin/Resonate.html', function (err, html) {

    if (err) throw err;    

    http.createServer(function(request, response) {  
        response.writeHead(200, {"Content-Type": "text/html"});  
        response.write(html);  
        response.end();  
    }).listen(PORT);
});}