// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/i18n.h"

#include "../debbackend.h"
#include "package_detail_panel.h"
#include "package_row.h"

#include "ui/button.h"
#include "ui/label.h"
#include "ui/list_view.h"
#include "ui/painter.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/view.h"

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace {

class ManageView : public View
{
public:
    std::function<void()> onRefresh;
    std::function<void(const std::string& name)> onRemove;
    std::function<void(const std::string& name)> onUpdate;
    std::function<void(const std::string& name)> onShowLog;
    std::function<void(const std::string& name)> onPackageSelected;

    ManageView()
    {
        m_search = addChild<TextField>();
        m_search->setPlaceholder(tr("Search installed packages\xe2\x80\xa6"));
        m_search->onTextChanged = [this] { applyFilter(); };

        m_refreshBtn = addChild<Button>(tr("Refresh"));
        m_refreshBtn->onClicked = [this] { if (onRefresh) onRefresh(); };

        m_colName = addChild<Label>(tr("Name"));
        m_colVer  = addChild<Label>(tr("Version"));
        m_colArch = addChild<Label>(tr("Arch"));
        m_colDesc = addChild<Label>(tr("Description"));
        for (auto* l : {m_colName, m_colVer, m_colArch, m_colDesc}) {
            l->setFontSize(11);
            l->setColor(ColorRecipe(ColorId::DisabledText));
        }

        m_list = addChild<ListView>();
        m_list->setRowHeight(36);
        m_list->setSpacing(1);
        m_list->createItem = []() -> std::unique_ptr<View> {
            return std::make_unique<PackageRow>();
        };
        m_list->bindItem = [this](View* item, size_t row) {
            if (row >= m_filtered.size()) return;
            static_cast<PackageRow*>(item)->bind(
                m_packages[m_filtered[row]], row % 2 == 1,
                row == (size_t)m_selectedRow, row,
                [this](size_t idx) { selectRow(idx); });
        };

        m_detailPanel = addChild<PackageDetailPanel>();

        m_countLabel = addChild<Label>();
        m_countLabel->setFontSize(12);
        m_countLabel->setColor(ColorRecipe(ColorId::DisabledText));

        m_removeBtn = addChild<Button>(tr("Remove"));
        m_removeBtn->setEnabled(false);
        m_removeBtn->onClicked = [this] {
            if (m_selectedRow >= 0 && m_selectedRow < (int)m_filtered.size())
                if (onRemove) onRemove(m_packages[m_filtered[m_selectedRow]].name);
        };

        m_updateBtn = addChild<Button>(tr("Update"));
        m_updateBtn->setEnabled(false);
        m_updateBtn->onClicked = [this] {
            if (m_selectedRow >= 0 && m_selectedRow < (int)m_filtered.size())
                if (onUpdate) onUpdate(m_packages[m_filtered[m_selectedRow]].name);
        };

        m_logBtn = addChild<Button>(tr("Show Log"));
        m_logBtn->setVisible(false);
        m_logBtn->onClicked = [this] {
            if (m_selectedRow >= 0 && m_selectedRow < (int)m_filtered.size())
                if (onShowLog) onShowLog(m_packages[m_filtered[m_selectedRow]].name);
        };

        m_statusLabel = addChild<Label>();
        m_statusLabel->setFontSize(12);
        m_statusLabel->setColor(ColorRecipe(ColorId::DisabledText));
        m_statusLabel->setElide(Label::Elide::Right);
    }

    void setPackages(std::vector<debinstaller::InstalledPackage> packages)
    {
        m_packages = std::move(packages);
        m_selectedRow = -1;
        m_detailPanel->clear();
        m_removeBtn->setEnabled(false);
        m_updateBtn->setEnabled(false);
        m_logBtn->setVisible(false);
        applyFilter();
        setRunning(false);
    }

    void setRunning(bool running)
    {
        m_refreshBtn->setEnabled(!running);
        m_removeBtn->setEnabled(!running && m_selectedRow >= 0);
        m_updateBtn->setEnabled(!running && m_selectedRow >= 0);
        m_search->setReadOnly(running);
    }

    void setStatus(const std::string& msg) { m_statusLabel->setText(msg); requestLayout(); }

    void setLogAvailable(bool available)
    {
        m_logBtn->setVisible(available && m_selectedRow >= 0);
        requestLayout();
    }

    void showDetail(const debinstaller::PackageDetail& detail, const std::string& version)
    {
        m_detailPanel->showDetail(detail, version);
    }

    SizeF sizeHint() const override { return {0, 580}; }

protected:
    void layout() override
    {
        const RectF r = localRect();
        constexpr double detailW = 280;
        constexpr double nw = 200, vw = 110, aw = 64;
        const double listW = r.width - detailW;

        m_search    ->setGeometry({16, 12, std::max(0.0, listW - 156), 36});
        m_refreshBtn->setGeometry({listW - 128, 12, 110, 36});

        m_colName->setGeometry({10, 58, nw, 18});
        m_colVer ->setGeometry({nw + 16, 58, vw, 18});
        m_colArch->setGeometry({nw + vw + 24, 58, aw, 18});
        m_colDesc->setGeometry({nw + vw + aw + 32, 58, std::max(0.0, listW - nw - vw - aw - 48), 18});

        const double listBottom = r.height - 54;
        m_list       ->setGeometry({0, 80, listW, std::max(0.0, listBottom - 80)});
        m_detailPanel->setGeometry({r.width - detailW, 0, detailW, listBottom});

        m_countLabel->setGeometry({16, listBottom + 8, 200, 18});
        m_logBtn    ->setGeometry({listW - 372, listBottom + 4, 100, 34});
        m_removeBtn ->setGeometry({listW - 264, listBottom + 4, 120, 34});
        m_updateBtn ->setGeometry({listW - 136, listBottom + 4, 120, 34});
        m_statusLabel->setGeometry({16, listBottom + 30, r.width - 32, 16});
    }

    void paint(Painter& painter) override
    {
        const Theme& t = Theme::current();
        painter.fillRect(localRect(), t.background);
        painter.drawHorizontalLine(0, 78, geometry().width,
                                   t.disabledText.withAlpha(0.08), painter.borderWidth(1));
    }

private:
    void applyFilter()
    {
        const std::string q = m_search->text();
        m_filtered.clear();
        for (size_t i = 0; i < m_packages.size(); ++i) {
            const auto& pkg = m_packages[i];
            if (q.empty()
                || pkg.name.find(q) != std::string::npos
                || pkg.description.find(q) != std::string::npos)
                m_filtered.push_back(i);
        }
        if (m_selectedRow >= (int)m_filtered.size())
            m_selectedRow = -1;
        m_list->setRowCount(m_filtered.size());
        m_list->reload();
        updateCount();
        m_removeBtn->setEnabled(m_selectedRow >= 0);
        m_updateBtn->setEnabled(m_selectedRow >= 0);
        requestLayout();
    }

    void selectRow(size_t row)
    {
        const int prev = m_selectedRow;
        m_selectedRow = (int)row;
        if (prev >= 0 && prev < (int)m_filtered.size())
            if (auto* v = m_list->itemForRow((size_t)prev))
                static_cast<PackageRow*>(v)->bind(
                    m_packages[m_filtered[(size_t)prev]],
                    (size_t)prev % 2 == 1, false, (size_t)prev,
                    [this](size_t idx) { selectRow(idx); });
        if (auto* v = m_list->itemForRow(row))
            static_cast<PackageRow*>(v)->bind(
                m_packages[m_filtered[row]], row % 2 == 1, true, row,
                [this](size_t idx) { selectRow(idx); });
        m_removeBtn->setEnabled(true);
        m_updateBtn->setEnabled(true);
        if (onPackageSelected)
            onPackageSelected(m_packages[m_filtered[row]].name);
    }

    void updateCount()
    {
        const size_t total = m_packages.size(), shown = m_filtered.size();
        if (shown == total)
            m_countLabel->setText(std::to_string(total) + tr(" packages installed"));
        else
            m_countLabel->setText(std::to_string(shown) + " / " +
                                  std::to_string(total) + tr(" shown"));
    }

    TextField*          m_search;
    Button*             m_refreshBtn;
    Label*              m_colName, *m_colVer, *m_colArch, *m_colDesc;
    ListView*           m_list;
    PackageDetailPanel* m_detailPanel;
    Label*              m_countLabel;
    Button*             m_removeBtn, *m_updateBtn, *m_logBtn;
    Label*              m_statusLabel;
    std::vector<debinstaller::InstalledPackage> m_packages;
    std::vector<size_t> m_filtered;
    int m_selectedRow = -1;
};

} // namespace
