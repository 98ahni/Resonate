//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024-2025 98ahni> Original file author

#include "Properties.h"
#include "MainWindow.h"
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>
#include <Serialization/Preferences.h>
#include <StringTools.h>
#include <misc/cpp/imgui_stdlib.h>
#include <Extensions/TouchInput.h>
#include <Defines.h>
#include <GamepadActions.h>

PropertiesWindow::EffectRecord::EffectRecord(History::Record::Type aType, std::string anEffectName, bool anIsLocal)
{
    myType = aType;
    myIsLocal = anIsLocal;
    myEffectName = anEffectName;
    switch (aType)
    {
    case History::Record::Edit:
    case History::Record::Remove:
        mySerializedEffect = Serialization::KaraokeDocument::SerializeEffectProperty((anIsLocal ? myLocalEffectAliases : Serialization::KaraokeDocument::Get().myEffectAliases)[anEffectName]);
        break;
    case History::Record::Insert:
        mySerializedEffect = "";
        break;
    }
}

void PropertiesWindow::EffectRecord::Undo()
{
    Serialization::KaraokeAliasMap& aliasMap = myIsLocal ? myLocalEffectAliases : Serialization::KaraokeDocument::Get().myEffectAliases;
    switch(myType)
    {
        case History::Record::Remove:
        if(!Serialization::Preferences::HasKey("StyleProperties/" + myEffectName))
        {
            Serialization::Preferences::SetString("StyleProperties/Keys", Serialization::Preferences::GetString("StyleProperties/Keys") + "," + myEffectName);
        }
        case History::Record::Edit:
            if(myIsLocal)
            {
                Serialization::Preferences::SetString("StyleProperties/" + myEffectName, mySerializedEffect);
            }
            delete aliasMap[myEffectName];
            aliasMap[myEffectName] = Serialization::KaraokeDocument::ParseEffectProperty(mySerializedEffect);
            break;
        case History::Record::Insert:
            if(myIsLocal)
            {
                if(Serialization::Preferences::HasKey("StyleProperties/Keys"))
                {
                    std::vector<std::string> keys = StringTools::Split(Serialization::Preferences::GetString("StyleProperties/Keys"), ",");
                    std::string uniqueKeys = "";
                    for(std::string key : keys)
                    {
                        if(myLocalEffectAliases.contains(key) || key == myEffectName) continue;
                        uniqueKeys += uniqueKeys == "" ? key : ("," + key);
                    }
                    Serialization::Preferences::SetString("StyleProperties/Keys", uniqueKeys);
                }
            }
            mySerializedEffect = Serialization::KaraokeDocument::SerializeEffectProperty(aliasMap[myEffectName]);
            delete aliasMap[myEffectName];
            aliasMap.erase(myEffectName);
            break;
    }
}

void PropertiesWindow::EffectRecord::Redo()
{
    if(myType == History::Record::Insert)
        myType = History::Record::Remove;
    else if(myType == History::Record::Remove)
        myType = History::Record::Insert;
    Undo();
    if(myType == History::Record::Insert)
        myType = History::Record::Remove;
    else if(myType == History::Record::Remove)
        myType = History::Record::Insert;
}

PropertiesWindow::PropertiesWindow()
{
    if(Serialization::Preferences::HasKey("StyleProperties/Keys") && Serialization::Preferences::GetString("StyleProperties/Keys") != "")
    {
        std::vector<std::string> keys = StringTools::Split(Serialization::Preferences::GetString("StyleProperties/Keys"), ",");
        std::string uniqueKeys = "";
        for(auto& [key, effect] : myLocalEffectAliases)
        {
            delete effect;
        }
        myLocalEffectAliases.clear();
        for(std::string key : keys)
        {
            if(myLocalEffectAliases.contains(key)) continue;
            uniqueKeys += uniqueKeys == "" ? key : ("," + key);
            myLocalEffectAliases[key] = Serialization::KaraokeDocument::ParseEffectProperty(Serialization::Preferences::GetString("StyleProperties/" + key));
        }
        Serialization::Preferences::SetString("StyleProperties/Keys", uniqueKeys);
    }
    myCurrentTab = DocumentTab;
    myEditingEffect = "";
    myShiftTimingsPopupOpen = false;
}

