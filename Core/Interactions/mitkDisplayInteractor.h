#ifndef _MITK_DISPLAY_INTERACTOR_H_
#define _MITK_DISPLAY_INTERACTOR_H_

#include "OperationActor.h"
#include "BaseRenderer.h"

namespace mitk {


  /*!
  \brief the class handles zooming and panning events

  One can connect it via itk::GlobalInteraction->AddStateMachine(new mitk::DisplayVectorInteractor("move", new DisplayInterActor()));
  to the global state machine.

  */
  class DisplayInteractor : public mitk::OperationActor
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
