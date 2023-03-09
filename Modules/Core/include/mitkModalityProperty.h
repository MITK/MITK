/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModalityProperty_h
#define mitkModalityProperty_h

#include "mitkEnumerationProperty.h"
#include <MitkCoreExports.h>
#include <itkObjectFactory.h>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
    \brief Enumerates all known modalities

    \ingroup DataManagement
  */
  class MITKCORE_EXPORT ModalityProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(ModalityProperty, EnumerationProperty);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    mitkNewMacro1Param(ModalityProperty, const IdType&);
    mitkNewMacro1Param(ModalityProperty, const std::string &);

    using BaseProperty::operator=;

  protected:
    ModalityProperty();
    ModalityProperty(const IdType &value);
    ModalityProperty(const std::string &value);

    ~ModalityProperty() override;
    virtual void AddEnumerationTypes();

  private:
    // purposely not implemented
    const ModalityProperty &operator=(const ModalityProperty &);

    itk::LightObject::Pointer InternalClone() const override;
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace

#endif
