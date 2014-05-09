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


#ifndef MITKANNOTATIONPROPERTY_H_HEADER_INCLUDED
#define MITKANNOTATIONPROPERTY_H_HEADER_INCLUDED

#include <MitkCoreExports.h>
#include "mitkBaseProperty.h"
#include "mitkNumericTypes.h"

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

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
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

  using BaseProperty::operator =;

protected:

  std::string m_Label;
  Point3D m_Position;

  AnnotationProperty();
  AnnotationProperty( const char *label, const Point3D &position );
  AnnotationProperty( const std::string &label, const Point3D &position );
  AnnotationProperty( const char *label, ScalarType x, ScalarType y, ScalarType z );
  AnnotationProperty( const std::string &label, ScalarType x, ScalarType y, ScalarType z );

  AnnotationProperty(const AnnotationProperty& other);

private:

  // purposely not implemented
  AnnotationProperty& operator=(const AnnotationProperty&);

  itk::LightObject::Pointer InternalClone() const;

  virtual bool IsEqual(const BaseProperty& property) const;
  virtual bool Assign(const BaseProperty & property);

};

} // namespace mitk

#endif /* MITKANNOTATIONPROPERTY_H_HEADER_INCLUDED */
