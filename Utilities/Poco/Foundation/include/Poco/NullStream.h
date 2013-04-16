//
// NullStream.h
//
// $Id$
//
// Library: Foundation
// Package: Streams
// Module:  NullStream
//
// Definition of the NullStreamBuf, NullInputStream and NullOutputStream classes.
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


#ifndef Foundation_NullStream_INCLUDED
#define Foundation_NullStream_INCLUDED


#include "Poco/Foundation.h"
#include "Poco/UnbufferedStreamBuf.h"
#include <istream>
#include <ostream>


namespace Poco {


class Foundation_API NullStreamBuf: public UnbufferedStreamBuf
	/// This stream buffer discards all characters written to it.
	/// Any read operation immediately yields EOF.
{
public:
	NullStreamBuf();
		/// Creates a NullStreamBuf.
	
	~NullStreamBuf();
		/// Destroys the NullStreamBuf.
		
protected:
	int readFromDevice();
	int writeToDevice(char c);
};


class Foundation_API NullIOS: public virtual std::ios
	/// The base class for NullInputStream and NullOutputStream.
	///
	/// This class is needed to ensure the correct initialization
	/// order of the stream buffer and base classes.
{
public:
	NullIOS();
	~NullIOS();

protected:
	NullStreamBuf _buf;
};


class Foundation_API NullInputStream: public NullIOS, public std::istream
	/// Any read operation from this stream immediately
	/// yields EOF.
{
public:
	NullInputStream();
		/// Creates the NullInputStream.

	~NullInputStream();
		/// Destroys the NullInputStream.
};


class Foundation_API NullOutputStream: public NullIOS, public std::ostream
	/// This stream discards all characters written to it.
{
public:
	NullOutputStream();
		/// Creates the NullOutputStream.
	
	~NullOutputStream();
		/// Destroys the NullOutputStream.
};


} // namespace Poco


#endif // Foundation_NullStream_INCLUDED
