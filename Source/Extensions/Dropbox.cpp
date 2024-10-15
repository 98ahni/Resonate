//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Dropbox.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <Defines.h>

// App key/Client id: pzgv8lp5thkigx4       // (Confusing name and for some reason not a secret)

EM_JS(bool, db_open_auth_popup, (), {
    getAuthenticationUrl(window.href, undefined, 'code', 'offline', undefined, undefined, true).then(authUrl => {
        const popup = window.open(authUrl, 'Log In with Dropbox', 'width=320,height=400');
        const checkWindow = setInterval(() => {
            if (popup.window.location.href.includes(window.href)){
                const authCode = new URLSearchParams(popup.window.location.search).get('code');
                popup.close();
                global_db_auth.getAccessTokenFromCode(window.href, authCode).then((res) => {
                    console.log(JSON.stringify(res.result));
                    global_db_auth.setAccessToken(res.result.access_token);
                    global_db_api = new Dropbox.Dropbox({auth: global_db_auth});
                });
            }
            if (!popup || !popup.closed) return;
            clearInterval(checkWindow);
        }, 100);
    });
}
var global_db_auth = new Dropbox.DropboxAuth({clientId: pzgv8lp5thkigx4});
var global_db_api = null;
);

EM_JS(void, db_open_chooser, (emscripten::EM_VAL file_callback_name, emscripten::EM_VAL done_callback_name, emscripten::EM_VAL cancel_callback_name, bool use_iframe), {
    const callback_func = Module[Emval.toValue(file_callback_name)];
    const done_callback_func = Module[Emval.toValue(done_callback_name)];
    const cancel_callback_func = Module[Emval.toValue(cancel_callback_name)];
    const options = {
        success: (files)=>{
            let loadPromises = [];
            files.forEach(function(file) {
                loadPromises.push(new Promise(async (resolve)=>{
                    console.log('Loading file "' + file.name + '" from Dropbox.');
                    fetch(file.link).then(res => res.blob()).then(blob => {
                        FS.writeFile("/Dropbox/" + file.name, blob);
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

bool Dropbox::HasToken()
{
    return false;
}

void Dropbox::RequestToken(bool aShowPopup, std::string aTokenCallback)
{
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
}
