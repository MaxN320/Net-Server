#include "Buffer.h"

Buffer::Buffer()
{

}
Buffer::~Buffer()
{

}

void Buffer::append(const char *data,size_t size)
{
    buf_.append(data,size);
}


// 返回buf_的大小。
size_t Buffer::size()                                                            
{
    return buf_.size();
}

// 返回buf_的首地址。
const char *Buffer::data()                                                  
{
    return buf_.data();
}

// 清空buf_。
void Buffer::clear()                                                            
{
    buf_.clear();
}


// 从buf_的pos开始，删除nn个字节，pos从0开始。
void Buffer::erase(size_t pos,size_t nn)                             
{
    buf_.erase(pos,nn);
}
