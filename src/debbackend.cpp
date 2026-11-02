// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debbackend.h"

#include "base/process.h"

#include <algorithm>
#include <chrono>
#include <sstream>

using namespace Lingmo::base;

namespace debinstaller {

static std::string trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return {};
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string humanSize(long long kbytes)
{
    if (kbytes <= 0)     return {};
    if (kbytes < 1024)   return std::to_string(kbytes) + " KB";
    if (kbytes < 1024 * 1024) return std::to_string(kbytes / 1024) + " MB";
    return std::to_string(kbytes / (1024 * 1024)) + " GB";
}

PackageInfo inspectDebFile(const std::string& path)
{
    PackageInfo info;
    info.filePath = path;

    Process proc;
    if (!proc.start("dpkg-deb", {"--field", path})) {
        info.error = "Could not run dpkg-deb";
        return info;
    }
    if (!proc.waitForFinished(std::chrono::seconds(10))) {
        info.error = "Timed out reading package information";
        return info;
    }

    std::istringstream stream(proc.standardOutput());
    std::string line;
    std::string currentField, currentValue;

    auto commit = [&]() {
        if (currentField.empty())
            return;
        const std::string key = currentField;
        const std::string val = trim(currentValue);
        if      (key == "Package")        info.name = val;
        else if (key == "Version")        info.version = val;
        else if (key == "Architecture")   info.architecture = val;
        else if (key == "Maintainer")     info.maintainer = val;
        else if (key == "Section")        info.section = val;
        else if (key == "Priority")       info.priority = val;
        else if (key == "Installed-Size") {
            try { info.installedSize = humanSize(std::stoll(val)); }
            catch (...) { info.installedSize = val + " KB"; }
        }
        else if (key == "Depends")        info.depends = val;
        else if (key == "Recommends")     info.recommends = val;
        else if (key == "Description") {
            size_t nl = val.find('\n');
            if (nl == std::string::npos) {
                info.description = val;
            } else {
                info.description = val.substr(0, nl);
                info.longDescription = val.substr(nl + 1);
            }
        }
        currentField.clear();
        currentValue.clear();
    };

    while (std::getline(stream, line)) {
        if (!line.empty() && (line[0] == ' ' || line[0] == '\t')) {
            if (!currentValue.empty())
                currentValue += '\n';
            currentValue += trim(line);
        } else {
            commit();
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                currentField = trim(line.substr(0, colon));
                currentValue = trim(line.substr(colon + 1));
            }
        }
    }
    commit();

    info.ok = !info.name.empty();
    if (!info.ok)
        info.error = "Could not read package information from this file";
    return info;
}

std::vector<InstalledPackage> listInstalledPackages()
{
    std::vector<InstalledPackage> packages;
    Process proc;
    if (!proc.start("dpkg-query", {
            "-W",
            "-f=${db:Status-Abbrev}|${Package}|${Version}|${Architecture}|${Description:Summary}\\n"
        }))
        return packages;
    if (!proc.waitForFinished(std::chrono::seconds(30)))
        return packages;

    std::istringstream stream(proc.standardOutput());
    std::string line;
    while (std::getline(stream, line)) {
        if (line.size() < 2 || line[0] != 'i' || line[1] != 'i')
            continue;
        size_t p0 = line.find('|');
        if (p0 == std::string::npos) continue;
        size_t p1 = line.find('|', p0 + 1);
        if (p1 == std::string::npos) continue;
        size_t p2 = line.find('|', p1 + 1);
        if (p2 == std::string::npos) continue;
        size_t p3 = line.find('|', p2 + 1);
        if (p3 == std::string::npos) continue;

        InstalledPackage pkg;
        pkg.name         = line.substr(p0 + 1, p1 - p0 - 1);
        pkg.version      = line.substr(p1 + 1, p2 - p1 - 1);
        pkg.architecture = line.substr(p2 + 1, p3 - p2 - 1);
        pkg.description = line.substr(p3 + 1);
        packages.push_back(std::move(pkg));
    }

    std::sort(packages.begin(), packages.end(),
              [](const InstalledPackage& a, const InstalledPackage& b) {
                  return a.name < b.name;
              });
    return packages;
}

