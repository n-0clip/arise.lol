#include "../settings/functions.h"

bool c_gui::section(std::string_view icon, bool* callback)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    static float hover_anim[64] = { 0 };
    static float active_anim[64] = { 0 };
    const int anim_id = (int)(std::hash<std::string_view>{}(icon) % 64);

    const ImGuiID id = window->GetID((std::string(icon) + "_section").c_str());
    const ImVec2 pos = window->DC.CursorPos;
    const ImVec2 size = elements->section.size; // например ImVec2(50, 50)
    const ImRect bb(pos, pos + size);

    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    if (pressed)
        *callback = !*callback;

    ImDrawList* draw_list = window->DrawList;

    const float speed = 1.0f - powf(0.05f, g.IO.DeltaTime);

    hover_anim[anim_id] = ImLerp(hover_anim[anim_id], (hovered || *callback) ? 1.0f : 0.0f, speed);
    active_anim[anim_id] = ImLerp(active_anim[anim_id], *callback ? 1.0f : 0.0f, speed);

    float hover_alpha = hover_anim[anim_id];
    float active_alpha = active_anim[anim_id];

    draw->fade_rect_filled(draw_list, bb.Min, bb.Max,
        draw->get_clr(clr->window.background_one),
        draw->get_clr(clr->window.background_two),
        fade_direction::vertically);

    if (hover_alpha > 0.01f)
    {
        ImU32 col = draw->get_clr(clr->accent);
        col = (col & 0x00FFFFFF) | ((ImU32)(hover_alpha * 60) << 24);
        draw_list->AddRectFilled(bb.Min, bb.Max, col, 8.0f);
    }

    if (active_alpha > 0.01f)
    {
        const float line_y = bb.Max.y - 2.0f;
        const ImVec2 line_from = ImVec2(bb.Min.x + 8, line_y);
        const ImVec2 line_to_full = ImVec2(bb.Max.x - 8, line_y);
        const ImVec2 line_to_current = ImLerp(line_from, line_to_full, active_alpha);

        ImU32 line_col = draw->get_clr(clr->accent);
        line_col = (line_col & 0x00FFFFFF) | ((ImU32)(255 * active_alpha) << 24);

        draw_list->AddLine(line_from, line_to_current, line_col, 2.5f);

        ImU32 glow_col = (line_col & 0x00FFFFFF) | ((ImU32)(80 * active_alpha) << 24);
        draw_list->AddLine(line_from, line_to_current, glow_col, 6.0f);
    }

    if (hovered || *callback)
    {
        ImU32 border_col = draw->get_clr(clr->accent);
        border_col = (border_col & 0x00FFFFFF) | ((ImU32)(hover_alpha * 180) << 24);
        draw_list->AddRect(bb.Min, bb.Max, border_col, 8.0f, ImDrawFlags_None, 1.3f);
    }

    float icon_alpha = 0.5f + 0.5f * hover_alpha;
    if (*callback)
        icon_alpha = 1.0f;

    ImU32 icon_color = draw->get_clr(*callback ? clr->accent : clr->widgets.text_inactive);
    icon_color = (icon_color & 0x00FFFFFF) | ((ImU32)(255 * icon_alpha) << 24);

    ImVec2 text_size = var->font.icons[0]->CalcTextSizeA(var->font.icons[0]->FontSize, FLT_MAX, 0.0f, icon.data());
    ImVec2 icon_pos = ImVec2(
        bb.Min.x + (bb.GetWidth() - text_size.x) * 0.5f,
        bb.Min.y + (bb.GetHeight() - text_size.y) * 0.5f
    );

    draw_list->AddText(var->font.icons[0], var->font.icons[0]->FontSize, icon_pos,
        icon_color, icon.data());

    gui->sameline();

    return pressed;
}

bool c_gui::sub_section(std::string_view label, int section_id, int& section_variable, float count)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label.data());
    const bool selected = section_id == section_variable;

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect rect(pos, pos + ImVec2((GetWindowWidth() - elements->content.window_padding.x * 2 - g.Style.ItemSpacing.x * (count - 1)) / count, elements->section.height));
    ItemSize(rect, style.FramePadding.y);
    if (!ItemAdd(rect, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(rect, id, &hovered, &held);

    if (pressed)
        section_variable = section_id;

    if (selected || hovered)
        draw->fade_rect_filled(window->DrawList, rect.Min, rect.Max, draw->get_clr(clr->window.background_one), draw->get_clr(clr->window.background_two), fade_direction::vertically);
    else
        draw->fade_rect_filled(window->DrawList, rect.Min, rect.Max, draw->get_clr(clr->window.background_two), draw->get_clr(clr->window.background_one), fade_direction::vertically);

    draw->rect(window->DrawList, rect.Min, rect.Max, draw->get_clr(clr->window.stroke));

    if (selected)
        draw->line(GetForegroundDrawList(), ImVec2(rect.Min.x + 1, rect.Max.y - 1), rect.Max - ImVec2(1, 1), draw->get_clr(clr->window.background_two));

    draw->text_clipped(window->DrawList, var->font.tahoma, rect.Min, rect.Max, selected ? draw->get_clr(clr->widgets.text) : draw->get_clr(clr->widgets.text_inactive), label.data(), NULL, NULL, ImVec2(0.5f, 0.5f));

    gui->sameline();

    return pressed;
}