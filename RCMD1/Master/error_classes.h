#pragma once

class ArgsError
{
public:
	const char* _hint;
	ArgsError(const char* hint) :
		_hint(hint)
	{}
};

class ResultError
{
public:
	int _code;
	ResultError(int code) :
		_code(code)
	{}
};

class UnconnectedError
{
public:
	UnconnectedError() {}
};
