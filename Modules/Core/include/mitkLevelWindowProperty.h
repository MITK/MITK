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

/**
 * @brief The LevelWindowProperty class Property for the mitk::LevelWindow
 *
 * @ingroup DataManagement
 *
 * @note If you want to apply the mitk::LevelWindowProperty to an mitk::Image,
 * make sure to set the mitk::RenderingModeProperty to a mode which supports
 * level window (e.g. LEVELWINDOW_COLOR). Make sure to check the documentation
 * of the mitk::RenderingModeProperty. For a code example how to use the
 * mitk::LevelWindowProperty check the mitkImageVtkMapper2DLevelWindowTest.cpp
 * in Core\Code\Testing.
 */
class MITKCORE_EXPORT LevelWindowProperty : public BaseProperty
{

protected:
    LevelWindow m_LevWin;

    LevelWindowProperty();

    LevelWindowProperty(const LevelWindowProperty& other);

    LevelWindowProperty(const mitk::LevelWindow &levWin);

public:
    mitkClassMacro(LevelWindowProperty, BaseProperty);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitkNewMacro1Param(LevelWindowProperty, const mitk::LevelWindow&);

    typedef LevelWindow ValueType;

    virtual ~LevelWindowProperty();

    const mitk::LevelWindow & GetLevelWindow() const;
    const mitk::LevelWindow & GetValue() const;

    void SetLevelWindow(const LevelWindow &levWin);
    void SetValue(const ValueType& levWin);

    virtual std::string GetValueAsString() const override;

    using BaseProperty::operator=;

private:

    // purposely not implemented
    LevelWindowProperty& operator=(const LevelWindowProperty&);

    itk::LightObject::Pointer InternalClone() const override;

    virtual bool IsEqual(const BaseProperty& property) const override;
    virtual bool Assign(const BaseProperty& property) override;

};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace mitk



#endif /* MITKLEVELWINDOWPROPERTY_H_HEADER_INCLUDED_C10EEAA8 */
