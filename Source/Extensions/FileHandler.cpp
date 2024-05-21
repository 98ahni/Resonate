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
            'cancel', () => {
                resolve("");
        });

        input.addEventListener(
            'change', () => {
                let files = Array.from(input.files);
                let promisedFiles = [];
                let exDir = "";
                if(files[0].webkitRelativePath.toString().includes("/"))
                {
                    if(!FS.analyzePath("/" + files[0].webkitRelativePath.split("/")[0]).exists) // Make this work when there is no containing folder.
                    {
                        FS.mkdir("/" + files[0].webkitRelativePath.split("/")[0]);
                    }
                }
                else
                {
                    exDir = "/WorkDir";
                    if(!FS.analyzePath("/WorkDir").exists)
                    { // Make this work when there is no containing folder.
                        FS.mkdir("/WorkDir");
                    }
                }
                for(const file of files)
                {
                    promisedFiles.push(new Promise((resolve) => {
                        console.log('Loading file ' + file.webkitRelativePath);
                        let reader = new FileReader();
                        reader.onload = (event) => {
                            const uint8_view = new Uint8Array(event.target.result);
                            FS.writeFile(exDir.length != 0 ? exDir + '/' + file.name : file.webkitRelativePath, uint8_view);
                            resolve();
                        };
                        reader.readAsArrayBuffer(file);
                    }));
                }
                input.remove();
                Promise.all(promisedFiles).then(() => {
                    resolve(exDir.length != 0 ? exDir : files[0].webkitRelativePath.split("/")[0]);
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

EM_JS(void, set_local_value, (emscripten::EM_VAL key, emscripten::EM_VAL value_to_store),
{
    localStorage.setItem(Emval.toValue(key), Emval.toValue(value_to_store));
});

EM_JS(emscripten::EM_VAL, get_local_value, (emscripten::EM_VAL key),
{
    return Emval.toHandle(localStorage.getItem(Emval.toValue(key)));
});

EM_JS(void, remove_local_value, (emscripten::EM_VAL key),
{
    localStorage.removeItem(Emval.toValue(key));
});

EM_JS(void, clear_local_storage, (),
{
    localStorage.clear();
});

std::string FileHandler::OpenFolder(const char* aMode)
{
    std::string output = VAR_FROM_JS(open_directory(VAR_TO_JS(aMode))).await().as<std::string>();
    printf(("It gets this far!" + output + "\n").c_str());
    return output + "/";
}

void FileHandler::DownloadDocument(const char *aPath)
{
    download_document(VAR_TO_JS(aPath));
}

void FileHandler::SyncLocalFS()
{
    VAR_FROM_JS((emscripten::EM_VAL)EM_ASM_INT({
        return Emval.toHandle(new Promise((resolve)=>{
            FS.syncfs(false, function (err) {
                if(err){
                    alert('Unable to sync IndexDB!\n' + err);
                }
                resolve();
            }); 
        }))
    })).await();
}

void FileHandler::SetLocalValue(std::string aName, std::string aValue)
{
    set_local_value(VAR_TO_JS(aName), VAR_TO_JS(aValue));
}

std::string FileHandler::GetLocalValue(std::string aName)
{
    return VAR_FROM_JS(get_local_value(VAR_TO_JS(aName))).as<std::string>();
}

void FileHandler::RemoveLocalValue(std::string aName)
{
    remove_local_value(VAR_TO_JS(aName));
}

void FileHandler::ClearLocalStorage()
{
    clear_local_storage();
}
