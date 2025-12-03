#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#define _CRT_SECURE_NO_WARNINGS
#include "../drawing/imgui/imgui.h"
#include "../overlay/overlay.hpp"
#include "../drawing/drawing.hpp"
#include <algorithm>
#include <D3DX11tex.h>
#include "../drawing/imgui/imgui_internal.h"
#include <string>
#include "../../globals/globals.h"
#include "../drawing/lua/luau.hpp"
#include "../drawing/imgui/settings/functions.h"

static std::string convert_ip_to_string(uint64_t raw_ip)
{
    uint32_t ip = static_cast<uint32_t>(raw_ip);
    std::stringstream ss;
    ss << ((ip >> 24) & 0xFF) << "."
        << ((ip >> 16) & 0xFF) << "."
        << ((ip >> 8) & 0xFF) << "."
        << (ip & 0xFF);
    return ss.str();
}

std::string get_pc_name() {
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "pc";
}

void gui_t::menu() {

}

namespace keybind_list {
    ImVec2 pos = ImVec2(20, 200);
    bool dragging = false;
}

void gui_t::render_watermark()
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 screen = ImGui::GetIO().DisplaySize;

    const char* text = "arise.lol";
    ImVec2 text_size = var->font.tahoma->CalcTextSizeA(var->font.tahoma->FontSize + 2.f, FLT_MAX, 0.f, text);

    const float padding = 16.f;
    const float rounding = 6.f;
    const float height = 38.f;

    ImVec2 pos = ImVec2(screen.x - text_size.x - padding * 2 - 20, 20);
    ImRect bb(pos, pos + ImVec2(text_size.x + padding * 2, height));

    dl->AddRectFilled(bb.Min + ImVec2(0, 2), bb.Max + ImVec2(0, 2),
        IM_COL32(0, 0, 0, 90), rounding);

    dl->AddRectFilled(bb.Min, bb.Max, IM_COL32(22, 22, 28, 250), rounding);

    ImU32 accent = draw->get_clr(clr->accent);
    dl->AddRectFilled(bb.Min, bb.Min + ImVec2(text_size.x + padding * 2, 3), accent, rounding, ImDrawFlags_RoundCornersTop);

    dl->AddRect(bb.Min, bb.Max, IM_COL32(60, 60, 70, 255), rounding, ImDrawFlags_None, 1.f);

    ImVec2 text_pos = bb.Min + ImVec2(padding, height * 0.5f - text_size.y * 0.5f);

    dl->AddText(var->font.tahoma, var->font.tahoma->FontSize + 2.f,
        text_pos + ImVec2(1, 1),
        IM_COL32(0, 0, 0, 200), text);

    dl->AddText(var->font.tahoma, var->font.tahoma->FontSize + 2.f,
        text_pos,
        IM_COL32(255, 255, 255, 255), text);
}