void PropertiesWindow::OnImGuiDraw()
{
    ImGui::SetNextWindowSize({DPI_SCALED(450), DPI_SCALED(350)}, ImGuiCond_FirstUseEver);
    Gui_Begin();
    ShiftTimingsPopupDraw();
    // Two tabs; Document and Local
    // "Document" contains the Echo headers and the singers used in the document.
    // "Local" contains a list of singers saved to the /.Resonate file.
    if(ImGui::Button("Document"))
    {
        myCurrentTab = DocumentTab;
    }
    ImGui::SameLine();
    if(ImGui::Button("Local"))
    {
        myCurrentTab = LocalTab;
    }
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    if(myCurrentTab == DocumentTab)
    {
        // Font size
        // Start/end color
        // Use direct
        ImGui::SeparatorText("Color");
        ImVec4 startCol = ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(doc.myBaseStartColor));
        if(ImGui::ColorEdit4("##StartCol", &startCol.x))
        {
            doc.myBaseStartColor = ImGui::ColorConvertFloat4ToU32(startCol);
            doc.myBaseStartColor = IM_COL32_FROM_DOC(doc.myBaseStartColor);
            doc.myHasBaseStartColor = true;
            doc.MakeDirty();
        }
        ImVec4 endCol = ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(doc.myBaseEndColor));
        if(ImGui::ColorEdit4("##EndCol", &endCol.x))
        {
            doc.myBaseEndColor = ImGui::ColorConvertFloat4ToU32(endCol);
            doc.myBaseEndColor = IM_COL32_FROM_DOC(doc.myBaseEndColor);
            doc.myHasBaseEndColor = true;
            doc.MakeDirty();
        }
        ImGui::SeparatorText("Text");
        ImGui::Text("Font Size"); ImGui::SameLine(); if(ImGui::DragInt("##FontSize", (int*)&doc.myFontSize)){doc.MakeDirty();}
        ImGui::TextDisabled("ECHO will show %i lines.", doc.myFontSize <= 43 ? 7 : doc.myFontSize <= 50 ? 6 : 5);
        if(ImGui::Ext::ToggleSwitch("Use Direct Text", &(doc.myUseDirectText))){doc.MakeDirty();}
        if(ImGui::Button("Shift Timings"))
        {
            ImGui::OpenPopup("Shift Timings");
            myShiftTimingsValue = 0;
            myShiftTimingsPopupOpen = true;
        }
    }
    auto& aliases = myCurrentTab == LocalTab ? myLocalEffectAliases : doc.myEffectAliases;
    for(auto&[alias, effect] : aliases)
    {
        DrawEffectWidget(alias, effect);
    }
    ImGui::BeginChild("New Effect", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
    ImGui::Text("Alias  <"); ImGui::SameLine();
    ImGui::InputText("##Alias", &myNewEffectName);
    TouchInput_ReadyKeyboard();
    ImGui::SameLine(); ImGui::Text(">");
    ImGui::Text("Create"); ImGui::SameLine();
    if(ImGui::Button("Color"))
    {
        // TODO: Check if the alias exist and remove it.
        Serialization::KaraokeColorEffect* colorEffect = new Serialization::KaraokeColorEffect();
        colorEffect->myType = Serialization::KaraokeEffect::Color;
        colorEffect->myStartColor = 0x0038F97C;
        colorEffect->myHasEndColor = false;
        colorEffect->myEndColor = 0x30FFCCE9;
        myEditingEffect = myNewEffectName.data();
        aliases[myEditingEffect] = colorEffect;
        History::AddRecord(new EffectRecord(History::Record::Insert, myEditingEffect, myCurrentTab == LocalTab), true);
        ApplyEdit(colorEffect);
        myNewEffectName = "";
    }
    ImGui::BeginDisabled();
    ImGui::SameLine();
    if(ImGui::Button("Image"))
    {}
    ImGui::SameLine();
    if(ImGui::Button("Raw"))
    {}
    ImGui::EndDisabled();
    ImGui::EndChild();
    Gui_End();
}

