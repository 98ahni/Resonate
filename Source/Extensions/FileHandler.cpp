#include "FileHandler.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/promise.h>

EM_ASYNC_JS(em_promise_t, open_directory, (emscripten::val mode), {
	let reader = new FileReader();
    // Feature detection. The API needs to be supported
    // and the app not run in an iframe.
    const supportsFileSystemAccess =
        "showDirectoryPicker" in window &&
        (() => {
            try
            {
                return window.self === window.top;
            }
            catch
            {
                return false;
            }
        })();
    // If the File System Access API is supported…
    if (supportsFileSystemAccess)
    {
        let directoryStructure = undefined;
        let directoryName = undefined;

        const getFiles = async(dirHandle, path = dirHandle.name) =>
        {
            for await(const entry of dirHandle.values())
            {
                const nestedPath = '${path}/${entry.name}';
                if (entry.kind === "file")
                {
                    const read = (data) => new Promise((resolve, reject) => {
                        reader.onload = (event) => resolve(event.target.result);
                        reader.onerror = reject;
                        reader.readAsArrayBuffer(data);
                    });
			        let result = await read(entry);
			        const uint8_view = new Uint8Array(result);
			        FS.writeFile(nestedPath, uint8_view);
                    
                    files.push(
                        entry.getFile().then((file) => {
                            file.directoryHandle = dirHandle;
                            file.handle = entry;
                            return Object.defineProperty(file, "webkitRelativePath", {
                                configurable : true,
                                enumerable : true,
                                get : () => nestedPath,
                            });
                        }));
                }
                else if (entry.kind === "directory")
                {
                    dirs.push(getFiles(entry, nestedPath));
                }
            }
            return [
                ...(await Promise.all(dirs)).flat(),
                ...(await Promise.all(files)),
            ];
        };

        try
        {
            const handle = await showDirectoryPicker({ mode, });
            directoryStructure = getFiles(handle, undefined);
            directoryName = handle.name;
        }
        catch (err)
        {
            if (err.name !== "AbortError")
            {
                console.error(err.name, err.message);
            }
        }
        return directoryName;
    }
    // Fallback if the File System Access API is not supported.
    return new Promise((resolve) => {
        const input = document.createElement('input');
        input.type = 'file';
        input.webkitdirectory = true;

        input.addEventListener(
            'change', () => {
                let files = Array.from(input.files);
                for(const file of files)
                {
                    const read = (data) => new Promise((resolve, reject) => {
                        reader.onload = (event) => resolve(event.target.result);
                        reader.onerror = reject;
                        reader.readAsArrayBuffer(data);
                    });
			        read(file).then((result) =>{
    			        const uint8_view = new Uint8Array(result);
	    		        FS.writeFile(nestedPath, uint8_view);
                    });
                }
                resolve(files[0]);
            });
        if ('showPicker' in HTMLInputElement.prototype)
        {
            input.showPicker();
        }
        else
        {
            input.click();
        }
    });
});

std::string FileHandler::OpenFolder(const char* aMode)
{
    std::string output = ((emscripten::val*)(emscripten_promise_await(open_directory(emscripten::val(aMode))).value))->as<std::string>();
    return output;
}