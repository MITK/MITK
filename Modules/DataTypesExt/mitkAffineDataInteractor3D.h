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

#include "mitkBaseRenderer.h"
#include "mitkDataInteractor.h"
#include "MitkDataTypesExtExports.h"
#include "mitkGeometry3D.h"

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
  virtual bool CheckOverObject (const InteractionEvent *);
  virtual bool SelectObject (StateMachineAction*, InteractionEvent*);
  virtual bool DeselectObject (StateMachineAction*, InteractionEvent*);
  virtual bool InitTranslate (StateMachineAction*, InteractionEvent*);
  virtual bool InitRotate (StateMachineAction*, InteractionEvent*);
  virtual bool TranslateObject (StateMachineAction*, InteractionEvent*);
  virtual bool RotateObject (StateMachineAction*, InteractionEvent*);

private:

  bool ColorizeSurface(BaseRenderer::Pointer renderer, double scalar = 0.0);

  double m_InitialPickedWorldPoint[4];
  Point2D m_InitialPickedDisplayPoint;

  //Point3D m_CurrentPickedPoint;
  //Point2D m_CurrentPickedDisplayPoint;

  Geometry3D::Pointer m_OriginalGeometry;

  Vector3D m_ObjectNormal;

};

}
#endif
