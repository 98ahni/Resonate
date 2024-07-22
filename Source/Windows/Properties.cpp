//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Properties.h"
#include "MainWindow.h"
#include <Serialization/KaraokeData.h>
#include <Extensions/imguiExt.h>
#include <Serialization/Preferences.h>
#include <StringTools.h>
#include <misc/cpp/imgui_stdlib.h>
#include <Extensions/TouchInput.h>

PropertiesWindow::PropertiesWindow()
{
    if(Serialization::Preferences::HasKey("StyleProperties/Keys"))
    {
        std::vector<std::string> keys = StringTools::Split(Serialization::Preferences::GetString("StyleProperties/Keys"), ",");
        for(std::string key : keys)
        {
            myLocalEffectAliases[key] = Serialization::KaraokeDocument::ParseEffectProperty(Serialization::Preferences::GetString("StyleProperties/" + key));
        }
    }
    myCurrentTab = DocumentTab;
    myEditingEffect = "";
}

void PropertiesWindow::OnImGuiDraw()
{
    Gui_Begin();
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
        }
        ImVec4 endCol = ImGui::ColorConvertU32ToFloat4(IM_COL32_FROM_DOC(doc.myBaseEndColor));
        if(ImGui::ColorEdit4("##EndCol", &endCol.x))
        {
            doc.myBaseEndColor = ImGui::ColorConvertFloat4ToU32(endCol);
            doc.myBaseEndColor = IM_COL32_FROM_DOC(doc.myBaseEndColor);
            doc.myHasBaseEndColor = true;
        }
        ImGui::SeparatorText("Text");
        ImGui::Text("Font Size"); ImGui::SameLine(); ImGui::DragInt("##FontSize", (int*)&doc.myFontSize);
        ImGui::TextDisabled("ECHO will show %i lines.", doc.myFontSize <= 43 ? 7 : doc.myFontSize <= 50 ? 6 : 5);
        ImGui::Ext::ToggleSwitch("Use Direct Text", nullptr);
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
    ImGui::SameLine();
    if(ImGui::Button("Image"))
    {}
    ImGui::SameLine();
    if(ImGui::Button("Raw"))
    {}
    ImGui::EndChild();
    Gui_End();
}

void PropertiesWindow::DrawEffectWidget(std::string anEffectAlias, Serialization::KaraokeEffect* anEffect)
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
    ImGui::SetCursorPos(ImVec2(size.x - 30, cursorPos.y));
    ImGui::PushFont(MainWindow::Font);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(-10, -10));
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(.5f, .35f));
    if(ImGui::Button(((myCurrentTab == LocalTab ? "+##" : "x##") + anEffectAlias).data(), ImVec2(20, 20)))
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
    if(myCurrentTab == LocalTab)
    {
        std::string keys = myEditingEffect;
        if(Serialization::Preferences::HasKey("StyleProperties/Keys"))
        {
            keys = Serialization::Preferences::GetString("StyleProperties/Keys") + "," + keys;
        }
        Serialization::Preferences::SetString("StyleProperties/Keys", keys);
        Serialization::Preferences::SetString("StyleProperties/" + myEditingEffect, Serialization::KaraokeDocument::SerializeEffectProperty(anEffect));
    }
}
