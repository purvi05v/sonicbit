#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h> 

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "RingBuffer.h"
#include "SonicBit.h"
#include <vector>
#include <stdio.h>

const int SAMPLE_RATE = 44100;
const int BLOCK_SIZE = 4096;
LockFreeRingBuffer audioBuffer(SAMPLE_RATE * 1); 

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    audioBuffer.push((const int16_t*)pInput, frameCount);
}

int main(int, char**)
{
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            
   
    GLFWwindow* window = glfwCreateWindow(1280, 720, "SonicBit Dashboard 2026", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark(); 
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.format = ma_format_s16;
    config.capture.channels = 1; 
    config.sampleRate = SAMPLE_RATE;
    config.dataCallback = data_callback;
    ma_device device;
    ma_device_init(NULL, &config, &device);
    ma_device_start(&device);

    SonicBit codec;
    std::vector<int16_t> rawBlock;
    rawBlock.reserve(BLOCK_SIZE);
    
    std::vector<float> waveform(BLOCK_SIZE, 0.0f);
    float currentRatio = 1.0f;
    int currentSavedPercent = 0;
    int bytesCompressed = 0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (audioBuffer.available() >= BLOCK_SIZE) {
            audioBuffer.pop(rawBlock, BLOCK_SIZE);
            
            const std::vector<uint8_t>& compressed = codec.compress(rawBlock);
            bytesCompressed = compressed.size();
            currentRatio = codec.getLastRatio(BLOCK_SIZE * 2);
            currentSavedPercent = (int)(100.0f * (1.0f - (1.0f/currentRatio)));
            for(int i=0; i<BLOCK_SIZE; i++) {
                waveform[i] = (float)rawBlock[i];
            }
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Dashboard", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        ImGui::Text("SonicBit Real-Time Engine");
        ImGui::Separator();

        // SECTION A: METRICS
        ImGui::Columns(3, "metrics");
        ImGui::Text("Compression Ratio");
        ImGui::SetWindowFontScale(2.0f);
        ImGui::Text("%.1f : 1", currentRatio);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::NextColumn();

        ImGui::Text("Bandwidth Saved");
        ImGui::SetWindowFontScale(2.0f);
        if(currentSavedPercent > 50) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
        ImGui::Text("%d%%", currentSavedPercent);
        if(currentSavedPercent > 50) ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        ImGui::NextColumn();

        ImGui::Text("Packet Size");
        ImGui::SetWindowFontScale(2.0f);
        ImGui::Text("%d B", bytesCompressed);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::Columns(1);
        
        ImGui::Separator();
        ImGui::Text("Live Microphone Input");
        ImGui::PlotLines("##Wave", waveform.data(), BLOCK_SIZE, 0, NULL, -32768.0f, 32768.0f, ImVec2(0, 150));

        ImGui::Text("Compression Efficiency (Target: >50%)");
        ImGui::ProgressBar((float)currentSavedPercent / 100.0f, ImVec2(-1, 0.0f));

        ImGui::End();
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
    ma_device_uninit(&device);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}