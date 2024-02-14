#include "FileHandler.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <Defines.h>

EM_ASYNC_JS(emscripten::EM_VAL, open_directory, (emscripten::EM_VAL mode), {
	//let reader = new FileReader();
    // Feature detection. The API needs to be supported
    // and the app not run in an iframe.
    //const supportsFileSystemAccess =
    //    "showDirectoryPicker" in window &&
    //    (() => {
    //        try
    //        {
    //            return window.self === window.top;
    //        }
    //        catch
    //        {
    //            return false;
    //        }
    //    })();
    //// If the File System Access API is supported…
    //if (supportsFileSystemAccess)
    //{
    //    let directoryStructure = undefined;
    //    let directoryName = undefined;
//
    //    const getFiles = async(dirHandle, path = dirHandle.name) =>
    //    {
    //        //for await(const entry of dirHandle.values())
    //        //{
    //        //    const nestedPath = '${path}/${entry.name}';
    //        //    if (entry.kind === "file")
    //        //    {
    //        //        //const read = (data) => new Promise((resolve, reject) => {
    //        //        //    reader.onload = (event) => resolve(event.target.result);
    //        //        //    reader.onerror = reject;
    //        //        //    reader.readAsText(data);
    //        //        //});
	//		//        //let result = await read(entry);
	//		//        //const uint8_view = new Uint8Array(result);
	//		//        //FS.writeFile(nestedPath, uint8_view);
    //        //        
    //        //        files.push(
    //        //            entry.getFile().then((file) => {
    //        //                file.directoryHandle = dirHandle;
    //        //                file.handle = entry;
    //        //                return Object.defineProperty(file, "webkitRelativePath", {
    //        //                    configurable : true,
    //        //                    enumerable : true,
    //        //                    get : () => nestedPath,
    //        //                });
    //        //            }));
    //        //    }
    //        //    else if (entry.kind === "directory")
    //        //    {
    //        //        dirs.push(getFiles(entry, nestedPath));
    //        //    }
    //        //}
    //        //return [
    //        //    ...(await Promise.all(dirs)).flat(),
    //        //    ...(await Promise.all(files)),
    //        //];
    //    };
//
    //    var sMode = Emval.toValue(mode);
    //    try
    //    {
    //        const handle = await showDirectoryPicker({ sMode, });
    //        //directoryStructure = getFiles(handle, undefined);
    //        directoryName = handle.name;
    //    }
    //    catch (err)
    //    {
    //        if (err.name !== "AbortError")
    //        {
    //            console.error(err.name, err.message);
    //        }
    //    }
    //    return Emval.toHandle(new Promise((resolve) => {resolve(directoryName.toString())}));
    //    //return Emval.toHandle(directoryName.toString());
    //}
    //// Fallback if the File System Access API is not supported.
    return Emval.toHandle(new Promise((resolve) => {
        const input = document.createElement('input');
        input.type = 'file';
        input.webkitdirectory = true;

        input.addEventListener(
            'change', () => {
                let files = Array.from(input.files);
                let promisedFiles = [];
                //const getFiles = async(fileList) => {
                    if(!FS.analyzePath("/" + files[0].webkitRelativePath.split("/")[0]).exists)
                    {
                        FS.mkdir("/" + files[0].webkitRelativePath.split("/")[0]);
                    }
                    for(const file of files)
                    {
                        promisedFiles.push(new Promise((resolve) => {
                            console.log('Loading file ' + file.webkitRelativePath);
                            let reader = new FileReader();
                            reader.onload = (event) => {
                                const uint8_view = new Uint8Array(event.target.result);
                                FS.writeFile('/' + file.webkitRelativePath, uint8_view);
                                resolve();
                            };
                            reader.readAsArrayBuffer(file);
                        }));
                        //const read = (data) => new Promise((resolve, reject) => {
                        //    reader.onload = (event) => resolve(event.target.result);
                        //    reader.onerror = reject;
                        //    reader.readAsArrayBuffer(data);
                        //});
			            //const result = await read(file);
    			        //const uint8_view = new Uint8Array(result);
	    		        //FS.writeFile(file.webkitRelativePath, uint8_view);
                    }
                    input.remove();
                //};
                //getFiles(files);
                Promise.all(promisedFiles).then(() => {
                    resolve(files[0].webkitRelativePath.split("/")[0]);
                });
            });
        if ('showPicker' in HTMLInputElement.prototype)
        {
            input.showPicker();
        }
        else
        {
            input.click();
        }
    }));
});

std::string FileHandler::OpenFolder(const char* aMode)
{
    std::string output = VAR_FROM_JS(open_directory(VAR_TO_JS(aMode))).await().as<std::string>();
    return output + "/";
}