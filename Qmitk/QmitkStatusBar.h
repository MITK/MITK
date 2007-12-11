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


#ifndef QMITKSTATUSBAR_H
#define QMITKSTATUSBAR_H
#include <mitkStatusBarImplementation.h>
#include <mitkCommon.h>
#include <qstatusbar.h>

//##Documentation
//## @brief QT-Toolkit/GUI dependent class that provides to send a Message to the QT's StatusBar
//##
//## A delay time can be set.
//##
//## All mitk-classes will call this class for output:
//## mitk::StatusBar::GetInstance();
class QmitkStatusBar : public mitk::StatusBarImplementation
{
public:

    mitkClassMacro(QmitkStatusBar, mitk::StatusBarImplementation);
 
    //##Documentation
    //##@brief Constructor;
    //## holds param instance internaly and connects this to the mitkStatusBar
    QmitkStatusBar(QStatusBar* instance);
    
    //##Documentation
    //##@brief Destructor
    virtual ~QmitkStatusBar();
    
   
    //##Documentation
    //## @brief Send a string to the applications StatusBar (QStatusBar).
    virtual void DisplayText(const char* t);
    virtual void DisplayText(const char* t, int ms);
        
    //##Documentation
    //## @brief Send a string as an error message to StatusBar. 
    //## The implementation calls DisplayText()
    virtual void DisplayErrorText(const char *t) { this->DisplayText(t); };
    virtual void DisplayWarningText(const char *t) { this->DisplayText(t); };
    virtual void DisplayWarningText(const char *t, int ms) { this->DisplayText(t, ms); };
    virtual void DisplayGenericOutputText(const char *t) {this->DisplayText(t);}
    virtual void DisplayDebugText(const char *t) { this->DisplayText(t); };

    //##Documentation
    //## @brief removes any temporary message being shown.
    virtual void Clear();

    //##Documentation
    //## @brief Set the QSizeGrip of the window 
    //## (the triangle in the lower right Windowcorner for changing the size) 
    //## to enabled or disabled 
    virtual void SetSizeGripEnabled(bool enable);
    
private:
    //static Pointer m_Instance;
    QStatusBar* m_StatusBar;

};

#endif /* define QMITKSTATUSBAR_H */
