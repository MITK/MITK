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


#ifndef MITKSTATUSBARIMPLEMENTATION_H
#define MITKSTATUSBARIMPLEMENTATION_H
#include "mitkCommon.h"

namespace mitk {
//##Documentation
//## @brief GUI indepentent Interface for all Gui depentent implementations of a StatusBar.
class MITK_CORE_EXPORT StatusBarImplementation
{
public:
  //##Documentation
  //## @brief Constructor
  StatusBarImplementation(){};
  //##Documentation
  //## @brief Destructor
  virtual ~StatusBarImplementation(){};

  //##Documentation
  //## @brief Send a string to the applications StatusBar
  virtual void DisplayText(const char* t)=0;
  
  //##Documentation
  //## @brief Send a string with a time delay to the applications StatusBar
  virtual void DisplayText(const char* t, int ms) = 0;
  virtual void DisplayErrorText(const char *t) = 0;
  virtual void DisplayWarningText(const char *t) = 0;
  virtual void DisplayWarningText(const char *t, int ms) = 0;
  virtual void DisplayGenericOutputText(const char *t) = 0;
  virtual void DisplayDebugText(const char *t) = 0;

  //##Documentation
  //## @brief removes any temporary message being shown.
  virtual void Clear() = 0;

  //##Documentation
  //## @brief Set the SizeGrip of the window 
  //## (the triangle in the lower right Windowcorner for changing the size) 
  //## to enabled or disabled 
  virtual void SetSizeGripEnabled(bool enable) = 0;
};

}// end namespace mitk
#endif /* define MITKSTATUSBARIMPLEMENTATION_H */
