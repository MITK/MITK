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
#include "mitkXMLIO.h"
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
 * @ingroup DataTree
 */
class MITK_CORE_EXPORT PropertyList : public itk::Object, public XMLIO
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
    mitk::BaseProperty* GetProperty(const char *propertyKey) const;

    /**
     * @brief Set a property in the list/map by value.
     * 
     * The actual OBJECT holding the value of the property is not replaced, but its value 
     * is modified to match that of @a property. To really replace the object holding the
     * property - which would make sense if you want to change the type (bool, string) of the property
     * - call ReplaceProperty.
     */
    void SetProperty(const char* propertyKey, BaseProperty* property);

    /**
     * @brief Set a property object in the list/map by reference.
     *
     * The actual OBJECT holding the value of the property is replaced by this function.
     * This is useful if you want to change the type of the property, like from BoolProperty to StringProperty.
     * Another use is to share one and the same property object among several ProperyList/DataTreeNode objects, which
     * makes them appear synchronized.
     */
    void ReplaceProperty(const char* propertyKey, BaseProperty* property);

    /**
     * @brief Set a property object in the list/map by reference.
     */ 
    void ConcatenatePropertyList(PropertyList *pList, bool replace = false);

    /**
     * @brief Get the timestamp of the last change of the map or the last change of one of 
     * the properties store in the list (whichever is later).
     */
    virtual unsigned long GetMTime() const;

    /**
     * @brief Remove a property from the list/map.
     */
    bool DeleteProperty(const char* propertyKey);

    const PropertyMap* GetMap() const { return &m_Properties; }

    bool IsEmpty() const { return m_Properties.empty(); }

    virtual Pointer Clone();

    virtual void Clear();

    static void PrepareXML_IO();

    virtual bool WriteXMLData( XMLWriter& xmlWriter );
    virtual bool ReadXMLData( XMLReader& xmlReader );
    virtual bool IsEnabled(const char *propertyKey);
    virtual void SetEnabled(const char *propertyKey,bool enabled);

    static const std::string XML_NODE_NAME;

  protected:

    PropertyList();


    virtual ~PropertyList();

    /**
     * @brief Map of properties.
     */
    PropertyMap m_Properties;

    virtual const std::string& GetXMLNodeName() const;

  private:

    /**
     * Needed for XML writing. Generates UIDs to identify memory addresses.
     */
    static UIDGenerator m_UIDGenerator;

    /**
     * Used during XML reading. Stores, which properties have already been read/created.
     */
    static std::map<std::string, BaseProperty*> m_AlreadyReadFromXML;

    /*
     * Used during XML writing. Stores, which properties have already been written, so that each property is only written once.
     */
    static std::map<BaseProperty*, std::string> m_AlreadyWrittenToXML;

    static const std::string XML_ALREADY_SEEN;
};

} // namespace mitk

#endif /* PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D */


