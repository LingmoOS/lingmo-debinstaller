// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deb_window.h"

#include "base/i18n.h"
#include "ui/application.h"

#include <cstdio>
#include <string>

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

int main(int argc, char** argv)
{
    Application app(argc, argv);
    app.setApplicationId("lingmo-debinstaller");
    i18n::setDomain("lingmo-debinstaller");

    DebWindow win;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            std::printf("Usage: lingmo-debinstaller [FILE.deb]\nLingmo Deb Installer\n");
            return 0;
        }
        if (!arg.starts_with("-"))
            win.openDebFile(arg);
    }
    win.window().show();
    return app.run();
}
