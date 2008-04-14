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


#ifndef MITKLookupTablePROPERTY_H_HEADER_INCLUDED_C10EEAA8
#define MITKLookupTablePROPERTY_H_HEADER_INCLUDED_C10EEAA8

#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include "mitkLookupTable.h"

namespace mitk {

//##ModelId=3ED953070392
//##Documentation
//## @brief Property for LookupTable data
//##
//## @ingroup DataTree
class MITK_CORE_EXPORT LookupTableProperty : public BaseProperty
{

protected:
    //##ModelId=3ED953090113
  LookupTable::Pointer m_LookupTable;

  LookupTableProperty();

  //##ModelId=3ED953090121
  LookupTableProperty(const mitk::LookupTable::Pointer lut);

  //##ModelId=3EF198D9012D
  //    LookupTableProperty(const mitk::LookupTable& aLookupTable);
  
public:
    mitkClassMacro(LookupTableProperty, BaseProperty);

    itkNewMacro(LookupTableProperty);
    mitkNewMacro1Param(LookupTableProperty, const mitk::LookupTable::Pointer);

    //##ModelId=3ED953090122
    virtual ~LookupTableProperty();

    //##ModelId=3ED953090124
    virtual bool operator==(const BaseProperty& property) const;

    itkGetObjectMacro(LookupTable, LookupTable );

    //##ModelId=3ED953090135
    void SetLookupTable(const mitk::LookupTable::Pointer aLookupTable);

    virtual bool WriteXMLData(XMLWriter &xmlWriter);

    virtual bool ReadXMLData(XMLReader &xmlReader);
};

} // namespace mitk



#endif /* MITKLookupTablePROPERTY_H_HEADER_INCLUDED_C10EEAA8 */
