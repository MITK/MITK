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


#ifndef MITKDisplayVectorInteractorLevelWindow_H_HEADER_INCLUDED_C10DC4EB
#define MITKDisplayVectorInteractorLevelWindow_H_HEADER_INCLUDED_C10DC4EB

#include <MitkExports.h>
#include "mitkBaseRenderer.h"
#include "mitkStateMachine.h"

namespace mitk {

  class Operation;
  class OperationActor;

  /**
  * @brief Interactor for adjusting both level- and window-values for an image
  *
  * This class implements an Interactor for adjusting the LevelWindow. It is defined by the 'LevelWindow'-statemachine which maps 'initmove' to right mousebutton pressed,
  * 'levelwindow' to right mousebutton and move and 'finishmove' to right mousebutton released.
  *
  * Using this tool, it is possible to increase the 'level'-value of the selected image
  * ( if no image has the 'selected'-property == true, the first image in the DataStorage is used )
  * by moving the mouse right and decreasing the level by moving the mouse to the left.
  *
  * The 'window'-value and also be adjusted by moving the mouse-curser up (increase) and down (decrease).
  *
  * @ingroup MITK_CORE_EXPORT
  **/
  class MITK_CORE_EXPORT DisplayVectorInteractorLevelWindow : public StateMachine
  {
  public:
    mitkClassMacro(DisplayVectorInteractorLevelWindow, StateMachine);
    mitkNewMacro1Param(Self, const char*);

    itkEventMacro( InteractionEvent, itk::AnyEvent );
    itkEventMacro( StartInteractionEvent, InteractionEvent );
    itkEventMacro( EndInteractionEvent, InteractionEvent );


    //static Pointer New(const char* type)
    //{
    //  Pointer smartPtr = new DisplayVectorInteractorLevelWindow ( type );
    //  smartPtr->UnRegister();
    //  return smartPtr;
    //}

    /**
    * @brief Method derived from OperationActor to recieve and execute operations
    **/
    virtual void ExecuteOperation(Operation* operation);

  protected:
    /**
    * @brief Default Constructor
    **/
    DisplayVectorInteractorLevelWindow(const char * type);

    /**
    * @brief Default Destructor
    **/
    virtual ~DisplayVectorInteractorLevelWindow();

    /**
    * @brief Method derived from StateMachine to implement the own actions
    **/
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  private:
    BaseRenderer::Pointer m_Sender;

    mitk::Point2D m_StartDisplayCoordinate;
    mitk::Point2D m_LastDisplayCoordinate;
    mitk::Point2D m_CurrentDisplayCoordinate;
  };

} // namespace mitk



#endif /* MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB */


