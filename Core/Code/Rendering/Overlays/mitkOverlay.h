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

#ifndef OVERLAY_H
#define OVERLAY_H

#include <MitkExports.h>
#include <itkObject.h>
#include <mitkCommon.h>
#include "mitkBaseRenderer.h"


namespace mitk {

class MITK_CORE_EXPORT Overlay : public itk::Object {
public:

  typedef std::map<const mitk::BaseRenderer*,mitk::PropertyList::Pointer> MapOfPropertyLists;

  /** \brief Base class for mapper specific rendering ressources.
   */
  class MITK_CORE_EXPORT BaseLocalStorage
  {
  public:


    bool IsGenerateDataRequired(mitk::BaseRenderer *renderer,mitk::Overlay *overlay);

    inline void UpdateGenerateDataTime()
    {
      m_LastGenerateDataTime.Modified();
    }

    PropertyList::Pointer GetPropertyList();

    inline itk::TimeStamp & GetLastGenerateDataTime() { return m_LastGenerateDataTime; }



  protected:

    /** \brief timestamp of last update of stored data */
    itk::TimeStamp m_LastGenerateDataTime;

    PropertyList::Pointer m_PropertyList;

  };

  virtual void UpdateOverlay(BaseRenderer *renderer) = 0;

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
  //## PropertyList of this DataNode.
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
  //## NULL, the BaseRenderer-independent PropertyList of this DataNode
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
  //## PropertyList of this DataNode is queried.
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
  //## PropertyList of this DataNode is queried.
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
  //## PropertyList of this DataNode is queried.
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
    {
      GenericProperty<T>* gp= dynamic_cast<GenericProperty<T>*>(GetProperty(propertyKey, renderer));
      if ( gp != NULL )
      {
        value = gp->GetValue();
        return true;
      }
      return false;
    }

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
  //## @brief Extra convenience access method to set the name of an object.
  //##
  //## The name will be stored in the non-renderer-specific PropertyList in a StringProperty named "name".
  virtual void SetName( const std::string name)
  {
    this->SetName(name.c_str());
  }

  mitkClassMacro(Overlay, itk::Object);

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit Overlay();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~Overlay();

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
  //## @brief Timestamp of the last change of m_Data
  itk::TimeStamp m_DataReferenceChangedTime;

  unsigned long m_PropertyListModifiedObserverTag;

private:

  /** \brief copy constructor */
  Overlay( const Overlay &);

  /** \brief assignment operator */
  Overlay &operator=(const Overlay &);

};

} // namespace mitk
#endif // OVERLAY_H


