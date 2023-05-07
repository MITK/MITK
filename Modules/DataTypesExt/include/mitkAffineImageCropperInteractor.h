/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAffineImageCropperInteractor_h
#define mitkAffineImageCropperInteractor_h

#include "MitkDataTypesExtExports.h"
#include "mitkBaseRenderer.h"
#include "mitkDataInteractor.h"
#include "mitkGeometry3D.h"

namespace mitk
{
  /**
    * \brief Affine interaction with objects in 3D windows.
    *
    * \ingroup Interaction
    */
  // Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
  class MITKDATATYPESEXT_EXPORT AffineImageCropperInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(AffineImageCropperInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      protected : AffineImageCropperInteractor();
    ~AffineImageCropperInteractor() override;
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
    virtual bool CheckOverObject(const InteractionEvent *interactionEvent);
    virtual void SelectObject(StateMachineAction *, InteractionEvent *);
    virtual void DeselectObject(StateMachineAction *, InteractionEvent *);
    virtual void InitTranslate(StateMachineAction *, InteractionEvent *);
    virtual void InitRotate(StateMachineAction *, InteractionEvent *);
    virtual void InitDeformation(StateMachineAction *, InteractionEvent *);
    virtual void TranslateObject(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void RotateObject(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void DeformObject(StateMachineAction *, InteractionEvent *);
    virtual void ScaleRadius(StateMachineAction *, InteractionEvent *interactionEvent);

  private:
    Geometry3D::Pointer m_OriginalGeometry;
    mitk::DataNode::Pointer m_SelectedNode;

    Point3D m_InitialPickedPoint;
    Point3D m_InitialOrigin;
    Point2D m_InitialPickedDisplayPoint;

    void Deselect();
  };
}
#endif
