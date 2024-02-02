
// Math
#define remap(value, istart, istop, ostart, ostop) (ostart + (ostop - ostart) * ((value - istart) / (istop - istart)))
#define clamp(value, min, max) (value < min ? min : (max < value ? max : value))

// Strings
#define ASSET_PATH "assets/"
#define SYLLABIFY_PATH(lang_code) "assets/" lang_code ".txt"
#define SYLLABIFY_PATHSTD(lang_code) ("assets/" + lang_code + ".txt")