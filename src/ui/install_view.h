// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/i18n.h"

#include "../debbackend.h"
#include "drop_zone.h"
#include "info_panel.h"

#include "ui/button.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/progress_bar.h"
#include "ui/theme.h"
#include "ui/view.h"

#include <functional>
#include <string>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace {

class InstallView : public View
{
public:
    std::function<void()> onOpenFile;
    std::function<void()> onInstall;
    std::function<void()> onShowLog;
    std::function<void(const std::string& path)> onDroppedFile;

    InstallView()
    {
        m_drop = addChild<DropZone>([this](const std::string& path) {
            if (onDroppedFile) onDroppedFile(path);
        });

        m_hint = addChild<Label>(tr("Open or drop a .deb file to inspect and install it"));
        m_hint->setFontSize(14);
        m_hint->setColor(ColorRecipe(ColorId::DisabledText));
        m_hint->setTextAlignment(Label::TextAlignment::Center);
        m_hint->setVerticallyCentered(true);

        m_fileLabel = addChild<Label>();
        m_fileLabel->setFontSize(12);
        m_fileLabel->setColor(ColorRecipe(ColorId::DisabledText));
        m_fileLabel->setElide(Label::Elide::Middle);
        m_fileLabel->setVisible(false);

        m_openBtn = addChild<Button>(tr("Open .deb File\xe2\x80\xa6"));
        m_openBtn->onClicked = [this] { if (onOpenFile) onOpenFile(); };

        m_infoPanel = addChild<PackageInfoPanel>();
        m_infoPanel->setVisible(false);

        m_progress = addChild<ProgressBar>();
        m_progress->setIndeterminate(true);
        m_progress->setVisible(false);

        m_busy = addChild<BusyIndicator>();
        m_busy->setRunning(false);
        m_busy->setVisible(false);

        m_progressLabel = addChild<Label>();
        m_progressLabel->setFontSize(12);
        m_progressLabel->setColor(ColorRecipe(ColorId::DisabledText));
        m_progressLabel->setElide(Label::Elide::Right);
        m_progressLabel->setVisible(false);

        m_installBtn = addChild<Button>(tr("Install Package"));
        m_installBtn->setEnabled(false);
        m_installBtn->onClicked = [this] { if (onInstall) onInstall(); };

        m_logBtn = addChild<Button>(tr("Show Log"));
        m_logBtn->setVisible(false);
        m_logBtn->onClicked = [this] { if (onShowLog) onShowLog(); };

        m_statusLabel = addChild<Label>();
        m_statusLabel->setFontSize(12);
        m_statusLabel->setColor(ColorRecipe(ColorId::DisabledText));
        m_statusLabel->setElide(Label::Elide::Right);
    }

    void showPackage(const debinstaller::PackageInfo& info)
    {
        m_hint->setVisible(false);
        m_fileLabel->setText(info.filePath);
        m_fileLabel->setVisible(true);
        m_infoPanel->showPackage(info);
        m_infoPanel->setVisible(true);
        m_installBtn->setEnabled(true);
        m_logBtn->setVisible(false);
        setStatus({});
        requestLayout();
    }

    void showError(const std::string& error)
    {
        m_hint->setText(error.empty() ? tr("Could not read this file") : error);
        m_hint->setVisible(true);
        m_fileLabel->setVisible(false);
        m_infoPanel->setVisible(false);
        m_installBtn->setEnabled(false);
        setStatus({});
        requestLayout();
    }

    void setRunning(bool running)
    {
        m_progress->setVisible(running);
        m_busy->setRunning(running);
        m_busy->setVisible(running);
        m_progressLabel->setVisible(running);
        m_openBtn->setEnabled(!running);
        m_installBtn->setEnabled(!running && m_infoPanel->isVisible());
    }

    void setOutputLine(const std::string& line) { m_progressLabel->setText(line); }

    void setStatus(const std::string& msg) { m_statusLabel->setText(msg); requestLayout(); }

    void setLogAvailable(bool available) { m_logBtn->setVisible(available); requestLayout(); }

    void reset()
    {
        m_hint->setText(tr("Open or drop a .deb file to inspect and install it"));
        m_hint->setVisible(true);
        m_fileLabel->setVisible(false);
        m_infoPanel->clear();
        m_infoPanel->setVisible(false);
        m_installBtn->setEnabled(false);
        setRunning(false);
        setStatus({});
    }

    SizeF sizeHint() const override { return {0, 580}; }

protected:
    void layout() override
    {
        const RectF r = localRect();
        m_drop->setGeometry(r);
        const double panelH = m_infoPanel->isVisible() ? m_infoPanel->sizeHint().height : 0;

        if (m_hint->isVisible()) {
            m_hint->setGeometry({24, r.height / 2 - 40, r.width - 48, 30});
            m_openBtn->setGeometry({r.width / 2 - 100, r.height / 2, 200, 36});
        } else {
            m_fileLabel->setGeometry({24, 16, std::max(0.0, r.width - 200), 20});
            m_openBtn->setGeometry({r.width - 140, 12, 120, 30});
            m_infoPanel->setGeometry({24, 50, r.width - 48, panelH});
        }

        double y = 50 + panelH + 24;
        if (m_progress->isVisible()) {
            m_busy->setGeometry({24, y, 22, 22});
            m_progress->setGeometry({54, y + 8, r.width - 78, 6});
            m_progressLabel->setGeometry({24, y + 26, r.width - 48, 16});
            y += 54;
        }
        m_installBtn->setGeometry({24, y, 160, 38});
        if (m_logBtn->isVisible())
            m_logBtn->setGeometry({196, y, 100, 38});
        y += 50;
        m_statusLabel->setGeometry({24, y, r.width - 48, 18});
    }

    void paint(Painter& painter) override
    {
        painter.fillRect(localRect(), Theme::current().background);
    }

private:
    DropZone*         m_drop;
    Label*            m_hint;
    Label*            m_fileLabel;
    Button*           m_openBtn;
    PackageInfoPanel* m_infoPanel;
    ProgressBar*      m_progress;
    BusyIndicator*    m_busy;
    Label*            m_progressLabel;
    Button*           m_installBtn;
    Button*           m_logBtn;
    Label*            m_statusLabel;
};

} // namespace
