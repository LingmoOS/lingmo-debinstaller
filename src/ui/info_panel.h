// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../debbackend.h"

#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/view.h"

#include <string>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace {

class InfoRow : public View
{
public:
    InfoRow(std::string key, bool alt) : m_alt(alt)
    {
        m_key = addChild<Label>(std::move(key));
        m_key->setFontSize(12);
        m_key->setColor(ColorRecipe(ColorId::DisabledText));
        m_val = addChild<Label>();
        m_val->setFontSize(13);
        m_val->setElide(Label::Elide::Right);
        setLayoutHints({.fillWidth = true});
    }

    void setValue(const std::string& v) { m_val->setText(v); }
    SizeF sizeHint() const override { return {0, 30}; }

protected:
    void layout() override
    {
        const double kw = 132, w = geometry().width;
        m_key->setGeometry({16, 7, kw, 18});
        m_val->setGeometry({kw + 20, 6, std::max(0.0, w - kw - 36), 20});
    }
    void paint(Painter& painter) override
    {
        if (m_alt)
            painter.fillRect(localRect(), Theme::current().disabledText.withAlpha(0.04));
    }

private:
    Label* m_key, *m_val;
    bool m_alt;
};

class PackageInfoPanel : public View
{
public:
    PackageInfoPanel()
    {
        m_name    = addChild<InfoRow>(tr("Package"),        false);
        m_version = addChild<InfoRow>(tr("Version"),        true);
        m_arch    = addChild<InfoRow>(tr("Architecture"),   false);
        m_maint   = addChild<InfoRow>(tr("Maintainer"),     true);
        m_size    = addChild<InfoRow>(tr("Installed Size"), false);
        m_section = addChild<InfoRow>(tr("Section"),        true);
        m_depends = addChild<InfoRow>(tr("Depends"),        false);
        m_desc    = addChild<InfoRow>(tr("Description"),    true);
        setLayoutHints({.fillWidth = true});
    }

    void showPackage(const debinstaller::PackageInfo& info)
    {
        m_name   ->setValue(info.name);
        m_version->setValue(info.version);
        m_arch   ->setValue(info.architecture);
        m_maint  ->setValue(info.maintainer);
        m_size   ->setValue(info.installedSize);
        m_section->setValue(info.section);
        m_depends->setValue(info.depends);
        m_desc   ->setValue(info.description);
    }

    void clear()
    {
        for (auto* r : {m_name, m_version, m_arch, m_maint, m_size, m_section, m_depends, m_desc})
            r->setValue({});
    }

    SizeF sizeHint() const override { return {0, 8 * 30.0}; }

protected:
    void layout() override
    {
        InfoRow* rows[] = {m_name, m_version, m_arch, m_maint, m_size, m_section, m_depends, m_desc};
        for (int i = 0; i < 8; ++i)
            rows[i]->setGeometry({0, i * 30.0, geometry().width, 30});
    }

    void paint(Painter& painter) override
    {
        const Theme& t = Theme::current();
        painter.drawRoundedRect(localRect(), {t.smallRadius}, t.alternateBackground,
                                painter.borderWidth(1), t.disabledText.withAlpha(0.14));
    }

private:
    InfoRow* m_name, *m_version, *m_arch, *m_maint, *m_size, *m_section, *m_depends, *m_desc;
};

} // namespace
