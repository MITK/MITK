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

#ifndef PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D
#define PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D

#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include "mitkUIDGenerator.h"

#include <itkObjectFactory.h>

#include <string>
#include <map> 

namespace mitk {

class XMLWriter;

/**
 * @brief Key-value list holding instances of BaseProperty
 *
 * This list is meant to hold an arbitrary list of "properties", 
 * which should describe the object associated with this list.
 *
 * Usually you will use PropertyList as part of a DataTreeNode 
 * object - in this context the properties describe the data object
 * held by the DataTreeNode (e.g. whether the object is rendered at
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
 * @ingroup DataManagement
 */
class MITK_CORE_EXPORT PropertyList : public itk::Object
{

  public:

    mitkClassMacro(PropertyList, itk::Object);

    /** 
     * Method for creation through the object factory. 
     */
    itkNewMacro(Self);

    /**
     * Map structure to hold the properties: the map key is a string,
     * the value consists of the actual property object (BaseProperty) and
     * a bool flag (indicating whether a property is "enabled").
     *
     * The "enabled" flag is there to "keep a property without showing it", i.e.
     * GetProperty will not tell that there such a thing. Is there any real world use for this? (bug #1052)
     */
    typedef std::map< std::string,std::pair<BaseProperty::Pointer,bool> > PropertyMap;
    typedef std::pair< std::string,std::pair<BaseProperty::Pointer,bool> > PropertyMapElementType;

    /**
     * @brief Get a property by its name. 
     */
    mitk::BaseProperty* GetProperty(const std::string& propertyKey) const;

    /**
     * @brief Set a property in the list/map by value.
     * 
     * The actual OBJECT holding the value of the property is not replaced, but its value 
     * is modified to match that of @a property. To really replace the object holding the
     * property - which would make sense if you want to change the type (bool, string) of the property
     * - call ReplaceProperty.
     */
    void SetProperty(const std::string& propertyKey, BaseProperty* property);

    /**
     * @brief Set a property object in the list/map by reference.
     *
     * The actual OBJECT holding the value of the property is replaced by this function.
     * This is useful if you want to change the type of the property, like from BoolProperty to StringProperty.
     * Another use is to share one and the same property object among several ProperyList/DataTreeNode objects, which
     * makes them appear synchronized.
     */
    void ReplaceProperty(const std::string& propertyKey, BaseProperty* property);

    /**
     * @brief Set a property object in the list/map by reference.
     */ 
    void ConcatenatePropertyList(PropertyList *pList, bool replace = false);

    //##Documentation
    //## @brief Convenience access method for GenericProperty<T> properties 
    //## (T being the type of the second parameter)
    //## @return @a true property was found
    template <typename T>
    bool GetPropertyValue(const char* propertyKey, T & value) const
#ifdef _MSC_VER
    {
      GenericProperty<T>* gp= dynamic_cast<GenericProperty<T>*>(GetProperty(propertyKey));
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

    /**
    * @brief Convenience method to access the value of a BoolProperty
    */    
    bool GetBoolProperty(const char* propertyKey, bool& boolValue) const;
    
    /**
    * @brief Convenience method to set the value of a BoolProperty
    */
    void SetBoolProperty( const char* propertyKey, bool boolValue);
    
    /**
    * @brief Convenience method to access the value of an IntProperty
    */   
    bool GetIntProperty(const char* propertyKey, int &intValue) const;

    /**
    * @brief Convenience method to set the value of an IntProperty
    */
    void SetIntProperty(const char* propertyKey, int intValue);

    /**
    * @brief Convenience method to access the value of a FloatProperty
    */   
    bool GetFloatProperty(const char* propertyKey, float &floatValue) const;

    /**
    * @brief Convenience method to set the value of a FloatProperty
    */
    void SetFloatProperty( const char* propertyKey, float floatValue);

    /**
    * @brief Convenience method to access the value of a StringProperty
    */   
    bool GetStringProperty(const char* propertyKey, std::string& stringValue) const;

    /**
    * @brief Convenience method to set the value of a StringProperty
    */
    void SetStringProperty( const char* propertyKey, const char* stringValue);

    /**
     * @brief Get the timestamp of the last change of the map or the last change of one of 
     * the properties store in the list (whichever is later).
     */
    virtual unsigned long GetMTime() const;

    /**
     * @brief Remove a property from the list/map.
     */
    bool DeleteProperty(const std::string& propertyKey);

    const PropertyMap* GetMap() const { return &m_Properties; }

    bool IsEmpty() const { return m_Properties.empty(); }

    virtual Pointer Clone();

    virtual void Clear();

    virtual bool IsEnabled(const std::string& propertyKey);
    virtual void SetEnabled(const std::string& propertyKey,bool enabled);


  protected:

    PropertyList();


    virtual ~PropertyList();

    /**
     * @brief Map of properties.
     */
    PropertyMap m_Properties;

};

} // namespace mitk

#endif /* PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D */


