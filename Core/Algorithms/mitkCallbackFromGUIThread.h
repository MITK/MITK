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

#ifndef MITK_CALLBACK_WITHIN_GUI_TREAD_H_INCLUDGEWQ
#define MITK_CALLBACK_WITHIN_GUI_TREAD_H_INCLUDGEWQ

#include<itkCommand.h>

namespace mitk
{

/*!
  \brief Toolkit specific implementation of mitk::CallbackFromGUIThread

  For any toolkit, this class has to be sub-classed. One instance of that sub-class has to
  be registered with mitk::CallbackFromGUIThread. See the (very simple) implmentation of 
  QmitkCallbackFromGUIThread for an example.
*/
class CallbackFromGUIThreadImplementation
{
  public:
    
    /// Change the current application cursor
    virtual void CallThisFromGUIThread(itk::Command*) = 0;

    virtual ~CallbackFromGUIThreadImplementation() {};

  protected:
  private:
};
  
/*!
  \brief Allows threads to call some method from within the GUI thread.

   This class is useful for use with GUI toolkits that are not thread-safe, e.g. Qt. Any thread that 
   needs to work with the GUI at some time during its execution (e.g. at the end, to display some results)
   can use this class to ask for a call to a member function from the GUI thread.
*/
class CallbackFromGUIThread
{
  public:

    /// This class is a singleton. 
    static CallbackFromGUIThread* GetInstance();

    /// To be called by a toolkit specific CallbackFromGUIThreadImplementation. 
    void RegisterImplementation(CallbackFromGUIThreadImplementation* implementation);

    /// Change the current application cursor
    void CallThisFromGUIThread(itk::Command*);
    
  protected:

    /// Purposely hidden - singleton
    CallbackFromGUIThread();

  private:
    
    CallbackFromGUIThreadImplementation* m_Implementation;
    static CallbackFromGUIThread* m_Instance;
};

} // namespace

#endif

