#include "sylar/sylar.h"
sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int g_count = 0;

// sylar::RWMutex s_mutex;
sylar::Mutex s_mutex;

void fun1()
{
    SYLAR_LOG_INFO(g_logger) << "thread_name:" << sylar::Thread::GetName()
                             << "  this.name:" << sylar::Thread::GetThis()->getName()
                             << "  thread_id:" << sylar::GetTheadId()
                             << "  this.id:" << sylar::Thread::GetThis()->getId();
    for (int i = 0; i < 1000000; ++i)
    {
        // sylar::RWMutex::WriteLock lock(s_mutex);
        sylar::Mutex::Lock lock(s_mutex);
        ++g_count;
    }
}

void fun2()
{
    int num = 10;
    while (num--)
    {
        SYLAR_LOG_INFO(g_logger) << "++++++++++++++++++++++++";
    }
}

void fun3()
{
    int num = 10;
    while (num--)
    {
        SYLAR_LOG_INFO(g_logger) << "########################";
    }
}
void fun4()
{
    int num = 10;
    while (num--)
    {
        SYLAR_LOG_INFO(g_logger) << "========================";
    }
}

int main()
{
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node root = YAML::LoadFile("/home/coding/cpp/sylar/bin/conf/temp.yml");
    /*
    std::vector<sylar::Thread::ptr> thrs;

    sylar::Config::LoadFromYaml(root);

    for (int i = 0; i < 3; ++i)
    {
        sylar::Thread::ptr thr2(new sylar::Thread(&fun2, "name_" + std::to_string(i)));
        sylar::Thread::ptr thr3(new sylar::Thread(&fun3, "name_" + std::to_string(i)));
        sylar::Thread::ptr thr4(new sylar::Thread(&fun4, "name_" + std::to_string(i)));
        thrs.push_back(thr2);
        thrs.push_back(thr3);
        thrs.push_back(thr4);
    }

    for (size_t i = 0; i < thrs.size(); ++i)
    {
        thrs[i]->join();
    }

    // //SYLAR_LOG_INFO(g_logger) << "g_count=" << g_count;

    */

    sylar::Config::Visit(
        [](sylar::ConfigVarBase::ptr var)
        {
            SYLAR_LOG_INFO(g_logger) << "name=" << var->getName()
                                     << " description=" << var->getDescription()
                                     << " typename=" << var->getTypeName()
                                     << " value=" << var->toString();
        });

    SYLAR_LOG_INFO(g_logger) << "thread test end";
    return 0;
}