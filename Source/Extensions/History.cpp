//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "History.h"
#include <vector>
#include <Serialization/KaraokeData.h>
#include <imgui.h>

struct HistoryQueue
{
    static inline std::vector<std::vector<History::Record*>> ourQueue = {{}};
    inline static size_t ourNextUndo = 0;
    static inline bool ourShouldStartNew = false;
    inline static int ourLastRecordFrame = 0;
};

void History::AddRecord(Record *aRecord, bool aIsLast)
{
    if(HistoryQueue::ourNextUndo + 1 < HistoryQueue::ourQueue.size())
    {
        HistoryQueue::ourNextUndo++;
        for(int i = HistoryQueue::ourNextUndo; i < HistoryQueue::ourQueue.size(); i++)
        {
            for(int cmd = 0; cmd < HistoryQueue::ourQueue[i].size(); cmd++)
            {
                delete HistoryQueue::ourQueue[i][cmd];
            }
            HistoryQueue::ourQueue[i].clear();
        }
        HistoryQueue::ourQueue.erase(HistoryQueue::ourQueue.begin() + HistoryQueue::ourNextUndo, HistoryQueue::ourQueue.end());
    }
    if(HistoryQueue::ourShouldStartNew || HistoryQueue::ourQueue.size() <= HistoryQueue::ourNextUndo
        || 100 < ImGui::GetFrameCount() - HistoryQueue::ourLastRecordFrame)
    {
        HistoryQueue::ourQueue.push_back({});
        HistoryQueue::ourNextUndo = HistoryQueue::ourQueue.size() - 1;
    }
    HistoryQueue::ourLastRecordFrame = ImGui::GetFrameCount();
    HistoryQueue::ourQueue[HistoryQueue::ourNextUndo].push_back(aRecord);
    HistoryQueue::ourShouldStartNew = aIsLast;
}

void History::ForceEndRecord()
{
    HistoryQueue::ourShouldStartNew = true;
}

void History::Clear()
{
    for(int i = 0; i < HistoryQueue::ourQueue.size(); i++)
    {
        for(int cmd = 0; cmd < HistoryQueue::ourQueue[i].size(); cmd++)
        {
            delete HistoryQueue::ourQueue[i][cmd];
        }
        HistoryQueue::ourQueue[i].clear();
    }
    HistoryQueue::ourQueue.erase(HistoryQueue::ourQueue.begin() + HistoryQueue::ourNextUndo, HistoryQueue::ourQueue.end());
    HistoryQueue::ourNextUndo = 0;
}

void History::Undo()
{
    printf("Trying to undo %i\n", HistoryQueue::ourNextUndo);
    if(HistoryQueue::ourNextUndo == 0) { return; }
    // Reverse for loop since what happened first needs to be undone last. 
    for(int cmd = HistoryQueue::ourQueue[HistoryQueue::ourNextUndo].size() - 1; 0 <= cmd; cmd--)
    {
        HistoryQueue::ourQueue[HistoryQueue::ourNextUndo][cmd]->Undo();
    }
    HistoryQueue::ourNextUndo--;
}

void History::Redo()
{
    HistoryQueue::ourNextUndo++;
    printf("Trying to redo %i\n", HistoryQueue::ourNextUndo);
    if(HistoryQueue::ourNextUndo >= HistoryQueue::ourQueue.size())
    {
        HistoryQueue::ourNextUndo--;
        return;
    }
    for(int cmd = 0; cmd < HistoryQueue::ourQueue[HistoryQueue::ourNextUndo].size(); cmd++)
    {
        HistoryQueue::ourQueue[HistoryQueue::ourNextUndo][cmd]->Redo();
    }
}
