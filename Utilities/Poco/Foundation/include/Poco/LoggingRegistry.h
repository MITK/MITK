//
// LoggingRegistry.h
//
// $Id$
//
// Library: Foundation
// Package: Logging
// Module:  LoggingRegistry
//
// Definition of the LoggingRegistry class.
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


#ifndef Foundation_LoggingRegistry_INCLUDED
#define Foundation_LoggingRegistry_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/AutoPtr.h"
#include "Poco/Channel.h"
#include "Poco/Formatter.h"
#include "Poco/Mutex.h"
#include <map>


namespace Poco {


class Foundation_API LoggingRegistry
	/// A registry for channels and formatters.
	///
	/// The LoggingRegistry class is used for configuring
	/// the logging framework.
{
public:
	LoggingRegistry();
		/// Creates the LoggingRegistry.

	~LoggingRegistry();
		/// Destroys the LoggingRegistry.

	Channel* channelForName(const std::string& name) const;
		/// Returns the Channel object which has been registered
		/// under the given name.
		///
		/// Throws a NotFoundException if the name is unknown.

	Formatter* formatterForName(const std::string& name) const;
		/// Returns the Formatter object which has been registered
		/// under the given name.
		///
		/// Throws a NotFoundException if the name is unknown.

	void registerChannel(const std::string& name, Channel* pChannel);
		/// Registers a channel under a given name.
		/// It is okay to re-register a different channel under an
		/// already existing name.
		
	void registerFormatter(const std::string& name, Formatter* pFormatter);
		/// Registers a formatter under a given name.
		/// It is okay to re-register a different formatter under an
		/// already existing name.

	void unregisterChannel(const std::string& name);
		/// Unregisters the given channel.
		///
		/// Throws a NotFoundException if the name is unknown.

	void unregisterFormatter(const std::string& name);
		/// Unregisters the given formatter.
		///
		/// Throws a NotFoundException if the name is unknown.

	void clear();
		/// Unregisters all registered channels and formatters.

	static LoggingRegistry& defaultRegistry();
		/// Returns a reference to the default
		/// LoggingRegistry.

private:
	typedef AutoPtr<Channel>   ChannelPtr;
	typedef AutoPtr<Formatter> FormatterPtr;
	typedef std::map<std::string, ChannelPtr> ChannelMap;
	typedef std::map<std::string, FormatterPtr> FormatterMap;
	
	ChannelMap   _channelMap;
	FormatterMap _formatterMap;
	mutable FastMutex _mutex;
};


} // namespace Poco


#endif // Foundation_LoggingRegistry_INCLUDED
