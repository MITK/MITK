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


#ifndef _MITK_DISPLAY_INTERACTOR_H_
#define _MITK_DISPLAY_INTERACTOR_H_

#include "mitkOperationActor.h"
//#include "mitkBaseRenderer.h"

namespace mitk {
  class BaseRenderer;

  //##Documentation
  //## @brief The class handles zooming and panning events
  //##
  //## One can connect it via itk::GlobalInteraction->AddStateMachine(new mitk::DisplayVectorInteractor("move", new DisplayInterActor()));
  //## to the global state machine.
  //## @ingroup Interaction
  class MITK_CORE_EXPORT DisplayInteractor : public mitk::OperationActor
  {
  public:

    /*! \brief constructor for renderer specific zooming panning initialization

    The window associated with the passed renderer will be the only one where
    this interactor works on.
    */
    DisplayInteractor(mitk::BaseRenderer *ren = 0);

    /*! 
    \brief implementation of ExecuteOperation from mitk::OperationActor interface
    this method triggers the zooming and panning stuff in the appropriate renderer
    */
    virtual void ExecuteOperation(mitk::Operation* operation);

  private:

    /*!
    the renderer which can be affected by the interactor instance
    */
    mitk::BaseRenderer * m_ParentRenderer;

  };

} // end of namespace mitk 

#endif //_MITK_DISPLAY_INTERACTOR_H_
