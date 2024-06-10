#include "bytearray.h"
#include "endian.h"
#include <string.h>

namespace sylar
{
    // Node => ByteArray
    ByteArray::Node::Node(size_t s)
        : ptr(new char[s]), size(s), next(nullptr)
    {
    }

    ByteArray::Node::Node()
        : ptr(nullptr), size(0), next(nullptr)
    {
    }
    ByteArray::Node::~Node()
    {
        if (ptr)
        {
            delete[] ptr;
        }
    }

    // byteArray

    ByteArray::ByteArray(size_t base_size = 4096) // 每块链表长度 4k
        : m_baseSize(base_size), m_position(0), m_capacity(base_size), m_size(0),
          m_endian(SYLAR_BIG_ENDIAN), m_root(new Node(base_size)), m_cur(m_root)
    {
    }
    ByteArray::~ByteArray()
    {
        Node *tmp = m_root;
        while (tmp)
        {
            m_cur = tmp;
            tmp = tmp->next;
            delete m_cur;
        }
    }

    // write  ：protobuf形式压缩数据
    /// @brief 写入固定长度int/uint类型的数据
    void ByteArray::writeFint8(int8_t value)
    {
        write(&value, sizeof(value));
    }

    void ByteArray::writeFuint8(uint8_t value)
    {
        write(&value, sizeof(value));
    }

    void ByteArray::writeFint16(int16_t value)
    {
        // 先判断传入数据字节序 是否是 当前系统使用字节序
        if (m_endian != SYLAR_BYTE_ORDER)
        {
            value = byteswap(value);
        }

        write(&value, sizeof(value));
    }

    void ByteArray::writeFuint16(uint16_t value)
    {
        if (m_endian != SYLAR_BYTE_ORDER)
        {
            value = byteswap(value);
        }

        write(&value, sizeof(value));
    }

    void ByteArray::writeFint32(int32_t value)
    {
        if (m_endian != SYLAR_BYTE_ORDER)
        {
            value = byteswap(value);
        }

        write(&value, sizeof(value));
    }

    void ByteArray::writeFuint32(uint32_t value)
    {
        if (m_endian != SYLAR_BYTE_ORDER)
        {
            value = byteswap(value);
        }

        write(&value, sizeof(value));
    }

    void ByteArray::writeFint64(int64_t value)
    {
        if (m_endian != SYLAR_BYTE_ORDER)
        {
            value = byteswap(value);
        }

        write(&value, sizeof(value));
    }

    void ByteArray::writeFuint64(uint64_t value)
    {
        if (m_endian != SYLAR_BYTE_ORDER)
        {
            value = byteswap(value);
        }

        write(&value, sizeof(value));
    }

    // 有符号转无符号
    // 格式转化
    static uint32_t EncodeZigzag32(const int32_t &v)
    {
        if (v < 0)
        {
            return ((uint32_t)(-v)) * 2 - 1;
        }
        else
        {
            return v * 2;
        }
    }

    // 可变长度的int/uint类型的数据 ：： 压缩
    // 压缩算法
    void ByteArray::writeInt32(int32_t value)
    {
        writeUint32(EncodeZigzag32(value));
    }
    void ByteArray::writeUint32(uint32_t value)
    {
        uint8_t tmp[5];
        uint8_t i = 0;
        while (value >= 0x80)
        {
            tmp[i++] = (value & 0x7f) | 0x80;
            value >>= 7;
        }
        tmp[i++] = value;
        write(tmp, i);
    }

    static uint32_t EncodeZigzag64(const int64_t &v)
    {
        if (v < 0)
        {
            return ((uint64_t)(-v)) * 2 - 1;
        }
        else
        {
            return v * 2;
        }
    }
    void ByteArray::writeInt64(int64_t value)
    {
        writeUint64(EncodeZigzag64(value));
    }
    void ByteArray::writeUint64(uint64_t value)
    {
        uint8_t tmp[10];
        uint8_t i = 0;
        while (value >= 0x80)
        {
            tmp[i++] = (value & 0x7f) | 0x80;
            value >>= 7;
        }
        tmp[i++] = value;
        write(tmp, i);
    }

    // 固定浮点型类型的数据 - float - 单精度
    void ByteArray::writeFloat(float value)
    {
        uint32_t v;
        memcpy(&v, &value, sizeof(value));
        writeFuint32(v);
    }
    //              --> double 双精度
    void ByteArray::writeDouble(double value)
    {
        uint64_t v;
        memcpy(&v, &value, sizeof(value));
        writeFuint64(v);
    }

    // 固定字符串型数据 string
    // length: int16,   data
    void ByteArray::writeStringF16(const std::string &value)
    {
    }
    // length: int32,   data
    void ByteArray::writeStringF32(const std::string &value)
    {
    }
    // length: int64,   data
    void ByteArray::writeStringF64(const std::string &value)
    {
    }
    // length: varint,  data
    void ByteArray::writeStringVint(const std::string &value)
    {

    } // 表示压缩长度 -- 根据长度压缩一个int进去
    // data 没有length
    void ByteArray::writeStringWithoutLength(const std::string &value)
    {
    }

    // read
    int8_t ByteArray::readFint8()
    {
    }
    uint8_t ByteArray::readFuint8()
    {
    }
    int16_t ByteArray::readFint16()
    {
    }
    uint16_t ByteArray::readFuint16()
    {
    }
    int32_t ByteArray::readFint32()
    {
    }
    uint32_t ByteArray::readFuint32()
    {
    }
    int64_t ByteArray::readFint64()
    {
    }
    uint64_t ByteArray::readFuint64()
    {
    }

    int32_t ByteArray::readInt32()
    {
    }
    int32_t ByteArray::readUint32()
    {
    }
    int64_t ByteArray::readInt64()
    {
    }
    int64_t ByteArray::readUint64()
    {
    }

    float ByteArray::readFloat()
    {
    }
    double ByteArray::readDouble()
    {
    }

    // length: 16,  data
    std::string ByteArray::readStringF16()
    {
    }
    // length: 32,  data
    std::string ByteArray::readStringF32()
    {
    }
    // length: 64,  data
    std::string ByteArray::readStringF64()
    {
    }
    // length: varint,  data
    std::string ByteArray::readStringVint()
    {
    }

    // 内部操作
    void ByteArray::clear()
    {
    }

    void ByteArray::write(const void *buf, size_t size)
    {
    }
    void ByteArray::read(char *buf, size_t size)
    {
    }

    bool ByteArray::writeToFile(const std::string &name) const
    {
    }
    void ByteArray::readFromFile(const std::string &name)
    {
    }

    // 小端 - 网络字节序
    bool ByteArray::isLittleEndian() const
    {
        return m_endian == SYLAR_LITTLE_ENDIAN;
    }
    // 设置大端小端
    void ByteArray::setLittleEndian(bool v)
    {
        if (v)
        {
            m_endian = SYLAR_LITTLE_ENDIAN;
        }
        else
        {
            m_endian = SYLAR_BIG_ENDIAN;
        }
    }

    /// @brief 扩容ByteArray,使其可以容纳size个数据(如果原本可以可以容纳,则不扩容)
    void ByteArray::addCapacity(size_t size)
    {
    }

}