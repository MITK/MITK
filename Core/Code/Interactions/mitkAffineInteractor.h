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


#ifndef MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F
#define MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F

#include <MitkCoreExports.h>
#include "mitkInteractor.h"
#include "mitkNumericTypes.h"

namespace mitk {
class DisplayPositionEvent;

//##Documentation
//## @brief Interactor for Affine transformations translate, rotate and scale
//##
//## An object of this class can translate, rotate and scale the data objects
//## by modifying its geometry.
//## @ingroup Interaction

//create events for interactions
#pragma GCC visibility push(default)
itkEventMacro(AffineInteractionEvent, itk::AnyEvent);
itkEventMacro(ScaleEvent, AffineInteractionEvent);
itkEventMacro(RotateEvent, AffineInteractionEvent);
itkEventMacro(TranslateEvent, AffineInteractionEvent);
#pragma GCC visibility pop

class MITK_CORE_EXPORT AffineInteractor : public Interactor
{
public:
  mitkClassMacro(AffineInteractor,Interactor);
//  itkFactorylessNewMacro(Self)
//  itkCloneMacro(Self)
  mitkNewMacro2Param(Self, const char*, DataNode*);

protected:
//  AffineInteractor(); //obsolete

  //##Documentation
  //## @brief Constructor
  //##
  //## @param dataNode is the node, this Interactor is connected to
  //## @param type is the type of StateMachine like declared in the XML-Configure-File
  AffineInteractor(const char * type, DataNode* dataNode);

  //##Documentation
  //## @brief Destructor
  ~AffineInteractor(){};

  virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  //##Documentation
  //## @brief calculates how good the data this state machine handles is hit by the event.
  //##
  //## Returns a value between 0 and 1.
  //## (Used by GlobalInteraction to decide which DESELECTED state machine to send the event to.)
  //##
  //## \WARNING This is interactor currently does not work for interaction in 3D. Try using mitkAffineInteractor3D instead.
  virtual float CanHandleEvent(StateEvent const* stateEvent) const;

  bool CheckSelected(const mitk::Point3D& worldPoint, int timestep);
  bool ConvertDisplayEventToWorldPosition(mitk::DisplayPositionEvent const* displayEvent, mitk::Point3D& worldPoint);

  mitk::Point3D m_LastMousePosition;
};

} // namespace mitk

#endif /* MITKAFFINEINTERACTOR_H_HEADER_INCLUDED_C188C29F */
