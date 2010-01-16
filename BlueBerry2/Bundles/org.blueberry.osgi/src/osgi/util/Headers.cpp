/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "Headers.h"

namespace osgi
{
namespace util
{

Headers::Headers() :
  Dictionary(), readOnly(false)
{

}

Headers::Headers(const Dictionary& dict) throw (IllegalArgumentException) :
  readOnly(false), map(dict.Size())
{
  const std::set<Key> keys(dict.KeySet());
  for (std::set<Key>::const_iterator i = keys.begin(); i != keys.end(); ++i)
  {
    Set(*i, dict[*i]);
  }
}

Headers::Headers(std::size_t initialReserve) :
  readOnly(false), map(initialReserve)
{

}

void Headers::Clear()
{
  Poco::FastMutex::ScopedLock lock(mutex);
  if (readOnly)
    throw UnsupportedOperationException();
}

bool Headers::ContainsKey(const Key& key)
{
  Poco::FastMutex::ScopedLock lock(mutex);
  return map.find(HeaderString(key)) != map.end();
}

bool Headers::ContainsValue(const Value& value)
{
  throw UnsupportedOperationException();
}

std::set<Dictionary::Entry> Headers::EntrySet() const
{
  throw UnsupportedOperationException();
}

std::set<Dictionary::Key> Headers::KeySet() const
{
  Poco::FastMutex::ScopedLock lock(mutex);
  std::set<Dictionary::Key> keys;
  for (MapImpl::ConstIterator i = map.begin(); i != map.end(); ++i)
  {
    keys.insert(i->first);
  }
  return keys;
}

std::vector<Dictionary::ConstValue> Headers::Values() const
{
  throw UnsupportedOperationException();
}

std::vector<Dictionary::Value> Headers::Values()
{
  throw UnsupportedOperationException();
}

const Dictionary::Value Headers::Get(const Key& key) const
{
  return operator[](key);
}

Dictionary::Value Headers::Get(const Key& key)
{
  return operator[](key);
}

Dictionary::Value Headers::Put(const Key& key, const Value& value)
{
  {
    Poco::FastMutex::ScopedLock lock(mutex);
    if (readOnly)
      throw UnsupportedOperationException();
  }

  return Set(key, value, true);
}

void Headers::PutAll(const Dictionary& dict)
{
  throw UnsupportedOperationException();
}

const Dictionary::Value& Headers::operator[](const Key& key) const
{
  Poco::FastMutex::ScopedLock lock(mutex);
  return map[HeaderString(key)];
}

Dictionary::Value& Headers::operator[](const Key& key)
{
  if (readOnly)
    throw UnsupportedOperationException();

  Poco::FastMutex::ScopedLock lock(mutex);
  return map[HeaderString(key)];
}

Dictionary::Value Headers::Remove(const Key& key)
{
  throw UnsupportedOperationException();
}

std::size_t Headers::Size() const
{
  Poco::FastMutex::ScopedLock lock(mutex);
  return map.size();
}

bool Headers::IsEmpty() const
{
  Poco::FastMutex::ScopedLock lock(mutex);
  return map.empty();
}

Dictionary::Value Headers::Set(const Key& k, const Value& value, bool replace)
    throw (IllegalArgumentException)
{
  Poco::FastMutex::ScopedLock lock(mutex);
  if (readOnly)
    throw new UnsupportedOperationException();

  HeaderString key(k);
  MapImpl::Iterator i = map.find(key);
  if (!value)
  { /* remove */
    if (i != map.end())
    {
      Value val(i->second);
      map.erase(i);
      return val;
    }
  }
  else
  { /* put */
    if (i != map.end())
    { /* duplicate key */
      if (!replace)
        throw IllegalArgumentException("The key \"" + key
            + "\" already exists in another case variation");

      Value oldVal(i->second);
      map[key] = value;
      return oldVal;
    }
    map.insert(std::make_pair(key, value));
  }
  return Value(0);
}

void Headers::SetReadOnly()
{
  Poco::FastMutex::ScopedLock lock(mutex);
  readOnly = true;
}

}
}
