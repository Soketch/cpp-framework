#include <iostream>
#include "../sylar/config.h"
#include "../sylar/log.h"
#include "yaml-cpp/yaml.h"

#define YAML_TEST_FILE "/home/coding/cpp/sylar/bin/conf/log.yml"

sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::Config::Lookup("system.port", (int)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config =
    sylar::Config::Lookup("system.value", (float)10.82, "system value");

// 测试复杂类型 vector<int>
sylar::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config =
    sylar::Config::Lookup("system.int_vec", std::vector<int>{100, 20}, "system int_vector");

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

void test_sylar1()
{
    SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << g_int_value_config->toString();
    std::cout << " ------------------------" << std::endl;

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();
    std::cout << " ------------------------" << std::endl;
}

void test_config()
{
    SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << "before: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << "before: " << g_float_value_config->toString();

    auto v = g_int_vec_value_config->getValue();
    for (auto &i : v)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before vector<int>: " << i;
    }

    YAML::Node root = YAML::LoadFile("/home/coding/cpp/sylar/bin/conf/log.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << "after: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_lOG_ROOT()) << "after: " << g_float_value_config->toString();

    v = g_int_vec_value_config->getValue();
    for (auto &i : v)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after vector<int>: " << i;
    }
}

int main(int argc, char **argv)
{
    // test_sylar1();

    // test_yaml();

    test_config();

    return 0;
}