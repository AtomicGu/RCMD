#pragma once

/* global configure */
#include "platform.h"
#define HEADER_TYPE INT64

/* interface headers */
#include <exception>

namespace eztcp
{
	namespace err
	{
		//
		// 基本错误类
		//
		class BaseError :public std::exception
		{
		public:
			BaseError(int code) :
				_code(code)
			{}
			virtual int what()
			{
				return _code;
			}

		private:
			int _code;
		};

		//
		// Socket错误类
		//
		class SocketError : public BaseError
		{
		public:
			SocketError(int code) :
				BaseError(code)
			{}
		};

		//
		// 文件IO错误类
		//
		class FileIOError : public BaseError
		{
		public:
			FileIOError(int code) :
				BaseError(code)
			{}
		};
	}

	//
	// 包的统一接口
	//
	class EZTCP_API Package
	{
	public:
		virtual HEADER_TYPE size_left() const = 0;
		virtual SIZET extract(void* buffer_p, SIZET buffer_size) = 0;
	};

	//
	// 内存包（不可拷贝、不可移动）（简单地包装内存空间，当心内存泄漏和指针悬挂）
	//
	class EZTCP_API MemPack : public Package
	{
	public:
		MemPack(const void* src_p, SIZET size);
		MemPack(const MemPack& cpy) = delete;

	public:
		MemPack& operator=(const MemPack& cpy) = delete;

	public:
		virtual HEADER_TYPE size_left() const { return _size_left; }
		virtual SIZET extract(void* buffer_p, SIZET buffer_size);

	private:
		const char* _src_p;
		const char* _src_end_p;
		SIZET _size_left;

	private:
		MemPack(MemPack&& mov) noexcept;
		MemPack& operator=(MemPack&& mov) noexcept;

	private:
		friend class BlockPack;
		friend class Session;
	};

	//
	// 块包（在MemPack基础上添加内存管理的功能）
	//
	class RecvPack;
	class EZTCP_API BlockPack :public MemPack
	{
	public:
		BlockPack();
		BlockPack(const void* src_p, SIZET size);
		BlockPack(RecvPack& recv_pack);
		BlockPack(const BlockPack& cpy);
		BlockPack(BlockPack&& mov) noexcept;
		~BlockPack() noexcept;

	public:
		BlockPack& operator=(const BlockPack& cpy);
		BlockPack& operator=(BlockPack&& mov) noexcept;

	public:
		char* get_buf() { return const_cast<char*>(_src_p); }
	};

	//
	// 文件包（将文件包装为包）
	//
	class EZTCP_API FilePack :public Package
	{
	public:
		FilePack(const char* file_path_ps);
		FilePack(RecvPack& recv_pack, const char* save_path_ps);
		FilePack(const FilePack& cpy) = delete;
		FilePack(FilePack&& mov) = delete;
		~FilePack() noexcept;

	public:
		FilePack& operator=(const FilePack& cpy) = delete;
		FilePack& operator=(FilePack&& mov) = delete;

	public:
		virtual HEADER_TYPE size_left() const { return _size_left; }
		virtual SIZET extract(void* buffer_p, SIZET buffer_size);

#ifdef _WINDOWS
#ifdef EZTCP_EXPORTS
	private:
		HANDLE _file_h;
		char* _file_path_ps;
		HEADER_TYPE _size_left;
#else
	private:
		void* _file_h;
		char* _file_path_ps;
		HEADER_TYPE _size_left;
#endif
#endif

#ifdef __linux
#ifdef EZTCP_EXPORTS
	private:
		int _file_h;
		char* _file_path_ps;
		HEADER_TYPE _size_left;
#else
	private:
		int _file_h;
		char* _file_path_ps;
		HEADER_TYPE _size_left;
#endif
#endif
	private:
		friend class Session;
	};

	//
	// 接收包
	//
	class EZTCP_API RecvPack :public Package
	{

	public:
		RecvPack(const RecvPack& cpy) = delete;

	public:
		RecvPack& operator=(const RecvPack& cpy) = delete;

