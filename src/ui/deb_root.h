// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/i18n.h"

#include "install_view.h"
#include "manage_view.h"
#include "tab_button.h"

#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/view.h"

#include <functional>
#include <string>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace {

class DebRoot : public View
{
public:
    explicit DebRoot(std::function<void(int)> onTabSwitch)
        : m_onTabSwitch(std::move(onTabSwitch))
    {
        m_appTitle = addChild<Label>(tr("Deb Installer"));
        m_appTitle->setFontSize(17);
        m_appTitle->setWeight(600);

        m_installTab = addChild<TabButton>(tr("Install"));
        m_installTab->setActive(true);
        m_installTab->onClicked = [this] { switchTab(0); };

        m_manageTab = addChild<TabButton>(tr("Installed Packages"));
        m_manageTab->onClicked = [this] { switchTab(1); };

        m_installView = addChild<InstallView>();
        m_manageView  = addChild<ManageView>();
        m_manageView->setVisible(false);
    }

    InstallView* installView() { return m_installView; }
    ManageView*  manageView()  { return m_manageView; }

    void switchTab(int tab)
    {
        if (m_currentTab == tab) return;
        m_currentTab = tab;
        m_installTab->setActive(tab == 0);
        m_manageTab ->setActive(tab == 1);
        m_installView->setVisible(tab == 0);
        m_manageView ->setVisible(tab == 1);
        requestLayout();
        if (m_onTabSwitch) m_onTabSwitch(tab);
    }

    int currentTab() const { return m_currentTab; }
    SizeF sizeHint() const override { return {1100, 700}; }

protected:
    void layout() override
    {
        const RectF r = localRect();
        m_appTitle->setGeometry({24, 14, 200, 28});
        const double tw0 = m_installTab->sizeHint().width;
        const double tw1 = m_manageTab ->sizeHint().width;
        const double tx  = (r.width - tw0 - tw1) / 2.0;
        m_installTab->setGeometry({tx,       0, tw0, 56});
        m_manageTab ->setGeometry({tx + tw0, 0, tw1, 56});
        const RectF content = {0, 56, r.width, std::max(0.0, r.height - 56)};
        m_installView->setGeometry(content);
        m_manageView ->setGeometry(content);
    }

    void paint(Painter& painter) override
    {
        const Theme& t = Theme::current();
        painter.fillRect(localRect(), t.background);
        painter.drawHorizontalLine(0, 56, geometry().width,
                                   t.disabledText.withAlpha(0.08), painter.borderWidth(1));
    }

private:
    std::function<void(int)> m_onTabSwitch;
    int          m_currentTab = 0;
    Label*       m_appTitle;
    TabButton*   m_installTab, *m_manageTab;
    InstallView* m_installView;
    ManageView*  m_manageView;
};

} // namespace
