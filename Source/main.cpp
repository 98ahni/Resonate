#include <stdio.h>
#include <emscripten.h>

void loop(void* window){

}

int main(){
    emscripten_set_main_loop_arg(loop, (void*)0, 0, false);
    return 0;
}