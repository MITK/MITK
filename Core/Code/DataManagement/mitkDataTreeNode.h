/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef DATATREENODE_H_HEADER_INCLUDED_C1E14338
#define DATATREENODE_H_HEADER_INCLUDED_C1E14338

#include "mitkImageSource.h"
#include "mitkBaseData.h"
#include "mitkMapper.h"
#include "mitkInteractor.h"

#ifdef MBI_NO_STD_NAMESPACE
#define MBI_STD
#include <iostream.h>
#include <fstream.h>
#else
#define MBI_STD std
#include <iostream>
#include <fstream>
#endif

#include "mitkStringProperty.h"
#include "mitkColorProperty.h"
#include "mitkPropertyList.h"
#include "mitkMapper.h"

#include <map>
#include <set>
#include "mitkLevelWindow.h"
#include "mitkXMLIO.h"

class vtkLinearTransform;

namespace mitk {

class BaseRenderer;

//##Documentation
//## @brief Class for nodes of the DataTree
//##
//## Contains the data (instance of BaseData), a list of mappers, which can
//## draw the data, a transform (vtkTransform) and a list of properties
//## (PropertyList).
//## @ingroup DataManagement
//##
//## @todo clean up all the GetProperty methods. There are too many different flavours... Can most probably be reduced to <tt>bool GetProperty<type>(type&)</tt>
//##
//## @warning Change in semantics of SetProperty() since Aug 25th 2006. Check your usage of this method if you do
//##          more with properties than just call <tt>SetProperty( "key", new SomeProperty("value") )</tt>.
class MITK_CORE_EXPORT DataTreeNode : public itk::DataObject, public XMLIO
{
public:

  typedef mitk::Geometry3D::Pointer Geometry3DPointer;
  typedef std::vector<Mapper::Pointer> MapperVector;
  typedef std::map<const mitk::BaseRenderer*,mitk::PropertyList::Pointer> MapOfPropertyLists;
  typedef std::set<std::string> GroupTagList;

  mitkClassMacro(DataTreeNode, itk::DataObject);

  itkNewMacro(Self);

  mitk::Mapper* GetMapper(MapperSlotId id) const;
  //##Documentation
  //## @brief Get the data object (instance of BaseData, e.g., an Image)
  //## managed by this DataTreeNode
  BaseData* GetData() const;
  //##Documentation
  //## @brief Get the transformation applied prior to displaying the data as
  //## a vtkTransform
  //## \deprecated use GetData()->GetGeometry()->GetVtkTransform() instead
  vtkLinearTransform* GetVtkTransform(int t=0) const;
  //##Documentation
  //## @brief Get the Interactor
  Interactor* GetInteractor() const;
  //##Documentation
  //## @brief Set the data object (instance of BaseData, e.g., an Image)
  //## managed by this DataTreeNode
  //## @warning the actor-mode of the vtkInteractor does not work any more, if the transform of the
  //## data-tree-node is connected to the transform of the basedata via vtkTransform->SetInput.
  virtual void SetData(mitk::BaseData* baseData);
  //##Documentation
  //## @brief Set the Interactor
  virtual void SetInteractor(Interactor* interactor);

  mitk::DataTreeNode& operator=(const DataTreeNode& right);

  mitk::DataTreeNode& operator=(BaseData* right);
  virtual void SetMapper(MapperSlotId id, mitk::Mapper* mapper);
  virtual void UpdateOutputInformation();

  virtual void SetRequestedRegionToLargestPossibleRegion();

  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

  virtual bool VerifyRequestedRegion();

  virtual void SetRequestedRegion(itk::DataObject *data);

  virtual void CopyInformation(const itk::DataObject *data);

