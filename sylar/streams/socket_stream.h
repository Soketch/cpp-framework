#ifndef __SYLAR_SOCKET_STREAM_H_
#define __SYLAR_SOCKET_STREAM_H_
/// Socket流式接口封装
#include "sylar/stream.h"
#include "sylar/socket.h"

namespace sylar
{
    /**
     * @brief Socket流
     */
    class SocketStream : public Stream
    {
    public:
        using ptr = std::shared_ptr<SocketStream>;

        /**
         * @brief 构造函数
         * @param[in] sock Socket类
         * @param[in] owner 是否完全控制
         */
        SocketStream(Socket::ptr sock, bool owner = true);

        /**
         * @brief 析构函数
         * @details 如果m_owner=true,则close
         */
        ~SocketStream();

        /**
         * @brief 读取数据
         * @param[out] buffer 待接收数据的内存
         * @param[in] length 待接收数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int read(void *buffer, size_t length) override;

        /**
         * @brief 读取数据
         * @param[out] ba 接收数据的ByteArray
         * @param[in] length 待接收数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int read(ByteArray::ptr ba, size_t length) override;

        /**
         * @brief 写入数据
         * @param[in] buffer 待发送数据的内存
         * @param[in] length 待发送数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int write(const void *buffer, size_t length) override;

        /**
         * @brief 写入数据
         * @param[in] ba 待发送数据的ByteArray
         * @param[in] length 待发送数据的内存长度
         * @return
         *      @retval >0 返回实际接收到的数据长度
         *      @retval =0 socket被远端关闭
         *      @retval <0 socket错误
         */
        virtual int write(ByteArray::ptr ba, size_t length) override;

        /**
         * @brief socket_stream -- 关闭socket
         */
        virtual void close() override;

        /**
         *  @brief socketstream,流操作的socket返回是否连接
         */
        bool isConnected() const;

        /**
         * @brief 返回Socket类
         */
        Socket::ptr getSocket() const { return m_sock; }

    private:
        Socket::ptr m_sock;
        /// 是否主控
        bool m_owner;
    };
}

#endif