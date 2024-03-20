var http = require('http');
var fs = require('fs');
const path = require('path'); 
const { exec } = require('node:child_process');
const { execSync } = require('node:child_process');
var os = require('os');

// Skip compile steps:
const REBUILD_Imgui = false;
const REBUILD_Source = false;
const SKIP_ImguiCompile = true;
const SKIP_SourceCompile = false;
const SKIP_Linking = false;

const WIN32 = os.platform() === "win32";
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

let projectPath = "/workspaces/Resonate/";
let compilerPath = "emcc";
if(WIN32)
{
    //projectPath = __dirname + "/../";
    projectPath = "./";
    compilerPath = "\"D:\\Documents\\Visual Studio 2022\\Visual Studio Projects\\emsdk-main\\upstream\\emscripten\\emcc\"";
    //execSync('cd "..\\..\\..\\Visual Studio 2022\\Visual Studio Projects\\emsdk-main\\upstream\\emscripten\\"', execOutFunc);
}

//(async() =>{
//let readImguiDone = false;
//let readSourceDone = false;
let imguiFiles = [];
let sourceFiles = [];
let objectFiles = [];
if(WIN32)
{
    function getFiles(baseDir, currdir)
    {
        let output = [];
        let files = fs.readdirSync(baseDir + currdir);
        files.forEach(file => {
            if(fs.lstatSync(path.join(baseDir + currdir, file)).isDirectory())
            {
                output.push(...getFiles(baseDir, path.join(currdir, file)));
            }
            else if(fs.lstatSync(path.join(baseDir + currdir, file)).isFile())
            {
                output.push(path.join(currdir, file));
            }
        });
        return output;
    }

    imguiFiles = getFiles(projectPath + 'imgui/', '');
    sourceFiles = getFiles(projectPath + 'Source/', '');
}
else
{
    imguiFiles = fs.readdirSync(projectPath + 'imgui/', { recursive: true });//, (err, files)=>{readImguiDone = true; if(err){console.error('Files not found (imgui)'); return;} imguiFiles = files; });
    sourceFiles = fs.readdirSync(projectPath + 'Source/', { recursive: true });//, (err, files)=>{readSourceDone = true; if(err){console.error('Files not found (Source)'); return;} sourceFiles = files; });
}

if(!WIN32) {exec('sudo su root');}

//while(!(readImguiDone && readSourceDone))
//while(!readSourceDone)
//{}
//console.log('done ' + readImguiDone + ', ' + readSourceDone);

//if(WIN32) {exec('cd "../../../Visual Studio 2022/Visual Studio Projects/emsdk-main/upstream/emscripten/"');}

var lastCompileTime = 0;
if(fs.existsSync(projectPath + 'bin/public/Resonate.wasm'))
{
    lastCompileTime = fs.statSync(projectPath + 'bin/public/Resonate.wasm').mtimeMs;
}
let hasUnlinkedFiles = false;

imguiFiles.forEach(file => {
    if(path.extname(file).startsWith('.c'))
    {
        if(REBUILD_Imgui || (!SKIP_ImguiCompile && lastCompileTime < fs.statSync(projectPath + 'imgui/' + file).mtimeMs))
        {
            console.log(new TextDecoder().decode(execSync(compilerPath + ' \"' + projectPath + 'imgui/' + file + '\" -D\"EMSCRIPTEN=1\" -D\"__EMSCRIPTEN__=1\" -pedantic -x c++ -I\"' + projectPath + '\" -I\"' + projectPath + 'imgui/\" -I\"' + projectPath + 'imgui/backends/\" -g -D\"NO_FREETYPE\" -D\"DEBUG\" -D\"_DEBUG\" -D\"_DEBUG_\" -c -O2 -std=c++23 -w -o \"' + projectPath + 'bin/intermediate/' + path.basename(file, path.extname(file)) + '.o\"', {env: process.env})));//
            hasUnlinkedFiles = true;
        }
        objectFiles.push(projectPath + 'bin/intermediate/' + path.basename(file, path.extname(file)) + '.o');
    }
});
sourceFiles.forEach(file => {
    if(path.extname(file).startsWith('.c'))
    {
        if(REBUILD_Source || (!SKIP_SourceCompile && lastCompileTime < fs.statSync(projectPath + 'Source/' + file).mtimeMs))
        {
            console.log(new TextDecoder().decode(execSync(compilerPath + ' \"' + projectPath + 'Source/' + file + '\" -D\"EMSCRIPTEN=1\" -D\"__EMSCRIPTEN__=1\" -pedantic -x c++ -I\"' + projectPath + '\" -I\"' + projectPath + 'Source/\" -I\"' + projectPath + 'imgui/\" -I\"' + projectPath + 'imgui/backends/\" -g -D\"NO_FREETYPE\" -D\"DEBUG\" -D\"_DEBUG\" -D\"_DEBUG_\" -c -O2 -std=c++23 -w -o \"' + projectPath + 'bin/intermediate/' + path.basename(file, path.extname(file)) + '.o\"', {env: process.env})));//
            hasUnlinkedFiles = true;
        }
        objectFiles.push(projectPath + 'bin/intermediate/' + path.basename(file, path.extname(file)) + '.o');
    }
});
if(!SKIP_Linking && hasUnlinkedFiles)
{
    console.log(new TextDecoder().decode(execSync(compilerPath + ' \"' + objectFiles.join('\" \"') + '\" -o \"' + projectPath + 'bin/public/Resonate.html\" --bind -O2 --shell-file \"' + projectPath + '.vscode/imgui_shell.html\" -g3 -lglfw -lGL -sUSE_GLFW=3 -sUSE_WEBGPU=1 -sUSE_WEBGL2=1 -sASYNCIFY -sEXPORTED_FUNCTIONS="[\'_malloc\',\'_free\',\'_main\']" -sEXPORTED_RUNTIME_METHODS="[\'allocateUTF8\']" -sALLOW_MEMORY_GROWTH --embed-file ' + projectPath + 'bin/Syllabify/@Syllabify/', {env: process.env})));// --embed-file Emscripten/Assets/
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

if(WIN32)
{
    const rt = exec("start http://localhost:8080/");
    rt.on("data", (data)=>{console.log(data);});
    //rt.on("close", (code)=>{process.exit(code);});
}

//fs.readFile('/bin/Resonate.html', function (err, html) {

//    if (err) throw err;    

//    http.createServer(function(request, response) {  
//        response.writeHead(200, {"Content-Type": "text/html"});
//        response.write(html.toString());
//        response.end();
//    }).listen(PORT);
//});
//})();