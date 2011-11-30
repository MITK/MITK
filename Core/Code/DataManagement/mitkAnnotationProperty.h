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


#ifndef MITKANNOTATIONPROPERTY_H_HEADER_INCLUDED
#define MITKANNOTATIONPROPERTY_H_HEADER_INCLUDED

#include <MitkExports.h>
#include "mitkBaseProperty.h"
#include "mitkVector.h"

#include <itkConfigure.h>

#include <string>


namespace mitk {

/**
 * \brief Property for annotations
 * \ingroup DataManagement
 */
class MITK_CORE_EXPORT AnnotationProperty : public BaseProperty
{
public:
  mitkClassMacro(AnnotationProperty, BaseProperty);
  typedef std::string ValueType;
  
  itkNewMacro( AnnotationProperty );
  mitkNewMacro2Param( AnnotationProperty, 
    const char *, const Point3D & );
  mitkNewMacro2Param( AnnotationProperty, 
    const std::string &, const Point3D & );
  mitkNewMacro4Param( AnnotationProperty,
    const char *, ScalarType, ScalarType, ScalarType );
  mitkNewMacro4Param( AnnotationProperty,
    const std::string &, ScalarType, ScalarType, ScalarType );
  
  itkGetStringMacro( Label );
  itkSetStringMacro( Label );

  const Point3D &GetPosition() const;
  void SetPosition( const Point3D &position );

  virtual std::string GetValueAsString() const;
  virtual BaseProperty& operator=(const BaseProperty& other) { return Superclass::operator=(other); } \


protected:

  std::string m_Label;
  Point3D m_Position;
  
  AnnotationProperty();
  AnnotationProperty( const char *label, const Point3D &position );
  AnnotationProperty( const std::string &label, const Point3D &position );
  AnnotationProperty( const char *label, ScalarType x, ScalarType y, ScalarType z );
  AnnotationProperty( const std::string &label, ScalarType x, ScalarType y, ScalarType z );

private:

  // purposely not implemented
  AnnotationProperty(const AnnotationProperty&);
  AnnotationProperty& operator=(const AnnotationProperty&);

  virtual bool IsEqual(const BaseProperty& property) const;
  virtual bool Assign(const BaseProperty & property);

};

} // namespace mitk

#endif /* MITKANNOTATIONPROPERTY_H_HEADER_INCLUDED */
