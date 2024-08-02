//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

// Math
#define remap(value, istart, istop, ostart, ostop) (ostart + (ostop - ostart) * ((value - istart) / (istop - istart)))
#define clamp(value, min, max) (value < min ? min : (max < value ? max : value))

// Strings
#define ASSET_PATH "assets/"
#define SYLLABIFY_PATH(lang_code) "Syllabify/" lang_code ".txt"
#define SYLLABIFY_PATHSTD(lang_code) ("Syllabify/" + lang_code + ".txt")

// Emscripten
#define VAR_TO_JS(var) (emscripten::val(var).as_handle())
#define VEC_TO_JS(vec) (emscripten::val::array(vec).as_handle())
#define VAR_FROM_JS(var) (emscripten::val::take_ownership(var))