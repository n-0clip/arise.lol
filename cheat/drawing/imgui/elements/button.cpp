#include "../settings/functions.h"

bool c_gui::button(std::string_view label, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label.data());

    // Размер текста
    const ImVec2 label_size = var->font.tahoma->CalcTextSizeA(
        var->font.tahoma->FontSize, FLT_MAX, 0.0f, label.data(), label.data() + label.size()
    );

    const ImVec2 padding = ImVec2(16.0f, 10.0f);

    ImVec2 size = size_arg;
    if (size.x <= 0.0f) size.x = label_size.x + padding.x * 2;
    if (size.y <= 0.0f) size.y = elements->widgets.dropdown_height;

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect rect(pos, pos + size);
    const ImRect inner_rect(rect.Min + ImVec2(2, 2), rect.Max - ImVec2(2, 2));

    ItemSize(rect, style.FramePadding.y);
    if (!ItemAdd(rect, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(rect, id, &hovered, &held, ImGuiButtonFlags_None);

    ImDrawList* drawlist = window->DrawList;

    // Фон (градиент)
    draw->fade_rect_filled(drawlist,
        inner_rect.Min, inner_rect.Max,
        draw->get_clr(clr->window.background_two),
        draw->get_clr(clr->window.background_one),
        fade_direction::vertically
    );

    // Ховер / нажатие — акцентный градиент
    if (pressed || held || hovered)
    {
        ImColor top = clr->accent;
        ImColor bottom = ImColor(
            clr->accent.Value.x * 0.85f,
            clr->accent.Value.y * 0.85f,
            clr->accent.Value.z * 0.85f,
            1.0f
        );

        draw->fade_rect_filled(drawlist,
            inner_rect.Min, inner_rect.Max,
            draw->get_clr(top),
            draw->get_clr(bottom),
            fade_direction::vertically
        );
    }

    // Внешняя обводка (ховер = акцент)
    draw->rect(drawlist, rect.Min, rect.Max,
        draw->get_clr(var->window.hover_hightlight && hovered ? clr->accent : clr->widgets.stroke_two));

    // Внутренняя обводка
    draw->rect(drawlist, rect.Min + ImVec2(1, 1), rect.Max - ImVec2(1, 1),
        draw->get_clr(clr->window.stroke));

    // ТЕКСТ ПО ЦЕНТРУ — САМЫЙ НАДЁЖНЫЙ СПОСОБ (без E0304)
    const ImVec2 text_pos(
        rect.Min.x + (rect.GetWidth() - label_size.x) * 0.5f,
        rect.Min.y + (rect.GetHeight() - label_size.y) * 0.5f
    );

    ImU32 text_color = draw->get_clr(hovered ? clr->widgets.text : clr->widgets.text_inactive);

    drawlist->AddText(
        var->font.tahoma,
        var->font.tahoma->FontSize,
        text_pos,
        text_color,
        label.data(),
        label.data() + label.size()  // ← ВОТ ЭТО ОБЯЗАТЕЛЬНО!
    );

    return pressed;
}