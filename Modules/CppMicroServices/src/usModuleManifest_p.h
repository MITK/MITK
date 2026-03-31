/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEMANIFEST_P_H
#define USMODULEMANIFEST_P_H

#include <usAny.h>

namespace us {

class ModuleManifest
{
public:

  ModuleManifest();

  void Parse(std::istream& is);

  bool Contains(const std::string& key) const;

  Any GetValue(const std::string& key) const;

  std::vector<std::string> GetKeys() const;

  void SetValue(const std::string& key, const Any& value);

private:

  typedef std::map<std::string, Any> AnyMap;

  AnyMap m_Properties;
};

}

#endif // USMODULEMANIFEST_P_H
