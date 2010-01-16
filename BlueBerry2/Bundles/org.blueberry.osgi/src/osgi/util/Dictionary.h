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


#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include "../framework/Object.h"
#include "../framework/Macros.h"

#include <set>
#include <vector>

namespace osgi {
namespace util {

using namespace osgi::framework;

struct Dictionary : public Object
{
  typedef std::string Key;
  typedef Object::Pointer Value;
  typedef Object::ConstPointer ConstValue;
  typedef std::pair<Key, Value> Entry;

  osgiInterfaceMacro(osgi::util::Dictionary)

  virtual void Clear() = 0;
  virtual bool ContainsKey(const Key& key) = 0;
  virtual bool ContainsValue(const Value& value) = 0;

  virtual std::set<Entry> EntrySet() const = 0;
  virtual std::set<Key> KeySet() const = 0;
  virtual std::vector<ConstValue> Values() const = 0;
  virtual std::vector<Value> Values() = 0;

  virtual const Value Get(const Key& key) const = 0;
  virtual Value Get(const Key& key) = 0;

  virtual Value Put(const Key& key, const Value& value) = 0;
  virtual void PutAll(const Dictionary& dict) = 0;

  virtual const Value& operator[](const Key& key) const = 0;
  virtual Value& operator[](const Key& key) = 0;

  virtual Value Remove(const Key& key) = 0;
  virtual std::size_t Size() const = 0;
  virtual bool IsEmpty() const = 0;
};

}
}

#endif /* DICTIONARY_H_ */
