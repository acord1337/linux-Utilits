#pragma once
#include <expected>
#include <string>
#include <vector>
#include <sys/types.h>

enum class ProcessError 
{
    AccessDenied,
    InvalidIdentifier,
    SourceUnavailable,
    ReadError,
    NotFound,
    NotFiltered,
};

class IProcessReader
{
public:
    virtual std::expected<std::string, ProcessError> readProcessComm(pid_t pid) const = 0;
    virtual std::expected<std::string, ProcessError> readProcessCmdline(pid_t pid) const = 0;
    virtual std::expected<std::vector<std::string>, ProcessError> readProcessMaps(pid_t pid) const = 0;

    virtual ~IProcessReader() = default;
};

class IProcessScanner 
{
public:
    virtual std::expected<std::vector<pid_t>, ProcessError> enumerateProcessPid() const = 0;

    virtual ~IProcessScanner() = default;
};