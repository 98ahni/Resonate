//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

#include "Base/EditorWindow.h"

#ifndef RELEASE_VERSION
#define RELEASE_VERSION 10'10'10    // split into major'minor'patch
#endif

class NewsWindow : public EditorWindow
{
public:
    NewsWindow();
    void OnImGuiDraw();

private:
};