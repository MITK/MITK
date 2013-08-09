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


#ifndef __mitkLabelSetProperty_H_
#define __mitkLabelSetProperty_H_

#include <MitkExports.h>
#include "mitkBaseProperty.h"
#include "mitkLookupTable.h"

namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

  //##Documentation
  //## @brief Property for LookupTable data
  //##
  //## @ingroup DataManagement
  class MITK_CORE_EXPORT LabelSetProperty : public BaseProperty
  {

  protected:
    LookupTable::Pointer m_LookupTable;

    LabelSetProperty();

    LabelSetProperty(const LabelSetProperty&);

    LabelSetProperty(const mitk::LookupTable::Pointer lut);

  public:

    typedef LookupTable::Pointer ValueType;

    mitkClassMacro(LabelSetProperty, BaseProperty);

    itkNewMacro(LabelSetProperty);
    mitkNewMacro1Param(LabelSetProperty, const mitk::LookupTable::Pointer);

    itkGetObjectMacro(LookupTable, LookupTable );
    ValueType GetValue() const;

    void SetLookupTable(const mitk::LookupTable::Pointer aLookupTable);
    void SetValue(const ValueType&);

    virtual std::string GetValueAsString() const;

    using BaseProperty::operator=;

  private:

    // purposely not implemented
    LabelSetProperty& operator=(const LabelSetProperty&);

    itk::LightObject::Pointer InternalClone() const;

    virtual bool IsEqual(const BaseProperty& property) const;
    virtual bool Assign(const BaseProperty& property);

  };

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk



#endif // __mitkLabelSetProperty_H_
