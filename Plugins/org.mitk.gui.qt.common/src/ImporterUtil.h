#pragma once

#include <QString>

class ImporterUtil {
public:
	/**
	* @brief Convert a given QString to a utf-8 encoded string (platform-independently)
	*/
	static const char* getUTF8String(const QString& string) {
#ifdef Q_OS_WIN
		return string.toLocal8Bit().constData();
#elif Q_OS_MAC
		return string.toUtf8().constData();
#elif BERRY_OS_LINUX
		return string.toStdString().constDatat(); // needs check/test
#endif
}
};
