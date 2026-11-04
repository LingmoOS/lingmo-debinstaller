// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "debbackend.h"
#include "ui/deb_root.h"
#include "ui/operation_log_dialog.h"

#include "base/i18n.h"
#include "ui/dialog_window.h"
#include "ui/file_chooser.h"
#include "ui/window.h"

#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

class DebWindow
{
public:
    DebWindow();

    Window& window() { return m_window; }

    void openDebFile(const std::string& path);

private:
    void openWithChooser();
    void installPackage();
    void removePackage(const std::string& name);
    void updatePackage(const std::string& name);
    void loadInstalledPackages();
    void loadPackageDetail(const std::string& name);
    void onTabSwitched(int tab);
    void showInstallLog();
    void showManageLog();

    static std::vector<std::string> splitLines(const std::string& s);

    Window m_window;
    DebRoot* m_root = nullptr;

    std::unique_ptr<FileChooser>         m_chooser;
    std::unique_ptr<ConfirmDialog>       m_confirm;
    std::unique_ptr<OperationLogDialog>  m_installLogDlg;
    std::unique_ptr<OperationLogDialog>  m_manageLogDlg;

    debinstaller::PackageInfo      m_currentPackage;
    debinstaller::InstallOperation m_installOp;
    debinstaller::RemoveOperation  m_removeOp;
    debinstaller::UpdateOperation  m_updateOp;
    debinstaller::ListOperation    m_listOp;
    debinstaller::DetailOperation  m_detailOp;
    std::string m_installLog;
    std::string m_manageLog;
};
