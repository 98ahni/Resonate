#include "FileHandler.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <Defines.h>

EM_ASYNC_JS(emscripten::EM_VAL, open_directory, (emscripten::EM_VAL mode), {
    return Emval.toHandle(new Promise((resolve) => {
        const input = document.createElement('input');
        input.type = 'file';
        if(typeof input.webkitdirectory !== "boolean")
        {
            input.multiple = true;
        }
        else
        {
            input.webkitdirectory = true;
        }

        input.addEventListener(
            'change', () => {
                let files = Array.from(input.files);
                let promisedFiles = [];
                if(!FS.analyzePath("/" + files[0].webkitRelativePath.split("/")[0]).exists) // Make this work when there is no containing folder.
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
                }
                input.remove();
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

EM_JS(void, download_document, (emscripten::EM_VAL path), {
	const docPath = Emval.toValue(path);
    const docData = FS.readFile(docPath);
	const docBlob = new Blob([docData.buffer], {type: 'application/octet-binary'});
	const docURL = URL.createObjectURL(docBlob);

	const link = document.createElement('a');
	link.href = docURL;
	link.download = docPath.split('/').pop();
	document.body.appendChild(link);
    link.click();
	document.body.removeChild(link);
});

std::string FileHandler::OpenFolder(const char* aMode)
{
    std::string output = VAR_FROM_JS(open_directory(VAR_TO_JS(aMode))).await().as<std::string>();
    return output + "/";
}

void FileHandler::DownloadDocument(const char *aPath)
{
    download_document(VAR_TO_JS(aPath));
}
