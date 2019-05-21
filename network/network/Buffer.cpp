#include "Buffer.h"

Buffer::Buffer()
	:buffer_(NULL)
	,capacity_(0)
	,sustain_(false)
	,initSize_(0)
	,dataSize_(0)
	,beginIndex_(0)
	,readIndex_(0)
	,notAlloc_(false)
{

}

Buffer::Buffer(size_t initSize, bool sustain)
	:buffer_(NULL)
	, capacity_(0)
	, sustain_(sustain)
	, initSize_(initSize)
	, dataSize_(0)
	, beginIndex_(0)
	, readIndex_(0)
	, notAlloc_(false)
{
	reallocate(initSize);
}

Buffer::Buffer(const Buffer& rhs)
	:buffer_(new char(rhs.capacity_))
	, capacity_(rhs.capacity_)
	, sustain_(rhs.sustain_)
	, initSize_(rhs.initSize_)
	, dataSize_(rhs.dataSize_)
	, beginIndex_(rhs.beginIndex_)
	, readIndex_(rhs.readIndex_)
	, notAlloc_(rhs.notAlloc_)
{
	memcpy(buffer_, rhs.buffer_, rhs.capacity_);
}

Buffer& Buffer::operator=(const Buffer& rhs)
{
	if (this!=&rhs)
	{
		if (buffer_!=NULL)
		{
			delete[] buffer_;
		}
		buffer_ = (new char(rhs.capacity_));
		capacity_ = rhs.capacity_;
		sustain_ = rhs.sustain_;
		initSize_ = rhs.initSize_;
		dataSize_ = rhs.dataSize_;
		beginIndex_ = rhs.beginIndex_;
		readIndex_ = rhs.readIndex_;
		notAlloc_ = rhs.notAlloc_;
		memcpy(buffer_, rhs.buffer_, rhs.capacity_);
	}

	return *this;
}

Buffer::Buffer(const Buffer&& rhs)
	:buffer_(rhs.buffer_)
	, capacity_(rhs.capacity_)
	, sustain_(rhs.sustain_)
	, initSize_(rhs.initSize_)
	, dataSize_(rhs.dataSize_)
	, beginIndex_(rhs.beginIndex_)
	, readIndex_(rhs.readIndex_)
	, notAlloc_(rhs.notAlloc_)
{
	rhs.buffer_ = NULL;
}

Buffer& Buffer::operator=(const Buffer&& rhs)
{
	if (this != &rhs)
	{
		if (buffer_ != NULL)
		{
			delete[] buffer_;
		}
		buffer_ = rhs.buffer_;
		capacity_ = rhs.capacity_;
		sustain_ = rhs.sustain_;
		initSize_ = rhs.initSize_;
		dataSize_ = rhs.dataSize_;
		beginIndex_ = rhs.beginIndex_;
		readIndex_ = rhs.readIndex_;
		notAlloc_ = rhs.notAlloc_;
		rhs.buffer_ = NULL;
	}

	return *this;
}

Buffer::~Buffer()
{
	if (notAlloc_)
	{
		return;
	}
	if (buffer_!=NULL)
	{
		delete[] buffer_;
	}
}

Buffer& Buffer::operator+(const Buffer& rhs)
{
	append(rhs.data(), rhs.size());
	return *this;
}

Buffer& Buffer::operator+=(const Buffer& rhs)
{
	append(rhs.data(), rhs.size());
	return *this;
}


const char* Buffer::data() const
{
	if (buffer_==NULL)
	{
		return NULL;
	}
	return buffer_ + beginIndex_;
}

size_t Buffer::size() const
{
	if (buffer_==NULL)
	{
		return 0;
	}
	return dataSize_;
}



void Buffer::discard(size_t len)
{
	if (len==0)
	{
		return;
	}
	if (len>=dataSize_)
	{
		len = dataSize_;
	}
	dataSize_ -= len;
	beginIndex_ += len;
	deallocate(dataSize_);
}

void Buffer::clear()
{
	dataSize_ = 0;
	readIndex_ = 0;
	beginIndex_ = 0;
	deallocate(0);
}

char* Buffer::beginWrite(size_t position)
{
	if (buffer_ == NULL)
	{
		return NULL;
	}
	return buffer_ + beginIndex_ + position;
}

void Buffer::ensureWrite(size_t requestSize)
{
	if (requestSize>0)
	{
		reallocate(requestSize + dataSize_);
		dataSize_ += requestSize;
	}
}

void Buffer::unwrite(size_t len)
{
	if (len>dataSize_)
	{
		len = dataSize_;
	}
	dataSize_ -= len;
	deallocate(dataSize_);
}

bool Buffer::read(void* ptr, size_t size)
{
	if (size>0)
	{
		if (size > dataSize_)
		{
			throw DATA_LACK;
			return false;
		}

		dataSize_ = size;
		memcpy(ptr, buffer_ + beginIndex_, size);
		beginIndex_ += size;
	}
	deallocate(dataSize_);
	return true;
}

std::string Buffer::readString(size_t len)
{
	if (len>0)
	{
		std::string s;
		s.resize(len);
		if (read(const_cast<char*>(s.data()),len))
		{
			return s;
		}
	}
	return std::string();
}



Buffer Buffer::readBuffer(size_t size)
{
	Buffer rhs;
	rhs.append(data(), size);
	dataSize_ -= size;
	beginIndex_ += size;
	deallocate(size);
	return rhs;
}

void Buffer::append(const void* data, size_t size)
{
	if (size>0)
	{
		reallocate(size + dataSize_);
		memcpy(buffer_ + beginIndex_ + dataSize_, data, data);
		dataSize_ += size;
	}
}

void Buffer::append(struct evbuffer* data, size_t size, bool sustain = false)
{
	if (size > 0)
	{
		reallocate(size + dataSize_);
		if (!sustain)
		{
			evbuffer_remove(data, buffer_ + beginIndex_ + dataSize_, size);
		}
		else
		{
			evbuffer_copyout(data, buffer_ + beginIndex_ + dataSize_, size);
		}
		dataSize_ += size;
	}
}

size_t Buffer::reallocate(size_t requestSize)
{
	if (notAlloc_)
	{
		return 0;
	}
	if (requestSize<=capacity_-beginIndex_)
	{
		return 0;
	}

	if (requestSize<= capacity_)
	{
		if (dataSize_>0)
		{
			memmove(buffer_, buffer_ + beginIndex_, dataSize_);
		}
		beginIndex_ = 0;
		return capacity_;
	}

	size_t newSize=capacity_;
	if (newSize<PAGE_SIZE)
	{
		newSize = PAGE_SIZE;
	}

	while (requestSize>newSize)
	{
		newSize *= 2;
	}

	char* newBuf = new char[newSize];
	if (buffer_)
	{
		if (dataSize_)
		{
			memmove(newBuf, buffer_ + beginIndex_, dataSize_);
		}
		delete[] buffer_;
	}

	buffer_ = newBuf;
	capacity_ = newSize;
	beginIndex_ = 0;
	return capacity_;
}

size_t Buffer::deallocate(size_t requestSize)
{
	if (requestSize<=0)
	{
		return 0;
	}
	if (notAlloc_)
	{
		return 0;
	}
	if (sustain_)
	{
		return 0;
	}
	if (capacity_<=initSize_)
	{
		return 0;
	}
	if (requestSize<dataSize_)
	{
		requestSize = initSize_;
	}

	size_t newSize = capacity_;
	while (newSize>=requestSize*2)
	{
		newSize /= 2;
	}

	if (newSize==capacity_)
	{
		return 0;
	}

	char* newBuf = new char[newSize];
	if (buffer_)
	{
		if (dataSize_)
		{
			memmove(newBuf, buffer_ + beginIndex_, dataSize_);
		}
		delete[] buffer_;
	}

	buffer_ = newBuf;
	capacity_ = newSize;
	beginIndex_ = 0;
	return capacity_;
}