bool PropertiesWindow::DrawFontSizeGamepadPopup()
{
    if(ImGui::BeginPopupModal("Font Size##Gamepad"))
    {
        ImGui::SetWindowSize({DPI_SCALED(400), DPI_SCALED(300)}, ImGuiCond_Once);
        DrawHudSprite(HUDSprite::ArrowLeftBtn, {ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing()});
        ImGui::SameLine();
        PropertiesWindow::DrawFontSizeGamepadPopup();
        int fontSize = Serialization::KaraokeDocument::Get().GetFontSize();
        if(ImGui::InputInt("##FontSize", &fontSize))
        {
            History::AddRecord(new Serialization::KaraokeDocument::EchoRecord());
            Serialization::KaraokeDocument::Get().myFontSize = fontSize;
        }
        if(Gamepad_RepeatDelayed(Gamepad::D_Left, .1f, 1.5f))
        {
            History::AddRecord(new Serialization::KaraokeDocument::EchoRecord());
            Serialization::KaraokeDocument::Get().myFontSize = fontSize - (Gamepad::GetTimeSinceToggled(Gamepad::D_Right) < 3 ? 1 : 5);
        }
        if(Gamepad_RepeatDelayed(Gamepad::D_Right, .1f, 1.5f))
        {
            History::AddRecord(new Serialization::KaraokeDocument::EchoRecord());
            Serialization::KaraokeDocument::Get().myFontSize = fontSize + (Gamepad::GetTimeSinceToggled(Gamepad::D_Right) < 3 ? 1 : 5);
        }
        ImGui::SameLine();
        DrawHudSprite(HUDSprite::ArrowRightBtn, {ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing()});
        if(Gamepad::GetButtonDown(Gamepad::X) || Gamepad::GetButtonDown(Gamepad::Cross) || Gamepad::GetButtonDown(Gamepad::Circle))
        {
            History::ForceEndRecord();
            ImGui::CloseCurrentPopup();
        }

        Gamepad::Mapping conMap = Gamepad::GetMapping(Gamepad::GetControllerWithLastEvent());
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - (ImGui::GetTextLineHeightWithSpacing() + DPI_SCALED(5)));
        if(conMap <= Gamepad::PSClassic && conMap != Gamepad::Xinput)
        {
            ImGui::Text("(X) / (O) Close");
        }
        else
        {
            ImGui::Text("(A) / (B) Close");
        }
        ImGui::EndPopup();
        return true;
    }
    return false;
}

bool PropertiesWindow::DrawShiftTimingsGamepadPopup()
{
    if(ImGui::BeginPopupModal("Shift Timings##Gamepad"))
    {
        ImGui::SetWindowSize({DPI_SCALED(400), DPI_SCALED(300)}, ImGuiCond_Once);
        DrawHudSprite(HUDSprite::ArrowLeftBtn, {ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing()});
        ImGui::SameLine();
        ImGui::Dummy({0, DPI_SCALED(5)});
        ImGui::Text("If the syllables light up too early, enter a positive offset.");
        ImGui::Text("If the syllables light up after the audio, enter a negative offset.");
        ImGui::Dummy({0, DPI_SCALED(10)});
        bool valChanged = ImGui::InputInt("##ShiftTime", &myShiftTimingsValue);
        if(Gamepad_RepeatDelayed(Gamepad::D_Left, .1f, 1.5f))
        {
            myShiftTimingsValue -= (Gamepad::GetTimeSinceToggled(Gamepad::D_Right) < 3 ? 1 : 5);
            valChanged = true;
        }
        if(Gamepad_RepeatDelayed(Gamepad::D_Right, .1f, 1.5f))
        {
            myShiftTimingsValue += (Gamepad::GetTimeSinceToggled(Gamepad::D_Right) < 3 ? 1 : 5);
            valChanged = true;
        }
        if(valChanged)
        {
            Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
            Serialization::KaraokeToken& token = doc.GetToken(0, 0);
            if(doc.IsNull(token) || !token.myHasStart)
            {
                token = doc.GetTimedTokenAfter(0, 0);
            }
            if(((int)token.myStartTime) < -myShiftTimingsValue)
            {
                myShiftTimingsValue = -(int)token.myStartTime;
            }
        }
        ImGui::SameLine();
        DrawHudSprite(HUDSprite::ArrowRightBtn, {ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing()});
        if(Gamepad::GetButtonDown(Gamepad::A))
        {
            Serialization::KaraokeDocument::Get().ShiftTimings(myShiftTimingsValue);
            History::ForceEndRecord();
            Serialization::KaraokeDocument::Get().MakeDirty();
            myShiftTimingsValue = 0;
            ImGui::CloseCurrentPopup();
        }
        if(Gamepad::GetButtonDown(Gamepad::X) || Gamepad::GetButtonDown(Gamepad::Circle))
        {
            ImGui::CloseCurrentPopup();
        }

        Gamepad::Mapping conMap = Gamepad::GetMapping(Gamepad::GetControllerWithLastEvent());
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - (ImGui::GetTextLineHeightWithSpacing() + DPI_SCALED(5)));
        if(conMap <= Gamepad::PSClassic && conMap != Gamepad::Xinput)
        {
            ImGui::Text("(X) Apply   (O) Close");
        }
        else
        {
            ImGui::Text("(A) Apply   (B) Close");
        }
        ImGui::EndPopup();
        return true;
    }
    return false;
}

