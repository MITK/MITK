/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <itkObjectFactory.h>
#include "mitkBaseProperty.h"
#include <string>
#include <map> 
#include <mitkXMLIO.h>

namespace mitk {

	class XMLWriter;

//##ModelId=3E031F200392
//##Documentation
//## @brief List of properties (instances of subclasses of BaseProperty)
//##
//## List of properties (instances of subclasses of BaseProperty). The
//## properties are stored in a map, thus each property is associated with a
//## key of type string. 
//## @ingroup DataTree
class PropertyList : public itk::Object, public XMLIO
{
public:
    mitkClassMacro(PropertyList, itk::Object);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);
    typedef std::map< std::string,std::pair<BaseProperty::Pointer,bool> > PropertyMap;
    typedef std::pair< std::string,std::pair<BaseProperty::Pointer,bool> > PropertyMapElementType;
public:
    //##ModelId=3D6A0E9E0029
    //##Documentation
    //## @brief Get a property by its name. 
    mitk::BaseProperty::Pointer GetProperty(const char *propertyKey) const;

    //##ModelId=3D78B966005F
    //##Documentation
    //## @brief Set a property in the list/map. This is set-by-value. 
    //##
    //## @warning Change in semantics since Aug 25th 2006. Check your usage of this method if you do
    //##          more with properties than just call <tt>SetProperty( "key", new SomeProperty("value") )</tt>.
    //##
    //## The actual OBJECT holding the value of the property is not replaced, but its value 
    //## is modified to match that of @a property. To really replace the object holding the
    //## property - which would make sense if you want to change the type (bool, string) of the property
    //## - call ReplaceProperty.
    void SetProperty(const char* propertyKey, BaseProperty* property);

    //## @brief Set a property object in the list/map. This is kind of set-by-reference (seems the wrong word - delete this comment if it's not). 
    //##
    //## The actual OBJECT holding the value of the property is replaced by this function.
    //## This is useful if you want to change the type of the property, like from BoolProperty to StringProperty.
    //## Another use is to share one and the same property object among several ProperyList/DataTreeNode objects, which
    //## makes them appear synchronized.
    void ReplaceProperty(const char* propertyKey, BaseProperty* property);

    //##ModelId=3ED94AAE0075
    //##Documentation
    //## @brief Get the timestamp of the last change of the map or the last change of one of 
    //## the properties store in the list (whichever is later).
    virtual unsigned long GetMTime() const;

    //##ModelId=3EF1B0160286
    //##Documentation
    //## @brief Remove a property from the list/map.
    bool DeleteProperty(const char* propertyKey);

    const PropertyMap* GetMap() const { return &m_Properties; }

    virtual Pointer Clone();

    virtual void Clear();

    virtual bool WriteXMLData( XMLWriter& xmlWriter );
    virtual bool ReadXMLData( XMLReader& xmlReader );
    virtual bool IsEnabled(const char *propertyKey);
    virtual void SetEnabled(const char *propertyKey,bool enabled);

    static const std::string XML_NODE_NAME;
protected:
    //##ModelId=3E38FEFE0125
    PropertyList();


    //##ModelId=3E38FEFE0157
    virtual ~PropertyList();

    //##ModelId=3D6A0F0B00BC
    //##Documentation
    //## @brief Map of properties.
    PropertyMap m_Properties;

    //##Documentation
    virtual const std::string& GetXMLNodeName() const;
};

} // namespace mitk



#endif /* PROPERTYLIST_H_HEADER_INCLUDED_C1C77D8D */
