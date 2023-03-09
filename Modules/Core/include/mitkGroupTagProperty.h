/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGroupTagProperty_h
#define mitkGroupTagProperty_h

#include <mitkBaseProperty.h>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /*! @brief Property class that has no value.

    @ingroup DataManagement

      The GroupTag property is used to tag a datatree node to show, that it is member of a
      group of datatree nodes. This can be used to build groups of datatreenodes without the
      need to contain them in a specific hiearchic order in the datatree
  */
  class MITKCORE_EXPORT GroupTagProperty : public BaseProperty
  {
  public:
    mitkClassMacro(GroupTagProperty, BaseProperty);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      using BaseProperty::operator=;

  protected:
    GroupTagProperty();
    GroupTagProperty(const GroupTagProperty &);

  private:
    // purposely not implemented
    GroupTagProperty &operator=(const GroupTagProperty &);

    itk::LightObject::Pointer InternalClone() const override;

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace mitk

#endif
