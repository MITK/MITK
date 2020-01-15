/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyDescriptions_h
#define mitkPropertyDescriptions_h

#include <map>
#include <mitkIPropertyDescriptions.h>

namespace mitk
{
  class PropertyDescriptions : public IPropertyDescriptions
  {
  public:
    PropertyDescriptions();
    ~PropertyDescriptions() override;

    bool AddDescription(const std::string &propertyName,
                        const std::string &description,
                        const std::string &className,
                        bool overwrite) override;
    bool AddDescriptionRegEx(const std::string &propertyRegEx,
                             const std::string &description,
                             const std::string &className,
                             bool overwrite) override;
    std::string GetDescription(const std::string &propertyName,
                               const std::string &className,
                               bool overwrite) const override;
    bool HasDescription(const std::string &propertyName, const std::string &className, bool overwrite) const override;
    void RemoveAllDescriptions(const std::string &className) override;
    void RemoveDescription(const std::string &propertyName, const std::string &className) override;

  private:
    typedef std::map<std::string, std::string> DescriptionMap;
    typedef DescriptionMap::const_iterator DescriptionMapConstIterator;
    typedef DescriptionMap::iterator DescriptionMapIterator;

    PropertyDescriptions(const PropertyDescriptions &);
    PropertyDescriptions &operator=(const PropertyDescriptions &);

    std::map<std::string, DescriptionMap> m_Descriptions;
    std::map<std::string, DescriptionMap> m_DescriptionsRegEx;
  };
}

#endif
