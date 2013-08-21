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

#ifndef __MITKRESLICEMETHODENUMPROPERTY_H
#define __MITKRESLICEMETHODENUMPROPERTY_H

#include "mitkEnumerationProperty.h"

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * Encapsulates the thick slices method enumeration
 */
class MITK_CORE_EXPORT ResliceMethodProperty : public EnumerationProperty
{
public:

  mitkClassMacro( ResliceMethodProperty, EnumerationProperty );
  itkNewMacro(ResliceMethodProperty);
  mitkNewMacro1Param(ResliceMethodProperty, const IdType&);
  mitkNewMacro1Param(ResliceMethodProperty, const std::string&);

  using BaseProperty::operator=;

protected:

  ResliceMethodProperty( );
  ResliceMethodProperty( const IdType& value );
  ResliceMethodProperty( const std::string& value );

  void AddThickSlicesTypes();

private:

  // purposely not implemented
  ResliceMethodProperty& operator=(const ResliceMethodProperty&);

  virtual itk::LightObject::Pointer InternalClone() const;
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // end of namespace mitk

#endif //_MITK_VTK_SCALARMODE_PROPERTY__H_


