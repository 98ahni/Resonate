//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Properties.h"
#include "MainWindow.h"
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>
#include <Serialization/Preferences.h>
#include <StringTools.h>
#include <misc/cpp/imgui_stdlib.h>
#include <Extensions/TouchInput.h>
#include <Defines.h>

PropertiesWindow::PropertiesWindow()
{
    if(Serialization::Preferences::HasKey("StyleProperties/Keys"))
    {
        std::vector<std::string> keys = StringTools::Split(Serialization::Preferences::GetString("StyleProperties/Keys"), ",");
        std::string uniqueKeys = "";
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

void PropertiesWindow::ShiftTimingsPopupDraw()
{
    if(ImGui::BeginPopupModal("Shift Timings", &myShiftTimingsPopupOpen))
    {
        //ImGui::SetWindowSize({DPI_SCALED(400), DPI_SCALED(300)}, ImGuiCond_Once);
        if(ImGui::Ext::StepInt("Offset (cs)", myShiftTimingsValue, 1, 10))
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
        ImGui::Dummy({0, DPI_SCALED(5)});
        ImGui::Text("If the syllables light up too early, enter a positive offset.");
        ImGui::Text("If the syllables light up after the audio, enter a negative offset.");
        ImGui::Dummy({0, DPI_SCALED(10)});
        if(ImGui::Button("Shift"))
        {
            Serialization::KaraokeDocument::Get().ShiftTimings(myShiftTimingsValue);
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
                colorEffect->myStartColor = ImGui::ColorConvertFloat4ToU32(startCol);
                colorEffect->myStartColor = IM_COL32_FROM_DOC(colorEffect->myStartColor);
                ApplyEdit(anEffect);
            }
            TouchInput_ReadyKeyboard();
            ImVec4 endCol = ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(colorEffect->myEndColor));
            ImGui::Text("End Color"); ImGui::SameLine();
            if(ImGui::ColorEdit4("##End Color", &endCol.x))
            {
                colorEffect->myEndColor = ImGui::ColorConvertFloat4ToU32(endCol);
                colorEffect->myEndColor = IM_COL32_FROM_DOC(colorEffect->myEndColor);
                ApplyEdit(anEffect);
            }
            TouchInput_ReadyKeyboard();
            ImGui::SameLine();
            if(ImGui::Ext::ToggleSwitch("##UseEndCol", &(colorEffect->myHasEndColor)))
            {
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
            doc.myEffectAliases[anEffectAlias] = doc.ParseEffectProperty(doc.SerializeEffectProperty(anEffect));
        }
        else
        {
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
