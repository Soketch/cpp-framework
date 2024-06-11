#ifndef __SYLAR_BYTEARRAY_H_
#define __SYLAR_BYTEARRAY_H_

#include <memory>
#include <string>

/**
 * @brief 二进制数组,提供基础类型的序列化,反序列化功能
 */
namespace sylar
{
    class ByteArray
    {
    public:
        using ptr = std::shared_ptr<ByteArray>;

        ///  @brief ByteArray的存储节点
        struct Node
        {
            Node(size_t s);
            Node();
            ~Node();

            char *ptr;
            size_t size;
            Node *next;
        };

        ByteArray(size_t basr_size = 4096); // 每块链表长度 4k
        ~ByteArray();

        // write  ：protobuf形式压缩数据
        /// @brief 写入固定长度int/uint类型的数据
        void writeFint8(int8_t value);

        void writeFuint8(uint8_t value);

        void writeFint16(int16_t value);

        void writeFuint16(uint16_t value);

        void writeFint32(int32_t value);

        void writeFuint32(uint32_t value);

        void writeFint64(int64_t value);

        void writeFuint64(uint64_t value);

        // 可变长度的int/uint类型的数据 ：： 压缩类型
        void writeInt32(int32_t value);
        void writeUint32(uint32_t value);
        void writeInt64(int64_t value);
        void writeUint64(uint64_t value);

        // 固定浮点型类型的数据 - float - 单精度
        void writeFloat(float value);
        //              --> double 双精度
        void writeDouble(double value);

        // 固定字符串型数据 string
        // length: int16,   data
        void writeStringF16(const std::string &value);
        // length: int32,   data
        void writeStringF32(const std::string &value);
        // length: int64,   data
        void writeStringF64(const std::string &value);
        // length: varint,  data
        void writeStringVint(const std::string &value); // 表示压缩长度 -- 根据长度压缩一个int进去
        // data 没有length
        void writeStringWithoutLength(const std::string &value);

        // read
        int8_t readFint8();
        uint8_t readFuint8();
        int16_t readFint16();
        uint16_t readFuint16();
        int32_t readFint32();
        uint32_t readFuint32();
        int64_t readFint64();
        uint64_t readFuint64();

        int32_t readInt32();
        int32_t readUint32();
        int64_t readInt64();
        int64_t readUint64();

        float readFloat();
        double readDouble();

        // length: 16,  data
        std::string readStringF16();
        // length: 32,  data
        std::string readStringF32();
        // length: 64,  data
        std::string readStringF64();
        // length: varint,  data
        std::string readStringVint();

        // 内部操作
        void clear();

        void write(const void *buf, size_t size);
        void read(void *buf, size_t size);

        size_t getPosition() const { return m_position; }
        void setPosition(size_t v);

        bool writeToFile(const std::string &name) const;
        bool readFromFile(const std::string &name);

        size_t getBaseSize() const { return m_baseSize; }

        // 获取剩余的可读数据大小
        size_t getReadSize() const { return m_size - m_position; }

        // 小端 - 网络字节序
        bool isLittleEndian() const;
        void setLittleEndian(bool v);

    private:
        /// @brief 扩容ByteArray,使其可以容纳size个数据(如果原本可以可以容纳,则不扩容)
        void addCapacity(size_t size);

        /// @brief 获取当前的可写入容量
        size_t getCapacity() const { return m_capacity - m_position; }

    private:
        size_t m_baseSize; // 内存块的大小
        size_t m_position; // 当前操作位置
        size_t m_capacity; // 当前总容量
        size_t m_size;     // 当前数据大小

        // 考虑网络字节序， 大端小端
        int8_t m_endian;

        Node *m_root; // 第一个内存块节点（指针）
        Node *m_cur;  // 当前的内存块节点（指针）
    };
}

#endif