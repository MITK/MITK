/*=========================================================================
 
 Program:   openCherry Platform
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

#ifndef HEADERS_H_
#define HEADERS_H_

#include "Dictionary.h"

#include "../framework/Exceptions.h"

#include <Poco/HashMap.h>
#include <Poco/Mutex.h>
#include <Poco/String.h>
#include <Poco/Hash.h>

namespace osgi
{
namespace util
{

using namespace osgi::framework;

/**
 * Headers classes. This class implements a Dictionary that has
 * the following behaviour:
 * <ul>
 * <li>insert, erase, clear etc. throw UnsupportedOperationException.
 * The Dictionary is thus read-only to others.
 * <li>The string keys in the Dictionary are case-preserved,
 * but the get operations are case-insensitive.
 * </ul>
 */
class Headers: public Dictionary
{
private:

  class HeaderString: public std::string
  {

  public:

    HeaderString()
    {

    }

    inline HeaderString(const std::string& str) :
      std::string(str)
    {

    }

    inline bool operator==(const std::string& str)
    {
      return Poco::icompare(*static_cast<std::string*> (this), str) == 0;
    }
  };

  struct HeaderStringHash
  {
    inline std::size_t operator()(const std::string& str) const
    {
      return Poco::hash(Poco::toLower(str));
    }
  };

  typedef Poco::HashMap<HeaderString, Value, HeaderStringHash> MapImpl;

  bool readOnly;
  MapImpl map;
  mutable Poco::FastMutex mutex;

public:

  /**
   * Create an empty Headers dictionary.
   *
   */
  Headers();

  /**
   * Create a Headers dictionary from a Dictionary.
   *
   * @param dict The initial dictionary for this Headers object.
   * @exception IllegalArgumentException If a case-variant of the key is
   * in the dictionary parameter.
   */
  Headers(const Dictionary& dict) throw (IllegalArgumentException);

  /**
   * Create an empty Headers dictionary.
   *
   * @param initialCapacity The initial capacity of this Headers object.
   */
  Headers(std::size_t initialReserve);

  void Clear();

  bool ContainsKey(const Key& key);

  bool ContainsValue(const Value& value);

  std::set<Entry> EntrySet() const;

  std::set<Key> KeySet() const;

  std::vector<ConstValue> Values() const;

  std::vector<Value> Values();

  const Value Get(const Key& key) const;

  Value Get(const Key& key);

  Value Put(const Key& key, const Value& value);

  void PutAll(const Dictionary& dict);

  const Value& operator[](const Key& key) const;

  Value& operator[](const Key& key);

  Value Remove(const Key& key);

  std::size_t Size() const;

  bool IsEmpty() const;

  /**
   * Set a header value or optionally replace it if it already exists.
   *
   * @param key Key name.
   * @param value Value of the key or null to remove key.
   * @param replace A value of true will allow a previous
   * value of the key to be replaced.  A value of false
   * will cause an IllegalArgumentException to be thrown
   * if a previous value of the key exists.
   * @return the previous value to which the key was mapped,
   * or null if the key did not have a previous mapping.
   *
   * @exception IllegalArgumentException If a case-variant of the key is
   * already present.
   * @since 3.2
   */
  Value Set(const Key& k, const Value& value, bool replace = false)
      throw (IllegalArgumentException);

  void SetReadOnly();
};

}
}

#endif /* HEADERS_H_ */