  //##Documentation
  //## @brief Set the property (instance of BaseProperty) with key @a propertyKey in the PropertyList
  //## of the @a renderer (if NULL, use BaseRenderer-independent PropertyList). This is set-by-value.
  //##
  //## @warning Change in semantics since Aug 25th 2006. Check your usage of this method if you do
  //##          more with properties than just call <tt>SetProperty( "key", new SomeProperty("value") )</tt>.
  //##
  //## @sa GetProperty
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  void SetProperty(const char *propertyKey, BaseProperty* property, const mitk::BaseRenderer* renderer = NULL);

  //##Documentation
  //## @brief Replace the property (instance of BaseProperty) with key @a propertyKey in the PropertyList
  //## of the @a renderer (if NULL, use BaseRenderer-independent PropertyList). This is set-by-reference.
  //##
  //## If @a renderer is @a NULL the property is set in the BaseRenderer-independent
  //## PropertyList of this DataTreeNode.
  //## @sa GetProperty
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  void ReplaceProperty(const char *propertyKey, BaseProperty* property, const mitk::BaseRenderer* renderer = NULL);

  //##Documentation
  //## @brief Add the property (instance of BaseProperty) if it does
  //## not exist (or always if \a overwrite is \a true)
  //## with key @a propertyKey in the PropertyList
  //## of the @a renderer (if NULL, use BaseRenderer-independent
  //## PropertyList). This is set-by-value.
  //##
  //## For \a overwrite == \a false the property is \em not changed
  //## if it already exists. For \a overwrite == \a true the method
  //## is identical to SetProperty.
  //##
  //## @sa SetProperty
  //## @sa GetProperty
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  void AddProperty(const char *propertyKey, BaseProperty* property, const mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

  //##Documentation
  //## @brief Get the PropertyList of the @a renderer. If @a renderer is @a
  //## NULL, the BaseRenderer-independent PropertyList of this DataTreeNode
  //## is returned.
  //## @sa GetProperty
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  mitk::PropertyList* GetPropertyList(const mitk::BaseRenderer* renderer = NULL) const;

  //##Documentation
  //## @brief Add values from another PropertyList.
  //##
  //## Overwrites values in m_PropertyList only when possible (i.e. when types are compatible).
  //## If you want to allow for object type changes (replacing a "visible":BoolProperty with "visible":IntProperty,
  //## set the @param replace.
  //##
  //## @param replace true: if @param pList contains a property "visible" of type ColorProperty and our m_PropertyList also has a "visible" property of a different type (e.g. BoolProperty), change the type, i.e. replace the objects behind the pointer.
  //##
  //## @sa SetProperty
  //## @sa ReplaceProperty
  //## @sa m_PropertyList
  void ConcatenatePropertyList(PropertyList* pList, bool replace = false);

  //##Documentation
  //## @brief Get the property (instance of BaseProperty) with key @a propertyKey from the PropertyList
  //## of the @a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
  //##
  //## If @a renderer is @a NULL or the @a propertyKey cannot be found
  //## in the PropertyList specific to @a renderer or is disabled there, the BaseRenderer-independent
  //## PropertyList of this DataTreeNode is queried.
  //## @sa GetPropertyList
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  mitk::BaseProperty* GetProperty(const char *propertyKey, const mitk::BaseRenderer* renderer = NULL) const;

  //##Documentation
  //## @brief Get the property of type T with key @a propertyKey from the PropertyList
  //## of the @a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
  //##
  //## If @a renderer is @a NULL or the @a propertyKey cannot be found
  //## in the PropertyList specific to @a renderer or is disabled there, the BaseRenderer-independent
  //## PropertyList of this DataTreeNode is queried.
  //## @sa GetPropertyList
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  template <typename T>
    bool GetProperty(itk::SmartPointer<T> &property, const char *propertyKey, const mitk::BaseRenderer* renderer = NULL) const
  {
    property = dynamic_cast<T *>(GetProperty(propertyKey, renderer));
    return property.IsNotNull();
  }

  //##Documentation
  //## @brief Get the property of type T with key @a propertyKey from the PropertyList
  //## of the @a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
  //##
  //## If @a renderer is @a NULL or the @a propertyKey cannot be found
  //## in the PropertyList specific to @a renderer or is disabled there, the BaseRenderer-independent
  //## PropertyList of this DataTreeNode is queried.
  //## @sa GetPropertyList
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  template <typename T>
    bool GetProperty(T* &property, const char *propertyKey, const mitk::BaseRenderer* renderer = NULL) const
  {
    property = dynamic_cast<T *>(GetProperty(propertyKey, renderer));
    return property!=NULL;
  }

  //##Documentation
  //## @brief Convenience access method for GenericProperty<T> properties
  //## (T being the type of the second parameter)
  //## @return @a true property was found
  template <typename T>
    bool GetPropertyValue(const char* propertyKey, T & value, mitk::BaseRenderer* renderer=NULL) const
#ifdef _MSC_VER
    {
      GenericProperty<T>* gp= dynamic_cast<GenericProperty<T>*>(GetProperty(propertyKey, renderer));
      if ( gp != NULL )
      {
        value = gp->GetValue();
        return true;
      }
      return false;
    }
#else
  ;
#endif

  // @brief Get a set of all group tags from this node's property list
  GroupTagList GetGroupTags() const;

  //##Documentation
  //## @brief Convenience access method for bool properties (instances of
  //## BoolProperty)
  //## @return @a true property was found
  bool GetBoolProperty(const char* propertyKey, bool &boolValue, mitk::BaseRenderer* renderer = NULL) const;

  //##Documentation
  //## @brief Convenience access method for int properties (instances of
  //## IntProperty)
  //## @return @a true property was found
  bool GetIntProperty(const char* propertyKey, int &intValue, mitk::BaseRenderer* renderer=NULL) const;

  //##Documentation
  //## @brief Convenience access method for float properties (instances of
  //## FloatProperty)
  //## @return @a true property was found
  bool GetFloatProperty(const char* propertyKey, float &floatValue, mitk::BaseRenderer* renderer = NULL) const;

  //##Documentation
  //## @brief Convenience access method for string properties (instances of
  //## StringProperty)
  //## @return @a true property was found
  bool GetStringProperty(const char* propertyKey, std::string& string, mitk::BaseRenderer* renderer = NULL) const;

  //##Documentation
  //## @brief Convenience access method for color properties (instances of
  //## ColorProperty)
  //## @return @a true property was found
  bool GetColor(float rgb[3], mitk::BaseRenderer* renderer = NULL, const char* propertyKey = "color") const;

  //##Documentation
  //## @brief Convenience access method for level-window properties (instances of
  //## LevelWindowProperty)
  //## @return @a true property was found
  bool GetLevelWindow(mitk::LevelWindow &levelWindow, mitk::BaseRenderer* renderer = NULL, const char* propertyKey = "levelwindow") const;

  //##
  //##Documentation
  //## @brief set the node as selected
  void SetSelected(bool selected, mitk::BaseRenderer* renderer=NULL);

  //##
  //##Documentation
  //## @brief set the node as selected
  //## @return @a true node is selected
  bool IsSelected(mitk::BaseRenderer* renderer=NULL);

  //##Documentation
  //## @brief Convenience access method for accessing the name of an object (instance of
  //## StringProperty with property-key "name")
  //## @return @a true property was found
  bool GetName(std::string& nodeName, mitk::BaseRenderer* renderer = NULL, const char* propertyKey = "name") const
  {
    return GetStringProperty(propertyKey, nodeName, renderer);
  }

  //##Documentation
  //## @brief Extra convenience access method for accessing the name of an object (instance of
  //## StringProperty with property-key "name").
  //##
  //## This method does not take the renderer specific
  //## propertylists into account, because the name of an object should never be renderer specific.
  //## @returns a std::string with the name of the object (content of "name" Property).
  //## If there is no "name" Property, an empty string will be returned.
  virtual std::string GetName() const
  {
    mitk::StringProperty* sp = dynamic_cast<mitk::StringProperty*>(this->GetProperty("name"));
    if (sp == NULL)
      return "";
    return sp->GetValue();
  }

  //##Documentation
  //## @brief Extra convenience access method to set the name of an object.
  //##
  //## The name will be stored in the non-renderer-specific PropertyList in a StringProperty named "name".
  virtual void SetName( const char* name)
  {
    if (name == NULL)
      return;
    this->SetProperty("name", StringProperty::New(name));
  }

  //##Documentation
  //## @brief Convenience access method for visibility properties (instances
  //## of BoolProperty with property-key "visible")
  //## @return @a true property was found
  //## @sa IsVisible
  bool GetVisibility(bool &visible, mitk::BaseRenderer* renderer, const char* propertyKey = "visible") const
  {
    return GetBoolProperty(propertyKey, visible, renderer);
  }

  //##Documentation
  //## @brief Convenience access method for opacity properties (instances of
  //## FloatProperty)
  //## @return @a true property was found
  bool GetOpacity(float &opacity, mitk::BaseRenderer* renderer, const char* propertyKey = "opacity") const;

  //##Documentation
  //## @brief Convenience access method for boolean properties (instances
  //## of BoolProperty). Return value is the value of the property. If the property is
  //## not found, the value of @a defaultIsOn is returned.
  //##
  //## Thus, the return value has a different meaning than in the
  //## GetBoolProperty method!
  //## @sa GetBoolProperty
  bool IsOn(const char* propertyKey, mitk::BaseRenderer* renderer, bool defaultIsOn = true) const
  {
    if(propertyKey==NULL)
      return defaultIsOn;
    GetBoolProperty(propertyKey, defaultIsOn, renderer);
    return defaultIsOn;
  }

  //##Documentation
  //## @brief Convenience access method for visibility properties (instances
  //## of BoolProperty). Return value is the visibility. Default is
  //## visible==true, i.e., true is returned even if the property (@a
  //## propertyKey) is not found.
  //##
  //## Thus, the return value has a different meaning than in the
  //## GetVisibility method!
  //## @sa GetVisibility
  //## @sa IsOn
  bool IsVisible(mitk::BaseRenderer* renderer, const char* propertyKey = "visible", bool defaultIsOn = true) const
  {
    return IsOn(propertyKey, renderer, defaultIsOn);
  }

  //##Documentation
  //## @brief Convenience method for setting color properties (instances of
  //## ColorProperty)
  void SetColor(const mitk::Color &color, mitk::BaseRenderer* renderer = NULL, const char* propertyKey = "color");
  //##Documentation
  //## @brief Convenience method for setting color properties (instances of
  //## ColorProperty)
  void SetColor(float red, float green, float blue, mitk::BaseRenderer* renderer = NULL, const char* propertyKey = "color");
  //##Documentation
  //## @brief Convenience method for setting color properties (instances of
  //## ColorProperty)
  void SetColor(const float rgb[3], mitk::BaseRenderer* renderer = NULL, const char* propertyKey = "color");

  //##Documentation
  //## @brief Convenience method for setting visibility properties (instances
  //## of BoolProperty)
  //## @param visible If set to true, the data will be rendered. If false, the render will skip this data.
  //## @param renderer Specify a renderer if the visibility shall be specific to a renderer
  //## @param propertykey Can be used to specify a user defined name of the visibility propery.
  void SetVisibility(bool visible, mitk::BaseRenderer* renderer = NULL, const char* propertyKey = "visible");

  //##Documentation
  //## @brief Convenience method for setting opacity properties (instances of
  //## FloatProperty)
  void SetOpacity(float opacity, mitk::BaseRenderer* renderer = NULL, const char* propertyKey = "opacity");

  //##Documentation
  //## @brief Convenience method for setting level-window properties
  //## (instances of LevelWindowProperty)
  void SetLevelWindow(mitk::LevelWindow levelWindow, mitk::BaseRenderer* renderer = NULL, const char* propertyKey = "levelwindow");

  //##Documentation
  //## @brief Convenience method for setting int properties (instances of
  //## IntProperty)
  void SetIntProperty(const char* propertyKey, int intValue, mitk::BaseRenderer* renderer=NULL);

  //##Documentation
  //## @brief Convenience method for setting int properties (instances of
  //## IntProperty)
  void SetBoolProperty(const char* propertyKey, bool boolValue, mitk::BaseRenderer* renderer=NULL);

  //##Documentation
  //## @brief Convenience method for setting int properties (instances of
  //## IntProperty)
  void SetFloatProperty(const char* propertyKey, float floatValue, mitk::BaseRenderer* renderer=NULL);

  //##Documentation
  //## @brief Convenience method for setting int properties (instances of
  //## IntProperty)
  void SetStringProperty(const char* propertyKey, const char* string, mitk::BaseRenderer* renderer=NULL);

  //##Documentation
  //## @brief Get the timestamp of the last change of the contents of this node or
  //## the referenced BaseData.
  virtual unsigned long GetMTime() const;

  //##Documentation
  //## @brief Get the timestamp of the last change of the reference to the
  //## BaseData.
  unsigned long GetDataReferenceChangedTime() const
  {
    return m_DataReferenceChangedTime.GetMTime();
  }

  //##Documentation
  //## @brief Adds or removes the associated interactor to mitk::GLobalInteraction.
  //##
  virtual void SetInteractorEnabled( const bool& enabled );

  //##Documentation
  //## @brief Adds the interactor to mitk::GlobalInteraction
  //##
  virtual void EnableInteractor();

  //##Documentation
  //## @brief Removes the Interactor from mitk::GlobalInteraction
  //##
  virtual void DisableInteractor();

  //##Documentation
  //## @brief Tests, if the interactor is already added to mitk::GlobalInteraction
  //##
  virtual bool IsInteractorEnabled() const;

  //##
  virtual bool WriteXMLData( XMLWriter& xmlWriter );
  //##
  virtual bool ReadXMLData( XMLReader& xmlReader );

  virtual const std::string& GetXMLNodeName() const;

  static const std::string XML_NODE_NAME;

protected:
  DataTreeNode();

  virtual ~DataTreeNode();

  //##Documentation
  //## @brief Mapper-slots
  mutable MapperVector m_Mappers;

  //##Documentation
  //## @brief The data object (instance of BaseData, e.g., an Image) managed
  //## by this DataTreeNode
  BaseData::Pointer m_Data;

  //##Documentation
  //## @brief BaseRenderer-independent PropertyList
  //##
  //## Properties herein can be overwritten specifically for each BaseRenderer
  //## by the BaseRenderer-specific properties defined in m_MapOfPropertyLists.
  PropertyList::Pointer m_PropertyList;

  //##Documentation
  //## @brief Map associating each BaseRenderer with its own PropertyList
  mutable MapOfPropertyLists m_MapOfPropertyLists;

  //##Documentation
  //## @brief Interactor, that handles the Interaction
  Interactor::Pointer m_Interactor;

  //##Documentation
  //## @brief Timestamp of the last change of m_Data
  itk::TimeStamp m_DataReferenceChangedTime;
};


#if (_MSC_VER > 1200) || !defined(_MSC_VER)
MBI_STD::istream& operator>>( MBI_STD::istream& i, DataTreeNode::Pointer& dtn );

MBI_STD::ostream& operator<<( MBI_STD::ostream& o, DataTreeNode::Pointer& dtn);
#endif
} // namespace mitk

#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
MBI_STD::istream& operator>>( MBI_STD::istream& i, mitk::DataTreeNode::Pointer& dtn );

MBI_STD::ostream& operator<<( MBI_STD::ostream& o, mitk::DataTreeNode::Pointer& dtn);
#endif


#endif /* DATATREENODE_H_HEADER_INCLUDED_C1E14338 */


