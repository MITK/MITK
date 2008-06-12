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


#ifndef MITKCLIPPINGPROPERTY_H_HEADER_INCLUDED
#define MITKCLIPPINGPROPERTY_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include "mitkVector.h"

#include <itkConfigure.h>

#include <string>


namespace mitk {

/**
 * @brief Property for clipping datasets; currently only
 * clipping planes are possible
 * @ingroup DataTree
 */
class MITK_CORE_EXPORT ClippingProperty : public BaseProperty
{
public:
  mitkClassMacro(ClippingProperty, BaseProperty);
  typedef std::string ValueType;
  
  itkNewMacro( ClippingProperty );
  mitkNewMacro2Param( ClippingProperty, 
    const Point3D &, const Vector3D & );
  
  const Point3D &GetOrigin() const;
  void SetOrigin( const Point3D &origin );

  const Vector3D &GetNormal() const;
  void SetNormal( const Vector3D &normal );

  //##ModelId=3E3FF04F00E1
  virtual bool operator==(const BaseProperty& property ) const;
  virtual std::string GetValueAsString() const;

  //## 
  virtual bool WriteXMLData( XMLWriter& xmlWriter );
  //##
  virtual bool ReadXMLData( XMLReader& xmlReader );


protected:
  static const char *ORIGIN;
  static const char *NORMAL;

  Point3D m_Origin;
  Vector3D m_Normal;
  
  ClippingProperty();
  ClippingProperty( const Point3D &origin, const Vector3D &normal );

};

} // namespace mitk

#endif /* MITKCLIPPINGPROPERTY_H_HEADER_INCLUDED */
