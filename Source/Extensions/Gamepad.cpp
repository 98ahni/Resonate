//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Gamepad.h"
#include <emscripten.h>
#include <stdio.h>

EM_JS(void, print_gamepad_state, (), {
    const gp = navigator.getGamepads()[0];
    let output = '';
    gp.buttons.forEach(b => output += '[v:' + b.value + ',p:' + b.pressed + ']');
    console.log(output);
});

void Gamepad::TestGamepad()
{
    print_gamepad_state();
}