//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include <string>
#include <vector>
#include <unordered_map>

class Gamepad
{
public:
    enum Button
    {
        std0, std1, std2, std3, std4, std5, std6, std7, std8, std9, std10, std11, std12, std13, std14, std15, std16,
        xA=std0, xB, xX, xY, xLB, xRB, xLT, xRT, xBack, xStart, xLSB, xRSB, xD_Up, xD_Down, xD_Left, xD_Right, xGuide,
        psCross=std0, psCircle, psSquare, psTriangle, psL1, psR1, psL2, psR2, psShare, psOption, psL3, psR3, psD_Up, psD_Down, psD_Left, psD_Right, psPS, PS=psPS,
        nsB=std0, nsA, nsY, nsX, nsL, nsR, nsZL, nsZR, 
    };
    void TestGamepad();

private:
    struct ButtonState
    {
        bool myIsDown;
        bool myIsInitial;
    };
    struct AxisState
    {
        float myValue;
        bool myIsInitial;
    };
    struct State
    {
        int myID;
        std::string myName;
        std::vector<ButtonState> myButtons;
        std::vector<AxisState> myAxies;
    };
    std::unordered_map<int, State> myGamepads;
};