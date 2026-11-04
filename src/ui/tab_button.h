// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/i18n.h"

#include <functional>
#include <string>

#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/view.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace {

class TabButton : public View
{
public:
    explicit TabButton(std::string text)
    {
        m_label = addChild<Label>(std::move(text));
        m_label->setFontSize(14);
        setCursorShape(CursorShape::Pointer);
    }

    void setActive(bool active)
    {
        if (m_active == active) return;
        m_active = active;
        m_label->setWeight(m_active ? 600 : 400);
        m_label->setColor(m_active
            ? std::optional<ColorRecipe>{}
            : std::optional<ColorRecipe>{ColorRecipe(ColorId::DisabledText)});
        requestPaint();
    }

    bool isActive() const { return m_active; }
    SizeF sizeHint() const override { return {m_label->sizeHint().width + 32, 48}; }
    bool acceptsPointer() const override { return true; }
    std::function<void()> onClicked;

protected:
    void layout() override
    {
        SizeF s = m_label->sizeHint();
        m_label->setGeometry({16, (geometry().height - s.height) / 2.0 - 1, s.width, s.height});
    }

    void paint(Painter& painter) override
    {
        const Theme& t = Theme::current();
        if (m_active)
            painter.drawHorizontalLine(8, geometry().height - 2, geometry().width - 16,
                                       accent::blue, painter.borderWidth(2));
        else if (m_hovered)
            painter.fillRect(localRect(), t.disabledText.withAlpha(0.06));
    }

    void pointerEnter() override { m_hovered = true;  requestPaint(); }
    void pointerLeave() override { m_hovered = false; requestPaint(); }
    void pointerPress(const PointerEvent& e) override
    {
        if (e.button == MouseButton::Left && onClicked) onClicked();
    }

private:
    Label* m_label;
    bool m_active = false, m_hovered = false;
};

} // namespace