bool PropertiesWindow::DrawDefaultColorsGamepadPopup(int aSelectedSlider)
{
    if(ImGui::BeginPopupModal("Default Colors##Gamepad"))
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        ImGui::SetWindowSize({DPI_SCALED(400), DPI_SCALED(300)}, ImGuiCond_Once);
        uint startCol = doc.myBaseStartColor;
        uint endCol = doc.myBaseEndColor;
        if(DrawColorGamepadMenu(aSelectedSlider, startCol, endCol))
        {
            History::AddRecord(new Serialization::KaraokeDocument::EchoRecord());
            doc.myBaseStartColor = startCol;
            doc.myBaseEndColor = endCol;
            doc.MakeDirty();
        }
        if(Gamepad::GetButtonDown(Gamepad::X) || Gamepad::GetButtonDown(Gamepad::A) || Gamepad::GetButtonDown(Gamepad::Circle))
        {
            History::ForceEndRecord();
            ImGui::CloseCurrentPopup();
        }

        Gamepad::Mapping conMap = Gamepad::GetMapping(Gamepad::GetControllerWithLastEvent());
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - (ImGui::GetTextLineHeightWithSpacing() + DPI_SCALED(5)));
        if(conMap <= Gamepad::PSClassic && conMap != Gamepad::Xinput)
        {
            ImGui::Text("(X) / (O) Close");
        }
        else
        {
            ImGui::Text("(A) / (B) Close");
        }
        ImGui::EndPopup();
        return true;
    }
    return false;
}

