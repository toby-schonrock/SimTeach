#include "imgui.h"
#include <cstdint>

// custom drag interfaces
inline bool ImGui_DragDouble(const char* label, double* v, float v_speed = 1.0f, double v_min = 0,
                             double v_max = 0, const char* format = 0, ImGuiSliderFlags flags = 0) {
    return ImGui::DragScalar(label, ImGuiDataType_Double, v, v_speed, &v_min, &v_max, format,
                             flags);
}

inline bool ImGui_DragUnsigned(const char* label, std::uint32_t* v, float v_speed = 1.0f,
                               std::uint32_t v_min = 0, std::uint32_t v_max = 0,
                               const char* format = "%d", ImGuiSliderFlags flags = 0) {
    return ImGui::DragScalar(label, ImGuiDataType_U32, v, v_speed, &v_min, &v_max, format, flags);
}

// taken from dear imgui demo
// https://github.com/ocornut/imgui/blob/9aae45eb4a05a5a1f96be1ef37eb503a12ceb889/imgui_demo.cpp#L191
// makes hoverable help marker
inline void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}