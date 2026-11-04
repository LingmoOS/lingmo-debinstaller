// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/i18n.h"

#include "ui/dialog_window.h"
#include "ui/window.h"
#include "ui/text_area.h"

#include <string>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

class OperationLogDialog
{
public:
    OperationLogDialog(Window* parent, std::string title)
        : m_dialog(parent, std::move(title), 640)
    {
        m_log = m_dialog.setContent<TextArea>();
        m_log->setReadOnly(true);
        m_log->setWrapMode(TextArea::WrapMode::WrapAnywhere);
        m_log->setFontSize(12);
        m_log->setPadding({8, 8, 8, 8});
        m_log->setFrameVisible(false);
        m_dialog.window()->resize({640, 480});
    }

    void appendLine(const std::string& line)
    {
        const std::string cur = m_log->text();
        m_log->setText(cur.empty() ? line : cur + "\n" + line);
    }

    void show() { m_dialog.show(); }
    void close() { m_dialog.close(); }

private:
    DialogWindow m_dialog;
    TextArea* m_log = nullptr;
};
