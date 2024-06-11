#include "bytearray.h"
#include "endian.h"
#include <string.h>
#include <fstream>
#include "log.h"

namespace sylar
{

    static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

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

    // 反解码
    static int32_t DecodeZigzag32(const uint32_t &v)
    {
        return (v >> 1) ^ -(v & 1);
    }
    static int64_t DecodeZigzag64(const uint64_t &v)
    {
        return (v >> 1) ^ -(v & 1);
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
        writeFuint16(value.size());
        write(value.c_str(), value.size());
    }
    // length: int32,   data
    void ByteArray::writeStringF32(const std::string &value)
    {
        writeFuint32(value.size());
        write(value.c_str(), value.size());
    }
    // length: int64,   data
    void ByteArray::writeStringF64(const std::string &value)
    {
        writeFuint64(value.size());
        write(value.c_str(), value.size());
    }
    // length: varint,  data
    void ByteArray::writeStringVint(const std::string &value)
    {
        writeUint64(value.size());
        write(value.c_str(), value.size());
    }
    // 表示压缩长度 -- 根据长度压缩一个int进去
    // data 没有length
    void ByteArray::writeStringWithoutLength(const std::string &value)
    {
        write(value.c_str(), value.size());
    }

    // read
    int8_t ByteArray::readFint8()
    {
        int8_t v;
        read(&v, sizeof(v));
        return v;
    }
    uint8_t ByteArray::readFuint8()
    {
        uint8_t v;
        read(&v, sizeof(v));
        return v;
    }

#define XX(type)                      \
    type v;                           \
    read(&v, sizeof(v));              \
    if (m_endian == SYLAR_BYTE_ORDER) \
    {                                 \
        return v;                     \
    }                                 \
    else                              \
    {                                 \
        return byteswap(v);           \
    }

    int16_t ByteArray::readFint16()
    {
        XX(int16_t);
    }
    uint16_t ByteArray::readFuint16()
    {
        XX(uint16_t);
    }
    int32_t ByteArray::readFint32()
    {
        XX(int32_t);
    }
    uint32_t ByteArray::readFuint32()
    {
        XX(uint32_t);
    }
    int64_t ByteArray::readFint64()
    {
        XX(int64_t);
    }
    uint64_t ByteArray::readFuint64()
    {
        XX(uint64_t);
    }
#undef XX

    int32_t ByteArray::readInt32()
    {
        return DecodeZigzag32(readUint32());
    }
    int32_t ByteArray::readUint32()
    {
        // 解析
        uint32_t ret = 0;
        for (int i = 0; i < 32; i += 7)
        {
            uint8_t b = readFuint8();
            if (b < 0x80)
            {
                ret |= ((uint32_t)b) << i;
                break; // 没有下一个
            }
            else
            {
                ret |= (((uint32_t)(b & 0x7f)) << i);
            }
        }
        return ret;
    }

    int64_t ByteArray::readInt64()
    {
        return DecodeZigzag64(readUint64());
    }
    int64_t ByteArray::readUint64()
    {
        // 解析
        uint64_t ret = 0;
        for (int i = 0; i < 64; i += 7)
        {
            uint8_t b = readFuint8();
            if (b < 0x80)
            {
                ret |= ((uint64_t)b) << i;
                break; // 没有下一个
            }
            else
            {
                ret |= (((uint64_t)(b & 0x7f)) << i);
            }
        }
        return ret;
    }

    float ByteArray::readFloat()
    {
        uint32_t v = readFuint32();
        float value;
        memcpy(&value, &v, sizeof(value));
        return value;
    }
    double ByteArray::readDouble()
    {
        uint64_t v = readFuint64();
        double value;
        memcpy(&value, &v, sizeof(value));
        return value;
    }

    // length: 16,  data
    std::string ByteArray::readStringF16()
    {
        uint16_t len = readFuint16();
        std::string buf;
        buf.resize(len);

        read(&buf[0], len);
        return buf;
    }

    // length: 32,  data
    std::string ByteArray::readStringF32()
    {
        uint32_t len = readFuint32();
        std::string buf;
        buf.resize(len);

        read(&buf[0], len);
        return buf;
    }

    // length: 64,  data
    std::string ByteArray::readStringF64()
    {
        uint64_t len = readFuint64();
        std::string buf;
        buf.resize(len);

        read(&buf[0], len);
        return buf;
    }
    // length: varint,  data
    std::string ByteArray::readStringVint()
    {
        uint64_t len = readUint64();
        std::string buf;
        buf.resize(len);

        read(&buf[0], len);
        return buf;
    }

    // 内部操作
    void ByteArray::clear()
    {
        m_position = m_size = 0;
        m_capacity = m_baseSize;
        Node *tmp = m_root->next;
        while (tmp)
        {
            m_cur = tmp;
            tmp = tmp->next;
            delete m_cur;
        }
        m_cur = m_root;
        m_root->next = nullptr; // 只留下一个节点
    }

