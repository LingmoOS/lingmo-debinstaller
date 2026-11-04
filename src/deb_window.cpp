// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deb_window.h"

#include "base/i18n.h"
#include "ui/application.h"
#include "ui/dialog_window.h"
#include "ui/file_chooser.h"
#include "ui/window.h"

#include <string>
#include <string_view>
#include <vector>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

DebWindow::DebWindow()
{
    m_window.setTitle(tr("Deb Installer"));
    m_window.resize({1100, 700});
    m_window.setMinimumSize({720, 520});
    m_window.setRestoresSize("lingmo-debinstaller");

    auto root = std::make_unique<DebRoot>([this](int tab) { onTabSwitched(tab); });
    m_root = root.get();
    m_window.setContentView(std::move(root));

    auto* iv = m_root->installView();
    iv->onOpenFile    = [this] { openWithChooser(); };
    iv->onInstall     = [this] { installPackage(); };
    iv->onDroppedFile = [this](const std::string& p) { openDebFile(p); };
    iv->onShowLog     = [this] { showInstallLog(); };

    auto* mv = m_root->manageView();
    mv->onRefresh         = [this] { loadInstalledPackages(); };
    mv->onRemove          = [this](const std::string& n) { removePackage(n); };
    mv->onUpdate          = [this](const std::string& n) { updatePackage(n); };
    mv->onShowLog         = [this](const std::string&)   { showManageLog(); };
    mv->onPackageSelected = [this](const std::string& n) { loadPackageDetail(n); };
}

void DebWindow::openDebFile(const std::string& path)
{
    auto info = debinstaller::inspectDebFile(path);
    if (!info.ok) {
        m_root->installView()->showError(info.error);
        return;
    }
    m_currentPackage = std::move(info);
    m_root->installView()->showPackage(m_currentPackage);
    if (m_root->currentTab() != 0)
        m_root->switchTab(0);
}

void DebWindow::openWithChooser()
{
    m_chooser = std::make_unique<FileChooser>(&m_window);
    m_chooser->setMode(FileChooser::Mode::OpenFile);
    m_chooser->setTitle(tr("Open Debian Package"));
    m_chooser->setFilters({{tr("Debian Packages"), {"*.deb"}, {}},
                           {tr("All Files"),        {"*"},    {}}});
    m_chooser->onAccepted = [this](const std::vector<std::string>& paths,
                                   const std::string&) {
        if (!paths.empty()) openDebFile(paths.front());
    };
    m_chooser->open();
}

void DebWindow::installPackage()
{
    if (m_currentPackage.filePath.empty()) return;
    m_installLog.clear();
    m_root->installView()->setRunning(true);
    m_root->installView()->setStatus(tr("Authenticating\xe2\x80\xa6"));
    m_root->installView()->setLogAvailable(false);

    m_installOp.start(
        m_currentPackage.filePath,
        [this](std::string_view line) {
            m_installLog += line;
            m_installLog += '\n';
            m_root->installView()->setOutputLine(std::string(line));
        },
        [this](bool ok, const std::string& msg) {
            m_root->installView()->setRunning(false);
            m_root->installView()->setStatus(msg);
            m_root->installView()->setLogAvailable(!m_installLog.empty());
            if (ok) {
                m_root->installView()->reset();
                m_root->installView()->setLogAvailable(!m_installLog.empty());
            }
        });
}

void DebWindow::removePackage(const std::string& name)
{
    m_confirm = std::make_unique<ConfirmDialog>(&m_window,
        tr("Remove Package"),
        tr("Remove ") + name +
        tr("?\n\nThis will uninstall the package and may remove dependent packages."));
    m_confirm->setDestructive(true);
    m_confirm->setConfirmText(tr("Remove"));
    m_confirm->onAccepted = [this, name] {
        m_manageLog.clear();
        m_root->manageView()->setRunning(true);
        m_root->manageView()->setStatus(tr("Removing ") + name + "\xe2\x80\xa6");
        m_root->manageView()->setLogAvailable(false);
        m_removeOp.start(name,
            [this](std::string_view line) {
                m_manageLog += line; m_manageLog += '\n';
            },
            [this](bool ok, const std::string& msg) {
                m_root->manageView()->setRunning(false);
                m_root->manageView()->setStatus(msg);
                m_root->manageView()->setLogAvailable(!m_manageLog.empty());
                loadInstalledPackages();
            });
    };
    m_confirm->show();
}

void DebWindow::updatePackage(const std::string& name)
{
    m_manageLog.clear();
    m_root->manageView()->setRunning(true);
    m_root->manageView()->setStatus(tr("Updating package lists\xe2\x80\xa6"));
    m_root->manageView()->setLogAvailable(false);
    m_updateOp.start(name,
        [this](std::string_view line) {
            m_manageLog += line; m_manageLog += '\n';
            m_root->manageView()->setStatus(std::string(line));
        },
        [this](bool ok, const std::string& msg) {
            m_root->manageView()->setStatus(msg);
            m_root->manageView()->setLogAvailable(!m_manageLog.empty());
            loadInstalledPackages();
        });
}

void DebWindow::onTabSwitched(int tab)
{
    if (tab == 1)
        loadInstalledPackages();
}

void DebWindow::loadInstalledPackages()
{
    m_root->manageView()->setRunning(true);
    m_root->manageView()->setStatus(tr("Loading package list\xe2\x80\xa6"));
    m_listOp.start([this](std::vector<debinstaller::InstalledPackage> packages) {
        m_root->manageView()->setPackages(std::move(packages));
        m_root->manageView()->setStatus({});
    });
}

void DebWindow::loadPackageDetail(const std::string& name)
{
    m_detailOp.start(name, [this](debinstaller::PackageDetail detail) {
        m_root->manageView()->showDetail(detail, {});
    });
}

void DebWindow::showInstallLog()
{
    auto dlg = std::make_unique<OperationLogDialog>(&m_window, tr("Install Log"));
    for (const auto& line : splitLines(m_installLog))
        dlg->appendLine(line);
    m_installLogDlg = std::move(dlg);
    m_installLogDlg->show();
}

void DebWindow::showManageLog()
{
    auto dlg = std::make_unique<OperationLogDialog>(&m_window, tr("Operation Log"));
    for (const auto& line : splitLines(m_manageLog))
        dlg->appendLine(line);
    m_manageLogDlg = std::move(dlg);
    m_manageLogDlg->show();
}

std::vector<std::string> DebWindow::splitLines(const std::string& s)
{
    std::vector<std::string> lines;
    size_t pos = 0, next;
    while ((next = s.find('\n', pos)) != std::string::npos) {
        lines.push_back(s.substr(pos, next - pos));
        pos = next + 1;
    }
    if (pos < s.size())
        lines.push_back(s.substr(pos));
    return lines;
}

