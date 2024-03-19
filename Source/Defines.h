
// Math
#define remap(value, istart, istop, ostart, ostop) (ostart + (ostop - ostart) * ((value - istart) / (istop - istart)))
#define clamp(value, min, max) (value < min ? min : (max < value ? max : value))

// Strings
#define ASSET_PATH "assets/"
#define SYLLABIFY_PATH(lang_code) "Syllabify/" lang_code ".txt"
#define SYLLABIFY_PATHSTD(lang_code) ("Syllabify/" + lang_code + ".txt")

// Emscripten
#define VAR_TO_JS(var) (emscripten::val(var).as_handle())
#define VAR_FROM_JS(var) (emscripten::val::take_ownership(var))