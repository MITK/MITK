/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkPropertyDescriptions_h
#define mitkPropertyDescriptions_h

#include <mitkIPropertyDescriptions.h>
#include <map>

namespace mitk
{
  class PropertyDescriptions : public IPropertyDescriptions
  {
  public:
    PropertyDescriptions();
    ~PropertyDescriptions();

    bool AddDescription(const std::string& propertyName, const std::string& description, bool overwrite);
    std::string GetDescription(const std::string& propertyName) const;
    bool HasDescription(const std::string& propertyName) const;
    void RemoveAllDescriptions();
    void RemoveDescription(const std::string& propertyName);

  private:
    PropertyDescriptions(const PropertyDescriptions&);
    PropertyDescriptions& operator=(const PropertyDescriptions&);

    std::map<std::string, std::string> m_Descriptions;
  };
}

#endif
