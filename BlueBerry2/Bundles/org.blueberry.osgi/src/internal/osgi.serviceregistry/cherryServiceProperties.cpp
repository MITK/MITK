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

#include "cherryServiceProperties.h"

#include <osgi/framework/Constants.h>
#include <osgi/framework/ObjectVector.h>

namespace cherry {
namespace osgi {
namespace internal {

  Object::Pointer ServiceProperties::CloneValue(Object::Pointer value) {
    if (!value)
      return value;

    return value->Clone();
  }

  ServiceProperties::ServiceProperties(const Dictionary& props)
  : Headers(props.Size() + 2)
  {
     Poco::Mutex::ScopedLock lock(mutex);
      std::set<Dictionary::Key> keys(props.KeySet());

      for (std::set<Dictionary::Key>::iterator i = keys.begin(); i != keys.end(); ++i)
      {
        SetProperty(*i, props.Get(*i));
      }
  }

  Object::Pointer ServiceProperties::GetProperty(const std::string& key) const {
    return CloneValue(Get(key));
  }

  Object::Pointer ServiceProperties::SetProperty(const std::string& key, Object::Pointer value) {
    Poco::Mutex::ScopedLock lock(mutex);
    return Set(key, CloneValue(value));
  }

  std::string ServiceProperties::ToString() const {
    Poco::Mutex::ScopedLock lock(mutex);

    std::set<std::string> keys(KeySet());

    std::string sb("{");

    int n = 0;
    for (std::set<std::string>::iterator i = keys.begin(); i != keys.end(); ++i) {
      const std::string& key = *i;
      if (key != Constants::OBJECTCLASS) {
        if (n > 0)
          sb += ", " + key + "=";
        Object::Pointer value = Get(key);
        if (ObjectVector<Object::Pointer>::Pointer v = value.Cast<ObjectVector<Object::Pointer> >()) {
          sb += '[';
          unsigned int length = v->size();
          for (unsigned int j = 0; j < length; j++) {
            if (j > 0)
              sb += ',';
            sb += v->operator[](j)->ToString();
          }
          sb += ']';
        } else {
          sb += value->ToString();
        }
        n++;
      }
    }

    sb += '}';

    return sb;
  }

}
}
}
