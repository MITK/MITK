/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataNode_h
#define mitkDataNode_h

#include <mitkBaseData.h>
//#include "mitkMapper.h"
#include <mitkDataInteractor.h>
#include <mitkIdentifiable.h>
#include <mitkIPropertyOwner.h>

#include <fstream>
#include <iostream>

#include <mitkColorProperty.h>
#include <mitkPropertyList.h>
#include <mitkStringProperty.h>
//#include "mitkMapper.h"

#include <mitkGeometry3D.h>
#include <mitkLevelWindow.h>
#include <map>
#include <set>

class vtkLinearTransform;

namespace mitk
{
  class BaseRenderer;
  class Mapper;

  /**
   * \brief Definition of an itk::Event that is invoked when
   * a DataInteractor is set on this DataNode.
   */
  itkEventMacroDeclaration(InteractorChangedEvent, itk::AnyEvent);

  /**
   * \brief Central node class of the MITK data tree, associating a data object with properties and mappers.
   *
   * A DataNode holds a reference to a BaseData object (e.g., an Image, Surface, or PointSet)
   * along with a set of properties that control visualization and application behavior.
   * Each DataNode can carry renderer-specific property lists in addition to its default
   * (renderer-independent) PropertyList, enabling per-renderer customization of rendering
   * parameters such as visibility, color, and opacity.
   *
   * Mappers are created on demand for each mapper slot (e.g., 2D mapper, 3D mapper) via
   * the CoreObjectFactory and are responsible for translating the node's data and properties
   * into visual representations.
   *
   * DataNodes are managed by a DataStorage, which organizes them in a directed acyclic graph
   * with parent-child ("was created by") relationships.
   *
   * \ingroup DataManagement
   *
   * \sa DataStorage, BaseData, Mapper, PropertyList, BaseRenderer
   *
   * \todo clean up all the GetProperty methods. There are too many different flavours... Can most probably be reduced
   * to
   * <tt>bool GetProperty<type>(type&)</tt>
   *
   * \warning Change in semantics of SetProperty() since Aug 25th 2006. Check your usage of this method if you do
   *          more with properties than just call <tt>SetProperty( "key", new SomeProperty("value") )</tt>.
   */
  class MITKCORE_EXPORT DataNode : public itk::DataObject, public IPropertyOwner
  {
  public:
    /** \brief Smart pointer type for Geometry3D. */
    typedef mitk::Geometry3D::Pointer Geometry3DPointer;
    /** \brief Vector of Mapper smart pointers, indexed by MapperSlotId. */
    typedef std::vector<itk::SmartPointer<Mapper>> MapperVector;
    /** \brief Map from renderer name strings to renderer-specific PropertyList instances. */
    typedef std::map<std::string, mitk::PropertyList::Pointer> MapOfPropertyLists;
    /** \brief Vector of renderer name keys used in the MapOfPropertyLists. */
    typedef std::vector<MapOfPropertyLists::key_type> PropertyListKeyNames;
    /** \brief Set of group tag name strings (keys of GroupTagProperty entries). */
    typedef std::set<std::string> GroupTagList;

    mitkClassMacroItkParent(DataNode, itk::DataObject);
    itkFactorylessNewMacro(Self);

    // IPropertyProvider

    /**
     * \brief Retrieve a property as a const pointer (IPropertyProvider interface).
     *
     * Looks up the property first in the context-specific PropertyList identified
     * by \a contextName (i.e., a renderer name). If not found and \a fallBackOnDefaultContext
     * is true (or \a contextName is empty), the default PropertyList and then the data
     * object's properties are searched.
     *
     * \param[in] propertyKey  The key of the property to retrieve.
     * \param[in] contextName  Renderer name context. Empty string means the default context.
     * \param[in] fallBackOnDefaultContext  If true, falls back to the default PropertyList
     *            when the property is not found in the specified context.
     * \return The property, or nullptr if not found.
     * \sa GetNonConstProperty, GetProperty
     */
    BaseProperty::ConstPointer GetConstProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = true) const override;

