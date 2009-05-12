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


#ifndef BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54
#define BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54

#include <string>
#include <itkObjectFactory.h>
#include "mitkCommon.h"
#include <mitkXMLIO.h>

namespace mitk {

/*! @brief Abstract base class for properties
    
  @ingroup DataTree

    Base class for properties. Properties are arbitrary additional information
    (to define a new type of information you have to define a subclass of
    BaseProperty) that can be added to a PropertyList.
    Concrete subclasses of BaseProperty should define Set-/Get-methods to assess 
    the property value, which should be stored by value (not by reference).
    Subclasses must implement an operator==(const BaseProperty& property), which
    is used by PropertyList to check whether a property has been changed.
*/
class MITK_CORE_EXPORT BaseProperty : public itk::Object, public XMLIO
{
  public:

    mitkClassMacro(BaseProperty,itk::Object);

    /*! @brief Subclasses must implement this operator==.
        Operator== which is used by PropertyList to check whether a property has been changed.
    */
    virtual bool operator==(const BaseProperty& property) const = 0;
    
    virtual BaseProperty& operator=(const BaseProperty& property);
    
    virtual std::string GetValueAsString() const;
    
    virtual bool Assignable(const BaseProperty& ) const;
    
    virtual bool WriteXMLData( XMLWriter& xmlWriter );
    
    virtual bool ReadXMLData( XMLReader& xmlReader );
    
    virtual const std::string& GetXMLNodeName() const;


    static const std::string XML_NODE_NAME;

  protected:
    BaseProperty();
    
    virtual ~BaseProperty();

    friend class PropertyList; // for VALUE

    static std::string VALUE;
};

} // namespace mitk



#endif /* BASEPROPERTY_H_HEADER_INCLUDED_C1F4DF54 */


