#ifdef EMSTD4_EXPORTS
#define EMSTD4_API __declspec(dllexport)
#else
#define EMSTD4_API __declspec(dllimport)
#endif

#include <exception>
#include <string>

namespace EMSTD
{
	class _Machine;

	class EMSTD4_API Machine
	{
	public:
		Machine(const std::string& key);//最大密钥长度24，仅支持数字和大小写字母
		Machine(const Machine& obj);
		Machine(Machine&& obj) noexcept;
		Machine& operator=(const Machine& obj);
		~Machine();
		char code(char x);
	private:
		_Machine* _self;
	};

	namespace EXCP
	{
		class InvalidKey :public std::exception
		{
		public:
			InvalidKey(const char* message) :
				exception(message)
			{}
		};

		class OverLenth :public InvalidKey
		{
		public:
			OverLenth() :
				InvalidKey("OverLenth")
			{}
		};

		class InvalidCharacter :public InvalidKey
		{
		public:
			InvalidCharacter() :
				InvalidKey("InvalidCharacter")
			{}
		};
	}
}