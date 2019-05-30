#ifndef __BYTEBUF_H__
#define __BYTEBUF_H__
#include"Buffer.h"
#include<string>
#include<event.h>
#include "boost/noncopyable.hpp"


class Bytebuf :private boost::noncopyable
{
public:
	typedef boost::shared_ptr<std::string> StringPtr;

	Bytebuf()
		:alloc_(true)
		,evbuffer_(NULL)
	{
		evbuffer_ = evbuffer_new();
		assert(evbuffer_ != NULL);
	}

	Bytebuf(struct evbuffer* ef,bool managed=true)
		:alloc_(managed)
		, evbuffer_(ef)
	{
		assert(evbuffer_ != NULL);
	}

	Bytebuf(Bytebuf&& rhs)
		:alloc_(rhs.alloc_)
		,evbuffer_(rhs.evbuffer_)
	{
		rhs.evbuffer_ = NULL;
	}

	~Bytebuf()
	{
		if (alloc_&& evbuffer_!=NULL)
		{
			evbuffer_free(evbuffer_);
		}
	}

	Bytebuf& operator=(Bytebuf&& rhs)
	{
		if (this!=&rhs)
		{
			if (evbuffer_!=NULL)
			{
				evbuffer_free(evbuffer_);
			}
			alloc_ = rhs.alloc_;
			evbuffer_ = rhs.evbuffer_;
			rhs.evbuffer_ = NULL;
		}
		return *this;
	}

	void swap(Bytebuf& rhs)
	{
		if (this != &rhs)
		{
			std::swap(alloc_, rhs.alloc_);
			std::swap(evbuffer_, rhs.evbuffer_);
		}
	}

	bool internalNullPointer() const { return evbuffer_==NULL; }
	bool empty() const { return readalbeBytes() == 0; }
	size_t readalbeBytes() const { return evbuffer_get_length(evbuffer_); }
	bool discardBytest(size_t len) const { return evbuffer_drain(evbuffer_, len) == 0; }

	ev_ssize_t readBytes(void* buf, size_t len) const
	{
		return static_cast<ev_ssize_t>(evbuffer_remove(evbuffer_, buf, len));
	}

	ev_ssize_t readBytes(Buffer& buf) const
	{
		size_t bytes = readalbeBytes();
		buf.append(evbuffer_, bytes);
		return static_cast<ev_ssize_t>(bytes);
	}

	ev_ssize_t readBytes(Buffer& buf, size_t len) const
	{
		size_t bytes = readalbeBytes();
		size_t readLen = bytes > len ? len : bytes;
		buf.append(evbuffer_, readLen);
		return static_cast<ev_ssize_t>(readLen);
	}

	ev_ssize_t readBytes(std::string& data) const
	{
		size_t bytes = readalbeBytes();
		size_t oldLen = data.size();
		data.resize(oldLen + bytes);
		return static_cast<ev_ssize_t>(evbuffer_remove(evbuffer_, const_cast<char*>(data.data()+oldLen), bytes));
	}


	ev_ssize_t readBytes(std::string& data,size_t len) const
	{
		size_t bytes = readalbeBytes();
		size_t readLen = bytes > len ? len : bytes;
		size_t oldLen = data.size();
		data.resize(oldLen + readLen);
		return static_cast<ev_ssize_t>(evbuffer_remove(evbuffer_, const_cast<char*>(data.data() + oldLen), bytes));
	}

	ev_ssize_t readBytes(Bytebuf& rhs) const
	{
		return static_cast<ev_ssize_t>(evbuffer_remove_buffer(evbuffer_, rhs.evbuffer_, readalbeBytes()));
	}

	ev_ssize_t readBytes(Bytebuf& rhs,size_t len) const
	{
		size_t bytes = readalbeBytes();
		size_t readLen = bytes  > len ? len : bytes;
		return static_cast<ev_ssize_t>(evbuffer_remove_buffer(evbuffer_, rhs.evbuffer_, readLen));
	}

	ev_ssize_t peekBytes(void* buf, size_t len) const
	{
		return static_cast<ev_ssize_t>(evbuffer_copyout(evbuffer_, buf, len));
	}

	ev_ssize_t peekBytes(Buffer& buf) const
	{
		size_t bytes = readalbeBytes();
		buf.append(evbuffer_, bytes, true);
		return static_cast<ev_ssize_t>(bytes);
	}

	ev_ssize_t peekBytes(Buffer& buf,size_t len) const
	{
		size_t bytes = readalbeBytes();
		size_t readLen = bytes  > len ? len : bytes;
		buf.append(evbuffer_, readLen, true);
		return static_cast<ev_ssize_t>(readLen);
	}

