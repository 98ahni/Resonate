//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "GoogleDrive.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <Defines.h>
#include "APIKeys.h"

/// AppID: 824603127976
/// client_id: 824603127976-vjf2sbqo99s9kulm1jp847c453ctmv65.apps.googleusercontent.com
/// client_secret: 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

EM_JS(bool, gapi_loaded, (), {
    if(global_gis_inited){
        return true;
    }
    if(!Object.hasOwn(window, 'gapi')){
        //alert('GAPI is slow to load.');
        console.log('GAPI is slow to load.');
        return false;
    }
    gapi.load('client', ()=>{
        gapi.load('client:picker', _GAPI_Init_Client);
    });
    return true;
});

EM_JS(bool, gapi_ready, (), {
    return global_gapi_inited && global_gis_inited;
}
var global_gapi_inited = false;
var global_gis_inited = false;
);

EM_JS(bool, gis_loaded, (), {
    if(global_gis_inited){
        return true;
    }
    if(!Object.hasOwn(window, 'google')){
        //alert('GSI is slow to load.');
        console.log('GSI is slow to load.');
        return false;
    }
    global_client_token = google.accounts.oauth2.initTokenClient({
        client_id: '824603127976-vjf2sbqo99s9kulm1jp847c453ctmv65.apps.googleusercontent.com',
        scope: 'https://www.googleapis.com/auth/drive https://www.googleapis.com/auth/userinfo.profile',
        prompt: '',
        callback: '', // defined later
    });
    //google.accounts.id.initialize({
    //  client_id: 'YOUR_GOOGLE_CLIENT_ID',
    //  auto_select: true,
    //  callback: (creds) => {}
    //});
    //google.accounts.id.prompt();
    global_gis_inited = true;
    return true;
}
var global_client_token;
);

bool GAPIHasLoaded = gapi_loaded();
bool GiSHasLoaded = gis_loaded();

extern"C" EMSCRIPTEN_KEEPALIVE void GAPI_Init_Client()
{
    EM_ASM({init_gapi_with_key($0);}, VAR_TO_JS(APIKeys::Google()));
}
EM_JS(void, init_gapi_with_key, (emscripten::EM_VAL APIKey), {
    gapi.client.init({
        apiKey: Emval.toValue(APIKey),
        discoveryDocs: ['https://www.googleapis.com/discovery/v1/apis/drive/v3/rest', 'https://people.googleapis.com/$discovery/rest?version=v1']
    }).then(()=>{global_gapi_inited = true;});
});

EM_JS(bool, has_gapi_token, (), {
    //if(gapi.client.getToken() !== null){
    //    console.log('Already logged in');
    //}else{
    //    console.log('Not logged in');
    //}
    return gapi.client.getToken() !== null;
    //return gapi.auth2.getAuthInstance().isSignedIn.get();
});

EM_JS(void, request_client_token, (emscripten::EM_VAL prompt, emscripten::EM_VAL token_callback), {
    // Prompt the user to select a Google Account and ask for consent to share their data
    // when establishing a new session.
    var client_token_callback = Module[Emval.toValue(token_callback)];
    global_client_token.callback = (token_data) => {
        gapi.client.people.people.get({
            "resourceName": "people/me",
            "requestMask.includeField": "person.names,person.photos",
            "sources": [
                "READ_SOURCE_TYPE_PROFILE"
            ]
        }).then((user_data)=>{
            client_token_callback(
                Emval.toHandle(Date.now() + (token_data.expires_in * 1000)),
                Emval.toHandle(user_data.result.names.length != 0 ? user_data.result.names[0].displayName : '(Anon)'),
                Emval.toHandle(user_data.result.photos.length != 0 ? user_data.result.photos[0].url : '')
            );
        });
    };
    global_client_token.requestAccessToken({prompt: Emval.toValue(prompt)});
    //global_client_token.requestAccessToken();
});

EM_JS(void, revoke_client_token, (), {
    const token = gapi.client.getToken();
    if (token !== null) {
        google.accounts.oauth2.revoke(token.access_token);
        gapi.client.setToken('');
    }
});

