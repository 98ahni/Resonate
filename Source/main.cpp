#include <stdio.h>
#include <emscripten.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void loop(void* window){
    ImGui::Begin("Test");

}

int main(){
    GLFWwindow* window;
    glfwCreateWindow(1600, 900, "Resonate", 0, 0);
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    emscripten_set_main_loop_arg(loop, (void*)window, 0, false);
    return 0;
}