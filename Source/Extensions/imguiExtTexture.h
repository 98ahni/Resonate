
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
        void LoadImage(const char* anID, const char* anFSPath);
        bool RenderTexture(const char* anID, ImExtTexture& aTexture);
    }
}