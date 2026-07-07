/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyPersistence_h
#define mitkPropertyPersistence_h

#include <map>
#include <mitkIPropertyPersistence.h>

namespace mitk
{
  /** \brief Implementation of the IPropertyPersistence service interface.
   *
   * Manages persistence information for base data properties. Supports both
   * exact property name matching and regular expression matching for flexible
   * persistence rule definitions.
   *
   * \sa IPropertyPersistence
   */
  class PropertyPersistence : public IPropertyPersistence
  {
  public:
    PropertyPersistence();
    ~PropertyPersistence() override;

    typedef IPropertyPersistence::InfoResultType InfoResultType;

    /** \copydoc IPropertyPersistence::AddInfo */
    bool AddInfo(const PropertyPersistenceInfo *info, bool overwrite) override;
    /** \copydoc IPropertyPersistence::GetInfo(const std::string &, bool) const */
    InfoResultType GetInfo(const std::string &propertyName, bool allowNameRegEx) const override;
    /** \copydoc IPropertyPersistence::GetInfo(const std::string &, const MimeTypeNameType &, bool, bool) const */
    InfoResultType GetInfo(const std::string &propertyName,
                           const MimeTypeNameType &mime,
                           bool allowMimeWildCard,
                           bool allowNameRegEx) const override;
    /** \copydoc IPropertyPersistence::GetInfoByKey */
    InfoResultType GetInfoByKey(const std::string &persistenceKey, bool allowKeyRegEx) const override;
    /** \copydoc IPropertyPersistence::HasInfo */
    bool HasInfo(const std::string &propertyName, bool allowNameRegEx) const override;
    /** \copydoc IPropertyPersistence::RemoveAllInfo */
    void RemoveAllInfo() override;
    /** \copydoc IPropertyPersistence::RemoveInfo(const std::string &) */
    void RemoveInfo(const std::string &propertyName) override;
    /** \copydoc IPropertyPersistence::RemoveInfo(const std::string &, const MimeTypeNameType &) */
    void RemoveInfo(const std::string &propertyName, const MimeTypeNameType &mime) override;

  private:
    typedef std::multimap<const std::string, PropertyPersistenceInfo::ConstPointer> InfoMap;

    /** \brief Helper function that selects entries from the info map using a predicate. */
    using SelectFunctionType = std::function<bool(const InfoMap::value_type &)>;
    static InfoMap SelectInfo(const InfoMap &infoMap, const SelectFunctionType &selectFunction);

    PropertyPersistence(const PropertyPersistence &);
    PropertyPersistence &operator=(const PropertyPersistence &);

    InfoMap m_InfoMap;
  };

  /** \brief Creates an unmanaged (!) instance of PropertyPersistence for testing purposes.
   *
   * The caller is responsible for managing the lifetime of the returned instance.
   *
   * \return A raw pointer to a new PropertyPersistence instance.
   */
  MITKCORE_EXPORT IPropertyPersistence *CreateTestInstancePropertyPersistence();
}

#endif