	ev_ssize_t peekBytes(std::string& data) const
	{
		size_t bytes = readalbeBytes();
		size_t oldLen = data.size();
		data.resize(oldLen + bytes);
		return static_cast<ev_ssize_t>(evbuffer_copyout(evbuffer_,const_cast<char*>(data.data()+oldLen), bytes));
	}

	ev_ssize_t peekBytes(std::string& data,size_t len) const
	{
		size_t bytes = readalbeBytes();
		size_t readLen = bytes > len ? len : bytes;
		size_t oldLen = data.size();
		data.resize(oldLen + bytes);
		return static_cast<ev_ssize_t>(evbuffer_copyout(evbuffer_, const_cast<char*>(data.data() + oldLen), bytes));
	}

	ev_ssize_t peekBytes(Bytebuf& rhs) const
	{
		size_t bytes = readalbeBytes();
		Buffer* ref = new Buffer(bytes);
		ref->append(evbuffer_, bytes, true);
		evbuffer_add_reference(rhs.evbuffer_, ref->data(), ref->size(), evbuffer_ref_clean_cb<Buffer>, ref);
		return bytes;
	}

	ev_ssize_t peekBytes(Bytebuf& rhs,size_t len) const
	{
		size_t bytes = readalbeBytes();
		size_t readLen = bytes > len ? len : bytes;
		Buffer* ref = new Buffer(readLen);
		ref->append(evbuffer_, bytes, true);
		evbuffer_add_reference(rhs.evbuffer_, ref->data(), ref->size(), evbuffer_ref_clean_cb<Buffer>, ref);
		return readLen;
	}

	void append(const void* data, size_t dataLen) const
	{
		evbuffer_add(evbuffer_, data, dataLen);
	}

	void append(const std::string& data) const
	{
		evbuffer_add(evbuffer_, data.data(), data.size());
	}

	void append(const Buffer& buffer) const
	{
		evbuffer_add(evbuffer_, buffer.data(), buffer.size());
	}

	void append(const Bytebuf& rhs) const
	{
		evbuffer_add_buffer(evbuffer_, rhs.evbuffer_);
	}

	void appendReference(std::string&& data) const
	{
		std::string* ref = new std::string(std::move(data));
		evbuffer_add_reference(evbuffer_, ref->data(), ref->size(), evbuffer_ref_clean_cb<Buffer>, ref);
	}

	void appendReference(Buffer&& data) const
	{
		Buffer* ref = new Buffer(std::move(data));
		evbuffer_add_reference(evbuffer_, ref->data(), ref->size(), evbuffer_ref_clean_cb<Buffer>, ref);
	}

	void appendReference(Bytebuf&& rhs) const
	{
		Bytebuf safe(std::move(rhs));
		evbuffer_add_buffer_reference(evbuffer_, safe.evbuffer_);
	}

	void appendReference(std::string* data, bool collect = false) const
	{
		std::string* ref = data;
		if (!collect)
		{
			ref = new std::string();
			ref->swap(*data);
		}
		evbuffer_add_reference(evbuffer_, ref->data(), ref->size(), evbuffer_ref_clean_cb<std::string>, ref);
	}

	void appendReference(Buffer* buf, bool collect = false) const
	{
		Buffer* ref = buf;
		if (!collect)
		{
			ref = new Buffer();
			ref->swap(*buf);
		}
		evbuffer_add_reference(evbuffer_, ref->data(), ref->size(), evbuffer_ref_clean_cb<Buffer>, ref);
	}


	void appendReference(const BufferPtr& buf) const
	{
		BufferPtr* ref = new BufferPtr(buf);
		evbuffer_add_reference(evbuffer_, (*ref)->data(), (*ref)->size(), evbuffer_ref_clean_cb<Buffer>, ref);
	}


	void appendReference(const StringPtr& data) const
	{
		StringPtr* ref = new StringPtr(data);
		evbuffer_add_reference(evbuffer_, (*ref)->data(), (*ref)->size(), evbuffer_ref_clean_cb<Buffer>, ref);
	}

	class Unsafe :boost::noncopyable
	{
	public:
		Unsafe(Bytebuf* internalBuf)
			:internalBuf_(internalBuf)
		{

		}

		~Unsafe(){}

		struct evbuffer* internalBuffer() const 
		{
			return internalBuf_->evbuffer_;
		}
	private:
		Bytebuf* internalBuf_;
	};
private:
	template<typename T>
	static void evbuffer_ref_clean_cb(const void* data, size_t dataLen, void* garbage)
	{
		T* ref = static_cast<T*>(garbage);
		delete ref;
	}
private:
	bool alloc_;
	struct evbuffer* evbuffer_;
};
#endif
