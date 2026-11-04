// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <system_reminder>
#include <string>
#include <string_view>
#include <vector>

namespace debinstaller {

struct PackageInfo {
    bool ok = false;
    std::string error;
    std::string name;
    std::string version;
    std::string architecture;
    std::string maintainer;
    std::string section;
    std::string priority;
    std::string installedSize;   // human-readable, e.g. "1.2 MB"
    std::string depends;
    std::string recommends;
    std::string description;     // short (first line)
    std::string longDescription; // continuation lines
    std::string filePath;
};

struct InstalledPackage {
    std::string name;
    std::string version;
    std::string architecture;
    std::string description;
};

struct PackageDetail {
    bool ok = false;
    std::string error;
    std::string name;
    std::string status;           // e.g. "install ok installed"
    std::string longDescription;  // multi-line, may include " ." paragraph breaks
    std::string depends;
    std::string recommends;
    std::string suggests;
    std::string conflicts;
    std::string breaks_;
    std::string provides;
};

// Runs dpkg-deb --field synchronously; fast enough for the UI thread.
PackageInfo inspectDebFile(const std::string& path);

// Async listing: runs dpkg-query in the background and calls onFinished
// on the event loop thread.  Destroy the object to cancel.
class ListOperation
{
public:
    ListOperation();
    ~ListOperation();

    void start(std::function<void(std::vector<InstalledPackage>)> onFinished);
    bool isRunning() const;

private:
    struct Private;
    std::unique_ptr<Private> d;
};

// Async single-package detail: runs dpkg-query -s in the background.
class DetailOperation
{
public:
    DetailOperation();
    ~DetailOperation();

    void start(const std::string& packageName,
               std::function<void(PackageDetail)> onFinished);
    bool isRunning() const;

private:
    struct Private;
    std::unique_ptr<Private> d;
};

// Async install: runs pkexec dpkg -i <debPath>.
class InstallOperation
{
public:
    InstallOperation();
    ~InstallOperation();

    void start(const std::string& debPath,
               std::function<void(std::string_view line)> onOutput,
               std::function<void(bool ok, const std::string& message)> onFinished);
    void cancel();
    bool isRunning() const;

private:
    struct Private;
    std::unique_ptr<Private> d;
};

// Async remove: runs pkexec apt-get remove -y <name>.
class RemoveOperation
{
public:
    RemoveOperation();
    ~RemoveOperation();

    void start(const std::string& packageName,
               std::function<void(std::string_view line)> onOutput,
               std::function<void(bool ok, const std::string& message)> onFinished);
    void cancel();
    bool isRunning() const;

private:
    struct Private;
    std::unique_ptr<Private> d;
};

// Async two-step upgrade: first runs pkexec apt-get update, then
// pkexec apt-get install --only-upgrade -y <name>.
class UpdateOperation
{
public:
    UpdateOperation();
    ~UpdateOperation();

    void start(const std::string& packageName,
               std::function<void(std::string_view line)> onOutput,
               std::function<void(bool ok, const std::string& message)> onFinished);
    void cancel();
    bool isRunning() const;
    // "Updating package lists…" or "Upgrading <name>…"
    const std::string& currentStep() const;

private:
    struct Private;
    std::unique_ptr<Private> d;
};

} // namespace debinstaller
