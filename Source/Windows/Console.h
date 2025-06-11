//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#pragma once

#include "Base/EditorWindow.h"
#include <string>
#include <vector>
#include <unordered_map>

class Console
{
    enum Severity
    {
        Message, Warning, Error
    };
    struct LogContent
    {
        Severity mySeverity;
        std::string myMessage;
        int myLine = -1;
    };

public:
    static void SearchForErrors();
    static void ValidateProject();
    static void Log(std::string aMessage, int aLine = -1);
    static void LogWarning(std::string aMessage, int aLine = -1);
    static void LogError(std::string aMessage, int aLine = -1);
    static void LineMargin(int aLine);
    static std::string MenuIcon(bool aUsePadding = false);

private:
    static inline std::vector<LogContent> ourLogs;
    inline static std::unordered_map<int, std::vector<int>> ourLineToLogInds;
    static inline Severity ourHighestSeverity = (Severity)-1;
    friend class ConsoleWindow;

    static void DrawIcon(Severity aType, float aSize);
    static bool DrawCompactIcon(Severity aType, float aSize);
};

class ConsoleWindow : public EditorWindow
{
public:
    ConsoleWindow();
    void OnImGuiDraw();

private:
    int myExpandedLog = -1;
    friend class Console;
};