    /**
     * \brief Retrieve all property keys for a given context (IPropertyProvider interface).
     *
     * \param[in] contextName  Renderer name context. Empty string returns keys from the default PropertyList.
     * \param[in] includeDefaultContext  If true, keys from the default PropertyList are
     *            included in addition to the context-specific keys.
     * \return A vector of property key strings.
     */
    std::vector<std::string> GetPropertyKeys(const std::string &contextName = "", bool includeDefaultContext = false) const override;

    /**
     * \brief Retrieve all available property context names (IPropertyProvider interface).
     *
     * Returns the names of all renderer-specific PropertyLists that have been created
     * for this node. These names correspond to BaseRenderer names.
     *
     * \return A vector of context name strings.
     */
    std::vector<std::string> GetPropertyContextNames() const override;

    // IPropertyOwner

    /**
     * \brief Retrieve a mutable property pointer (IPropertyOwner interface).
     *
     * Behaves like GetConstProperty() but returns a non-const pointer.
     *
     * \param[in] propertyKey  The key of the property to retrieve.
     * \param[in] contextName  Renderer name context. Empty string means the default context.
     * \param[in] fallBackOnDefaultContext  If true, falls back to the default PropertyList
     *            when the property is not found in the specified context.
     * \return A mutable pointer to the property, or nullptr if not found.
     * \sa GetConstProperty
     */
    BaseProperty * GetNonConstProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = true) override;

    /**
     * \brief Set a property in the given context (IPropertyOwner interface).
     *
     * \param[in] propertyKey  The key under which to store the property. Must not be empty.
     * \param[in] property     The property instance to store.
     * \param[in] contextName  Renderer name context. Empty string targets the default PropertyList.
     * \param[in] fallBackOnDefaultContext  If true, falls back to the default PropertyList
     *            when the specified context is not found.
     * \throw mitk::Exception if \a propertyKey is empty or the context is unknown and
     *        \a fallBackOnDefaultContext is false.
     */
    void SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;

    /**
     * \brief Remove a property from the given context (IPropertyOwner interface).
     *
     * \param[in] propertyKey  The key of the property to remove. Must not be empty.
     * \param[in] contextName  Renderer name context. Empty string targets the default PropertyList.
     * \param[in] fallBackOnDefaultContext  If true, falls back to the default PropertyList
     *            when the specified context is not found.
     * \throw mitk::Exception if \a propertyKey is empty or the context is unknown and
     *        \a fallBackOnDefaultContext is false.
     */
    void RemoveProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;

    /**
     * \brief Get the Mapper for the given mapper slot, creating it on demand if necessary.
     *
     * Each mapper slot corresponds to a specific rendering context (e.g., 2D or 3D).
     * If no mapper exists yet for the given slot, one is created via CoreObjectFactory.
     *
     * \param[in] id  The mapper slot identifier.
     * \return Pointer to the mapper, or nullptr if no mapper could be created.
     * \sa SetMapper
     */
    mitk::Mapper *GetMapper(MapperSlotId id) const;

    /**
     * \brief Get the data object (instance of BaseData, e.g., an Image)
     * managed by this DataNode
     */
    BaseData *GetData() const;

    /**
     * \brief Convenience method to get the data object cast to a specific type.
     *
     * Performs a dynamic_cast of the data returned by GetData() to the requested type.
     *
     * \tparam T  The target data type (must inherit from BaseData).
     * \return Pointer to the data object cast to T, or nullptr if the cast fails or no data is set.
     * \sa GetData
     */
    template <typename T>
    T* GetDataAs() const
    {
      return dynamic_cast<T*>(this->GetData());
    }

    /**
     * \brief Get the transformation applied prior to displaying the data as
     * a vtkTransform
     * \deprecated use GetData()->GetGeometry()->GetVtkTransform() instead
     */
    vtkLinearTransform *GetVtkTransform(int t = 0) const;

    /**
     * \brief Set the data object (instance of BaseData, e.g., an Image)
     * managed by this DataNode
     *
     * Prior set properties are kept if previous data of the node already exists and has the same
     * type as the new data to be set. Otherwise, the default properties are used.
     * In case that previous data already exists, the property list of the data node is cleared
     * before setting new default properties.
     *
     * \warning the actor-mode of the vtkInteractor does not work any more, if the transform of the
     * data-tree-node is connected to the transform of the basedata via vtkTransform->SetInput.
     */
    virtual void SetData(mitk::BaseData *baseData);

    /**
     * \brief Set the DataInteractor associated with this node.
     *
     * Replaces any previously set interactor and fires an InteractorChangedEvent.
     *
     * \param[in] interactor  The new DataInteractor, or nullptr to clear the interactor.
     * \sa GetDataInteractor, InteractorChangedEvent
     */
    virtual void SetDataInteractor(const DataInteractor::Pointer interactor);

    /**
     * \brief Get the DataInteractor associated with this node.
     *
     * \return The current DataInteractor, or nullptr if none is set.
     * \sa SetDataInteractor
     */
    virtual DataInteractor::Pointer GetDataInteractor() const;

    /**
     * \brief Assignment operator. Creates a new DataNode with the data from \a right.
     * \param[in] right  The source DataNode to copy data from.
     * \return Reference to a newly created DataNode.
     */
    mitk::DataNode &operator=(const DataNode &right);

    /**
     * \brief Assignment operator. Creates a new DataNode with the given BaseData.
     * \param[in] right  The BaseData to assign.
     * \return Reference to a newly created DataNode.
     */
    mitk::DataNode &operator=(BaseData *right);

    /**
     * \brief Set a specific Mapper for the given mapper slot.
     *
     * \param[in] id      The mapper slot identifier (e.g., 2D or 3D rendering).
     * \param[in] mapper  The Mapper instance to set. If non-null, its DataNode reference is updated.
     * \sa GetMapper
     */
    virtual void SetMapper(MapperSlotId id, mitk::Mapper *mapper);

    /**
     * \brief Update the output information by calling the source's UpdateOutputInformation().
     */
    void UpdateOutputInformation() override;

    /**
     * \brief Set the requested region to the largest possible region (no-op for DataNode).
     */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Check if the requested region is outside of the buffered region.
     * \return Always returns false for DataNode.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Verify that the requested region is valid.
     * \return Always returns true for DataNode.
     */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Set the requested region from another data object (no-op for DataNode).
     * \param[in] data  The data object to copy the requested region from.
     */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /**
     * \brief Copy information from another data object (no-op for DataNode).
     * \param[in] data  The data object to copy information from.
     */
    void CopyInformation(const itk::DataObject *data) override;

    /**
     * \brief The "names" used for (renderer-specific) PropertyLists in GetPropertyList(string).
     *
     * All possible values for the "renderer" parameters of
     * the diverse GetProperty/List() methods.
     */
    PropertyListKeyNames GetPropertyListNames() const;

    /**
     * \brief Set the property (instance of BaseProperty) with key \a propertyKey in the PropertyList
     * of the \a renderer (if nullptr, use BaseRenderer-independent PropertyList). This is set-by-value.
     *
     * \warning Change in semantics since Aug 25th 2006. Check your usage of this method if you do
     *          more with properties than just call <tt>SetProperty( "key", new SomeProperty("value") )</tt>.
     *
     * \sa GetProperty
     * \sa m_PropertyList
     * \sa m_MapOfPropertyLists
     */
    void SetProperty(const char *propertyKey, BaseProperty *property, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Replace the property (instance of BaseProperty) with key \a propertyKey in the PropertyList
     * of the \a renderer (if nullptr, use BaseRenderer-independent PropertyList). This is set-by-reference.
     *
     * If \a renderer is \a nullptr the property is set in the BaseRenderer-independent
     * PropertyList of this DataNode.
     * \sa GetProperty
     * \sa m_PropertyList
     * \sa m_MapOfPropertyLists
     */
    void ReplaceProperty(const char *propertyKey, BaseProperty *property, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Add the property (instance of BaseProperty) if it does
     * not exist (or always if\a overwrite is\a true)
     * with key \a propertyKey in the PropertyList
     * of the \a renderer (if nullptr, use BaseRenderer-independent
     * PropertyList). This is set-by-value.
     *
     * For\a overwrite ==\a false the property is\em not changed
     * if it already exists. For\a overwrite ==\a true the method
     * is identical to SetProperty.
     *
     * \sa SetProperty
     * \sa GetProperty
     * \sa m_PropertyList
     * \sa m_MapOfPropertyLists
     */
    void AddProperty(const char *propertyKey,
                     BaseProperty *property,
                     const mitk::BaseRenderer *renderer = nullptr,
                     bool overwrite = false);

    /**
     * \brief Get the PropertyList of the \a renderer. If \a renderer is \a
     * nullptr, the BaseRenderer-independent PropertyList of this DataNode
     * is returned.
     * \sa GetProperty
     * \sa m_PropertyList
     * \sa m_MapOfPropertyLists
     */
    mitk::PropertyList *GetPropertyList(const mitk::BaseRenderer *renderer = nullptr) const;
    /**
     * \brief Get the PropertyList for a renderer identified by name.
     *
     * If \a rendererName is empty, the BaseRenderer-independent PropertyList is returned.
     * If no PropertyList exists yet for the given renderer name, a new empty one is created.
     *
     * \param[in] rendererName  The name of the renderer. Empty string returns the default list.
     * \return The PropertyList for the specified renderer.
     * \sa GetPropertyList(const mitk::BaseRenderer*), GetPropertyListNames
     */
    mitk::PropertyList *GetPropertyList(const std::string &rendererName) const;

    /**
     * \brief Add values from another PropertyList.
     *
     * Overwrites values in m_PropertyList only when possible (i.e. when types are compatible).
     * If you want to allow for object type changes (replacing a "visible":BoolProperty with "visible":IntProperty,
     * set \c replace .
     *
     * \param replace true: if \param pList contains a property "visible" of type ColorProperty and our m_PropertyList
     * also has a "visible" property of a different type (e.g. BoolProperty), change the type, i.e. replace the objects
     * behind the pointer.
     *
     * \sa SetProperty
     * \sa ReplaceProperty
     * \sa m_PropertyList
     */
    void ConcatenatePropertyList(PropertyList *pList, bool replace = false);

    /**
     * \brief Get the property (instance of BaseProperty) with key \a propertyKey from the PropertyList
     * of the \a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
     *
     * If \a renderer is \a nullptr or the \a propertyKey cannot be found
     * in the PropertyList specific to \a renderer or is disabled there, the BaseRenderer-independent
     * PropertyList of this DataNode is queried.
     *
     * If \a fallBackOnDataProperties is true, the data property list is queried as a last resort.
     *
     * \sa GetPropertyList
     * \sa m_PropertyList
     * \sa m_MapOfPropertyLists
     */
    mitk::BaseProperty *GetProperty(const char *propertyKey, const mitk::BaseRenderer *renderer = nullptr, bool fallBackOnDataProperties = true) const;

    /**
     * \brief Get the property of type T with key \a propertyKey from the PropertyList
     * of the \a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
     *
     * If \a renderer is \a nullptr or the \a propertyKey cannot be found
     * in the PropertyList specific to \a renderer or is disabled there, the BaseRenderer-independent
     * PropertyList of this DataNode is queried.
     * \sa GetPropertyList
     * \sa m_PropertyList
     * \sa m_MapOfPropertyLists
     */
    template <typename T>
    bool GetProperty(itk::SmartPointer<T> &property,
                     const char *propertyKey,
                     const mitk::BaseRenderer *renderer = nullptr) const
    {
      property = dynamic_cast<T *>(GetProperty(propertyKey, renderer));
      return property.IsNotNull();
    }

    /**
     * \brief Get the property of type T with key \a propertyKey from the PropertyList
     * of the \a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
     *
     * If \a renderer is \a nullptr or the \a propertyKey cannot be found
     * in the PropertyList specific to \a renderer or is disabled there, the BaseRenderer-independent
     * PropertyList of this DataNode is queried.
     * \sa GetPropertyList
     * \sa m_PropertyList
     * \sa m_MapOfPropertyLists
     */
    template <typename T>
    bool GetProperty(T *&property, const char *propertyKey, const mitk::BaseRenderer *renderer = nullptr) const
    {
      property = dynamic_cast<T *>(GetProperty(propertyKey, renderer));
      return property != nullptr;
    }

    /**
     * \brief Convenience access method for GenericProperty<T> properties.
     *
     * Retrieves the value of a GenericProperty<T> identified by \a propertyKey.
     *
     * \tparam T  The value type of the GenericProperty.
     * \param[in]  propertyKey  The key of the property to look up.
     * \param[out] value        Receives the property value if found.
     * \param[in]  renderer     If non-null, the renderer-specific PropertyList is checked first.
     * \return \a true if the property was found and successfully cast to GenericProperty<T>.
     */
    template <typename T>
    bool GetPropertyValue(const char *propertyKey, T &value, const mitk::BaseRenderer *renderer = nullptr) const
    {
      GenericProperty<T> *gp = dynamic_cast<GenericProperty<T> *>(GetProperty(propertyKey, renderer));
      if (gp != nullptr)
      {
        value = gp->GetValue();
        return true;
      }
      return false;
    }

    /// \brief Get a set of all group tags from this node's property list
    GroupTagList GetGroupTags() const;

    /**
     * \brief Convenience access method for bool properties (instances of BoolProperty).
     *
     * \param[in]  propertyKey  The key of the BoolProperty.
     * \param[out] boolValue    Receives the boolean value if the property is found.
     * \param[in]  renderer     If non-null, the renderer-specific PropertyList is checked first.
     * \return \a true if the property was found.
     * \sa SetBoolProperty, IsOn
     */
    bool GetBoolProperty(const char *propertyKey, bool &boolValue, const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for int properties (instances of IntProperty).
     *
     * \param[in]  propertyKey  The key of the IntProperty.
     * \param[out] intValue     Receives the integer value if the property is found.
     * \param[in]  renderer     If non-null, the renderer-specific PropertyList is checked first.
     * \return \a true if the property was found.
     * \sa SetIntProperty
     */
    bool GetIntProperty(const char *propertyKey, int &intValue, const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for float properties (instances of FloatProperty).
     *
     * \param[in]  propertyKey  The key of the FloatProperty.
     * \param[out] floatValue   Receives the float value if the property is found.
     * \param[in]  renderer     If non-null, the renderer-specific PropertyList is checked first.
     * \return \a true if the property was found.
     * \sa SetFloatProperty
     */
    bool GetFloatProperty(const char *propertyKey,
                          float &floatValue,
                          const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for double properties (instances of DoubleProperty).
     *
     * If there is no DoubleProperty for the given \a propertyKey, the method
     * falls back to looking for a corresponding FloatProperty instance and
     * promoting its value to double.
     *
     * \param[in]  propertyKey  The key of the DoubleProperty (or FloatProperty).
     * \param[out] doubleValue  Receives the double value if the property is found.
     * \param[in]  renderer     If non-null, the renderer-specific PropertyList is checked first.
     * \return \a true if the property was found (either as DoubleProperty or FloatProperty).
     * \sa SetDoubleProperty, GetFloatProperty
     */
    bool GetDoubleProperty(const char *propertyKey,
                           double &doubleValue,
                           const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for string properties (instances of StringProperty).
     *
     * \param[in]  propertyKey  The key of the StringProperty.
     * \param[out] string       Receives the string value if the property is found.
     * \param[in]  renderer     If non-null, the renderer-specific PropertyList is checked first.
     * \return \a true if the property was found.
     * \sa SetStringProperty
     */
    bool GetStringProperty(const char *propertyKey,
                           std::string &string,
                           const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for color properties (instances of ColorProperty).
     *
     * \param[out] rgb          Array of 3 floats receiving the RGB color values.
     * \param[in]  renderer     If non-null, the renderer-specific PropertyList is checked first.
     * \param[in]  propertyKey  The key of the ColorProperty (default: "color").
     * \return \a true if the property was found.
     * \sa SetColor
     */
    bool GetColor(float rgb[3], const mitk::BaseRenderer *renderer = nullptr, const char *propertyKey = "color") const;

    /**
     * \brief Convenience access method for level-window properties (instances of LevelWindowProperty).
     *
     * \param[out] levelWindow  Receives the LevelWindow value if the property is found.
     * \param[in]  renderer     If non-null, the renderer-specific PropertyList is checked first.
     * \param[in]  propertyKey  The key of the LevelWindowProperty (default: "levelwindow").
     * \return \a true if the property was found.
     * \sa SetLevelWindow
     */
    bool GetLevelWindow(mitk::LevelWindow &levelWindow,
                        const mitk::BaseRenderer *renderer = nullptr,
                        const char *propertyKey = "levelwindow") const;

    /**
     * \brief Set the selection state of this node.
     *
     * Sets the "selected" BoolProperty. If it does not exist, it is created.
     * A ModifiedEvent is invoked if the selection state changes.
     *
     * \param[in] selected  The new selection state.
     * \param[in] renderer  If non-null, the property is set in the renderer-specific PropertyList.
     * \sa IsSelected
     */
    void SetSelected(bool selected, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Query whether this node is currently selected.
     *
     * \param[in] renderer  If non-null, the renderer-specific "selected" property is checked.
     * \return \a true if the node is selected, \a false if unselected or the property is missing.
     * \sa SetSelected
     */
    bool IsSelected(const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Convenience access method for accessing the name of an object (instance of
     * StringProperty with property-key "name")
     * \return \a true property was found
     */
    bool GetName(std::string &nodeName,
                 const mitk::BaseRenderer *renderer = nullptr,
                 const char *propertyKey = "name") const
    {
      return GetStringProperty(propertyKey, nodeName, renderer);
    }

    /**
     * \brief Extra convenience access method for accessing the name of an object (instance of
     * StringProperty with property-key "name").
     *
     * This method does not take the renderer specific
     * propertylists into account, because the name of an object should never be renderer specific.
     * \returns a std::string with the name of the object (content of "name" Property).
     * If there is no "name" Property, an empty string will be returned.
     */
    virtual std::string GetName() const
    {
      mitk::StringProperty *sp = dynamic_cast<mitk::StringProperty *>(this->GetProperty("name"));
      if (sp == nullptr)
        return "";
      return sp->GetValue();
    }

    /**
     * \brief Sentinel value indicating that a node name has not been set yet.
     * \return The string "No Name!".
     */
    static std::string NO_NAME_VALUE()
    {
      return "No Name!";
    }

    /**
     * \brief Extra convenience access method to set the name of an object.
     *
     * If the data has already a "name" property, the name will be stored in it. Otherwise, the name will be stored in
     * the non-renderer-specific PropertyList in a StringProperty named "name".
     */
    virtual void SetName(const char *name)
    {
      if (name == nullptr)
        return;

      auto* data = this->GetData();

      if (nullptr != data)
      {
        auto property = data->GetProperty("name");

        if (property.IsNotNull())
        {
          auto* stringProperty = dynamic_cast<StringProperty*>(property.GetPointer());

          if (nullptr != stringProperty)
          {
            stringProperty->SetValue(name);
            return;
          }
        }
      }

      this->SetStringProperty("name", name);
    }
    /**
     * \brief Extra convenience access method to set the name of an object.
     *
     * \sa SetName(const char*)
     */
    virtual void SetName(const std::string& name) { this->SetName(name.c_str()); }
    /**
     * \brief Convenience access method for visibility properties (instances
     * of BoolProperty with property-key "visible")
     * \return \a true property was found
     * \sa IsVisible
     */
    bool GetVisibility(bool &visible, const mitk::BaseRenderer *renderer, const char *propertyKey = "visible") const
    {
      return GetBoolProperty(propertyKey, visible, renderer);
    }

    /**
     * \brief Convenience access method for opacity properties (instances of FloatProperty).
     *
     * \param[out] opacity      Receives the opacity value (0.0 to 1.0) if the property is found.
     * \param[in]  renderer     The renderer whose PropertyList is checked first.
     * \param[in]  propertyKey  The key of the FloatProperty (default: "opacity").
     * \return \a true if the property was found.
     * \sa SetOpacity
     */
    bool GetOpacity(float &opacity, const mitk::BaseRenderer *renderer, const char *propertyKey = "opacity") const;

    /**
     * \brief Convenience access method for boolean properties (instances
     * of BoolProperty). Return value is the value of the property. If the property is
     * not found, the value of \a defaultIsOn is returned.
     *
     * Thus, the return value has a different meaning than in the
     * GetBoolProperty method!
     * \sa GetBoolProperty
     */
    bool IsOn(const char *propertyKey, const mitk::BaseRenderer *renderer, bool defaultIsOn = true) const
    {
      if (propertyKey == nullptr)
        return defaultIsOn;
      GetBoolProperty(propertyKey, defaultIsOn, renderer);
      return defaultIsOn;
    }

    /**
     * \brief Convenience access method for visibility properties (instances
     * of BoolProperty). Return value is the visibility. Default is
     * visible==true, i.e., true is returned even if the property (\a
     * propertyKey) is not found.
     *
     * Thus, the return value has a different meaning than in the
     * GetVisibility method!
     * \sa GetVisibility
     * \sa IsOn
     */
    bool IsVisible(const mitk::BaseRenderer *renderer,
                   const char *propertyKey = "visible",
                   bool defaultIsOn = true) const
    {
      return IsOn(propertyKey, renderer, defaultIsOn);
    }

    /**
     * \brief Convenience method for setting color properties (instances of ColorProperty).
     *
     * \param[in] color        The color value to set.
     * \param[in] renderer     If non-null, set in the renderer-specific PropertyList.
     * \param[in] propertyKey  The key of the ColorProperty (default: "color").
     * \sa GetColor
     */
    void SetColor(const mitk::Color &color,
                  const mitk::BaseRenderer *renderer = nullptr,
                  const char *propertyKey = "color");

    /**
     * \brief Convenience method for setting color properties from individual RGB components.
     *
     * \param[in] red          Red component (0.0 to 1.0).
     * \param[in] green        Green component (0.0 to 1.0).
     * \param[in] blue         Blue component (0.0 to 1.0).
     * \param[in] renderer     If non-null, set in the renderer-specific PropertyList.
     * \param[in] propertyKey  The key of the ColorProperty (default: "color").
     * \sa GetColor
     */
    void SetColor(float red,
                  float green,
                  float blue,
                  const mitk::BaseRenderer *renderer = nullptr,
                  const char *propertyKey = "color");

    /**
     * \brief Convenience method for setting color properties from a float array.
     *
     * \param[in] rgb          Array of 3 floats (red, green, blue), each in range 0.0 to 1.0.
     * \param[in] renderer     If non-null, set in the renderer-specific PropertyList.
     * \param[in] propertyKey  The key of the ColorProperty (default: "color").
     * \sa GetColor
     */
    void SetColor(const float rgb[3], const mitk::BaseRenderer *renderer = nullptr, const char *propertyKey = "color");

    /**
     * \brief Convenience method for setting visibility properties (instances
     * of BoolProperty)
     * \param visible If set to true, the data will be rendered. If false, the render will skip this data.
     * \param renderer Specify a renderer if the visibility shall be specific to a renderer
     * \param propertyKey Can be used to specify a user defined name of the visibility property.
     */
    void SetVisibility(bool visible, const mitk::BaseRenderer *renderer = nullptr, const char *propertyKey = "visible");

    /**
     * \brief Convenience method for setting opacity properties (instances of FloatProperty).
     *
     * \param[in] opacity      The opacity value (0.0 fully transparent to 1.0 fully opaque).
     * \param[in] renderer     If non-null, set in the renderer-specific PropertyList.
     * \param[in] propertyKey  The key of the FloatProperty (default: "opacity").
     * \sa GetOpacity
     */
    void SetOpacity(float opacity, const mitk::BaseRenderer *renderer = nullptr, const char *propertyKey = "opacity");

    /**
     * \brief Convenience method for setting level-window properties (instances of LevelWindowProperty).
     *
     * \param[in] levelWindow  The LevelWindow value to set.
     * \param[in] renderer     If non-null, set in the renderer-specific PropertyList.
     * \param[in] propertyKey  The key of the LevelWindowProperty (default: "levelwindow").
     * \sa GetLevelWindow
     */
    void SetLevelWindow(mitk::LevelWindow levelWindow,
                        const mitk::BaseRenderer *renderer = nullptr,
                        const char *propertyKey = "levelwindow");

    /**
     * \brief Convenience method for setting int properties (instances of IntProperty).
     *
     * \param[in] propertyKey  The key of the IntProperty.
     * \param[in] intValue     The integer value to set.
     * \param[in] renderer     If non-null, set in the renderer-specific PropertyList.
     * \sa GetIntProperty
     */
    void SetIntProperty(const char *propertyKey, int intValue, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Convenience method for setting boolean properties (instances of BoolProperty).
     *
     * \param[in] propertyKey  The key of the BoolProperty.
     * \param[in] boolValue    The boolean value to set.
     * \param[in] renderer     If non-null, set in the renderer-specific PropertyList.
     * \sa GetBoolProperty
     */
    void SetBoolProperty(const char *propertyKey, bool boolValue, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Convenience method for setting float properties (instances of FloatProperty).
     *
     * \note If a DoubleProperty with the same key already exists, a warning is issued.
     *
     * \param[in] propertyKey  The key of the FloatProperty.
     * \param[in] floatValue   The float value to set.
     * \param[in] renderer     If non-null, set in the renderer-specific PropertyList.
     * \sa GetFloatProperty
     */
    void SetFloatProperty(const char *propertyKey, float floatValue, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Convenience method for setting double properties (instances of DoubleProperty).
     *
     * \note If a FloatProperty with the same key already exists, a warning is issued.
     *
     * \param[in] propertyKey   The key of the DoubleProperty.
     * \param[in] doubleValue   The double value to set.
     * \param[in] renderer      If non-null, set in the renderer-specific PropertyList.
     * \sa GetDoubleProperty
     */
    void SetDoubleProperty(const char *propertyKey, double doubleValue, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Convenience method for setting string properties (instances of StringProperty).
     *
     * \param[in] propertyKey  The key of the StringProperty.
     * \param[in] string       The string value to set.
     * \param[in] renderer     If non-null, set in the renderer-specific PropertyList.
     * \sa GetStringProperty
     */
    void SetStringProperty(const char *propertyKey, const char *string, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Get the timestamp of the last change of the contents of this node or
     * the referenced BaseData.
     */
    itk::ModifiedTimeType GetMTime() const override;

    /**
     * \brief Get the timestamp of the last change of the reference to the
     * BaseData.
     */
    unsigned long GetDataReferenceChangedTime() const { return m_DataReferenceChangedTime.GetMTime(); }

  protected:
    DataNode();

    ~DataNode() override;

    /// Invoked when the property list was modified. Calls Modified() of the DataNode
    virtual void PropertyListModified(const itk::Object *caller, const itk::EventObject &event);

    /// \brief Mapper-slots
    mutable MapperVector m_Mappers;

    /**
     * \brief The data object (instance of BaseData, e.g., an Image) managed
     * by this DataNode
     */
    BaseData::Pointer m_Data;

    /**
     * \brief BaseRenderer-independent PropertyList
     *
     * Properties herein can be overwritten specifically for each BaseRenderer
     * by the BaseRenderer-specific properties defined in m_MapOfPropertyLists.
     */
    PropertyList::Pointer m_PropertyList;

    /// \brief Map associating each BaseRenderer with its own PropertyList
    mutable MapOfPropertyLists m_MapOfPropertyLists;

    DataInteractor::Pointer m_DataInteractor;

    /// \brief Timestamp of the last change of m_Data
    itk::TimeStamp m_DataReferenceChangedTime;

    unsigned long m_PropertyListModifiedObserverTag;
  };

  /**
   * \brief Stream input operator for DataNode smart pointers.
   *
   * Creates a new empty DataNode and assigns it to \a dtn.
   *
   * \param[in,out] i    The input stream.
   * \param[out]    dtn  Receives a newly created DataNode.
   * \return The input stream.
   */
  MITKCORE_EXPORT std::istream &operator>>(std::istream &i, DataNode::Pointer &dtn);

  /**
   * \brief Stream output operator for DataNode smart pointers.
   *
   * Writes the class name of the node's data object to the stream, or "empty data" if no data is set.
   *
   * \param[in,out] o    The output stream.
   * \param[in]     dtn  The DataNode to write.
   * \return The output stream.
   */
  MITKCORE_EXPORT std::ostream &operator<<(std::ostream &o, DataNode::Pointer &dtn);
} // namespace mitk

#endif
