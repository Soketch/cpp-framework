#include <iostream>
#include "../sylar/config.h"
#include "../sylar/log.h"
#include "yaml-cpp/yaml.h"

#define YAML_TEST_FILE "/home/coding/cpp/sylar/bin/conf/log.yml"

sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::Config::Lookup("system.port", (int)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config =
    sylar::Config::Lookup("system.port", (float)10.82, "system port");

void print_yaml(const YAML::Node &node, int level)
{
    if (node.IsScalar())
    {
        SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << std::string(level * 4, ' ')
                                         << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if (node.IsNull())
    {
        SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << std::string(level * 4, ' ')
                                         << "Null - " << node.Type() << " - " << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << std::string(level * 4, ' ')
                                             << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); ++i)
        {
            SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << std::string(level * 4, ' ')
                                             << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

// yaml 测试
void test_yaml()
{
    YAML::Node root = YAML::LoadFile(YAML_TEST_FILE);
    print_yaml(root, 0);
}

void test_config()
{
}

int main(int argc, char **argv)
{
    SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << g_int_value_config->toString();
    std::cout << " ------------------------" << std::endl;

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();
    std::cout << " ------------------------" << std::endl;

    test_yaml();

    return 0;
}