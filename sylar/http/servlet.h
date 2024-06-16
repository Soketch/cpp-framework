/**   @brief Servlet封装
 */
#ifndef __SYLAR_HTTP_SERVLET_H_
#define __SYLAR_HTTP_SERVLET_H_

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include "http.h"
#include "http_session.h"
#include "sylar/thread.h"
#include "sylar/util.h"

namespace sylar
{
    namespace http
    {
        /**
         * @brief Servlet封装  抽象类
         */
        class Servlet
        {
        public:
            using ptr = std::shared_ptr<Servlet>;

            /**
             * @brief 构造函数
             * @param[in] name 名称
             */
            Servlet(const std::string &name) : m_name(name) {}

            ~Servlet() {}

            /**
             * @brief 处理请求
             * @param[in] request HTTP请求
             * @param[in] response HTTP响应
             * @param[in] session HTTP连接
             * @return 是否处理成功
             */
            virtual int32_t handle(sylar::http::HttpRequest::ptr request,
                                   sylar::http::HttpResponse::ptr response,
                                   sylar::http::HttpSession::ptr session) = 0;

            /**
             * @brief 返回Servlet名称
             */
            const std::string &getName() const { return m_name; }

        protected:
            /// Servlet名称
            std::string m_name;
        };

        /**
         * @brief 函数式Servlet
         */
        class FunctionServlet : public Servlet
        {
        public:
            using ptr = std::shared_ptr<FunctionServlet>;

            /// 函数回调类型定义
            using callback = std::function<int32_t(HttpRequest::ptr request,
                                                   HttpResponse::ptr response,
                                                   HttpSession::ptr session)>;

            /**
             * @brief 构造函数
             * @param[in] cb 回调函数
             */
            FunctionServlet(callback cb);

            /**
             * @brief 处理请求(FunctionServlet) 重写Servlet纯虚函数
             * @param[in] request HTTP请求
             * @param[in] response HTTP响应
             * @param[in] session HTTP连接
             * @return 是否处理成功
             */
            virtual int32_t handle(HttpRequest::ptr request,
                                   HttpResponse::ptr response,
                                   HttpSession::ptr session) override;

        private:
            // 回调函数
            callback m_cb;
        };

        /**
         * @brief Servlet分发器
         */

        class ServletDispatch : public Servlet
        {
        public:
            using ptr = std::shared_ptr<ServletDispatch>;
            using RWMutexType = RWMutex;

            /**
             * @brief 构造函数
             */
            ServletDispatch();

            /**
             * @brief 处理请求(ServletDispatch) 重写Servlet纯虚函数
             * @param[in] request HTTP请求
             * @param[in] response HTTP响应
             * @param[in] session HTTP连接
             * @return 是否处理成功
             */
            virtual int32_t handle(HttpRequest::ptr request,
                                   HttpResponse::ptr response,
                                   HttpSession::ptr session) override;

            /**
             * @brief 添加servlet-精准
             * @param[in] uri uri
             * @param[in] slt serlvet
             */
            void addServlet(const std::string &uri, Servlet::ptr slt);

            /**
             * @brief 添加servlet-(精准)
             * @param[in] uri uri
             * @param[in] cb FunctionServlet回调函数
             */
            void addServlet(const std::string &uri, FunctionServlet::callback cb);

            /**
             * @brief 添加模糊匹配servlet
             * @param[in] uri uri 模糊匹配 /sylar_*
             * @param[in] slt servlet
             */
            void addGlobServlet(const std::string &uri, Servlet::ptr slt);

            /**
             * @brief 添加模糊匹配servlet
             * @param[in] uri uri 模糊匹配 /sylar_*
             * @param[in] cb FunctionServlet回调函数
             */
            void addGlobServlet(const std::string &uri, FunctionServlet::callback cb);

            /**
             * @brief 删除servlet
             * @param[in] uri uri
             */
            void delServlet(const std::string &uri);

            /**
             * @brief 删除模糊匹配servlet
             * @param[in] uri uri
             */
            void delGlobServlet(const std::string &uri);

            /**
             * @brief 返回默认servlet
             */
            Servlet::ptr getDefault() const { return m_default; }

            void setDefault(Servlet::ptr v) { m_default = v; }

            /**
             * @brief 通过uri获取servlet
             * @param[in] uri uri
             * @return 返回对应的servlet
             */
            Servlet::ptr getServlet(const std::string &uri);

            /**
             * @brief 通过uri获取模糊匹配servlet
             * @param[in] uri uri
             * @return 返回对应的servlet
             */
            Servlet::ptr getGlobServlet(const std::string &uri);

            /**
             * @brief 通过uri获取servlet
             * @param[in] uri uri
             * @return 优先精准匹配,其次模糊匹配,最后返回默认
             */
            Servlet::ptr getMatchedServlet(const std::string &uri);

        private:
            /// 读写互斥量
            RWMutexType m_mutex;

            /// 精准匹配servlet MAP   优先命中精准匹配的
            /// uri(/sylar/xxx) -> servlet
            std::unordered_map<std::string, Servlet::ptr> m_datas;
            /// 模糊匹配servlet 数组
            /// uri(/sylar/*) -> servlet
            std::vector<std::pair<std::string, Servlet::ptr>> m_globs;

            /// 默认servlet，所有路径都没匹配到时使用
            Servlet::ptr m_default;
        };

        /**
         * @brief NotFoundServlet(默认返回404)
         */
        class NotFoundServlet : public Servlet
        {
        public:
            /// 智能指针类型定义
            typedef std::shared_ptr<NotFoundServlet> ptr;
            /**
             * @brief 构造函数
             */
            NotFoundServlet(const std::string &name);
            virtual int32_t handle(sylar::http::HttpRequest::ptr request, sylar::http::HttpResponse::ptr response, sylar::http::HttpSession::ptr session) override;

        private:
            std::string m_name;
            std::string m_content;
        };
    }
}
#endif