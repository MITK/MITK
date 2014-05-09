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


#ifndef MITKCLIPPINGPROPERTY_H_HEADER_INCLUDED
#define MITKCLIPPINGPROPERTY_H_HEADER_INCLUDED

#include <MitkCoreExports.h>
#include "mitkBaseProperty.h"
#include "mitkNumericTypes.h"

#include <itkConfigure.h>

#include <string>


namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * \brief Property for clipping datasets; currently only
 * clipping planes are possible
 * \ingroup DataManagement
 */
class MITK_CORE_EXPORT ClippingProperty : public BaseProperty
{
public:
  mitkClassMacro(ClippingProperty, BaseProperty);

  typedef std::string ValueType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  mitkNewMacro2Param( ClippingProperty,
    const Point3D &, const Vector3D & );

  bool GetClippingEnabled() const;
  void SetClippingEnabled( bool enabled );

  const Point3D &GetOrigin() const;
  void SetOrigin( const Point3D &origin );

  const Vector3D &GetNormal() const;
  void SetNormal( const Vector3D &normal );

  virtual std::string GetValueAsString() const;

  using BaseProperty::operator =;

protected:

  bool m_ClippingEnabled;

  Point3D m_Origin;
  Vector3D m_Normal;

  ClippingProperty();
  ClippingProperty(const ClippingProperty& other);
  ClippingProperty( const Point3D &origin, const Vector3D &normal );

private:

  // purposely not implemented
  ClippingProperty& operator=(const ClippingProperty&);

  virtual bool IsEqual(const BaseProperty& property) const;
  virtual bool Assign(const BaseProperty& property);

  virtual itk::LightObject::Pointer InternalClone() const;

};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk

#endif /* MITKCLIPPINGPROPERTY_H_HEADER_INCLUDED */
