/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyAliases_h
#define mitkPropertyAliases_h

#include <map>
#include <mitkIPropertyAliases.h>

namespace mitk
{
  /** \brief Implementation of the IPropertyAliases service interface.
   *
   * Manages aliases for property names, optionally restricted to specific
   * data node class names. Aliases can be used to display user-friendly
   * property names in the property view.
   *
   * \sa IPropertyAliases
   */
  class PropertyAliases : public IPropertyAliases
  {
  public:
    PropertyAliases();
    ~PropertyAliases() override;

    /** \copydoc IPropertyAliases::AddAlias */
    bool AddAlias(const std::string &propertyName, const std::string &alias, const std::string &className) override;
    /** \copydoc IPropertyAliases::GetAliases */
    std::vector<std::string> GetAliases(const std::string &propertyName, const std::string &className) override;
    /** \copydoc IPropertyAliases::GetPropertyName */
    std::string GetPropertyName(const std::string &alias, const std::string &className) override;
    /** \copydoc IPropertyAliases::HasAliases */
    bool HasAliases(const std::string &propertyName, const std::string &className) override;
    /** \copydoc IPropertyAliases::RemoveAlias */
    void RemoveAlias(const std::string &propertyName, const std::string &alias, const std::string &className) override;
    /** \copydoc IPropertyAliases::RemoveAliases */
    void RemoveAliases(const std::string &propertyName, const std::string &className) override;
    /** \copydoc IPropertyAliases::RemoveAllAliases */
    void RemoveAllAliases(const std::string &className) override;

  private:
    typedef std::map<std::string, std::vector<std::string>> AliasesMap;
    typedef AliasesMap::const_iterator AliasesMapConstIterator;
    typedef AliasesMap::iterator AliasesMapIterator;

    PropertyAliases(const PropertyAliases &);
    PropertyAliases &operator=(const PropertyAliases &);

    std::map<std::string, AliasesMap> m_Aliases;
  };
}

#endif
