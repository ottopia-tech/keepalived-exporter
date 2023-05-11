#include "file_logger.h"

#include "class_factory.h"

ClassFactory* ClassFactory::m_instance = nullptr;

const ClassFactory& ClassFactory::Get()
{
    if (m_instance == nullptr)
    {
        m_instance = new ClassFactory();
    }
    return *m_instance;
}

std::shared_ptr<ILogger> ClassFactory::CreateFileLogger(const std::string& file_name) const
{
    if (m_logger == nullptr)
    {
        m_logger = std::make_shared<FileLogger>("/tmp/" + file_name + ".log");
    }

    return m_logger;
}
