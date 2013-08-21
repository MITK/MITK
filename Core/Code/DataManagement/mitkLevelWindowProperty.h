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


#ifndef MITKLEVELWINDOWPROPERTY_H_HEADER_INCLUDED_C10EEAA8
#define MITKLEVELWINDOWPROPERTY_H_HEADER_INCLUDED_C10EEAA8

#include "mitkBaseProperty.h"
#include "mitkLevelWindow.h"

namespace mitk {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

//##Documentation
//## @brief Property for level/window data
//##
//## @ingroup DataManagement
class MITK_CORE_EXPORT LevelWindowProperty : public BaseProperty
{

protected:
    LevelWindow m_LevWin;

    LevelWindowProperty();

    LevelWindowProperty(const LevelWindowProperty& other);

    LevelWindowProperty(const mitk::LevelWindow &levWin);

public:
    mitkClassMacro(LevelWindowProperty, BaseProperty);

    itkNewMacro(LevelWindowProperty);
    mitkNewMacro1Param(LevelWindowProperty, const mitk::LevelWindow&);

    typedef LevelWindow ValueType;

    virtual ~LevelWindowProperty();

    const mitk::LevelWindow & GetLevelWindow() const;
    const mitk::LevelWindow & GetValue() const;

    void SetLevelWindow(const LevelWindow &levWin);
    void SetValue(const ValueType& levWin);

    virtual std::string GetValueAsString() const;

    using BaseProperty::operator=;

private:

    // purposely not implemented
    LevelWindowProperty& operator=(const LevelWindowProperty&);

    itk::LightObject::Pointer InternalClone() const;

    virtual bool IsEqual(const BaseProperty& property) const;
    virtual bool Assign(const BaseProperty& property);

};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk



#endif /* MITKLEVELWINDOWPROPERTY_H_HEADER_INCLUDED_C10EEAA8 */