bool PropertiesWindow::DrawSingerColorsGamepadPopup(int aSelectedSlider, bool anIsLocal, std::string anEditingName)
{
    if(ImGui::BeginPopupModal("Edit Singer##Gamepad"))
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        ImGui::SetWindowSize({DPI_SCALED(400), DPI_SCALED(300)}, ImGuiCond_Once);
        if(anEditingName == "")
        {
            if(!myNewEffectToAdd)
            {
                myNewEffectToAdd = new Serialization::KaraokeColorEffect();
                myNewEffectToAdd->myType = Serialization::KaraokeEffect::Color;
                myNewEffectToAdd->myStartColor = 0x0038F97C;
                myNewEffectToAdd->myHasEndColor = false;
                myNewEffectToAdd->myEndColor = 0x30FFCCE9;
            }
            ImGui::InputText("Name: ", &myNewEffectName);
        }
        else
        {
            if(!myNewEffectToAdd)
            {
                myNewEffectToAdd = (Serialization::KaraokeColorEffect*)(anIsLocal ? myLocalEffectAliases : doc.myEffectAliases)[anEditingName];
                myNewEffectToAdd = (Serialization::KaraokeColorEffect*)doc.ParseEffectProperty(doc.SerializeEffectProperty(myNewEffectToAdd));
            }
            ImGui::SeparatorText(anEditingName.data());
        }
        DrawColorGamepadMenu(aSelectedSlider, myNewEffectToAdd->myStartColor, myNewEffectToAdd->myEndColor);
        if((Gamepad::GetButtonDown(Gamepad::X) || Gamepad::GetButtonDown(Gamepad::A)) && (anEditingName != "" || myNewEffectName != ""))
        {
            myCurrentTab = (TabIndex)anIsLocal;
            if(anEditingName == "")
            {
                History::AddRecord(new EffectRecord(History::Record::Insert, myNewEffectName, anIsLocal));
            }
            else 
            {
                History::AddRecord(new EffectRecord(History::Record::Edit, anEditingName, anIsLocal));
            }
            myNewEffectName = anEditingName == "" ? myNewEffectName : anEditingName;
            ApplyEdit(myNewEffectToAdd);
            Serialization::KaraokeAliasMap& aliases = anIsLocal ? myLocalEffectAliases : doc.myEffectAliases;
            if(aliases.contains(myNewEffectName))
            {
                delete aliases[myNewEffectName];
            }
            aliases[myNewEffectName] = myNewEffectToAdd;
            if(Gamepad::GetButtonDown(Gamepad::X))
            {
                myCurrentTab = (TabIndex)!anIsLocal;
                Serialization::KaraokeAliasMap& antiAliases = !anIsLocal ? myLocalEffectAliases : doc.myEffectAliases;
                if(antiAliases.contains(myNewEffectName))
                {
                    History::AddRecord(new EffectRecord(History::Record::Edit, anEditingName, !anIsLocal));
                }
                else 
                {
                    History::AddRecord(new EffectRecord(History::Record::Insert, myNewEffectName, !anIsLocal));
                }
                ApplyEdit(myNewEffectToAdd);
                if(antiAliases.contains(myNewEffectName))
                {
                    delete antiAliases[myNewEffectName];
                }
                antiAliases[myNewEffectName] = doc.ParseEffectProperty(doc.SerializeEffectProperty(myNewEffectToAdd));
            }
            myNewEffectToAdd = nullptr;
            myNewEffectName = "";
            ImGui::CloseCurrentPopup();
        }
        if(Gamepad::GetButtonDown(Gamepad::Circle))
        {
            if(Gamepad::GetButton(Gamepad::Triangle))
            {
                if(anEditingName == "")
                {
                    delete myNewEffectToAdd;
                }
                else
                {
                    History::AddRecord(new EffectRecord(History::Record::Remove, myNewEffectName, anIsLocal));
                    delete (anIsLocal ? myLocalEffectAliases : doc.myEffectAliases)[myNewEffectName];
                    (anIsLocal ? myLocalEffectAliases : doc.myEffectAliases).erase(myNewEffectName);
                }
            }
            myNewEffectToAdd = nullptr;
            myNewEffectName = "";
            ImGui::CloseCurrentPopup();
        }

        Gamepad::Mapping conMap = Gamepad::GetMapping(Gamepad::GetControllerWithLastEvent());
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - (ImGui::GetTextLineHeightWithSpacing() + DPI_SCALED(5)));
        if(conMap <= Gamepad::PSClassic && conMap != Gamepad::Xinput)
        {
            if(anIsLocal) ImGui::Text("(X) Apply   (O) Cancel   ([]) Add to project   (/\\) + (O) Delete");
            else          ImGui::Text("(X) Apply   (O) Cancel   ([]) Store local   (/\\) + (O) Delete");
        }
        else
        {
            if(anIsLocal) ImGui::Text("(A) Apply   (B) Cancel   (X) Add to project   (Y) + (B) Delete");
            else          ImGui::Text("(A) Apply   (B) Cancel   (X) Store local   (Y) + (B) Delete");
        }
        ImGui::EndPopup();
        return true;
    }
    return false;
}

const Serialization::KaraokeAliasMap &PropertiesWindow::GetDocumentEffectAliases()
{
    return Serialization::KaraokeDocument::Get().GetEffectAliases();
}

const Serialization::KaraokeAliasMap &PropertiesWindow::GetLocalEffectAliases()
{
    return myLocalEffectAliases;
}

