/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-14 19:45:53 +0200 (Mo, 14 Apr 2008) $
Version:   $Revision: 14081 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef __MITKRESLICEMETHODENUMPROPERTY_H
#define __MITKRESLICEMETHODENUMPROPERTY_H

#include "mitkEnumerationProperty.h"

namespace mitk
{

/**
 * Encapsulates the thick slices method enumeration
 */
class MITK_CORE_EXPORT ResliceMethodEnumProperty : public EnumerationProperty
{
public:

  mitkClassMacro( ResliceMethodEnumProperty, EnumerationProperty );
  itkNewMacro(ResliceMethodEnumProperty);
  mitkNewMacro1Param(ResliceMethodEnumProperty, const IdType&); 
  mitkNewMacro1Param(ResliceMethodEnumProperty, const std::string&);  
  
protected:
  
  ResliceMethodEnumProperty( );  
  ResliceMethodEnumProperty( const IdType& value );  
  ResliceMethodEnumProperty( const std::string& value );
  
  void AddThickSlicesTypes();
};

} // end of namespace mitk

#endif //_MITK_VTK_SCALARMODE_PROPERTY__H_


