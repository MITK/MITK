/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkClippingPlaneInteractor3D_h
#define mitkClippingPlaneInteractor3D_h

#include "MitkDataTypesExtExports.h"
#include <mitkBaseRenderer.h>
#include <mitkDataInteractor.h>
#include <mitkGeometry3D.h>

namespace mitk
{
  /**
    * \brief Specialized interactor for clipping planes.
    *
    * \ingroup Interaction
    */
  // Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
  class MITKDATATYPESEXT_EXPORT ClippingPlaneInteractor3D : public DataInteractor
  {
  public:
    mitkClassMacro(ClippingPlaneInteractor3D, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      protected : ClippingPlaneInteractor3D();
    ~ClippingPlaneInteractor3D() override;
    /**
      * Here actions strings from the loaded state machine pattern are mapped to functions of
      * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
      */
    void ConnectActionsAndFunctions() override;
    /**
      * This function is called when a DataNode has been set/changed.
      */
    void DataNodeChanged() override;

    /**
      * Initializes the movement, stores starting position.
      */
    virtual bool CheckOverObject(const InteractionEvent *);
    virtual void SelectObject(StateMachineAction *, InteractionEvent *);
    virtual void DeselectObject(StateMachineAction *, InteractionEvent *);
    virtual void InitTranslate(StateMachineAction *, InteractionEvent *);
    virtual void InitRotate(StateMachineAction *, InteractionEvent *);
    virtual void TranslateObject(StateMachineAction *, InteractionEvent *);
    virtual void RotateObject(StateMachineAction *, InteractionEvent *);

  private:
    void ColorizeSurface(BaseRenderer::Pointer renderer, double scalar = 0.0);

    double m_InitialPickedWorldPoint[4];
    Point2D m_InitialPickedDisplayPoint;

    Geometry3D::Pointer m_OriginalGeometry;

    Vector3D m_ObjectNormal;
  };
}
#endif // mitkClippingPlaneInteractor3D_h