bool PropertiesWindow::DrawColorGamepadMenu(int aSelectedSlider, uint& aStartColor, uint& anEndColor)
{
    Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
    int startR = IM_COL32_GET_R(aStartColor);
    int startG = IM_COL32_GET_G(aStartColor);
    int startB = IM_COL32_GET_B(aStartColor);
    int startA = 255 - IM_COL32_GET_A(aStartColor);
    bool changed = false;
    ImGui::Dummy({5, ImGui::GetTextLineHeightWithSpacing()});
    if(DrawGamepadColorComponent("##startR", aSelectedSlider == 0, startR))
    {
        IM_COL32_GET_R(aStartColor) = startR;
        changed = true;
    }
    ImGui::SameLine();
    if(DrawGamepadColorComponent("##startG", aSelectedSlider == 1, startG))
    {
        IM_COL32_GET_G(aStartColor) = startG;
        changed = true;
    }
    ImGui::SameLine();
    if(DrawGamepadColorComponent("##startB", aSelectedSlider == 2, startB))
    {
        IM_COL32_GET_B(aStartColor) = startB;
        changed = true;
    }
    ImGui::SameLine();
    if(DrawGamepadColorComponent("##startA", aSelectedSlider == 3, startA))
    {
        IM_COL32_GET_A(aStartColor) = ~(ImU8)startA;
        changed = true;
    }
    ImGui::SameLine();
    float halfX = ImGui::GetCursorPosX();
    ImGui::Spacing();
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    int endR = IM_COL32_GET_R(anEndColor);
    int endG = IM_COL32_GET_G(anEndColor);
    int endB = IM_COL32_GET_B(anEndColor);
    int endA = 255 - IM_COL32_GET_A(anEndColor);
    if(DrawGamepadColorComponent("##endR", aSelectedSlider == 4, endR))
    {
        IM_COL32_GET_R(anEndColor) = endR;
        changed = true;
    }
    ImGui::SameLine();
    if(DrawGamepadColorComponent("##endG", aSelectedSlider == 5, endG))
    {
        IM_COL32_GET_G(anEndColor) = endG;
        changed = true;
    }
    ImGui::SameLine();
    if(DrawGamepadColorComponent("##endB", aSelectedSlider == 6, endB))
    {
        IM_COL32_GET_B(anEndColor) = endB;
        changed = true;
    }
    ImGui::SameLine();
    if(DrawGamepadColorComponent("##endA", aSelectedSlider == 7, endA))
    {
        IM_COL32_GET_A(anEndColor) = ~(ImU8)endA;
        changed = true;
    }
    ImGui::Dummy({5, ImGui::GetTextLineHeightWithSpacing()});
    ImGui::ColorButton("##startCol", ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(aStartColor)), ImGuiColorEditFlags_AlphaBar, {halfX, ImGui::GetTextLineHeightWithSpacing()});
    ImGui::SameLine();
    ImGui::ColorButton("##endCol", ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(anEndColor)), ImGuiColorEditFlags_AlphaBar, {halfX, ImGui::GetTextLineHeightWithSpacing()});
    return changed;
}

bool PropertiesWindow::DrawGamepadColorComponent(const char* aLabel, bool aIsSelected, int &aColorComponent)
{
    bool changed = false;
    if(aIsSelected)
    {
        if(Gamepad::GetButtonRepeating(Gamepad::D_Up) || Gamepad::GetButtonRepeating(Gamepad::D_Down))
        {
            int val = aColorComponent + (Gamepad::GetButton(Gamepad::D_Up) ? (Gamepad::GetTimeSinceToggled(Gamepad::D_Up) < 1 ? 1 : 5) : (Gamepad::GetTimeSinceToggled(Gamepad::D_Down) < 1 ? -1 : -5));
            aColorComponent = clamp(val, 0, 255);
            changed = true;
        }
        ImVec2 topLeft = ImGui::GetCursorPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        topLeft.x += ImGui::GetWindowPos().x;
        topLeft.y += ImGui::GetWindowPos().y;
        drawList->AddRect(
            {topLeft.x - DPI_SCALED(5), topLeft.y - DPI_SCALED(5)},
            {topLeft.x + ImGui::GetTextLineHeightWithSpacing() + DPI_SCALED(5), topLeft.y + DPI_SCALED(155)},
            ImGui::GetColorU32(ImGuiCol_NavHighlight), ImGui::GetStyle().FrameRounding + DPI_SCALED(2), 0, ImGui::GetStyle().WindowBorderSize);
        AddHudSpriteTo(drawList, HUDSprite::ArrowUpBtn, {(topLeft.x - DPI_SCALED(10)) + (ImGui::GetTextLineHeightWithSpacing() * .5f), topLeft.y - DPI_SCALED(20)}, {DPI_SCALED(20), DPI_SCALED(20)});
        AddHudSpriteTo(drawList, HUDSprite::ArrowDownBtn, {(topLeft.x - DPI_SCALED(10)) + (ImGui::GetTextLineHeightWithSpacing() * .5f), topLeft.y + DPI_SCALED(150)}, {DPI_SCALED(20), DPI_SCALED(20)});
        AddHudSpriteTo(drawList, HUDSprite::ArrowLeftBtn, {topLeft.x - DPI_SCALED(20), topLeft.y + DPI_SCALED(65)}, {DPI_SCALED(20), DPI_SCALED(20)});
        AddHudSpriteTo(drawList, HUDSprite::ArrowRightBtn, {topLeft.x + DPI_SCALED(0) + ImGui::GetTextLineHeightWithSpacing(), topLeft.y + DPI_SCALED(65)}, {DPI_SCALED(20), DPI_SCALED(20)});
    }
    if(ImGui::VSliderInt(aLabel, {ImGui::GetTextLineHeightWithSpacing(), DPI_SCALED(150)}, &aColorComponent, 0, 255, "", ImGuiSliderFlags_AlwaysClamp))
    {
        changed = true;
    }
    ImGui::SameLine();
    ImGui::Spacing();
    return changed;
}

