//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#pragma once
typedef unsigned int uint;
typedef void* ImTextureID;
struct ImExtTexture
{
    ImTextureID myID;
    ImTextureID myHandle;
};
namespace ImGui{
    namespace Ext{
        
        void LoadVideo(const char* anID, const char* anFSPath);
        void PlayVideo(const char* anID);
        void PauseVideo(const char* anID);
        void SetVideoProgress(const char* anID, uint aProgress);
        void SetVideoSpeed(const char* anID, int aSpeed);
        bool IsVideoPaused(const char* anID);
        void LoadImage(const char* anID, const char* anFSPath);
        void LoadImageFromURL(const char* anID, const char* aURL);
        bool RenderTexture(const char* anID, ImExtTexture& aTexture);
        bool DeleteTexture(const char* anID, ImExtTexture& aTexture);
    }
}