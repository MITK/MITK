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


#ifndef MITKLookupTablePROPERTY_H_HEADER_INCLUDED_C10EEAA8
#define MITKLookupTablePROPERTY_H_HEADER_INCLUDED_C10EEAA8

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
  class MITK_CORE_EXPORT LookupTableProperty : public BaseProperty
  {

  protected:
    LookupTable::Pointer m_LookupTable;

    LookupTableProperty();

    LookupTableProperty(const mitk::LookupTable::Pointer lut);

  public:

    typedef LookupTable::Pointer ValueType;

    mitkClassMacro(LookupTableProperty, BaseProperty);

    itkNewMacro(LookupTableProperty);
    mitkNewMacro1Param(LookupTableProperty, const mitk::LookupTable::Pointer);

    itkGetObjectMacro(LookupTable, LookupTable );
    ValueType GetValue() const;

    void SetLookupTable(const mitk::LookupTable::Pointer aLookupTable);
    void SetValue(const ValueType&);

    virtual std::string GetValueAsString() const;

    using BaseProperty::operator=;

  private:

    // purposely not implemented
    LookupTableProperty(const LookupTableProperty&);
    LookupTableProperty& operator=(const LookupTableProperty&);

    virtual bool IsEqual(const BaseProperty& property) const;
    virtual bool Assign(const BaseProperty& property);

  };

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk



#endif /* MITKLookupTablePROPERTY_H_HEADER_INCLUDED_C10EEAA8 */
