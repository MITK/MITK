/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCLIPPINGPROPERTY_H_HEADER_INCLUDED
#define MITKCLIPPINGPROPERTY_H_HEADER_INCLUDED

#include "mitkBaseProperty.h"
#include "mitkNumericTypes.h"
#include <MitkCoreExports.h>

#include <itkConfigure.h>

#include <string>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * \brief Property for clipping datasets; currently only
   * clipping planes are possible
   * \ingroup DataManagement
   */
  class MITKCORE_EXPORT ClippingProperty : public BaseProperty
  {
  public:
    mitkClassMacro(ClippingProperty, BaseProperty);

    typedef std::string ValueType;

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self)
      mitkNewMacro2Param(ClippingProperty, const Point3D &, const Vector3D &);

    bool GetClippingEnabled() const;
    void SetClippingEnabled(bool enabled);

    const Point3D &GetOrigin() const;
    void SetOrigin(const Point3D &origin);

    const Vector3D &GetNormal() const;
    void SetNormal(const Vector3D &normal);

    std::string GetValueAsString() const override;

    using BaseProperty::operator=;

  protected:
    bool m_ClippingEnabled;

    Point3D m_Origin;
    Vector3D m_Normal;

    ClippingProperty();
    ClippingProperty(const ClippingProperty &other);
    ClippingProperty(const Point3D &origin, const Vector3D &normal);

  private:
    // purposely not implemented
    ClippingProperty &operator=(const ClippingProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;

    itk::LightObject::Pointer InternalClone() const override;
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace mitk

#endif /* MITKCLIPPINGPROPERTY_H_HEADER_INCLUDED */
