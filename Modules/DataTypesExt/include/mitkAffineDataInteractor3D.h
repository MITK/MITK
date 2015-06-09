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

#ifndef mitkAffineDataInteractor3D_h_
#define mitkAffineDataInteractor3D_h_

#include <mitkDataInteractor.h>
#include <mitkGeometry3D.h>
#include "MitkDataTypesExtExports.h"

namespace mitk
{
/**
  * \brief Affine interaction with objects in 3D windows.
  *
  * \ingroup Interaction
  */
// Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
class MITKDATATYPESEXT_EXPORT AffineDataInteractor3D: public DataInteractor
{

public:
  mitkClassMacro(AffineDataInteractor3D, DataInteractor);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void SetDataNode(NodeType node);
  void TranslateGeometry(mitk::Vector3D translate, mitk::BaseGeometry* geometry);
  void RotateGeometry(mitk::ScalarType angle, int rotationaxis, mitk::BaseGeometry* geometry);
  void ScaleGeometry(mitk::Point3D newScale, mitk::BaseGeometry* geometry);
  mitk::BaseGeometry *GetUpdatedTimeGeometry(mitk::InteractionEvent *interactionEvent);
protected:
  AffineDataInteractor3D();
  virtual ~AffineDataInteractor3D();
  /**
    * Here actions strings from the loaded state machine pattern are mapped to functions of
    * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
    */
  virtual void ConnectActionsAndFunctions() override;
  /**
    * This function is called when a DataNode has been set/changed.
    */
  virtual void DataNodeChanged() override;


  /**
    * Initializes the movement, stores starting position.
    */
  virtual bool CheckOverObject (const InteractionEvent*);
  virtual bool SelectObject (StateMachineAction*, InteractionEvent*);
  virtual bool DeselectObject (StateMachineAction*, InteractionEvent*);
  virtual bool InitTranslate (StateMachineAction*, InteractionEvent*);
  virtual bool InitRotate (StateMachineAction*, InteractionEvent*);
  virtual bool TranslateObject (StateMachineAction*, InteractionEvent*);
  virtual bool RotateObject (StateMachineAction*, InteractionEvent*);
  virtual bool TranslateUpKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool TranslateDownKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool TranslateLeftKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool TranslateRightKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool TranslateUpModifierKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool TranslateDownModifierKey(StateMachineAction*, InteractionEvent* interactionEvent);

  virtual bool RotateUpKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool RotateDownKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool RotateLeftKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool RotateRightKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool RotateUpModifierKey(StateMachineAction*, InteractionEvent* interactionEvent);
  virtual bool RotateDownModifierKey(StateMachineAction*, InteractionEvent* interactionEvent);

  virtual bool ScaleDownKey(mitk::StateMachineAction *, mitk::InteractionEvent* interactionEvent);
  virtual bool ScaleUpKey(mitk::StateMachineAction *, mitk::InteractionEvent* interactionEvent);


  virtual void RestoreNodeProperties();

private:

  Point3D m_InitialPickedWorldPoint;
  Point2D m_InitialPickedDisplayPoint;

  Geometry3D::Pointer m_OriginalGeometry;

  Vector3D m_ObjectNormal;

};

}
#endif
