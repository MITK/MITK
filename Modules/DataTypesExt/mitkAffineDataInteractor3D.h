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
class MitkDataTypesExt_EXPORT AffineDataInteractor3D: public DataInteractor
{

public:
  mitkClassMacro(AffineDataInteractor3D, DataInteractor);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void SetDataNode(NodeType node);
  void TranslateGeometry(mitk::Vector3D translate);
  void RotateGeometry(mitk::ScalarType angle, int rotationaxis);
protected:
  AffineDataInteractor3D();
  virtual ~AffineDataInteractor3D();
  /**
    * Here actions strings from the loaded state machine pattern are mapped to functions of
    * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
    */
  virtual void ConnectActionsAndFunctions();
  /**
    * This function is called when a DataNode has been set/changed.
    */
  virtual void DataNodeChanged();


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
  virtual bool TranslateUpKey(StateMachineAction*, InteractionEvent*);
  virtual bool TranslateDownKey(StateMachineAction*, InteractionEvent*);
  virtual bool TranslateLeftKey(StateMachineAction*, InteractionEvent*);
  virtual bool TranslateRightKey(StateMachineAction*, InteractionEvent*);
  virtual bool TranslateUpModifierKey(StateMachineAction*, InteractionEvent*);
  virtual bool TranslateDownModifierKey(StateMachineAction*, InteractionEvent*);

  virtual bool RotateUpKey(StateMachineAction*, InteractionEvent*);
  virtual bool RotateDownKey(StateMachineAction*, InteractionEvent*);
  virtual bool RotateLeftKey(StateMachineAction*, InteractionEvent*);
  virtual bool RotateRightKey(StateMachineAction*, InteractionEvent*);
  virtual bool RotateUpModifierKey(StateMachineAction*, InteractionEvent*);
  virtual bool RotateDownModifierKey(StateMachineAction*, InteractionEvent*);

  virtual bool ScaleGeometry(mitk::StateMachineAction *, mitk::InteractionEvent *);


  virtual void RestoreNodeColor();

private:

  double m_InitialPickedWorldPoint[4];
  Point2D m_InitialPickedDisplayPoint;

  Geometry3D::Pointer m_OriginalGeometry;

  Vector3D m_ObjectNormal;

};

}
#endif
