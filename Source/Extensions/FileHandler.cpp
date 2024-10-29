//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

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

EM_ASYNC_JS(emscripten::EM_VAL, open_document, (emscripten::EM_VAL save_folder, emscripten::EM_VAL mime_type, emscripten::EM_VAL mode), {
    return Emval.toHandle(new Promise((resolve) => {
        const input = document.createElement('input');
        input.type = 'file';
        input.accept = Emval.toValue(mime_type);

        input.addEventListener(
            'cancel', () => {
                resolve("");
        });

        input.addEventListener(
            'change', () => {
                let files = Array.from(input.files);
                let promisedFiles = [];
                let exDir = Emval.toValue(save_folder);
                if(!FS.analyzePath(exDir).exists)
                {
                    FS.mkdir(exDir);
                }
                new Promise((resolveLoad) => {
                    console.log('Loading file ' + files[0].webkitRelativePath + '/' + files[0].name);
                    let reader = new FileReader();
                    reader.onload = (event) => {
                        const uint8_view = new Uint8Array(event.target.result);
                        FS.writeFile(exDir.length != 0 ? exDir + '/' + files[0].name : files[0].webkitRelativePath, uint8_view);
                        resolveLoad();
                    };
                    reader.readAsArrayBuffer(files[0]);
                }).then(() => {
                    resolve(exDir + '/' + files[0].name);
                });
                input.remove();
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

EM_JS(void, download_document, (emscripten::EM_VAL path, emscripten::EM_VAL mime_type), {
	const docPath = Emval.toValue(path);
	const mime = Emval.toValue(mime_type);
    const docData = FS.readFile(docPath);
	const docBlob = new Blob([docData.buffer], {type: 'application/octet-binary'});
	const docURL = URL.createObjectURL(docBlob);

	const link = document.createElement('a');
	link.href = docURL;
    link.type = mime;
	link.download = docPath.split('/').pop();
	document.body.appendChild(link);
    link.click();
	document.body.removeChild(link);
});

EM_JS(void, download_project_zip, (emscripten::EM_VAL zip_name, emscripten::EM_VAL path_list), {
	const zipName = Emval.toValue(zip_name);
	const pathList = Emval.toValue(path_list);
    var fileBlobs = [];
    for(let i = 0; i < pathList.length; i++){
        console.log(pathList[i]);
    }
    for(let i = 0; i < pathList.length; i++){
        const docData = FS.readFile(pathList[i]);
	    fileBlobs.push(new Blob([docData.buffer], {type: 'application/octet-binary'}));
    }
    const zip = new Zip(zipName);
    zip.files2zip(fileBlobs);
    zip.makeZip();
});

EM_JS(emscripten::EM_VAL, wait_for_sync_fs, (),
{
    return Emval.toHandle(new Promise((resolve)=>{
        FS.syncfs(false, function (err) {
            if(err){
                console.error('Unable to sync IndexDB!\n' + err);
            }
            resolve();
        }); 
    }));
});

EM_JS(void, set_local_value, (emscripten::EM_VAL key, emscripten::EM_VAL value_to_store),
{
    localStorage.setItem(Emval.toValue(key), Emval.toValue(value_to_store));
});

EM_JS(emscripten::EM_VAL, get_local_value, (emscripten::EM_VAL key),
{
    const keyname = Emval.toValue(key);
    if(!localStorage.getItem(keyname)){return Emval.toHandle("");}
    return Emval.toHandle(localStorage.getItem(keyname));
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
    return output + "/";
}

std::string FileHandler::OpenDocument(const char *aSaveFolder, const char *aFileType, const char *aMode)
{
    std::string output = VAR_FROM_JS(open_document(VAR_TO_JS(aSaveFolder), VAR_TO_JS(aFileType), VAR_TO_JS(aMode))).await().as<std::string>();
    return output;
}

void FileHandler::DownloadDocument(const char *aPath, const char *aFileType)
{
    download_document(VAR_TO_JS(aPath), VAR_TO_JS(aFileType));
}

void FileHandler::DownloadZip(std::vector<std::string> aPathList, const char *aZipName)
{
    std::vector<emscripten::val> output;
    for(int i = 0; i < aPathList.size(); i++)
    {
        output.push_back(emscripten::val(aPathList[i]));
    }
    download_project_zip(VAR_TO_JS(aZipName), VEC_TO_JS(output));
}

void FileHandler::SyncLocalFS()
{
    VAR_FROM_JS(wait_for_sync_fs()).await();
    //VAR_FROM_JS((emscripten::EM_VAL)EM_ASM_INT({
    //    return Emval.toHandle(new Promise((resolve)=>{
    //        FS.syncfs(false, function (err) {
    //            if(err){
    //                alert('Unable to sync IndexDB!\n' + err);
    //            }
    //            resolve();
    //        }); 
    //    }));
    //})).await();
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