void PropertiesWindow::ShiftTimingsPopupDraw()
{
    if(ImGui::BeginPopupModal("Shift Timings", &myShiftTimingsPopupOpen))
    {
        //ImGui::SetWindowSize({DPI_SCALED(400), DPI_SCALED(300)}, ImGuiCond_Once);
        if(ImGui::Ext::StepInt("Offset (cs)", myShiftTimingsValue, 1, 10))
        {
            Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
            Serialization::KaraokeToken token = doc.GetToken(0, 0);
            if(doc.IsNull(token) || !token.myHasStart)
            {
                token = doc.GetTimedTokenAfter(0, 0);
            }
            if(((int)token.myStartTime) < -myShiftTimingsValue)
            {
                myShiftTimingsValue = -(int)token.myStartTime;
            }
        }
        ImGui::Dummy({0, DPI_SCALED(5)});
        ImGui::Text("If the syllables light up too early, enter a positive offset.");
        ImGui::Text("If the syllables light up after the audio, enter a negative offset.");
        ImGui::Dummy({0, DPI_SCALED(10)});
        if(ImGui::Button("Shift"))
        {
            Serialization::KaraokeDocument::Get().ShiftTimings(myShiftTimingsValue);
            History::ForceEndRecord();
            Serialization::KaraokeDocument::Get().MakeDirty();
            myShiftTimingsValue = 0;
            myShiftTimingsPopupOpen = false;
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel"))
        {
            myShiftTimingsValue = 0;
            myShiftTimingsPopupOpen = false;
        }
        ImGui::EndPopup();
    }
}

void PropertiesWindow::DrawEffectWidget(std::string anEffectAlias, Serialization::KaraokeEffect *anEffect)
{
    // Name, Value, [Preview], EditBtn, [SaveBtn], DeleteBtn
    bool editingThis = myEditingEffect == anEffectAlias;
    ImGui::BeginChild(("##" + anEffectAlias).data(), ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
    ImVec2 size = ImGui::GetWindowSize();
    ImGui::Text("<%s>", anEffectAlias.data());
    ImGui::SameLine();
    ImVec2 cursorPos = ImGui::GetCursorPos();
    ImGui::TextDisabled("%s", anEffect->myECHOValue.data());
    switch (anEffect->myType)
    {
    case Serialization::KaraokeEffect::Color:
    {
        ImGui::SameLine();
        Serialization::KaraokeColorEffect* colorEffect = (Serialization::KaraokeColorEffect*)anEffect;
        ImGui::ColorButton("##startColPrev", ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(colorEffect->myStartColor)), 0, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
        if(colorEffect->myHasEndColor)
        {
            ImGui::SameLine();
            ImGui::ColorButton("##endColPrev", ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(colorEffect->myEndColor)), 0, ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
        }
        if(editingThis)
        {
            ImVec4 startCol = ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(colorEffect->myStartColor));
            ImGui::Text("Start Color"); ImGui::SameLine();
            if(ImGui::ColorEdit4("##Start Color", &startCol.x))
            {
                History::AddRecord(new EffectRecord(History::Record::Edit, anEffectAlias, myCurrentTab == LocalTab));
                colorEffect->myStartColor = ImGui::ColorConvertFloat4ToU32(startCol);
                colorEffect->myStartColor = IM_COL32_FROM_DOC(colorEffect->myStartColor);
                ApplyEdit(anEffect);
            }
            TouchInput_ReadyKeyboard();
            ImVec4 endCol = ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(colorEffect->myEndColor));
            ImGui::Text("End Color"); ImGui::SameLine();
            if(ImGui::ColorEdit4("##End Color", &endCol.x))
            {
                History::AddRecord(new EffectRecord(History::Record::Edit, anEffectAlias, myCurrentTab == LocalTab));
                colorEffect->myEndColor = ImGui::ColorConvertFloat4ToU32(endCol);
                colorEffect->myEndColor = IM_COL32_FROM_DOC(colorEffect->myEndColor);
                ApplyEdit(anEffect);
            }
            TouchInput_ReadyKeyboard();
            ImGui::SameLine();
            if(ImGui::Ext::ToggleSwitch("##UseEndCol", &(colorEffect->myHasEndColor)))
            {
                History::AddRecord(new EffectRecord(History::Record::Edit, anEffectAlias, myCurrentTab == LocalTab));
                ApplyEdit(anEffect);
            }
        }
        break;
    }
    case Serialization::KaraokeEffect::Image:
    case Serialization::KaraokeEffect::Raw:
        break;
    }
    ImGui::SetCursorPos(ImVec2(size.x - DPI_SCALED(30), cursorPos.y));
    ImGui::PushFont(MainWindow::Font);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(DPI_SCALED(-10), DPI_SCALED(-10)));
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(.5f, .35f));
    if(ImGui::Button(((myCurrentTab == LocalTab ? "+##" : "x##") + anEffectAlias).data(), ImVec2(DPI_SCALED(20), DPI_SCALED(20))))
    {
        Serialization::KaraokeDocument& doc = Serialization::KaraokeDocument::Get();
        if(myCurrentTab == LocalTab)
        {
            History::AddRecord(new EffectRecord(doc.myEffectAliases.contains(anEffectAlias) ? History::Record::Edit : History::Record::Insert, anEffectAlias, myCurrentTab == LocalTab));
            doc.myEffectAliases[anEffectAlias] = doc.ParseEffectProperty(doc.SerializeEffectProperty(anEffect));
        }
        else
        {
            History::AddRecord(new EffectRecord(History::Record::Remove, anEffectAlias, myCurrentTab == LocalTab));
            delete doc.myEffectAliases[anEffectAlias];
            doc.myEffectAliases.erase(anEffectAlias);
        }
    }
    ImGui::PopStyleVar(2);
    ImGui::PopFont();
    ImGui::EndChild();
    if(ImGui::IsItemClicked())
    {
        myEditingEffect = editingThis ? "" : anEffectAlias;
    }
}

