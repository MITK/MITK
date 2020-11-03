/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkIPersistable_h
#define mitkIPersistable_h
#include <string>

namespace mitk
{
  ///
  /// Just a base class for persistable classes
  ///
  class MITKPERSISTENCE_EXPORT IPersistable
  {
  public:
    virtual bool Save(const std::string &fileName = "") = 0;
    virtual bool Load(const std::string &fileName = "") = 0;
    virtual void SetId(const std::string &id) = 0;
    virtual std::string GetId() const = 0;
  };
}

#endif
