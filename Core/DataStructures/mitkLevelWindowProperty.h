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

//##ModelId=3ED953070392
//##Documentation
//## @brief Property for level/window data
//##
//## @ingroup DataTree
class MITK_CORE_EXPORT LevelWindowProperty : public BaseProperty
{

protected:
    //##ModelId=3ED953090113
    LevelWindow m_LevWin;

public:
    mitkClassMacro(LevelWindowProperty, BaseProperty);

    //##ModelId=3ED953090121
    LevelWindowProperty();

    //##ModelId=3EF198D9012D
    LevelWindowProperty(const mitk::LevelWindow &levWin);

    //##ModelId=3ED953090122
    virtual ~LevelWindowProperty();

    //##ModelId=3ED953090124
    virtual bool operator==(const BaseProperty& property) const;

    //##ModelId=3ED953090133
    const mitk::LevelWindow & GetLevelWindow() const;

    //##ModelId=3ED953090135
    void SetLevelWindow(const LevelWindow &levWin);

    virtual std::string GetValueAsString() const;

  //## 
  virtual bool WriteXMLData( XMLWriter& xmlWriter );
  //##
  virtual bool ReadXMLData( XMLReader& xmlReader );
};

} // namespace mitk



#endif /* MITKLEVELWINDOWPROPERTY_H_HEADER_INCLUDED_C10EEAA8 */
