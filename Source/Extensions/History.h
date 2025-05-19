//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#pragma once
#include <string>

namespace History
{
    struct Record
    {
        enum Type
        {
            Edit,   // Any token were changed, added, removed or timed. 
            Insert, // A checkpoint before something is added. 
            Remove  // The record before something is removed. 
        };
        Type myType;
        virtual void Undo() = 0;
        virtual void Redo() = 0;
    };

    /// @brief This function should be called BEFORE any change is made. 
    /// @param aRecord A pointer to a new Record. It is deleted when it's no longer needed. 
    /// @param aIsLast If true, sets the flag to start a new command list. 
    void AddRecord(Record* aRecord, bool aIsLast = false);
    // Sets the internal flag to start a new command list. 
    void ForceEndRecord();
    // Clear all records. 
    void Clear();
    void Undo();
    void Redo();
}