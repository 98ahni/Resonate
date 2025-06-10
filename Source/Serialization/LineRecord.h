//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include <string>
#include <Extensions/History.h>

namespace Serialization
{
    struct LineRecord : public History::Record
    {
        std::string myRecordedLine;
        size_t myRecordedLineNumber;
        LineRecord(History::Record::Type aType, size_t aLineNumber);
        void Undo() override;
        void Redo() override;
    };
}