#pragma once

class FeedbackError
{
public:
	int _code;
	FeedbackError(int code) :
		_code(code)
	{}
};
