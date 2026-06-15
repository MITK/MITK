/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyList_h
#define mitkPropertyList_h

#include <mitkIPropertyOwner.h>
#include <mitkGenericProperty.h>

#include <nlohmann/json_fwd.hpp>

namespace mitk
{
  /**
   * @brief Key-value list holding instances of BaseProperty
   *
   * This list is meant to hold an arbitrary list of "properties",
   * which should describe the object associated with this list.
   *
   * Usually you will use PropertyList as part of a DataNode
   * object - in this context the properties describe the data object
   * held by the DataNode (e.g. whether the object is rendered at
   * all, which color is used for rendering, what name should be
   * displayed for the object, etc.)
   *
   * The values in the list are not fixed, you may introduce any kind
   * of property that seems useful - all you have to do is inherit
   * from BaseProperty.
   *
   * The list is organized as a key-value pairs, i.e.
   *
   *   \li "name" : pointer to a StringProperty
   *   \li "visible" : pointer to a BoolProperty
   *   \li "color" : pointer to a ColorProperty
   *   \li "volume" : pointer to a FloatProperty
   *
   * Please see the documentation of SetProperty and ReplaceProperty for two
   * quite different semantics. Normally SetProperty is what you want - this
   * method will try to change the value of an existing property and will
   * not allow you to replace e.g. a ColorProperty with an IntProperty.
   *
   * Please also regard, that the key of a property must be a none empty string.
   * This is a precondition. Setting properties with empty keys will raise an exception.
   */
  class MITKCORE_EXPORT PropertyList : public itk::Object, public IPropertyOwner
  {
  public:
    mitkClassMacroItkParent(PropertyList, itk::Object);

    /**
     * Method for creation through the object factory.
     */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * Map structure to hold the properties: the map key is a string,
     * the value consists of the actual property object (BaseProperty).
     */
    typedef std::map<std::string, BaseProperty::Pointer> PropertyMap;
    typedef std::pair<std::string, BaseProperty::Pointer> PropertyMapElementType;

    // IPropertyProvider

    /**
     * \brief Retrieve a property by key (const version).
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] contextName Ignored in PropertyList (no context support). Provided for interface compatibility.
     * \param[in] fallBackOnDefaultContext Ignored in PropertyList.
     * \return A const smart pointer to the property, or \c nullptr if not found.
     *
     * \sa IPropertyProvider
     */
    BaseProperty::ConstPointer GetConstProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = true) const override;

    /**
     * \brief Retrieve all property keys.
     *
     * \param[in] contextName Ignored in PropertyList (no context support). Keys are returned only
     *            when contextName is empty or includeDefaultContext is true.
     * \param[in] includeDefaultContext If \c true and contextName is non-empty, default context keys
     *            are included.
     * \return A vector of all property key strings in this list.
     *
     * \sa IPropertyProvider
     */
    std::vector<std::string> GetPropertyKeys(const std::string &contextName = "", bool includeDefaultContext = false) const override;

    /**
     * \brief Return available property context names.
     *
     * PropertyList does not support contexts, so this always returns an empty vector.
     *
     * \return An empty vector.
     *
     * \sa IPropertyProvider
     */
    std::vector<std::string> GetPropertyContextNames() const override;

    // IPropertyOwner

