//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"

class LicenseWindow : public EditorWindow
{
public:
    LicenseWindow();
    void OnImGuiDraw();

private:
    bool myShowThirdParty = false;
};