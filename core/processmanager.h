#pragma once
#include <string>
#include <vector>
#include <sys/types.h>
#include <exception>

struct ProcessInfo
{
    std::string name;
    pid_t pid = 0;
};


class IProcessReader
{
public:
    virtual std::optional<std::string> readProcessComm(pid_t pid) const = 0;
    virtual std::optional<std::string> readProcessCmdline(pid_t pid) const = 0;
    virtual std::optional<std::vector<std::string>> readProcessMaps(pid_t pid) const = 0;

    virtual ~IProcessReader() = default;
};

class IProcessValidator
{
public:
 virtual std::optional<pid_t> parsePid(const std::string& name) const = 0;
 virtual bool isDigitPid(const std::string& name) const = 0;
 virtual bool isValidDirProcess(const std::string& name, pid_t pid) const = 0;
 virtual bool filterProcessCmdLine(const std::string& name) const = 0;
 virtual bool canReadCmdline(pid_t pid) const = 0;
 virtual std::optional<std::string> matchesProcessName(pid_t pid, const std::string& name) const = 0;

 virtual ~IProcessValidator() = default;
};

class IProcessManager
{
public:
    virtual std::optional<std::vector<pid_t>> enumerateProcessPid() const = 0;

    virtual ~IProcessManager() = default;
};


class ProcessSource : public IProcessReader,
                      public IProcessValidator,
                      public IProcessManager
{
public:
    std::optional<std::vector<pid_t>> enumerateProcessPid() const override;
    std::optional<std::string> readProcessComm(pid_t pid) const override;
    std::optional<std::string> readProcessCmdline(pid_t pid) const override;
    std::optional<std::vector<std::string>> readProcessMaps(pid_t pid) const override;
    std::optional<std::string> matchesProcessName(pid_t pid, const std::string& name) const override;
private:
    bool isValidDirProcess(const std::string& name, pid_t pid) const override;
    bool isDigitPid(const std::string& name) const override;
    bool canReadCmdline(pid_t pid) const override;
    bool filterProcessCmdLine(const std::string& name) const override;
    std::optional<pid_t> parsePid(const std::string& name) const override;
};


class ProcessFinder
{
public:
    explicit ProcessFinder(ProcessSource& source);

    std::optional<std::vector<ProcessInfo>>searhProcessInfoByName(const std::string& name) const;
private:
 ProcessSource& source;
};