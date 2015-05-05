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

#ifndef mitkClippingPlaneInteractor3D_h
#define mitkClippingPlaneInteractor3D_h

#include <mitkBaseRenderer.h>
#include <mitkDataInteractor.h>
#include <mitkGeometry3D.h>
#include "MitkDataTypesExtExports.h"

namespace mitk
{
/**
  * \brief Specialized interactor for clipping planes.
  *
  * \ingroup Interaction
  */
// Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
class MITKDATATYPESEXT_EXPORT ClippingPlaneInteractor3D: public DataInteractor
{

public:
  mitkClassMacro(ClippingPlaneInteractor3D, DataInteractor);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

protected:
  ClippingPlaneInteractor3D();
  virtual ~ClippingPlaneInteractor3D();
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

  Geometry3D::Pointer m_OriginalGeometry;

  Vector3D m_ObjectNormal;
};
}
#endif //mitkClippingPlaneInteractor3D_h
