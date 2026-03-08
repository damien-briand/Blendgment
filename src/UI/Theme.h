#pragma once

#include <imgui.h>

// ─────────────────────────────────────────────────────────────────────────────
// Palette de couleurs partagée (thème sombre inspiré de Blender)
// ─────────────────────────────────────────────────────────────────────────────
namespace Col {
    static const ImVec4 BgDark      { 0.11f, 0.11f, 0.14f, 1.f };
    static const ImVec4 BgPanel     { 0.15f, 0.15f, 0.19f, 1.f };
    static const ImVec4 BgCard      { 0.18f, 0.18f, 0.23f, 1.f };
    static const ImVec4 BgSidebar   { 0.13f, 0.13f, 0.17f, 1.f };
    static const ImVec4 BgTopbar    { 0.10f, 0.10f, 0.13f, 1.f };
    static const ImVec4 Accent      { 0.87f, 0.45f, 0.10f, 1.f };  // orange Blender
    static const ImVec4 AccentHover { 0.96f, 0.56f, 0.16f, 1.f };
    static const ImVec4 AccentPress { 0.68f, 0.33f, 0.06f, 1.f };
    static const ImVec4 AccentDim   { 0.87f, 0.45f, 0.10f, 0.25f };
    static const ImVec4 Blue        { 0.28f, 0.65f, 0.90f, 1.f };
    static const ImVec4 Green       { 0.35f, 0.80f, 0.42f, 1.f };
    static const ImVec4 Text        { 0.92f, 0.92f, 0.92f, 1.f };
    static const ImVec4 TextDim     { 0.58f, 0.58f, 0.64f, 1.f };
    static const ImVec4 TextHint    { 0.42f, 0.42f, 0.48f, 1.f };
    static const ImVec4 Separator   { 0.28f, 0.28f, 0.35f, 1.f };
}