void gui_t::render_keybinds()
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 mouse = ImGui::GetIO().MousePos;

    std::vector<std::pair<std::string, CKeybind*>> binds = {
        { "ESP", &globals::aim::aimbot_bind },
        { "Aimbot", &globals::aim::aimbot_bind },
        { "Fly", &globals::rage::fly_bind },
        { "Speed", &globals::rage::walkspeed_bind },
        { "Jump Power", &globals::rage::jumppower_bind },
        { "Infinite Jump", &globals::rage::infinite_bind },
        { "Hip Height", &globals::rage::hipheight_bind },
        { "Third Person", &globals::rage::third_personbind },
        { "Orbit", &globals::rage::orbitkeybind }
    };

    std::vector<std::tuple<std::string, std::string, CKeybind*>> active;

    auto is_active = [](CKeybind* b) -> bool {
        if (!b || b->key == 0) return false;
        if (b->type == CKeybind::ALWAYS) return true;
        if (b->type == CKeybind::TOGGLE) return b->enabled;
        if (b->type == CKeybind::HOLD) return GetAsyncKeyState(b->key) & 0x8000;
        return false;
        };

    for (auto& b : binds)
        if (is_active(b.second))
            active.push_back(std::make_tuple(b.first, b.second->get_key_name(), b.second));

    const float padding = 10.f;
    const float item_h = 22.f;
    const float width = 200.f;
    const float header_h = 28.f;
    const float spacing = 4.f;

    float content_height = active.empty() ? 0.f : (item_h * active.size() + spacing * (active.size() - 1));
    float height = header_h + (active.empty() ? 0.f : padding + content_height + padding);

    ImRect bb(keybind_list::pos, keybind_list::pos + ImVec2(width, height));
    ImRect header_bb(bb.Min, bb.Min + ImVec2(width, header_h));

    if (ImGui::IsMouseClicked(0) && header_bb.Contains(mouse))
        keybind_list::dragging = true;
    if (ImGui::IsMouseReleased(0))
        keybind_list::dragging = false;
    if (keybind_list::dragging && ImGui::IsMouseDragging(0))
        keybind_list::pos = mouse - ImVec2(width * 0.5f, 10);

    dl->AddRectFilled(bb.Min + ImVec2(0, 2), bb.Max + ImVec2(0, 2),
        IM_COL32(0, 0, 0, 100), 6.f);

    dl->AddRectFilled(bb.Min, bb.Max, IM_COL32(20, 20, 26, 250), 6.f);

    if (!active.empty()) {
        dl->AddLine(bb.Min + ImVec2(0, header_h), bb.Min + ImVec2(width, header_h),
            IM_COL32(255, 255, 255, 20), 1.f);
    }

    dl->AddRect(bb.Min, bb.Max, IM_COL32(60, 60, 70, 255), 6.f, ImDrawFlags_None, 1.f);

    ImVec2 icon_center = header_bb.Min + ImVec2(12, header_h * 0.5f);

    float dot_radius = 2.f;
    float dot_spacing = 3.5f;

    ImU32 icon_color = IM_COL32(200, 200, 210, 255);

    // 3x3 сетка точек
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            dl->AddCircleFilled(
                icon_center + ImVec2(x * dot_spacing, y * dot_spacing),
                dot_radius,
                icon_color,
                8
            );
        }
    }

    const char* title = "Keybind List";
    ImVec2 title_size = var->font.tahoma->CalcTextSizeA(13.f, FLT_MAX, 0.f, title);
    ImVec2 title_pos = header_bb.Min + ImVec2(28, header_h * 0.5f - title_size.y * 0.5f);

    dl->AddText(var->font.tahoma, 13.f,
        title_pos + ImVec2(1, 1),
        IM_COL32(0, 0, 0, 150), title);

    dl->AddText(var->font.tahoma, 13.f,
        title_pos,
        IM_COL32(220, 220, 230, 255), title);

    if (!active.empty()) {
        ImVec2 cur = bb.Min + ImVec2(padding, header_h + padding);

        for (size_t i = 0; i < active.size(); i++)
        {
            auto& item = active[i];
            std::string name = std::get<0>(item);
            std::string key = std::get<1>(item);
            CKeybind* bind = std::get<2>(item);

            std::string type_str;
            ImU32 type_color;

            if (bind->type == CKeybind::ALWAYS) {
                type_str = "[ Always ]";
                type_color = IM_COL32(140, 220, 140, 255);
            }
            else if (bind->type == CKeybind::TOGGLE) {
                type_str = "[ Toggled ]";
                type_color = IM_COL32(120, 190, 255, 255);
            }
            else if (bind->type == CKeybind::HOLD) {
                type_str = "[ Hold ]";
                type_color = IM_COL32(255, 190, 120, 255);
            }

            ImVec2 name_pos = cur + ImVec2(5, item_h * 0.5f - 6);

            dl->AddText(var->font.tahoma, 12.f,
                name_pos + ImVec2(1, 1),
                IM_COL32(0, 0, 0, 150), name.c_str());

            dl->AddText(var->font.tahoma, 12.f,
                name_pos,
                IM_COL32(220, 220, 230, 255), name.c_str());

            ImVec2 type_size = var->font.tahoma->CalcTextSizeA(12.f, FLT_MAX, 0.f, type_str.c_str());
            ImVec2 type_pos = cur + ImVec2(width - padding - type_size.x - 5, item_h * 0.5f - 6);

            dl->AddText(var->font.tahoma, 12.f,
                type_pos,
                type_color, type_str.c_str());

            cur.y += item_h + spacing;
        }
    }
}