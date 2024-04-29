#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <memory>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <string>
namespace sylar
{
    // 配置基类  -- 用于配置基本信息（包含转换方法）
    class ConfigVarBase
    {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;

        ConfigVarBase(const std::string name, const std::string desc = "") : m_name(name), m_description(desc) {}
        virtual ~ConfigVarBase() {}
        const std::string getName() const { return m_name; }
        const std::string getDescription() const { return m_description; }

        protected:
        std::string m_name;
        std::string m_description; // 描述

    private:
    };
}

#endif