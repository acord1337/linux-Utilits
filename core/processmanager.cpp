#include "processmanager.h"
#include <filesystem>
#include <ranges>
#include <algorithm>
#include <fstream>

ProcessFinder::ProcessFinder(ProcessSource& source) : source(source) {}

/*
Перечесляет pid всех запущеных процессов, сканируя виртуальную фс /proc
возвращает: при успехе ветор с pid , при не успехе std::nullopt
*/
std::optional<std::vector<pid_t>> ProcessSource::enumerateProcessPid() const
{
    std::vector<pid_t> pids{};
    try
    {
        std::filesystem::path path = std::filesystem::path("/proc");

        if(!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
            return std::nullopt;

        for(const auto& entry : std::filesystem::directory_iterator(path))
        {
            if(!entry.exists() || !entry.is_directory())
                continue;

            std::string name = entry.path().filename().string();

            auto pid = parsePid(name);

            if(!pid)
                continue;

            if(!isValidDirProcess(name, *pid))
                continue;

            pids.push_back(*pid);
        }
    }
    catch(const std::exception&)
    {
        return std::nullopt;
    }
    return pids;
}

/*
Преобразует pid процесса из string в pid_t 
Возвращает pid при успешном преобразовании, или std::nullopt при ошибке или если stoi выкинет исключение
*/
std::optional<pid_t>ProcessSource::parsePid(const std::string& name) const
{
    try
    {
        return static_cast<pid_t>(std::stoi(name));
    }
    catch(const std::exception&)
    {
        return std::nullopt;
    }
    return std::nullopt;
}

/*
Проверяет является ли имя директории в /proc числом и доступность процесса через чтение cmdline
Возвращает true, если имя валидно и процесс доступен для чтения
*/
bool ProcessSource::isValidDirProcess(const std::string& name, pid_t pid) const 
{
    if(name.empty() || pid <= 0 || name.size() > 10)
        return false;

    return isDigitPid(name) && canReadCmdline(pid);
}

/*
Проверяет является ли pid числом 
Возвращает true если pid является числом
*/
bool ProcessSource::isDigitPid(const std::string& name) const 
{
    if(name.empty())
        return false;

    return std::ranges::all_of(name, [](unsigned char c){ return std::isdigit(c); });
}

/*
Читает 1 байт /proc/pid/cmdline
Используется для проверки жив-ли процесс и используется- ли он пользователем
Возвращает true если чтение удалось, false при ошибке или неудачном чтении
*/
bool ProcessSource::canReadCmdline(pid_t pid) const
{
    try
    {
        if(pid <= 0)
            return false;

        std::filesystem::path cmdPath = std::filesystem::path("/proc") / std::to_string(pid) / "cmdline";

        if(!std::filesystem::exists(cmdPath) || !std::filesystem::is_regular_file(cmdPath))
            return false;

        std::ifstream file(cmdPath, std::ios::binary);

        if(!file.is_open())
            return false;

        char firstByteName;

        if(!(file >> firstByteName))
            return false;

        return true;
    }
    catch(std::exception&) { return false; }
}



std::optional<std::vector<std::string>> ProcessSource::readProcessMaps(pid_t pid) const {
    return std::nullopt;
}

/*
Ищет pid и все похожие имена процесса по тому , что введут в параметры функции в name
Возвращает вектор структур о процессе(имя и пид) , при ошибке или если ничего не нашло - nullopt
*/
std::optional<std::vector<ProcessInfo>>ProcessFinder::searhProcessInfoByName(const std::string& name) const 
{
   if(name.empty())
       return std::nullopt;

   std::vector<ProcessInfo> infoProcess{};

   auto pids = source.enumerateProcessPid();

   if (!pids)
        return std::nullopt;

    for(const auto& pid : *pids)
    {
        auto nameProcess = source.matchesProcessName(pid, name);

        if(nameProcess)
        {
            infoProcess.push_back({*nameProcess, pid});
        }
    }
    if(infoProcess.empty())
        return std::nullopt;
        
    return infoProcess;
}

/*
Фильтрует процессы, читая их в comm и cmdline 
Возвращает имя из comm если найденны совпадения и std::nullopt при ошибке или ничего не найдено
*/
std::optional<std::string> ProcessSource::matchesProcessName(pid_t pid, const std::string& name) const
{
    if(pid <= 0)
        return std::nullopt;

    auto nameCmdLine = readProcessCmdline(pid);
    auto nameComm = readProcessComm(pid);

    if(!nameCmdLine || !nameComm || !canReadCmdline(pid) || nameCmdLine->empty() || nameComm->empty())
        return std::nullopt;

    if (!filterProcessCmdLine(*nameCmdLine))
        return std::nullopt;

    if(nameCmdLine->find(name) != std::string::npos && nameComm->find(name) != std::string::npos)
    {
        return *nameComm;
    }
    return std::nullopt;
}

/*
Читает имя процесса через /proc/pid/comm, убрезается до 16 символов
Возвращает имя если чтение удалось и std::nullopt при неудачном чтении
*/
std::optional<std::string> ProcessSource::readProcessComm(pid_t pid) const 
{
    try
    {
        if(pid <= 0)
            return std::nullopt;

        std::filesystem::path commPath = std::filesystem::path("/proc") / std::to_string(pid) / "comm";

        if(!std::filesystem::exists(commPath) || !std::filesystem::is_regular_file(commPath))
            return std::nullopt;

        std::ifstream file(commPath);

        if(!file.is_open())
            return std::nullopt;

        std::string nameProc;

        if(std::getline(file, nameProc))
        {
            if(!nameProc.empty())
                return nameProc;
        }
        else
        {
            return std::nullopt;
        }
    }
    catch(std::exception& e) { return std::nullopt;}

    return std::nullopt;
}

/*
Читаем имя процесса через /proc/pid/cmdline
Возвращает имя если чтение удалось и std::nullopt при неудачном чтение
*/
std::optional<std::string> ProcessSource::readProcessCmdline(pid_t pid) const
{
    try
    {
        if(pid <= 0)
            return std::nullopt;

        std::filesystem::path path = std::filesystem::path("/proc") / std::to_string(pid) / "cmdline";

        std::ifstream file(path);

        if(!file.is_open())
            return std::nullopt;

        std::string fullName;
        std::string bufferName;

        while(std::getline(file, bufferName, '\0'))
        {
            fullName += bufferName + " ";
        }
        if(fullName.empty())
            return std::nullopt;
        return fullName;
    }
    catch(std::exception& e) { return std::nullopt; }
}

/*
Фильтрует служебные скрипты в cmdline
Возвращает false если процесс служебный и true если нет
*/
bool ProcessSource::filterProcessCmdLine(const std::string& name) const
{
    if(name.empty()) return false;
    if(name.find("/bin/sh") != std::string::npos) return false;
    if(name.find("python") != std::string::npos) return false;

    return true;
}