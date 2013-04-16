//
// FilesystemConfiguration.h
//
// $Id$
//
// Library: Util
// Package: Configuration
// Module:  FilesystemConfiguration
//
// Definition of the FilesystemConfiguration class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef Util_FilesystemConfiguration_INCLUDED
#define Util_FilesystemConfiguration_INCLUDED


#include "Poco/Util/Util.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Path.h"


namespace Poco {
namespace Util {


class Util_API FilesystemConfiguration: public AbstractConfiguration
	/// An implementation of AbstractConfiguration that stores configuration data
	/// in a directory hierarchy in the filesystem.
	///
	/// Every period-separated part of a property name is represented
	/// as a directory in the filesystem, relative to the base directory.
	/// Values are stored in files named "data".
	///
	/// All changes to properties are immediately persisted in the filesystem.
	///
	/// For example, a configuration consisting of the properties
	/// 
	///   logging.loggers.root.channel.class = ConsoleChannel
	///   logging.loggers.app.name = Application
	///   logging.loggers.app.channel = c1
	///   logging.formatters.f1.class = PatternFormatter
	///   logging.formatters.f1.pattern = [%p] %t
	///
	/// is stored in the filesystem as follows:
	///
	///   logging/
	///           loggers/
	///                   root/
	///                        channel/
	///                                class/
	///                                      data ("ConsoleChannel")
	///                   app/
	///                       name/
	///                            data ("Application")
	///                       channel/
	///                               data ("c1")
	///           formatters/
	///                      f1/
	///                         class/
	///                               data ("PatternFormatter")
	///                         pattern/
	///                                 data ("[%p] %t")                      
{
public:
	FilesystemConfiguration(const std::string& path);
		/// Creates a FilesystemConfiguration using the given path.
		/// All directories are created as necessary.

	void clear();
		/// Clears the configuration by erasing the configuration
		/// directory and all its subdirectories and files.

protected:
	bool getRaw(const std::string& key, std::string& value) const;
	void setRaw(const std::string& key, const std::string& value);
	void enumerate(const std::string& key, Keys& range) const;
	Poco::Path keyToPath(const std::string& key) const;
	~FilesystemConfiguration();

private:
	Poco::Path _path;
};


} } // namespace Poco::Util


#endif // Util_FilesystemConfiguration_INCLUDED
