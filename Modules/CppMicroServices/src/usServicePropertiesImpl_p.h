/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICEPROPERTIESIMPL_P_H
#define USSERVICEPROPERTIESIMPL_P_H

#include <usServiceProperties.h>

namespace us {

class ServicePropertiesImpl
{

public:

  explicit ServicePropertiesImpl(const ServiceProperties& props);

  const Any& Value(const std::string& key) const;
  const Any& Value(int index) const;

  int Find(const std::string& key) const;
  int FindCaseSensitive(const std::string& key) const;

  const std::vector<std::string>& Keys() const;

private:

  std::vector<std::string> keys;
  std::vector<Any> values;

  static Any emptyAny;

};

}

#endif // USSERVICEPROPERTIESIMPL_P_H
