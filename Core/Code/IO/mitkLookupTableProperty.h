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

#include <MitkCoreExports.h>
#include "mitkBaseProperty.h"
#include "mitkLookupTable.h"

namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * @brief The LookupTableProperty class Property to associate mitk::LookupTable
 * to an mitk::DataNode.
 * @ingroup DataManagement
 *
 * @note If you want to use this property to colorize an mitk::Image, make sure
 * to set the mitk::RenderingModeProperty to a mode which supports lookup tables
 * (e.g. LOOKUPTABLE_COLOR). Make sure to check the documentation of the
 * mitk::RenderingModeProperty. For a code example how to use the mitk::LookupTable
 * and this property check the mitkImageVtkMapper2DLookupTableTest.cpp in
 * Core\Code\Testing.
 */
class MITK_CORE_EXPORT LookupTableProperty : public BaseProperty
  {

  protected:
    LookupTable::Pointer m_LookupTable;

    LookupTableProperty();

    LookupTableProperty(const LookupTableProperty&);

    LookupTableProperty(const mitk::LookupTable::Pointer lut);

  public:

    typedef LookupTable::Pointer ValueType;

    mitkClassMacro(LookupTableProperty, BaseProperty);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitkNewMacro1Param(LookupTableProperty, const mitk::LookupTable::Pointer);

    itkGetObjectMacro(LookupTable, LookupTable );
    ValueType GetValue() const;

    void SetLookupTable(const mitk::LookupTable::Pointer aLookupTable);
    void SetValue(const ValueType&);

    virtual std::string GetValueAsString() const;

    using BaseProperty::operator=;

  private:

    // purposely not implemented
    LookupTableProperty& operator=(const LookupTableProperty&);

    itk::LightObject::Pointer InternalClone() const;

    virtual bool IsEqual(const BaseProperty& property) const;
    virtual bool Assign(const BaseProperty& property);

  };

#ifdef _MSC_VER
# pragma warning(pop)
#endif
} // namespace mitk

#endif /* MITKLookupTablePROPERTY_H_HEADER_INCLUDED_C10EEAA8 */
