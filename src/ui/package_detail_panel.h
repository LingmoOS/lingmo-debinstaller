// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/i18n.h"

#include "../debbackend.h"

#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "ui/view.h"

#include <string>
#include <utility>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace {

class PackageDetailPanel : public View
{
public:
    PackageDetailPanel()
    {
        m_titleLabel = addChild<Label>(tr("Package Details"));
        m_titleLabel->setFontSize(13);
        m_titleLabel->setWeight(600);

        m_nameLabel = addChild<Label>();
        m_nameLabel->setFontSize(16);
        m_nameLabel->setWeight(600);
        m_nameLabel->setElide(Label::Elide::Right);

        m_verLabel = addChild<Label>();
        m_verLabel->setFontSize(12);
        m_verLabel->setColor(ColorRecipe(ColorId::DisabledText));

        m_scroll = addChild<ScrollView>();
        m_content = m_scroll->setContent<Column>();
        m_content->setSpacing(0);
        m_content->setMargins({12, 8, 12, 8});

        m_descRow  = makeSection(tr("Description"));
        m_depRow   = makeSection(tr("Depends"));
        m_recRow   = makeSection(tr("Recommends"));
        m_sugRow   = makeSection(tr("Suggests"));
        m_conRow   = makeSection(tr("Conflicts"));
        m_provRow  = makeSection(tr("Provides"));
    }

    void showDetail(const debinstaller::PackageDetail& d, const std::string& version = {})
    {
        m_nameLabel->setText(d.name);
        m_verLabel->setText(version);
        setValue(m_descRow.second, d.longDescription);
        setValue(m_depRow.second,  d.depends);
        setValue(m_recRow.second,  d.recommends);
        setValue(m_sugRow.second,  d.suggests);
        setValue(m_conRow.second,  d.conflicts);
        setValue(m_provRow.second, d.provides);
        requestLayout();
    }

    void clear()
    {
        m_nameLabel->setText({});
        m_verLabel->setText({});
        for (auto& p : {m_descRow, m_depRow, m_recRow, m_sugRow, m_conRow, m_provRow})
            setValue(p.second, {});
    }

    SizeF sizeHint() const override { return {260, 0}; }

protected:
    void layout() override
    {
        const double w = geometry().width, h = geometry().height;
        m_titleLabel->setGeometry({12, 12, w - 24, 18});
        m_nameLabel ->setGeometry({12, 36, w - 24, 22});
        m_verLabel  ->setGeometry({12, 62, w - 24, 16});
        m_scroll    ->setGeometry({0, 84, w, std::max(0.0, h - 84)});
    }

    void paint(Painter& painter) override
    {
        const Theme& t = Theme::current();
        painter.fillRect(localRect(), t.secondBackground);
        painter.drawVerticalLine(0, 0, geometry().height,
                                 t.disabledText.withAlpha(0.10), painter.borderWidth(1));
    }

private:
    using Section = std::pair<Label*, Label*>;

    Section makeSection(const std::string& heading)
    {
        auto* hdr = m_content->addChild<Label>(heading);
        hdr->setFontSize(11);
        hdr->setWeight(600);
        hdr->setColor(ColorRecipe(ColorId::DisabledText));
        auto* val = m_content->addChild<Label>();
        val->setFontSize(12);
        val->setWrap(true);
        val->setLayoutHints({.fillWidth = true});
        return {hdr, val};
    }

    void setValue(Label* label, const std::string& text)
    {
        label->setText(text);
        label->setVisible(!text.empty());
        const auto& siblings = m_content->children();
        for (size_t i = 1; i < siblings.size(); ++i) {
            if (siblings[i].get() == label) {
                siblings[i - 1]->setVisible(!text.empty());
                break;
            }
        }
    }

    Label*      m_titleLabel;
    Label*      m_nameLabel;
    Label*      m_verLabel;
    ScrollView* m_scroll;
    Column*     m_content;
    Section     m_descRow, m_depRow, m_recRow, m_sugRow, m_conRow, m_provRow;
};

} // namespace
