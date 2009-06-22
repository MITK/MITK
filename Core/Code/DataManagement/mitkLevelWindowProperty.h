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

    virtual ~LevelWindowProperty();

    virtual bool operator==(const BaseProperty& property) const;

    const mitk::LevelWindow & GetLevelWindow() const;

    void SetLevelWindow(const LevelWindow &levWin);

    virtual std::string GetValueAsString() const;

};

} // namespace mitk



#endif /* MITKLEVELWINDOWPROPERTY_H_HEADER_INCLUDED_C10EEAA8 */
