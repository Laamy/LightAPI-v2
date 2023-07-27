enum MessageType {
    MESSAGE_OUTPUT,
    MESSAGE_WARNING,
    MESSAGE_ERROR,
};

void LogMessage(MessageType type, const char* message, ...) {
    char buffer[1024];
	va_list args;
	va_start(args, message);
	vsprintf_s(buffer, message, args);
	va_end(args);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	switch (type) {
	case MESSAGE_OUTPUT:
		std::cout << buffer << std::endl;
		break;
	case MESSAGE_WARNING:
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

		std::cout << buffer << std::endl;

		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	case MESSAGE_ERROR:
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

		std::cout << buffer << std::endl;

		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	}
}