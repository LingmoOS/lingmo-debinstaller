// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../debbackend.h"

#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/view.h"

#include <algorithm>
#include <functional>
#include <string>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace {

class PackageRow : public View
{
public:
    PackageRow()
    {
        m_name = addChild<Label>();
        m_name->setFontSize(13);
        m_ver  = addChild<Label>();
        m_ver->setFontSize(12);
        m_ver->setColor(ColorRecipe(ColorId::DisabledText));
        m_arch = addChild<Label>();
        m_arch->setFontSize(12);
        m_arch->setColor(ColorRecipe(ColorId::DisabledText));
        m_desc = addChild<Label>();
        m_desc->setFontSize(12);
        m_desc->setElide(Label::Elide::Right);
        setCursorShape(CursorShape::Pointer);
    }

    void bind(const debinstaller::InstalledPackage& pkg, bool alt, bool sel,
              size_t row, std::function<void(size_t)> onSelect)
    {
        m_name->setText(pkg.name);
        m_ver ->setText(pkg.version);
        m_arch->setText(pkg.architecture);
        m_desc->setText(pkg.description);
        m_alternate = alt;
        m_selected  = sel;
        m_row       = row;
        m_onSelect  = std::move(onSelect);
        requestPaint();
    }

    SizeF sizeHint() const override { return {0, 36}; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const double w = geometry().width;
        constexpr double nw = 200, vw = 120, aw = 70;
        m_name->setGeometry({10,  8, nw, 20});
        m_ver ->setGeometry({nw + 16,  8, vw, 20});
        m_arch->setGeometry({nw + vw + 24, 8, aw, 20});
        m_desc->setGeometry({nw + vw + aw + 32, 8, std::max(0.0, w - nw - vw - aw - 48), 20});
    }

    void paint(Painter& painter) override
    {
        const Theme& t = Theme::current();
        if (m_selected)
            painter.fillRect(localRect(), t.highlight.withAlpha(0.16));
        else if (m_alternate)
            painter.fillRect(localRect(), t.disabledText.withAlpha(0.04));
    }

    void pointerPress(const PointerEvent& e) override
    {
        if (e.button == MouseButton::Left && m_onSelect)
            m_onSelect(m_row);
    }

private:
    Label* m_name, *m_ver, *m_arch, *m_desc;
    bool m_alternate = false, m_selected = false;
    size_t m_row = 0;
    std::function<void(size_t)> m_onSelect;
};

} // namespace
