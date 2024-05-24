#include "GoogleDrive.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <Defines.h>
#include "APIKeys.h"

EM_JS(void, gapi_loaded, (), {
    gapi.load('client', _GAPI_Init_Client);
    gapi.load('client:picker', initializePicker);
});

EM_JS(bool, gapi_ready, (), {
    return global_gapi_inited && global_gis_inited;
}
var global_gapi_inited = false;
var global_gis_inited = false;
);

EM_JS(void, gis_loaded, (), {
    global_client_token = google.accounts.oauth2.initTokenClient({
        client_id: CLIENT_ID,
        scope: SCOPES,
        callback: '', // defined later
    });
    global_gis_inited = true;
}
var global_client_token;
);

extern"C" EMSCRIPTEN_KEEPALIVE void GAPI_Init_Client()
{
    EM_ASM({init_gapi_with_key(Emval.toValue($0));}, VAR_TO_JS(APIKeys::Google()));
}
EM_JS(void, init_gapi_with_key, (emscripten::EM_VAL APIKey), {
    gapi.client.init({
        apiKey: Emval.toValue(APIKey),
        discoveryDocs: ['https://www.googleapis.com/discovery/v1/apis/drive/v3/rest']
    }).then(()=>{global_gapi_inited = true;});
});

EM_JS(bool, has_gapi_token, (), {
    return gapi.client.getToken() !== null;
});

EM_JS(void, request_client_token, (), {
    if (gapi.client.getToken() === null) {
        // Prompt the user to select a Google Account and ask for consent to share their data
        // when establishing a new session.
        global_client_token.requestAccessToken({prompt: 'consent'});
    } else {
        // Skip display of account chooser and consent dialog for an existing session.
        global_client_token.requestAccessToken({prompt: ''});
    }
});

EM_JS(void, revoke_client_token, (), {
    const token = gapi.client.getToken();
    if (token !== null) {
        google.accounts.oauth2.revoke(token.access_token);
        gapi.client.setToken('');
    }
});

EM_JS(void, create_picker, (emscripten::EM_VAL APIKey), 
{
    
})

bool GoogleDrive::Ready()
{
    return gapi_ready();
}

bool GoogleDrive::HasToken()
{
    return has_gapi_token();
}

void GoogleDrive::RequestToken()
{
}

void GoogleDrive::LogOut()
{
}

std::string GoogleDrive::LoadProject()
{
    return std::string();
}

std::string GoogleDrive::SaveProject()
{
    return std::string();
}
