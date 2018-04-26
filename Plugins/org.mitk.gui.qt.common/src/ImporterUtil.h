#pragma once

#include <QString>

class ImporterUtil {
public:
	/**
	* @brief Convert a given QString to a utf-8 encoded string (platform-independently)
	*/
	static std::string getUTF8String(const QString& string) {
		return string.toLocal8Bit().toStdString();
	}
};
