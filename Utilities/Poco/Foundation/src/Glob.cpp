//
// Glob.cpp
//
// $Id$
//
// Library: Foundation
// Package: Filesystem
// Module:  Glob
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


#include "Poco/Glob.h"
#include "Poco/Path.h"
#include "Poco/Exception.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/File.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/Unicode.h"


namespace Poco {


Glob::Glob(const std::string& pattern, int options): 
	_pattern(pattern),
	_options(options)
{
	poco_assert (!_pattern.empty());
}


Glob::~Glob()
{
}


bool Glob::match(const std::string& subject)
{
	UTF8Encoding utf8;
	TextIterator itp(_pattern, utf8);
	TextIterator endp(_pattern);
	TextIterator its(subject, utf8);
	TextIterator ends(subject);
	
	if ((_options & GLOB_DOT_SPECIAL) && its != ends && *its == '.' && (*itp == '?' || *itp == '*'))
		return false;
	else
		return match(itp, endp, its, ends);
}


void Glob::glob(const std::string& pathPattern, std::set<std::string>& files, int options)
{
	glob(Path(Path::expand(pathPattern), Path::PATH_GUESS), files, options);
}


void Glob::glob(const char* pathPattern, std::set<std::string>& files, int options)
{
	glob(Path(Path::expand(pathPattern), Path::PATH_GUESS), files, options);
}


void Glob::glob(const Path& pathPattern, std::set<std::string>& files, int options)
{
	Path pattern(pathPattern);
	pattern.makeDirectory(); // to simplify pattern handling later on
	Path base(pattern);
	Path absBase(base);
	absBase.makeAbsolute();
	while (base.depth() > 0 && base[base.depth() - 1] != "..") 
	{
		base.popDirectory();
		absBase.popDirectory();
	}
	if (pathPattern.isDirectory()) options |= GLOB_DIRS_ONLY;
	collect(pattern, absBase, base, pathPattern[base.depth()], files, options);		
}


bool Glob::match(TextIterator& itp, const TextIterator& endp, TextIterator& its, const TextIterator& ends)
{
	while (itp != endp)
	{
		if (its == ends)
		{
			while (itp != endp && *itp == '*') ++itp;
			break;
		}
		switch (*itp)
		{
		case '?':
			++itp; ++its;
			break;
		case '*':
			if (++itp != endp)
			{
				while (its != ends && !matchAfterAsterisk(itp, endp, its, ends)) ++its;
				return its != ends;
			}
			return true;
		case '[':
			if (++itp != endp) 
			{
				bool invert = *itp == '!';
				if (invert) ++itp;
				if (itp != endp)
				{
					bool mtch = matchSet(itp, endp, *its++);
					if ((invert && mtch) || (!invert && !mtch)) return false;
					break;
				}
			}
			throw SyntaxException("bad range syntax in glob pattern");
		case '\\':
			if (++itp == endp) throw SyntaxException("backslash must be followed by character in glob pattern");
			// fallthrough
		default:
			if (_options & GLOB_CASELESS)
			{
				if (Unicode::toLower(*itp) != Unicode::toLower(*its)) return false;
			}
			else
			{
				if (*itp != *its) return false;
			}
			++itp; ++its;
		}
	}
	return itp == endp && its == ends;
}


bool Glob::matchAfterAsterisk(TextIterator itp, const TextIterator& endp, TextIterator its, const TextIterator& ends)
{
	return match(itp, endp, its, ends);
}


bool Glob::matchSet(TextIterator& itp, const TextIterator& endp, int c)
{
	if (_options & GLOB_CASELESS)
		c = Unicode::toLower(c);

	while (itp != endp)
	{
		switch (*itp)
		{
		case ']':
			++itp; 
			return false;
		case '\\':
			if (++itp == endp) throw SyntaxException("backslash must be followed by character in glob pattern");
		}
		int first = *itp;
		int last  = first;
		if (++itp != endp && *itp == '-')
		{
			if (++itp != endp)
				last = *itp++;
			else
				throw SyntaxException("bad range syntax in glob pattern");
		}
		if (_options & GLOB_CASELESS)
		{
			first = Unicode::toLower(first);
			last  = Unicode::toLower(last);
		}
		if (first <= c && c <= last)
		{
			while (itp != endp)
			{
				switch (*itp)
				{
				case ']':
					++itp;
					return true;
				case '\\':
					if (++itp == endp) break;
				default:
					++itp;
				}
			}
			throw SyntaxException("range must be terminated by closing bracket in glob pattern");
		}
	}
	return false;
}


void Glob::collect(const Path& pathPattern, const Path& base, const Path& current, const std::string& pattern, std::set<std::string>& files, int options)
{
	try
	{
		std::string pp = pathPattern.toString();
		std::string basep = base.toString();
		std::string curp  = current.toString();
		Glob g(pattern, options);
		DirectoryIterator it(base);
		DirectoryIterator end;
		while (it != end)
		{
			const std::string& name = it.name();
			if (g.match(name))
			{
				Path p(current);
				if (p.depth() < pathPattern.depth() - 1)
				{
					p.pushDirectory(name);
					collect(pathPattern, it.path(), p, pathPattern[p.depth()], files, options);
				}
				else
				{
					p.setFileName(name);
					if (isDirectory(p, (options & GLOB_FOLLOW_SYMLINKS) != 0))
					{
						p.makeDirectory();
						files.insert(p.toString());
					}
					else if (!(options & GLOB_DIRS_ONLY))
					{
						files.insert(p.toString());
					}
				}
			}
			++it;
		}
	}
	catch (Exception&)
	{
	}
}


bool Glob::isDirectory(const Path& path, bool followSymlink)
{
	File f(path);
	if (f.isDirectory())
	{
		return true;
	}
	else if (followSymlink && f.isLink())
	{
		try
		{
			// Test if link resolves to a directory.
			DirectoryIterator it(f);
			return true;
		}
		catch (Exception&)
		{
		}
	}
	return false;
}


} // namespace Poco
