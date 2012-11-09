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


#ifndef MITKDISPLAYVECTORINTERACTORSCROLL_H_HEADER_INCLUDED_C10DC4EB
#define MITKDISPLAYVECTORINTERACTORSCROLL_H_HEADER_INCLUDED_C10DC4EB

#include <MitkExports.h>
#include "mitkBaseRenderer.h"
#include "mitkStateMachine.h"

namespace mitk {

  class Operation;
  class OperationActor;

  /**
  * @brief Interactor for scrolling through the slices of an image
  *
  * This class implements an Interactor for slice-scrolling. It is defined by the 'Scroll'-statemachine which maps 'initmove' to left mousebutton pressed,
  * 'scroll' to left mousebutton and move and 'finishmove' to left mousebutton released.
  *
  * Thus it is possible to scroll through the slices of an image rapidly, without using the mousewheel.
  *
  * @ingroup MITK_CORE_EXPORT
  **/
  class MITK_CORE_EXPORT DisplayVectorInteractorScroll : public StateMachine
  {
  public:
    mitkClassMacro(DisplayVectorInteractorScroll, StateMachine);
    mitkNewMacro2Param(Self, const char*, OperationActor*);

    #pragma GCC visibility push(default)
    itkEventMacro( InteractionEvent, itk::AnyEvent );
    itkEventMacro( StartScrollInteractionEvent, InteractionEvent );
    itkEventMacro( EndScrollInteractionEvent, InteractionEvent );
    #pragma GCC visibility pop

    /**
    * @brief Method derived from OperationActor to recieve and execute operations
    **/
    virtual void ExecuteOperation(Operation* operation);

    /**
    * \brief Defines how many slices are scrolled per pixel that the mouse cursor has moved
    */
    void SetIndexToSliceModifier( int modifier );

    void SetAutoRepeat( bool autoRepeat );

    void SetInvertScrollingDirection( bool );

    bool IsAltModifierActive() const;

  protected:
    /**
    * @brief Default Constructor
    **/
    DisplayVectorInteractorScroll(const char * type, mitk::OperationActor* destination=NULL);

    /**
    * @brief Default Destructor
    **/
    virtual ~DisplayVectorInteractorScroll();

    /**
    * @brief Method derived from StateMachine to implement the own actions
    **/
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);
  private:
    BaseRenderer::Pointer m_Sender;

    mitk::Point2D m_StartDisplayCoordinate;
    mitk::Point2D m_LastDisplayCoordinate;
    mitk::Point2D m_CurrentDisplayCoordinate;

    OperationActor* m_Destination;

    /**
    * \brief Modifier that defines how many slices are scrolled per pixel that the mouse has moved
    *
    * This modifier defines how many slices the scene is scrolled per pixel that the mouse cursor has moved.
    * By default the modifier is 4. This means that when the user moves the cursor by 4 pixels in Y-direction
    * the scene is scrolled by one slice. If the user has moved the the cursor by 20 pixels, the scene is
    * scrolled by 5 slices.
    *
    * If the cursor has moved less than m_IndexToSliceModifier pixels the scene is scrolled by one slice.
    */
    int m_IndexToSliceModifier;

    /**
    * \brief Defines if it is possible to scroll endlessly
    *
    * If AutoRepeat is on, scrolling further than the last slice will restart at the first slice and vice versa
    */
    bool m_AutoRepeat;

    bool m_InvertScrollingDirection;

    bool m_IsModifierActive;

  };

} // namespace mitk



#endif /* MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB */


