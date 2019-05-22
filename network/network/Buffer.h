#ifndef __BUFFER_H__
#define __BUFFER_H__
#include<cstddef>
#include<string>
#include<assert.h>
#include"boost/smart_ptr.hpp"

#define DATA_LACK -1
#define DATA_ERROR -2
#define PAGE_SHIFT 12
#define PAGE_SIZE (1<<PAGE_SHIFT)

struct evbuffer;

class Buffer
{
public:
	Buffer();
	Buffer(size_t initSize,bool sustain=false);
	Buffer(const Buffer& rhs);
	Buffer& operator=(const Buffer& rhs);

	Buffer(const Buffer&& rhs);
	Buffer& operator=(const Buffer&& rhs);

	~Buffer();

	Buffer& operator+(const Buffer& rhs);
	Buffer& operator+=(const Buffer& rhs);

	size_t capacity() const { return capacity_; }

	void swap(Buffer& rhs)
	{
		if (this!=&rhs)
		{
			std::swap(buffer_, rhs.buffer_);
			std::swap(capacity_, rhs.capacity_);
			std::swap(sustain_, rhs.sustain_);
			std::swap(initSize_, rhs.initSize_);
			std::swap(dataSize_, rhs.dataSize_);
			std::swap(beginIndex_, rhs.beginIndex_);
			std::swap(readIndex_, rhs.readIndex_);
			std::swap(notAlloc_, rhs.notAlloc_);
		}
	}

	const char* data() const;
	size_t size() const;

	bool empty() const
	{
		return buffer_ == NULL || dataSize_== 0;
	}

	void discard(size_t len);

	void clear();
	char* beginWrite(size_t position);
	void ensureWrite(size_t requestSize);
	void unwrite(size_t len);

	bool read(void* ptr, size_t size);

	std::string readString(size_t len);

	int8_t readInt8()
	{
		int8_t x;
		bool succ = read(&x, sizeof(x));
		assert(succ);
		return x;
	}

	int16_t readInt16()
	{
		int16_t x;
		bool succ = read(&x, sizeof(x));
		assert(succ);
		return x;
	}

	int32_t readInt32()
	{
		int32_t x;
		bool succ = read(&x, sizeof(x));
		assert(succ);
		return x;
	}

	int64_t readInt64()
	{
		int64_t x;
		bool succ = read(&x, sizeof(x));
		assert(succ);
		return x;
	}

	Buffer readBuffer(size_t size);
	void append(const void* data, size_t size);
	void append(struct evbuffer* data, size_t size, bool sustain = false);

	void appendString(const std::string& s){append(s.data(), s.size());}
	void appendInt8(int8_t x) { append(&x, sizeof(x)); }
	void appendInt16(int16_t x) { append(&x, sizeof(x)); }
	void appendInt32(int32_t x) { append(&x, sizeof(x)); }
	void appendInt64(int64_t x) { append(&x, sizeof(x)); }
	void appendBuffer(const Buffer& buf){append(buf.data(), buf.size());}

private:
	size_t reallocate(size_t requestSize);
	size_t deallocate(size_t requestSize);

private:
	char* buffer_;
	size_t capacity_;
	bool sustain_;

	size_t initSize_;
	size_t dataSize_;

	size_t beginIndex_;
	size_t readIndex_;

	bool notAlloc_;
};

typedef boost::shared_ptr<Buffer> BufferPtr;
#endif