    void ByteArray::write(const void *buf, size_t size)
    {
        if (size == 0)
        {
            return;
        }
        addCapacity(size);

        size_t npos = m_position % m_baseSize;
        size_t ncap = m_cur->size - npos;
        size_t bpos = 0; // 记录buf写的位置

        while (size > 0)
        {
            if (ncap >= size)
            {
                memcpy(m_cur->ptr + npos, buf + bpos, size);

                if (m_cur->size == (npos + size))
                {
                    m_cur = m_cur->next;
                }

                m_position += size;
                bpos += size;
                size = 0;
            }
            else
            {
                memcpy(m_cur->ptr + npos, buf + bpos, ncap);
                m_position += ncap;
                bpos += ncap;
                size -= ncap;
                m_cur = m_cur->next;
                ncap = m_cur->size;
                npos = 0;
            }
        }
        if (m_position > m_size)
        {
            m_size = m_position;
        }
    }
    void ByteArray::read(void *buf, size_t size)
    {
        if (size > getReadSize())
        {
            throw std::out_of_range("not enough len.");
        }
        size_t npos = m_position % m_baseSize;
        size_t ncap = m_cur->size - npos;
        size_t bpos = 0;
        while (size > 0)
        {
            if (ncap >= size)
            {
                memcpy(buf + bpos, m_cur->ptr + npos, size);
                if (m_cur->size == (npos + size))
                {
                    m_cur = m_cur->next;
                }
                m_position += size;
                bpos += size;
                size = 0;
            }
            else
            {
                memcpy(buf + bpos, m_cur->ptr + npos, ncap);
                m_position += ncap;
                bpos += ncap;
                size -= ncap;
                m_cur = m_cur->next;
                ncap = m_cur->size;
                npos = 0;
            }
        }
    }
    void ByteArray::setPosition(size_t v)
    {
        if (v > m_size)
        {
            throw std::out_of_range("set_position out of range.");
        }
        m_position = v;
        m_cur = m_root;
        while (v > m_cur->size)
        {
            v -= m_cur->size;
            m_cur = m_cur->next;
        }
        if (v == m_cur->size)
        {
            m_cur = m_cur->next;
        }
    }

    bool ByteArray::writeToFile(const std::string &name) const
    {
        std::ofstream ofs;
        ofs.open(name, std::ios::trunc | std::ios::binary);
        if (!ofs)
        {
            SYLAR_LOG_ERROR(g_logger) << "writeToFile error, filename:" << name
                                      << ",  open fail,  errno=" << errno
                                      << ",  strerror:" << strerror(errno);
            return false;
        }

        int64_t readSize = getReadSize();
        int64_t pos = m_position;
        Node *cur = m_cur;

        while (readSize > 0)
        {
            int diff = pos % m_baseSize;
            int64_t len = (readSize > (int64_t)m_baseSize ? m_baseSize : readSize) - diff;

            ofs.write(cur->ptr + diff, len);
            cur = cur->next;
            pos += len;
            readSize -= len;
        }

        return true;
    }
    bool ByteArray::readFromFile(const std::string &name)
    {
        std::ifstream ifs;
        ifs.open(name, std::ios::binary);
        if (!ifs)
        {
            SYLAR_LOG_ERROR(g_logger) << "readFromFile error, filename:" << name
                                      << ",  open fail,  errno=" << errno
                                      << ",  strerror:" << strerror(errno);
            return false;
        }
        std::shared_ptr<char> buff(new char[m_baseSize], [](char *ptr)
                                   { delete[] ptr; });
        while (!ifs.eof())
        {
            ifs.read(buff.get(), m_baseSize);
            write(buff.get(), ifs.gcount());
        }
        return true;
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
        if (size == 0)
        {
            return;
        }

        int old_cap = getCapacity();
        if (old_cap >= size)
        {
            return;
        }

        size -= old_cap;
        size_t count = (size / m_baseSize) + (((size % m_baseSize) > old_cap) ? 1 : 0); // 新增的容量是节点容量的多少倍就加多少个，
        //                                                                              整除剩余的和节点还未使用的容量（剩余的）比较，超出就再增加一个节点
        Node *tmp = m_root;
        while (tmp->next)
        {
            tmp = tmp->next; // 找到末尾节点
        }

        Node *first = nullptr;
        for (size_t i = 0; i < count; ++i)
        {
            tmp->next = new Node(m_baseSize);
            if (first == nullptr)
            {
                first = tmp->next;
            }
            tmp = tmp->next;
            m_capacity += m_baseSize;
        }

        if (old_cap == 0)
        {
            m_cur = first;
        }
    }
}