#include "utils.h"

void outputStringStream(std::stringstream& msg) {
	std::vector<wchar_t> messageW(msg.str().length() + 1);

	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, &messageW[0], messageW.size(), msg.str().c_str(), msg.str().length());
	OutputDebugString(&messageW[0]);

	msg.clear();
}