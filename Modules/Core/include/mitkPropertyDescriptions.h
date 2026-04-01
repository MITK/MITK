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
  /** \brief Implementation of the IPropertyDescriptions service interface.
   *
   * Manages descriptions for property names with support for exact name matching
   * and regular expression matching, optionally restricted to specific data
   * node class names.
   *
   * \sa IPropertyDescriptions
   */
  class PropertyDescriptions : public IPropertyDescriptions
  {
  public:
    PropertyDescriptions();
    ~PropertyDescriptions() override;

    /** \copydoc IPropertyDescriptions::AddDescription */
    bool AddDescription(const std::string &propertyName,
                        const std::string &description,
                        const std::string &className,
                        bool overwrite) override;
    /** \copydoc IPropertyDescriptions::AddDescriptionRegEx */
    bool AddDescriptionRegEx(const std::string &propertyRegEx,
                             const std::string &description,
                             const std::string &className,
                             bool overwrite) override;
    /** \copydoc IPropertyDescriptions::GetDescription */
    std::string GetDescription(const std::string &propertyName,
                               const std::string &className,
                               bool allowNameRegEx) const override;
    /** \copydoc IPropertyDescriptions::HasDescription */
    bool HasDescription(const std::string &propertyName, const std::string &className, bool allowNameRegEx) const override;
    /** \copydoc IPropertyDescriptions::RemoveAllDescriptions */
    void RemoveAllDescriptions(const std::string &className) override;
    /** \copydoc IPropertyDescriptions::RemoveDescription */
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