PackageDetail getPackageDetail(const std::string& name)
{
    PackageDetail d;
    Process proc;
    if (!proc.start("dpkg-query", {"-s", name})) {
        d.error = "Could not run dpkg-query";
        return d;
    }
    if (!proc.waitForFinished(std::chrono::seconds(10))) {
        d.error = "Timed out reading package status";
        return d;
    }

    d.ok = true;
    d.name = name;
    std::istringstream stream(proc.standardOutput());
    std::string line;
    std::string field, value;
    auto commit = [&]() {
        if (field.empty()) return;
        if      (field == "Status")  d.status = trim(value);
        else if (field == "Depends") d.depends = trim(value);
        else if (field == "Recommends") d.recommends = trim(value);
        else if (field == "Suggests")  d.suggests = trim(value);
        else if (field == "Conflicts") d.conflicts = trim(value);
        else if (field == "Breaks")   d.breaks_ = trim(value);
        else if (field == "Provides") d.provides = trim(value);
        else if (field == "Description") d.longDescription = trim(value);
        field.clear();
        value.clear();
    };
    while (std::getline(stream, line)) {
        if (!line.empty() && line[0] == ' ') {
            value += '\n';
            value += trim(line);
        } else {
            commit();
            auto colon = line.find(':');
            if (colon != std::string::npos) {
                field = trim(line.substr(0, colon));
                value = trim(line.substr(colon + 1));
            }
        }
    }
    commit();
    return d;
}

// --- Helpers for async operations ---

static void processLines(std::string& buffer, std::string_view data,
                         std::function<void(std::string_view)>& onOutput)
{
    buffer.append(data);
    size_t pos;
    while ((pos = buffer.find('\n')) != std::string::npos) {
        std::string_view line = std::string_view(buffer).substr(0, pos);
        if (!line.empty() && onOutput)
            onOutput(line);
        buffer.erase(0, pos + 1);
    }
}

static std::string finishMessage(int exitCode, bool crashed,
                                 const char* success, const char* fail)
{
    if (!crashed && exitCode == 0)
        return success;
    if (crashed)
        return std::string(fail) + " (process crashed)";
    return std::string(fail) + " (exit code " + std::to_string(exitCode) + ")";
}

// --- InstallOperation ---

struct InstallOperation::Private {
    Process process;
    std::string outBuf, errBuf;
    std::function<void(std::string_view)> onOutput;
    std::function<void(bool, const std::string&)> onFinished;
};

InstallOperation::InstallOperation() : d(std::make_unique<Private>()) {}
InstallOperation::~InstallOperation() = default;

void InstallOperation::start(const std::string& debPath,
                              std::function<void(std::string_view)> onOutput,
                              std::function<void(bool, const std::string&)> onFinished)
{
    d->onOutput = std::move(onOutput);
    d->onFinished = std::move(onFinished);
    d->outBuf.clear();
    d->errBuf.clear();

    d->process.onStandardOutput = [this](std::string_view data) {
        processLines(d->outBuf, data, d->onOutput);
    };
    d->process.onStandardError = [this](std::string_view data) {
        processLines(d->errBuf, data, d->onOutput);
    };
    d->process.onFinished = [this](int code, bool crashed) {
        bool ok = !crashed && code == 0;
        auto msg = finishMessage(code, crashed,
                                 "Installation completed successfully", "Installation failed");
        if (d->onFinished)
            d->onFinished(ok, msg);
    };
    d->process.start("pkexec", {"dpkg", "-i", debPath});
}

void InstallOperation::cancel() { d->process.terminate(); }
bool InstallOperation::isRunning() const { return d->process.isRunning(); }

