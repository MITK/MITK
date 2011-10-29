/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKLEVELWINDOWPROPERTY_H_HEADER_INCLUDED_C10EEAA8
#define MITKLEVELWINDOWPROPERTY_H_HEADER_INCLUDED_C10EEAA8

#include "mitkBaseProperty.h"
#include "mitkLevelWindow.h"

namespace mitk {

//##Documentation
//## @brief Property for level/window data
//##
//## @ingroup DataManagement
class MITK_CORE_EXPORT LevelWindowProperty : public BaseProperty
{

protected:
    LevelWindow m_LevWin;

    LevelWindowProperty();

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
    LevelWindowProperty(const LevelWindowProperty&);
    LevelWindowProperty& operator=(const LevelWindowProperty&);

    virtual bool IsEqual(const BaseProperty& property) const;
    virtual bool Assign(const BaseProperty& property);

};

} // namespace mitk



#endif /* MITKLEVELWINDOWPROPERTY_H_HEADER_INCLUDED_C10EEAA8 */
