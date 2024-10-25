//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Dropbox.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <Defines.h>

// App key/Client id: pzgv8lp5thkigx4       // (Confusing name and for some reason not a secret)
// The minimized API: https://cdnjs.cloudflare.com/ajax/libs/dropbox.js/10.34.0/Dropbox-sdk.min.js      // (The regular API is an ES module and thus appearently incompatible)

EM_JS(bool, db_open_auth_popup, (emscripten::EM_VAL token_callback), {
    const callback_func = Module[Emval.toValue(token_callback)];
    global_db_auth.getAuthenticationUrl(window.location.href, undefined, 'code', 'offline', ['account_info.read', 'files.content.write', 'files.content.read'], undefined, true).then(authUrl => {
        const popup = window.open(authUrl, 'Log In with Dropbox', 'width=520,height=600');
        const message_func = function(msg){
            popup.close();
            global_db_auth.getAccessTokenFromCode(window.location.href, msg.data.code).then((res) => {
                global_db_auth.setAccessToken(res.result.access_token);
                global_db_auth.setRefreshToken(res.result.refresh_token);
                global_db_auth.setAccessTokenExpiresAt(res.result.expires_in);
                global_db_api = new Dropbox.Dropbox({auth: global_db_auth});
                global_db_api.usersGetCurrentAccount().then((user_res)=>{
                    callback_func(
                        Emval.toHandle(Date.now() + (global_db_auth.getAccessTokenExpiresAt() * 1000)),
                        Emval.toHandle(user_res.result.name.display_name),
                        Emval.toHandle(user_res.result.profile_photo_url || ''));
                });
            });
        };
        window.addEventListener('message', message_func);
        const checkWindow = setInterval(() => {
            if (!popup || !popup.closed) return;
            window.removeEventListener('message', message_func);
            clearInterval(checkWindow);
        }, 100);
    });
}
if(new URLSearchParams(window.location.search).has('code')){
    window.opener.postMessage({code: new URLSearchParams(window.location.search).get('code')});
}
var global_db_auth = new Dropbox.DropboxAuth({clientId: 'pzgv8lp5thkigx4'});
var global_db_api = null;
);

EM_JS(void, db_refresh_token, (emscripten::EM_VAL token_callback), {
    const callback_func = Module[Emval.toValue(token_callback)];
    global_db_auth.refreshAccessToken().then(() => {callback_func(Emval.toHandle(global_db_auth.getAccessTokenExpiresAt()));});
});

EM_JS(bool, has_db_token, (), {
    return global_db_auth.getAccessToken() !== undefined;
});

EM_JS(void, db_open_chooser, (emscripten::EM_VAL file_callback_name, emscripten::EM_VAL done_callback_name, emscripten::EM_VAL cancel_callback_name, bool use_iframe), {
    const callback_func = Module[Emval.toValue(file_callback_name)];
    const done_callback_func = Module[Emval.toValue(done_callback_name)];
    const cancel_callback_func = Module[Emval.toValue(cancel_callback_name)];
    const options = {
        success: (files)=>{
            if(!FS.analyzePath("/Dropbox").exists){
                FS.mkdir("/Dropbox");
            }
            let loadPromises = [];
            files.forEach(function(file) {
                loadPromises.push(new Promise(async (resolve)=>{
                    console.log('Loading file "' + file.name + '" from Dropbox.');
                    fetch(file.link).then(res => res.blob()).then(blob => blob.arrayBuffer()).then(buffer => {
                        FS.writeFile("/Dropbox/" + file.name, new Uint8Array(buffer));
                        callback_func(Emval.toHandle("/Dropbox/" + file.name), Emval.toHandle(file.id)); // User callback
                        resolve();
                    });
                }));
            });
            Promise.all(loadPromises).then(()=>{console.log('Done loading from Dropbox!');done_callback_func();});
        },
        cancel: cancel_callback_func,
        extensions: ['<no extension>'],
        linkType: 'direct',
        multiselect: true
    };
    if(use_iframe){
        //options.files_selected = () => {Dropbox.cleanupWidget();};
        //options.close_dialog = () => {Dropbox.cleanupWidget();};
        //var widget = Dropbox.createChooserWidget(options);
        //widget.referrerpolicy = 'same-origin';
        //widget.sandbox = 'allow-downloads allow-forms allow-modals allow-same-origin allow-scripts ';
        options.iframe = true;
        //widget.id = 'db_chooser';
        //widget.style.zIndex = 10;
        //widget.style.position = 'fixed';
        //widget.style.left = 'px';
        //widget.style.top = 'px';
        //widget.style.width = 'px';
        //widget.style.height = 'px';
        //document.body.appendChild(widget);
    }
    Dropbox.choose(options);
});

EM_ASYNC_JS(void, db_update_file, (emscripten::EM_VAL file_id, emscripten::EM_VAL fs_path), {
    const fileData = FS.readFile(Emval.toValue(fs_path), {encoding: 'utf8'});
    const response = await global_db_api.filesUpload({path: Emval.toValue(file_id), "mode": "overwrite", contents: fileData});
    console.log(JSON.stringify(response));
});

bool Dropbox::HasToken()
{
    return has_db_token();
}

void Dropbox::RequestToken(bool aShowPopup, std::string aTokenCallback)
{
    if(aShowPopup)
    {
        db_open_auth_popup(VAR_TO_JS(aTokenCallback));
    }
    else
    {
        db_refresh_token(VAR_TO_JS(aTokenCallback));
    }
}

void Dropbox::LogOut()
{
}

void Dropbox::LoadProject(std::string aFileCallbackName, std::string aDoneCallbackName, std::string aCancelCallbackName, bool aShouldUseIframe)
{
    db_open_chooser(VAR_TO_JS(aFileCallbackName), VAR_TO_JS(aDoneCallbackName), VAR_TO_JS(aCancelCallbackName), aShouldUseIframe);
}

void Dropbox::SaveProject(std::string aFileID, std::string aFilePath)
{
    db_update_file(VAR_TO_JS(aFileID), VAR_TO_JS(aFilePath));
}
