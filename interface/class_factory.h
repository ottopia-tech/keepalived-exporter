#pragma once

#include <memory>

class ILogger;

class ClassFactory
{
public:
    ~ClassFactory() = default;
    static const ClassFactory& Get();
    std::shared_ptr<ILogger> CreateFileLogger(const std::string& file_name) const;

private:
    ClassFactory() = default;
    static ClassFactory* m_instance;

    mutable std::shared_ptr<ILogger> m_logger;
};
