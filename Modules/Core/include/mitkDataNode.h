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

#ifndef DATATREENODE_H_HEADER_INCLUDED_C1E14338
#define DATATREENODE_H_HEADER_INCLUDED_C1E14338

#include "mitkBaseData.h"
//#include "mitkMapper.h"
#include "mitkDataInteractor.h"
#include "mitkIdentifiable.h"
#include "mitkIPropertyOwner.h"

#ifdef MBI_NO_STD_NAMESPACE
#define MBI_STD
#include <fstream.h>
#include <iostream.h>
#else
#define MBI_STD std
#include <fstream>
#include <iostream>
#endif

#include "mitkColorProperty.h"
#include "mitkPropertyList.h"
#include "mitkStringProperty.h"
//#include "mitkMapper.h"

#include "mitkGeometry3D.h"
#include "mitkLevelWindow.h"
#include <map>
#include <set>

class vtkLinearTransform;

namespace mitk
{
  class BaseRenderer;
  class Mapper;

  /**
   * \brief Class for nodes of the DataTree
   *
   * Contains the data (instance of BaseData), a list of mappers, which can
   * draw the data, a transform (vtkTransform) and a list of properties
   * (PropertyList).
   * \ingroup DataManagement
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
    typedef mitk::Geometry3D::Pointer Geometry3DPointer;
    typedef std::vector<itk::SmartPointer<Mapper>> MapperVector;
    typedef std::map<std::string, mitk::PropertyList::Pointer> MapOfPropertyLists;
    typedef std::vector<MapOfPropertyLists::key_type> PropertyListKeyNames;
    typedef std::set<std::string> GroupTagList;

    /**
     * \brief Definition of an itk::Event that is invoked when
     * a DataInteractor is set on this DataNode.
     */
    itkEventMacro(InteractorChangedEvent, itk::AnyEvent)
    mitkClassMacroItkParent(DataNode, itk::DataObject)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    // IPropertyProvider
    BaseProperty::ConstPointer GetConstProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = true) const override;
    std::vector<std::string> GetPropertyKeys(const std::string &contextName = "", bool includeDefaultContext = false) const override;
    std::vector<std::string> GetPropertyContextNames() const override;

    // IPropertyOwner
    BaseProperty * GetNonConstProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = true) override;
    void SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;
    void RemoveProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;

    mitk::Mapper *GetMapper(MapperSlotId id) const;

    /**
     * \brief Get the data object (instance of BaseData, e.g., an Image)
     * managed by this DataNode
     */
    BaseData *GetData() const;

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
     * \brief Set the Interactor.
     */
    virtual void SetDataInteractor(const DataInteractor::Pointer interactor);
    virtual DataInteractor::Pointer GetDataInteractor() const;

    mitk::DataNode &operator=(const DataNode &right);

    mitk::DataNode &operator=(BaseData *right);
    virtual void SetMapper(MapperSlotId id, mitk::Mapper *mapper);
    void UpdateOutputInformation() override;

    void SetRequestedRegionToLargestPossibleRegion() override;

    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    bool VerifyRequestedRegion() override;

    void SetRequestedRegion(const itk::DataObject *data) override;

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
    mitk::PropertyList *GetPropertyList(const std::string &rendererName) const;

    /**
     * \brief Add values from another PropertyList.
     *
     * Overwrites values in m_PropertyList only when possible (i.e. when types are compatible).
     * If you want to allow for object type changes (replacing a "visible":BoolProperty with "visible":IntProperty,
     * set the \param replace.
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
     * \brief Convenience access method for GenericProperty<T> properties
     * (T being the type of the second parameter)
     * \return \a true property was found
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
     * \brief Convenience access method for bool properties (instances of
     * BoolProperty)
     * \return \a true property was found
     */
    bool GetBoolProperty(const char *propertyKey, bool &boolValue, const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for int properties (instances of
     * IntProperty)
     * \return \a true property was found
     */
    bool GetIntProperty(const char *propertyKey, int &intValue, const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for float properties (instances of
     * FloatProperty)
     * \return \a true property was found
     */
    bool GetFloatProperty(const char *propertyKey,
                          float &floatValue,
                          const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for double properties (instances of
     * DoubleProperty)
     *
     * If there is no DoubleProperty for the given\c propertyKey argument, the method
     * looks for a corresponding FloatProperty instance.
     *
     * \return \a true property was found
     */
    bool GetDoubleProperty(const char *propertyKey,
                           double &doubleValue,
                           const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for string properties (instances of
     * StringProperty)
     * \return \a true property was found
     */
    bool GetStringProperty(const char *propertyKey,
                           std::string &string,
                           const mitk::BaseRenderer *renderer = nullptr) const;

    /**
     * \brief Convenience access method for color properties (instances of
     * ColorProperty)
     * \return \a true property was found
     */
    bool GetColor(float rgb[3], const mitk::BaseRenderer *renderer = nullptr, const char *propertyKey = "color") const;

    /**
     * \brief Convenience access method for level-window properties (instances of
     * LevelWindowProperty)
     * \return \a true property was found
     */
    bool GetLevelWindow(mitk::LevelWindow &levelWindow,
                        const mitk::BaseRenderer *renderer = nullptr,
                        const char *propertyKey = "levelwindow") const;

    /**
     * \brief set the node as selected
     */
    void SetSelected(bool selected, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief set the node as selected
     * \return \a true node is selected
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

    /** Value constant that is used indicate that node names are not set so far.*/
    static std::string NO_NAME_VALUE()
    {
      return "No Name!";
    }

    /**
     * \brief Extra convenience access method to set the name of an object.
     *
     * The name will be stored in the non-renderer-specific PropertyList in a StringProperty named "name".
     */
    virtual void SetName(const char *name)
    {
      if (name == nullptr)
        return;
      this->SetProperty("name", StringProperty::New(name));
    }
    /**
     * \brief Extra convenience access method to set the name of an object.
     *
     * The name will be stored in the non-renderer-specific PropertyList in a StringProperty named "name".
     */
    virtual void SetName(const std::string name) { this->SetName(name.c_str()); }
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
     * \brief Convenience access method for opacity properties (instances of
     * FloatProperty)
     * \return \a true property was found
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
     * \brief Convenience method for setting color properties (instances of
     * ColorProperty)
     */
    void SetColor(const mitk::Color &color,
                  const mitk::BaseRenderer *renderer = nullptr,
                  const char *propertyKey = "color");

    /**
     * \brief Convenience method for setting color properties (instances of
     * ColorProperty)
     */
    void SetColor(float red,
                  float green,
                  float blue,
                  const mitk::BaseRenderer *renderer = nullptr,
                  const char *propertyKey = "color");

    /**
     * \brief Convenience method for setting color properties (instances of
     * ColorProperty)
     */
    void SetColor(const float rgb[3], const mitk::BaseRenderer *renderer = nullptr, const char *propertyKey = "color");

    /**
     * \brief Convenience method for setting visibility properties (instances
     * of BoolProperty)
     * \param visible If set to true, the data will be rendered. If false, the render will skip this data.
     * \param renderer Specify a renderer if the visibility shall be specific to a renderer
     * \param propertykey Can be used to specify a user defined name of the visibility propery.
     */
    void SetVisibility(bool visible, const mitk::BaseRenderer *renderer = nullptr, const char *propertyKey = "visible");

    /**
     * \brief Convenience method for setting opacity properties (instances of
     * FloatProperty)
     */
    void SetOpacity(float opacity, const mitk::BaseRenderer *renderer = nullptr, const char *propertyKey = "opacity");

    /**
     * \brief Convenience method for setting level-window properties
     * (instances of LevelWindowProperty)
     */
    void SetLevelWindow(mitk::LevelWindow levelWindow,
                        const mitk::BaseRenderer *renderer = nullptr,
                        const char *propertyKey = "levelwindow");

    /**
     * \brief Convenience method for setting int properties (instances of
     * IntProperty)
     */
    void SetIntProperty(const char *propertyKey, int intValue, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Convenience method for setting boolean properties (instances of
     * BoolProperty)
     */
    void SetBoolProperty(const char *propertyKey, bool boolValue, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Convenience method for setting float properties (instances of
     * FloatProperty)
     */
    void SetFloatProperty(const char *propertyKey, float floatValue, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Convenience method for setting double properties (instances of
     * DoubleProperty)
     */
    void SetDoubleProperty(const char *propertyKey, double doubleValue, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Convenience method for setting string properties (instances of
     * StringProperty)
     */
    void SetStringProperty(const char *propertyKey, const char *string, const mitk::BaseRenderer *renderer = nullptr);

    /**
     * \brief Get the timestamp of the last change of the contents of this node or
     * the referenced BaseData.
     */
    unsigned long GetMTime() const override;

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

#if (_MSC_VER > 1200) || !defined(_MSC_VER)
  MITKCORE_EXPORT MBI_STD::istream &operator>>(MBI_STD::istream &i, DataNode::Pointer &dtn);

  MITKCORE_EXPORT MBI_STD::ostream &operator<<(MBI_STD::ostream &o, DataNode::Pointer &dtn);
#endif
} // namespace mitk

#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
MITKCORE_EXPORT MBI_STD::istream &operator>>(MBI_STD::istream &i, mitk::DataNode::Pointer &dtn);

MITKCORE_EXPORT MBI_STD::ostream &operator<<(MBI_STD::ostream &o, mitk::DataNode::Pointer &dtn);
#endif

#endif /* DATATREENODE_H_HEADER_INCLUDED_C1E14338 */
