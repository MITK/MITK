/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKSTATUSBAR_H
#define MITKSTATUSBAR_H
#include <itkObject.h>
#include "mitkStatusBarImplementation.h"


namespace mitk {
//##Documentation
//## @brief Sending a message to the applications StatusBar
//## @ingroup Interaction
//## holds a GUI dependent StatusBarImplementation and sends the text further.
//## nearly equal to itk::OutputWindow, 
//## no Window, but one line of text and a delay for clear. 
//## all mitk-classes use this class to display text on GUI-StatusBar.
//## The mainapplication has to set the internal held StatusBarImplementation with SetInstance(..).
class StatusBar : public itk::Object
{
public:
  itkTypeMacro(StatusBar, itk::Object);
  
  //##Documentation
  //## @brief static method to get the GUI dependent StatusBar-instance 
  //## so the methods DisplayText, etc. can be called
  //## No reference counting, cause of decentral static use!
  static StatusBarImplementation* GetInstance();

  //##Documentation
  //## @brief Supply a GUI- dependent StatusBar. Has to be set by the application
  //## to connect the application dependent subclass of mitkStatusBar
  //## if you create an instance, then call ->Delete() on the supplied
  //## instance after setting it.
  static void SetInstance(StatusBarImplementation* instance);

  //##Documentation
  //## @brief Send a string to the applications StatusBar
  static void DisplayText(const char* t);
  //##Documentation
  //## @brief Send a string with a time delay to the applications StatusBar
  static void DisplayText(const char* t, int ms);
  static void DisplayErrorText(const char *t);
  static void DisplayWarningText(const char *t);
  static void DisplayWarningText(const char *t, int ms);
  static void DisplayGenericOutputText(const char *t);
  static void DisplayDebugText(const char *t);

  //##Documentation
  //## @brief removes any temporary message being shown.
  static void Clear();

  //##Documentation
  //## @brief Set the SizeGrip of the window 
  //## (the triangle in the lower right Windowcorner for changing the size) 
  //## to enabled or disabled 
  static void SetSizeGripEnabled(bool enable);

protected:
  StatusBar();
  virtual ~StatusBar();
  static StatusBarImplementation* m_Instance;
};

}// end namespace mitk
#endif /* define MITKSTATUSBAR_H */
