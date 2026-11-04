// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/i18n.h"

#include "ui/file_drop_area.h"
#include "ui/painter.h"
#include "ui/theme.h"

#include <functional>
#include <string>
#include <vector>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace {

static std::string urlToPath(const std::string& url)
{
    const std::string prefix = "file://";
    if (url.starts_with(prefix))
        return url.substr(prefix.size());
    return url;
}

class DropZone : public FileDropArea
{
public:
    explicit DropZone(std::function<void(const std::string& path)> onFile)
        : m_onFile(std::move(onFile))
    {
        filter = [](const std::string& url) {
            return url.ends_with(".deb");
        };
        onDropped = [this](const std::vector<std::string>& urls) {
            for (const auto& url : urls) {
                const std::string path = urlToPath(url);
                if (!path.empty() && m_onFile)
                    m_onFile(path);
            }
        };
        onContainsDragChanged = [this] { requestPaint(); };
        setLayoutHints({.fillWidth = true, .fillHeight = true});
    }

protected:
    void paint(Painter& painter) override
    {
        if (!containsDrag())
            return;
        const Theme& t = Theme::current();
        painter.fillRect(localRect(), t.highlight.withAlpha(0.10));
        painter.strokeRoundedRect(localRect().adjusted(4, 4, -4, -4),
                                  t.mediumRadius, accent::blue, painter.borderWidth(2));
    }

private:
    std::function<void(const std::string&)> m_onFile;
};

} // namespace
