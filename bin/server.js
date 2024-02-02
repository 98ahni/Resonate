var http = require('http');
var fs = require('fs');
const path = require('path'); 
const { exec } = require('node:child_process')
const { execSync } = require('node:child_process')

// Skip compile steps:
const SKIP_ImguiCompile = true;
const SKIP_SourceCompile = false;
const SKIP_Linking = false;

//(async() =>{
let readImguiDone = false;
let readSourceDone = false;
let imguiFiles = [];
fs.readdir('/workspaces/Resonate/imgui/', { recursive: true }, (err, files)=>{imguiFiles = files; readImguiDone = true;});
let sourceFiles = [];
fs.readdir('/workspaces/Resonate/Source/', { recursive: true }, (err, files)=>{sourceFiles = files; readSourceDone = true;});
let objectFiles = [];

//exec('cd /workspaces/Resonate/');
exec('sudo su root');
let compileErr = false;
const execOutFunc = function(err, output)
{
    // once the command has completed, the callback function is called
    if (err) {
        // log and return if we encounter an error
        console.error("could not execute command: ", err);
        compileErr = true;
        //return
    }
    // log the output received from the command
    console.log("Output: \n", output)
}

while(!(readImguiDone && readSourceDone))
//while(!readSourceDone)
{}
console.log('done ' + readImguiDone + ', ' + readSourceDone);

imguiFiles.forEach(file => {
    if(path.extname(file).startsWith('.c'))
    {
        if(!SKIP_ImguiCompile)
        {
            execSync('emcc \"/workspaces/Resonate/imgui/' + file + '\" -D\"EMSCRIPTEN=1\" -D\"__EMSCRIPTEN__=1\" -pedantic -x c++ -I\"/workspaces/Resonate/\" -I\"/workspaces/Resonate/imgui/\" -I\"/workspaces/Resonate/imgui/backends/\" -g -D\"NO_FREETYPE\" -D\"DEBUG\" -D\"_DEBUG\" -D\"_DEBUG_\" -c -O2 -std=c++20 -o /workspaces/Resonate/bin/intermediate/' + path.basename(file, path.extname(file)) + '.o', execOutFunc);
        }
        objectFiles.push('/workspaces/Resonate/bin/intermediate/' + path.basename(file, path.extname(file)) + '.o');
    }
});
sourceFiles.forEach(file => {
    if(path.extname(file).startsWith('.c'))
    {
        if(!SKIP_SourceCompile)
        {
            execSync('emcc \"/workspaces/Resonate/Source/' + file + '\" -D\"EMSCRIPTEN=1\" -D\"__EMSCRIPTEN__=1\" -pedantic -x c++ -I\"/workspaces/Resonate/\" -I\"/workspaces/Resonate/Source/\" -I\"/workspaces/Resonate/imgui/\" -I\"/workspaces/Resonate/imgui/backends/\" -g -D\"NO_FREETYPE\" -D\"DEBUG\" -D\"_DEBUG\" -D\"_DEBUG_\" -c -O2 -std=c++20 -o /workspaces/Resonate/bin/intermediate/' + path.basename(file, path.extname(file)) + '.o', execOutFunc);
        }
        objectFiles.push('/workspaces/Resonate/bin/intermediate/' + path.basename(file, path.extname(file)) + '.o');
    }
});
if(!SKIP_Linking)
{
    execSync('emcc \"' + objectFiles.join('\" \"') + '\" -o /workspaces/Resonate/bin/public/Resonate.html --bind -O2 --shell-file \"/workspaces/Resonate/.vscode/imgui_shell.html\" -g3 -lglfw -lGL -sUSE_GLFW=3 -sUSE_WEBGPU=1 -sUSE_WEBGL2=1 -sASYNCIFY -sEXPORTED_FUNCTIONS="[\'_malloc\',\'_free\',\'_main\']" -sEXPORTED_RUNTIME_METHODS="[\'allocateUTF8\']" -sALLOW_MEMORY_GROWTH', execOutFunc);// --embed-file Emscripten/Assets/
}

// run the `ls` command using exec
//exec('emcc \"/workspaces/Resonate/Source/main.cpp\" -D\"EMSCRIPTEN=1\" -D\"__EMSCRIPTEN__=1\" -pedantic -x c++-header -I\"/workspaces/Resonate/\" -I\"/workspaces/Resonate/imgui/\" -I\"/workspaces/Resonate/imgui/backends/\" -g -x c++ -D\"NO_FREETYPE\" -D\"DEBUG\" -D\"_DEBUG\" -D\"_DEBUG_\" -O0 -std=c++20 -o /workspaces/Resonate/bin/Resonate.html --bind -O0 --shell-file \"/workspaces/Resonate/.vscode/imgui_shell.html\" -g3', (err, output) => {

if(compileErr)
{
    process.exit(1);
}
const PORT=8080;
var express = require('express');
var app = express();
app.use(express.static(path.join(__dirname, 'public')));
app.get('/', async(req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'Resonate.html'));
});
app.listen(PORT);

//fs.readFile('/bin/Resonate.html', function (err, html) {

//    if (err) throw err;    

//    http.createServer(function(request, response) {  
//        response.writeHead(200, {"Content-Type": "text/html"});
//        response.write(html.toString());
//        response.end();
//    }).listen(PORT);
//});
//})();