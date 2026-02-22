#pragma once
#include <string>
#include <vector>
#include <sys/types.h>
#include <expected>
#include <string_view>

struct ProcessInfo
{
    std::string name;
    pid_t pid = 0;
};

enum class ProcessError 
{
    AccessDenied,
    InvalidIdentifier,
    SourceUnavailable,
    ReadError,
    NotFound,
    NotFiltered
};

class IProcessReader
{
public:
    virtual std::expected<std::string, ProcessError> readProcessComm(pid_t pid) const = 0;
    virtual std::expected<std::string, ProcessError> readProcessCmdline(pid_t pid) const = 0;
    virtual std::expected<std::vector<std::string>, ProcessError> readProcessMaps(pid_t pid) const = 0;

    virtual ~IProcessReader() = default;
};

class IProcessValidator
{
public:
    virtual std::expected<pid_t, ProcessError> parsePid(const std::string& name) const = 0;;
    virtual bool isValidDirProcess(pid_t pid) const = 0;
    virtual bool filterProcessCmdLine(const std::string& line) const = 0;
    virtual bool canReadCmdline(pid_t pid) const = 0;
    virtual std::expected<std::string, ProcessError> matchesProcessName(pid_t pid, const std::string& name) const = 0;

    virtual ~IProcessValidator() = default;
};

class IProcessManager
{
public:
    virtual std::expected<std::vector<pid_t>, ProcessError> enumerateProcessPid() const = 0;
    virtual std::expected<std::string, ProcessError> tolowerString(std::string& line) const = 0;

    virtual ~IProcessManager() = default;
};


class ProcessSource : public IProcessReader,
                    public IProcessValidator,
                    public IProcessManager
{
public:
    std::expected<std::vector<pid_t>, ProcessError> enumerateProcessPid() const override;
    std::expected<std::string, ProcessError> tolowerString(std::string& line) const override;
    std::expected<std::string, ProcessError> readProcessComm(pid_t pid) const override;
    std::expected<std::string, ProcessError> readProcessCmdline(pid_t pid) const override;
    std::expected<std::vector<std::string>, ProcessError> readProcessMaps(pid_t pid) const override;
    std::expected<std::string, ProcessError> matchesProcessName(pid_t pid, const std::string& name) const override;
private:
    bool isValidDirProcess(pid_t pid) const override;
    bool canReadCmdline(pid_t pid) const override;
    bool filterProcessCmdLine(const std::string& line) const override;
    std::expected<pid_t, ProcessError> parsePid(const std::string& name) const override;
};


class ProcessFinder
{
public:
    explicit ProcessFinder(ProcessSource& source);

    std::expected<std::vector<ProcessInfo>, ProcessError>searhProcessInfoByFilter(std::string& name) const;
private:
    ProcessSource& source;
};