// --- RemoveOperation ---

struct RemoveOperation::Private {
    Process process;
    std::string outBuf, errBuf;
    std::function<void(std::string_view)> onOutput;
    std::function<void(bool, const std::string&)> onFinished;
};

RemoveOperation::RemoveOperation() : d(std::make_unique<Private>()) {}
RemoveOperation::~RemoveOperation() = default;

void RemoveOperation::start(const std::string& packageName,
                             std::function<void(std::string_view)> onOutput,
                             std::function<void(bool, const std::string&)> onFinished)
{
    d->onOutput = std::move(onOutput);
    d->onFinished = std::move(onFinished);
    d->outBuf.clear();
    d->errBuf.clear();

    d->process.onStandardOutput = [this](std::string_view data) {
        processLines(d->outBuf, data, d->onOutput);
    };
    d->process.onStandardError = [this](std::string_view data) {
        processLines(d->errBuf, data, d->onOutput);
    };
    d->process.onFinished = [this](int code, bool crashed) {
        bool ok = !crashed && code == 0;
        auto msg = finishMessage(code, crashed,
                                 "Package removed successfully", "Removal failed");
        if (d->onFinished)
            d->onFinished(ok, msg);
    };
    d->process.start("pkexec", {"apt-get", "remove", "-y", packageName});
}

void RemoveOperation::cancel() { d->process.terminate(); }
bool RemoveOperation::isRunning() const { return d->process.isRunning(); }

// --- UpdateOperation — two-step: apt-get update, then install --only-upgrade ---

struct UpdateOperation::Private {
    Process process;
    std::string packageName;
    std::string outBuf, errBuf;
    std::string step;   // current human-readable step
    std::function<void(std::string_view)> onOutput;
    std::function<void(bool, const std::string&)> onFinished;

    void startUpgrade()
    {
        step = "Upgrading " + packageName + "…";
        outBuf.clear();
        errBuf.clear();
        process.onStandardOutput = [this](std::string_view data) {
            processLines(outBuf, data, onOutput);
        };
        process.onStandardError = [this](std::string_view data) {
            processLines(errBuf, data, onOutput);
        };
        process.onFinished = [this](int code, bool crashed) {
            bool ok = !crashed && code == 0;
            auto msg = finishMessage(code, crashed,
                                     "Package updated successfully", "Update failed");
            if (onFinished)
                onFinished(ok, msg);
        };
        process.start("pkexec", {"apt-get", "install", "--only-upgrade", "-y", packageName});
    }
};

UpdateOperation::UpdateOperation() : d(std::make_unique<Private>()) {}
UpdateOperation::~UpdateOperation() = default;

void UpdateOperation::start(const std::string& packageName,
                              std::function<void(std::string_view)> onOutput,
                              std::function<void(bool, const std::string&)> onFinished)
{
    d->packageName = packageName;
    d->onOutput    = std::move(onOutput);
    d->onFinished  = std::move(onFinished);
    d->step        = "Updating package lists…";
    d->outBuf.clear();
    d->errBuf.clear();

    // Step 1: apt-get update
    d->process.onStandardOutput = [this](std::string_view data) {
        processLines(d->outBuf, data, d->onOutput);
    };
    d->process.onStandardError = [this](std::string_view data) {
        processLines(d->errBuf, data, d->onOutput);
    };
    d->process.onFinished = [this](int code, bool crashed) {
        if (crashed || code != 0) {
            // apt-get update failed — still attempt upgrade
            if (d->onOutput)
                d->onOutput("Warning: apt-get update failed, attempting upgrade anyway");
        }
        d->startUpgrade();
    };
    d->process.start("pkexec", {"apt-get", "update"});
}

void UpdateOperation::cancel()
{
    d->process.terminate();
}

bool UpdateOperation::isRunning() const { return d->process.isRunning(); }

const std::string& UpdateOperation::currentStep() const { return d->step; }

} // namespace debinstaller