	public:
		virtual HEADER_TYPE size_left() const { return _size_left; };
		virtual SIZET extract(void* buffer_p, SIZET buffer_size);

#ifdef _WINDOWS
#ifdef EZTCP_EXPORTS
	private:
		SOCKET _s;
		HEADER_TYPE _size_left;

	private:
		RecvPack();
		RecvPack(SOCKET s);

	private:
		RecvPack& operator=(RecvPack&& mov) noexcept;

#else
	private:
		void* _s;
		HEADER_TYPE _size_left;
#endif
#endif

#ifdef __linux
#ifdef EZTCP_EXPORTS
	private:
		int _s;
		HEADER_TYPE _size_left;

	private:
		RecvPack();

	private:
		RecvPack(int s);
		RecvPack& operator=(RecvPack&& mov) noexcept;

#else
	private:
		void* _s;
		HEADER_TYPE _size_left;
#endif
#endif

	private:
		friend class BlockPack;
		friend class Session;
	};

	//
	// 会话类
	//
	class EZTCP_API Session
	{
	public:
		Session();
		Session(const char* ip_ps, unsigned short port, int buffer_size = 8192);
		Session(const Session& cpy) = delete;
		Session(Session&& mov) noexcept;
		~Session();

	public:
		Session& operator=(const Session& cpy) = delete;
		Session& operator=(Session&& mov) noexcept;

	public:
		RecvPack& recv();
		void send(Package& package);
		void send(MemPack& package);
		void send(FilePack& package);
		unsigned short get_port() const;
		const char* get_ip() const;

#ifdef _WINDOWS
#ifdef EZTCP_EXPORTS
	public:
		Session(SOCKET s_listening, int buffer_size);

	private:
		SOCKET _s;
		char* _buffer_p;
		int _buffer_size;
		RecvPack _recving;
		sockaddr_in _s_addr;
#else
	private:
		void* _s;
		char* _buffer_p;
		int _buffer_size;
		RecvPack _recving;
		char _s_addr[16];
#endif
#endif

#ifdef __linux
#ifdef EZTCP_EXPORTS
	public:
		Session(int s_listening, int buffer_size);

	private:
		int _s;
		char* _buffer_p;
		size_t _buffer_size;
		RecvPack _recving;
		sockaddr_in _s_addr;
#else
	private:
		int _s;
		char* _buffer_p;
		size_t _buffer_size;
		RecvPack _recving;
		char _s_addr[16];
#endif
#endif
	};

	//
	// 监听类
	//
	class EZTCP_API Listener
	{
	public:
		Listener(unsigned short port, int backlog = 1024);
		Listener(const Listener& cpy) = delete;
		Listener(Listener&& mov) noexcept;
		~Listener();

	public:
		Listener& operator=(const Listener& cpy) = delete;
		Listener& operator=(Listener&& mov) noexcept;

	public:
		Session accept(SIZET buffer_size = 8192);

#ifdef _WINDOWS
#ifdef EZTCP_EXPORTS
	private:
		SOCKET _s;
		sockaddr_in _s_addr;
#else
	private:
		void* _s;
		char _s_addr[16];
#endif
#endif

#ifdef __linux
#ifdef EZTCP_EXPORTS
	private:
		int _s;
		sockaddr_in _s_addr;
#else
	private:
		int _s;
		char _s_addr[16];
#endif
#endif
	};
}

inline eztcp::Session& operator>>(eztcp::Session& from, eztcp::BlockPack& to)
{
	to = from.recv();
	return from;
}

inline eztcp::Session& operator>>(eztcp::Session& from, eztcp::FilePack& to)
{
	// TODO: 完善接口
	return from;
}

inline eztcp::Session& operator<<(eztcp::Session& to, eztcp::Package& from)
{
	to.send(from);
	return to;
}

inline eztcp::Session& operator<<(eztcp::Session& to, eztcp::MemPack& from)
{
	to.send(from);
	return to;
}

inline eztcp::Session& operator<<(eztcp::Session& to, eztcp::FilePack& from)
{
	to.send(from);
	return to;
}