    /**
     * \brief Retrieve a property by key (non-const version).
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] contextName Ignored in PropertyList.
     * \param[in] fallBackOnDefaultContext Ignored in PropertyList.
     * \return A raw pointer to the property, or \c nullptr if not found.
     *
     * \sa IPropertyOwner
     */
    BaseProperty * GetNonConstProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = true) override;

    /**
     * \brief Set a property in the list, preserving existing property type.
     *
     * If a property with the given key already exists and has the same type, its value
     * is updated via AssignProperty(). If the types differ, an error is logged and the
     * property is not replaced (use ReplaceProperty() instead). If the key does not
     * exist, the property is inserted.
     *
     * \param[in] propertyKey The key for the property. Must not be empty.
     * \param[in] property The property to set. If \c nullptr, the call is ignored.
     * \param[in] contextName Ignored in PropertyList.
     * \param[in] fallBackOnDefaultContext Ignored in PropertyList.
     *
     * \pre \p propertyKey must be a non-empty string; otherwise an exception is thrown.
     *
     * \sa ReplaceProperty
     * \sa IPropertyOwner
     */
    void SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;

    /**
     * \brief Remove a property from the list by key.
     *
     * \param[in] propertyKey The key of the property to remove.
     * \param[in] contextName Ignored in PropertyList.
     * \param[in] fallBackOnDefaultContext Ignored in PropertyList.
     *
     * \sa IPropertyOwner
     */
    void RemoveProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;

    /**
     * \brief Get a property by its name.
     *
     * \param[in] propertyKey The key identifying the property.
     * \return A raw pointer to the property, or \c nullptr if no property with the
     *         given key exists.
     */
    mitk::BaseProperty *GetProperty(const std::string &propertyKey) const;

    /**
     * \brief Replace a property object in the list by a new one.
     *
     * The actual property object is replaced, unlike SetProperty() which tries to
     * assign the value to an existing property of the same type. This is useful when
     * you want to change the type of a property (e.g., from BoolProperty to
     * StringProperty) or to share the same property object among several
     * PropertyList/DataNode instances for synchronization.
     *
     * \param[in] propertyKey The key for the property.
     * \param[in] property The new property object. If \c nullptr, the call is ignored.
     *
     * \sa SetProperty
     */
    void ReplaceProperty(const std::string &propertyKey, BaseProperty *property);

    /**
     * \brief Merge another PropertyList into this one.
     *
     * All properties from the given list are added to this list. If a property key
     * already exists, the behavior depends on the \p replace flag.
     *
     * \param[in] pList The property list to merge. If \c nullptr, the call is ignored.
     * \param[in] replace If \c true, existing properties are replaced via
     *            ReplaceProperty(). If \c false (default), existing properties are
     *            updated via SetProperty() (type-safe assignment).
     *
     * \sa SetProperty
     * \sa ReplaceProperty
     */
    void ConcatenatePropertyList(PropertyList *pList, bool replace = false);

    /**
     * \brief Convenience access method for GenericProperty<T> properties.
     *
     * Attempts to retrieve the value of a GenericProperty<T> identified by the
     * given key.
     *
     * \tparam T The value type of the GenericProperty.
     * \param[in] propertyKey The key identifying the property.
     * \param[out] value Receives the property value if found.
     * \return \c true if the property was found and is of the expected type,
     *         \c false otherwise.
     */
    template <typename T>
    bool GetPropertyValue(const char *propertyKey, T &value) const
    {
      GenericProperty<T> *gp = dynamic_cast<GenericProperty<T> *>(GetProperty(propertyKey));
      if (gp != nullptr)
      {
        value = gp->GetValue();
        return true;
      }
      return false;
    }

    /**
     * \brief Convenience method to access the value of a BoolProperty.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] boolValue Receives the boolean value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool GetBoolProperty(const char *propertyKey, bool &boolValue) const;

    /**
     * \brief Shortcut for GetBoolProperty().
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] boolValue Receives the boolean value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool Get(const char *propertyKey, bool &boolValue) const;

    /**
     * \brief Convenience method to set the value of a BoolProperty.
     *
     * Creates a new BoolProperty if the key does not exist yet.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] boolValue The boolean value to set.
     */
    void SetBoolProperty(const char *propertyKey, bool boolValue);

    /**
     * \brief Shortcut for SetBoolProperty().
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] boolValue The boolean value to set.
     */
    void Set(const char *propertyKey, bool boolValue);

    /**
     * \brief Convenience method to access the value of an IntProperty.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] intValue Receives the integer value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool GetIntProperty(const char *propertyKey, int &intValue) const;

    /**
     * \brief Shortcut for GetIntProperty().
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] intValue Receives the integer value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool Get(const char *propertyKey, int &intValue) const;

    /**
     * \brief Convenience method to set the value of an IntProperty.
     *
     * Creates a new IntProperty if the key does not exist yet.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] intValue The integer value to set.
     */
    void SetIntProperty(const char *propertyKey, int intValue);

    /**
     * \brief Shortcut for SetIntProperty().
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] intValue The integer value to set.
     */
    void Set(const char *propertyKey, int intValue);

    /**
     * \brief Convenience method to access the value of a FloatProperty.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] floatValue Receives the float value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool GetFloatProperty(const char *propertyKey, float &floatValue) const;

    /**
     * \brief Shortcut for GetFloatProperty().
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] floatValue Receives the float value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool Get(const char *propertyKey, float &floatValue) const;

    /**
     * \brief Convenience method to set the value of a FloatProperty.
     *
     * Creates a new FloatProperty if the key does not exist yet.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] floatValue The float value to set.
     */
    void SetFloatProperty(const char *propertyKey, float floatValue);

    /**
     * \brief Shortcut for SetFloatProperty().
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] floatValue The float value to set.
     */
    void Set(const char *propertyKey, float floatValue);

    /**
     * \brief Convenience method to access the value of a DoubleProperty.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] doubleValue Receives the double value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool GetDoubleProperty(const char *propertyKey, double &doubleValue) const;

    /**
     * \brief Shortcut for GetDoubleProperty().
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] doubleValue Receives the double value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool Get(const char *propertyKey, double &doubleValue) const;

    /**
     * \brief Convenience method to set the value of a DoubleProperty.
     *
     * Creates a new DoubleProperty if the key does not exist yet.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] doubleValue The double value to set.
     */
    void SetDoubleProperty(const char *propertyKey, double doubleValue);

    /**
     * \brief Shortcut for SetDoubleProperty().
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] doubleValue The double value to set.
     */
    void Set(const char *propertyKey, double doubleValue);

    /**
     * \brief Convenience method to access the value of a StringProperty.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] stringValue Receives the string value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool GetStringProperty(const char *propertyKey, std::string &stringValue) const;

    /**
     * \brief Shortcut for GetStringProperty().
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[out] stringValue Receives the string value if the property exists.
     * \return \c true if the property was found, \c false otherwise.
     */
    bool Get(const char *propertyKey, std::string &stringValue) const;

    /**
     * \brief Convenience method to set the value of a StringProperty.
     *
     * Creates a new StringProperty if the key does not exist yet.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] stringValue The C-string value to set.
     */
    void SetStringProperty(const char *propertyKey, const char *stringValue);

    /**
     * \brief Shortcut for SetStringProperty() accepting a C-string.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] stringValue The C-string value to set.
     */
    void Set(const char *propertyKey, const char *stringValue);

    /**
     * \brief Shortcut for SetStringProperty() accepting a std::string.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] stringValue The string value to set.
     */
    void Set(const char *propertyKey, const std::string &stringValue);

    /**
     * \brief Get the modification timestamp of the list.
     *
     * Returns the timestamp of the last change of the map or the last change of any
     * of the properties stored in the list, whichever is later.
     *
     * \return The modification time.
     */
    itk::ModifiedTimeType GetMTime() const override;

    /**
     * \brief Remove a property from the list/map by key.
     *
     * \param[in] propertyKey The key of the property to remove.
     * \return \c true if the property was found and removed, \c false otherwise.
     */
    bool DeleteProperty(const std::string &propertyKey);

    /**
     * \brief Get a read-only pointer to the internal property map.
     *
     * \return Pointer to the internal PropertyMap.
     */
    const PropertyMap *GetMap() const { return &m_Properties; }

    /**
     * \brief Check whether the property list contains no properties.
     *
     * \return \c true if the list is empty, \c false otherwise.
     */
    bool IsEmpty() const { return m_Properties.empty(); }

    /**
     * \brief Remove all properties from the list.
     */
    virtual void Clear();

    /**
     * @brief Serialize the property list to JSON.
     *
     * @note Properties of a certain type can only be deserialized again if their type has been
     * registered via the IPropertyDeserialization core service.
     *
     * @sa CoreServices
     * @sa IPropertyDeserialization::RegisterProperty
     */
    void ToJSON(nlohmann::json& j) const;

    /**
     * @brief Deserialize the property list from JSON.
     *
     * @note Properties of a certain type can only be deserialized again if their type has been
     * registered via the IPropertyDeserialization core service.
     *
     * @sa CoreServices
     * @sa IPropertyDeserialization::RegisterProperty
     */
    void FromJSON(const nlohmann::json& j);

  protected:
    PropertyList();
    PropertyList(const Self &other);

    ~PropertyList() override;

    mitkCloneMacro(Self);

    /**
     * @brief Map of properties.
     */
    PropertyMap m_Properties;
  };

  /**
   * @brief Serialize all properties from a PropertyList to self contained JSON representations.
   *
   * This is a helper methods that converts the property list to self contained json
   * representations (see ConvertPropertyToSelfContainedJson() for more details).
   *
   * @param propertyList The list to serialize. Must not be nullptr.
   * @return JSON object with property names as keys.
   * @throws mitk::Exception if propertyList is nullptr or a property fails to serialize.
   */
  MITKCORE_EXPORT nlohmann::json ConvertPropertyListToSelfContainedJson(const mitk::PropertyList* propertyList);

  /**
   * @brief Deserialize a property list from self contained JSON representations.
   *
   * This is a helper methods that converts the property list from self contained json
   * representations (see ConvertPropertyFromSelfContainedJson() for more details).
   *
   * @param json Source JSON object containing properties.
   * @return New PropertyList containing the deserialized properties.
   * @throws mitk::Exception if JSON is not an object or a property fails to deserialize.
   */
  MITKCORE_EXPORT PropertyList::Pointer ConvertPropertyListFromSelfContainedJson(const nlohmann::json& json);

} // namespace mitk

#endif