EM_JS(void, create_picker, (emscripten::EM_VAL APIKey, emscripten::EM_VAL mime_types, emscripten::EM_VAL file_callback_name, emscripten::EM_VAL done_callback_name, emscripten::EM_VAL cancel_callback_name), 
{
    //const view = new google.picker.View(google.picker.ViewId.FOLDERS);
    //view.setMimeTypes(Emval.toValue(mime_types));
    const view = new google.picker.DocsView()
          .setIncludeFolders(true)
          .setMimeTypes(Emval.toValue(mime_types))
          .setSelectFolderEnabled(true);
    const callback_func = Module[Emval.toValue(file_callback_name)];
    const done_callback_func = Module[Emval.toValue(done_callback_name)];
    const cancel_callback_func = Module[Emval.toValue(cancel_callback_name)];
    const picker = new google.picker.PickerBuilder()
        //.enableFeature(google.picker.Feature.NAV_HIDDEN)
        //.enableFeature(google.picker.Feature.MULTISELECT_ENABLED)
        .setDeveloperKey(Emval.toValue(APIKey))
        .setAppId(824603127976)
        .setOAuthToken(gapi.client.getToken().access_token)
        .setTitle('Choose a folder')
        .addView(view)
        .addView(new google.picker.DocsUploadView())
        .setCallback(async(data) => {if (data.action === google.picker.Action.CANCEL){cancel_callback_func();} if (data.action === google.picker.Action.PICKED){
            const documents = data[google.picker.Response.DOCUMENTS];
            if(!FS.analyzePath("/GoogleDrive").exists){
                FS.mkdir("/GoogleDrive");
            }
            let loadPromises = [];
            for(const document of documents){
                const fileId = document[google.picker.Document.ID];
                console.log(fileId);
                const files = [];
                const res = await gapi.client.drive.files.list({
                    q: "'" + fileId + "' in parents",
                    fields: 'nextPageToken, files(id, name, trashed)',
                    spaces: 'drive'
                });
                console.log(JSON.stringify(res.result.files));
                Array.prototype.push.apply(files, res.result.files);
                console.log(files);
                files.forEach(function(file) {
                    loadPromises.push(new Promise(async (resolve)=>{
                        if(file.trashed){
                            console.log('Found trashed file:', file.name, file.id, ', Skipping');
                            resolve();
                            return;
                        }
                        console.log('Found file:', file.name, file.id);
                        const fres = await gapi.client.drive.files.get({
                            'fileId': file.id,
                            'alt': 'media'
                        });
                        var bytes = [];
                        for (var i = 0; i < fres.body.length; ++i) {
                          bytes.push(fres.body.charCodeAt(i));
                        }
                        FS.writeFile("/GoogleDrive/" + file.name, new Uint8Array(bytes));
                        callback_func(Emval.toHandle("/GoogleDrive/" + file.name), Emval.toHandle(file.id)); // User callback
                        resolve();
                    }));
                });
            }
            Promise.all(loadPromises).then(()=>{console.log('Done loading from Google Drive!');done_callback_func();});
        }})
        .build();
    picker.setVisible(true);
});

EM_ASYNC_JS(void, save_to_drive, (emscripten::EM_VAL file_id, emscripten::EM_VAL fs_path), {
    const fileData = FS.readFile(Emval.toValue(fs_path), {encoding: 'utf8'});
    await gapi.client.request({
        path: 'https://www.googleapis.com/upload/drive/v3/files/' + Emval.toValue(file_id),
        method: 'PATCH',
        body: fileData,
        params: {
            uploadType: 'media',
            fields: 'id,version,name',
        },
    });
});

bool GoogleDrive::Ready()
{
    GAPIHasLoaded = gapi_loaded();
    GiSHasLoaded = gis_loaded();
    return gapi_ready();
}

bool GoogleDrive::HasToken()
{
    return has_gapi_token();
}

void GoogleDrive::RequestToken(bool aShowPopup, std::string aTokenCallback)
{
    request_client_token(VAR_TO_JS(aShowPopup ? "consent" : "none"), VAR_TO_JS(aTokenCallback));
}

void GoogleDrive::LogOut()
{
    revoke_client_token();
}

void GoogleDrive::LoadProject(std::string someMimeTypes, std::string aFileCallbackName, std::string aDoneCallbackName, std::string aCancelCallbackName)
{
    create_picker(VAR_TO_JS(APIKeys::Google()), VAR_TO_JS(someMimeTypes), VAR_TO_JS(aFileCallbackName), VAR_TO_JS(aDoneCallbackName), VAR_TO_JS(aCancelCallbackName));
}

void GoogleDrive::SaveProject(std::string aFileID, std::string aFilePath)
{
    save_to_drive(VAR_TO_JS(aFileID), VAR_TO_JS(aFilePath));
}
