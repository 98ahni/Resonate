//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "LineRecord.h"
#include "KaraokeData.h"
#include <Windows/TimingEditor.h>

namespace Serialization
{
    LineRecord::LineRecord(History::Record::Type aType, size_t aLineNumber)
    {
        myType = aType;
        if(aType == History::Record::Type::Insert) myRecordedLine = "";
        else myRecordedLine = KaraokeDocument::Get().SerializeLine(KaraokeDocument::Get().GetLine(aLineNumber));
        myRecordedLineNumber = aLineNumber;
    }
    void LineRecord::Undo()
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        std::string currentLine = doc.SerializeLine(doc.GetLine(myRecordedLineNumber));
        switch (myType)
        {
        case Type::Edit:
            doc.ParseLineAndReplace(myRecordedLine, myRecordedLineNumber);
            myRecordedLine = currentLine;
            break;
        case Type::Insert:
            myRecordedLine = currentLine;
            doc.GetData().erase(doc.GetData().begin() + myRecordedLineNumber);
            break;
        case Type::Remove:
            doc.GetData().insert(doc.GetData().begin() + myRecordedLineNumber, KaraokeLine());
            doc.ParseLineAndReplace(myRecordedLine, myRecordedLineNumber);
            break;
        }
        TimingEditor::Get().CheckMarkerIsSafe(false);
    }
    void LineRecord::Redo()
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        std::string currentLine = doc.SerializeLine(doc.GetLine(myRecordedLineNumber));
        switch (myType)
        {
        case Type::Edit:
            doc.ParseLineAndReplace(myRecordedLine, myRecordedLineNumber);
            myRecordedLine = currentLine;
            break;
        case Type::Insert:
            doc.GetData().insert(doc.GetData().begin() + myRecordedLineNumber, KaraokeLine());
            doc.ParseLineAndReplace(myRecordedLine, myRecordedLineNumber);
            break;
        case Type::Remove:
            myRecordedLine = currentLine;
            doc.GetData().erase(doc.GetData().begin() + myRecordedLineNumber);
            break;
        }
        TimingEditor::Get().CheckMarkerIsSafe(false);
    }
}