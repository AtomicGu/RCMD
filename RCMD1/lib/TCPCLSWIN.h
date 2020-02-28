/*************************************************
版本：V2.2.0
更新内容：
1.取消了富有争议的BlockPack接管空指针的构造函数
2.修复RecvPack向BlockPack的强制转换函数的bug
*************************************************/

#pragma once
#ifdef TCPCLASSESWINDOWS_EXPORTS
#define TCPCLS_API __declspec(dllexport)
#else
#define TCPCLS_API __declspec(dllimport)
#endif

class _TCPSession;
class _TCPListener;
class _RecvPack;
class _MemPack;
class _BlockPack;
class _FilePack;

namespace TCPCLS
{
	namespace ERR
	{
		//
		// Windows错误类
		//
		class TCPCLS_API WinError
		{
		public:
			int _code;
			WinError(int code);
		};

		//
		// WSA错误类
		//
		class TCPCLS_API WSAError : public WinError
		{
		public:
			WSAError(int code);
		};

		//
		// Windows文件API错误类
		//
		class TCPCLS_API FileError : public WinError
		{
		public:
			FileError(int code);
		};
	}

	//
	// 包的统一界面
	//
	class TCPCLS_API TCPPackage
	{
	public:
		virtual __int64 size() const = 0;
		virtual size_t extract(char* p_buffer, size_t buffer_size) = 0;
		virtual __int64 position() const = 0;
	};

	//
	// 内存包（不可拷贝、不可移动）（简单地包装内存空间，当心内存泄漏和指针悬挂）
	//
	class TCPCLS_API MemPack : public TCPPackage
	{
	public:
		MemPack(const void* _p_src, size_t size);
		MemPack(const MemPack& cpy) = delete;
		MemPack(MemPack&& mov) = delete;

	public:
		MemPack& operator=(const MemPack& cpy) = delete;
		MemPack& operator=(MemPack&& mov) = delete;

	public:
		virtual __int64 size() const;
		virtual size_t extract(char* p_buffer, size_t buffer_size);
		virtual __int64 position() const;

	private:
		_MemPack* _;
	};

	//
	// 块包（在MemPack基础上添加管理整块内存的功能）
	//
	class TCPCLS_API BlockPack :public TCPPackage
	{
	public:
		BlockPack();
		BlockPack(const void* p_src, size_t size);
		BlockPack(const BlockPack& cpy);
		BlockPack(BlockPack&& mov) noexcept;
		~BlockPack();

	public:
		BlockPack& operator=(const BlockPack& cpy);
		BlockPack& operator=(BlockPack&& mov) noexcept;

	public:
		virtual __int64 size() const;
		virtual size_t extract(char* p_buffer, size_t buffer_size);
		virtual __int64 position() const;

	private:
		_BlockPack* _;
		BlockPack(_BlockPack* p);
		friend class RecvPack;
	};

	//
	// 文件包（将文件包装为包）
	//
	class TCPCLS_API FilePack :public TCPPackage
	{
	public:
		FilePack(const char* file_path) throw(ERR::FileError);
		FilePack(const FilePack& cpy) = delete;
		FilePack(FilePack&& mov) = delete;
		~FilePack();

	public:
		FilePack& operator=(const FilePack& cpy) = delete;
		FilePack& operator=(FilePack&& mov) = delete;

	public:
		virtual __int64 size() const;
		virtual size_t extract(char* p_buffer, size_t buffer_size) throw(ERR::FileError);
		virtual __int64 position() const throw(ERR::FileError);

	private:
		_FilePack* _;
	};

	//
	// 接收包
	//
	class TCPCLS_API RecvPack :public TCPPackage
	{
	public:
		RecvPack(const RecvPack& cpy) = delete;
		~RecvPack();

	public:
		RecvPack& operator=(const RecvPack& cpy) = delete;
		operator BlockPack();

	public:
		virtual __int64 size() const;
		virtual size_t extract(char* p_buffer, size_t buffer_size) throw(ERR::WSAError);
		virtual __int64 position() const;

	private:
		_RecvPack* _;
		RecvPack(_RecvPack* p);
		RecvPack(RecvPack&& mov) noexcept;
		RecvPack& operator=(RecvPack&& mov) noexcept;
		friend class TCPSession;
	};

	//
	// 会话套接字类
	//
	class TCPCLS_API TCPSession
	{
	public:
		RecvPack _recving;

	public:
		TCPSession();
		TCPSession(
			const char* ip,
			unsigned short port,
			size_t buffer_size = 8192
		) throw (ERR::WSAError);
		TCPSession(const TCPSession& cpy) = delete;
		TCPSession(TCPSession&& mov) noexcept;
		~TCPSession();

	public:
		TCPSession& operator=(const TCPSession& cpy) = delete;
		TCPSession& operator=(TCPSession&& mov) noexcept;

	public:
		void send(TCPPackage& package) throw (ERR::WSAError);
		RecvPack& recv() throw (ERR::WSAError);
		unsigned short get_port() const;
		const char* get_ip() const;

	private:
		_TCPSession* _;
		TCPSession(_TCPSession* p);
		friend class TCPListener;
	};

	//
	// 监听套接字类
	//
	class TCPCLS_API TCPListener
	{
	public:
		TCPListener(unsigned short port, int backlog = 1024);
		TCPListener(const TCPListener& cpy) = delete;
		TCPListener(TCPListener&& mov) noexcept;
		~TCPListener();

	public:
		TCPListener& operator=(const TCPListener& cpy) = delete;
		TCPListener& operator=(TCPListener&& mov) noexcept;

	public:
		TCPSession accept(size_t buffer_size = 8192);

	private:
		_TCPListener* _;
	};
}