void PropertiesWindow::ApplyEdit(Serialization::KaraokeEffect *anEffect)
{
    switch (anEffect->myType)
    {
    case Serialization::KaraokeEffect::Color:
    {
        Serialization::KaraokeColorEffect* colorEffect = (Serialization::KaraokeColorEffect*)anEffect;
        colorEffect->myECHOValue = "<font color#" + Serialization::KaraokeDocument::ToHex(colorEffect->myStartColor);
        if(colorEffect->myHasEndColor)
        {
            colorEffect->myECHOValue += "#" + Serialization::KaraokeDocument::ToHex(colorEffect->myEndColor);
        }
        colorEffect->myECHOValue += ">";
        break;
    }
    case Serialization::KaraokeEffect::Image:
    case Serialization::KaraokeEffect::Raw:
    }
    if(myCurrentTab == DocumentTab)
    {
        Serialization::KaraokeDocument::Get().MakeDirty();
    }
    else if(myCurrentTab == LocalTab)
    {
        std::string keys = myEditingEffect;
        if(!Serialization::Preferences::HasKey("StyleProperties/Keys"))
        {
            Serialization::Preferences::SetString("StyleProperties/Keys", keys);
        }
        else if(!Serialization::Preferences::HasKey("StyleProperties/" + myEditingEffect))
        {
            keys = Serialization::Preferences::GetString("StyleProperties/Keys") + "," + keys;
            Serialization::Preferences::SetString("StyleProperties/Keys", keys);
        }
        Serialization::Preferences::SetString("StyleProperties/" + myEditingEffect, Serialization::KaraokeDocument::SerializeEffectProperty(anEffect));
    }
}
