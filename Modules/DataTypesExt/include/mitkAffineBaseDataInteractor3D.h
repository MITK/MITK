/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAffineBaseDataInteractor3D_h
#define mitkAffineBaseDataInteractor3D_h

#include "MitkDataTypesExtExports.h"
#include <mitkDataInteractor.h>
#include <mitkGeometry3D.h>

namespace mitk
{
////create events for interactions
#pragma GCC visibility push(default)
  itkEventMacroDeclaration(AffineInteractionEvent, itk::AnyEvent);
  itkEventMacroDeclaration(ScaleEvent, AffineInteractionEvent);
  itkEventMacroDeclaration(RotateEvent, AffineInteractionEvent);
  itkEventMacroDeclaration(TranslateEvent, AffineInteractionEvent);
#pragma GCC visibility pop

  /**
    * \brief Affine interaction with mitk::BaseGeometry.
    *
    * \ingroup Interaction
    */
  // Inherit from DataInteractor, this provides functionality of a state machine and configurable inputs.
  class MITKDATATYPESEXT_EXPORT AffineBaseDataInteractor3D : public DataInteractor
  {
  public:
    mitkClassMacro(AffineBaseDataInteractor3D, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetDataNode(DataNode *node) override;
    void TranslateGeometry(mitk::Vector3D translate, mitk::BaseGeometry *geometry);
    void RotateGeometry(mitk::ScalarType angle, int rotationaxis, mitk::BaseGeometry *geometry);
    void ScaleGeometry(mitk::Point3D newScale, mitk::BaseGeometry *geometry);
    mitk::BaseGeometry *GetUpdatedTimeGeometry(mitk::InteractionEvent *interactionEvent);

  protected:
    AffineBaseDataInteractor3D();
    ~AffineBaseDataInteractor3D() override;
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
    virtual void ScaleObject(StateMachineAction *, InteractionEvent *);
    virtual void TranslateUpKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void TranslateDownKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void TranslateLeftKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void TranslateRightKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void TranslateUpModifierKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void TranslateDownModifierKey(StateMachineAction *, InteractionEvent *interactionEvent);

    virtual void RotateUpKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void RotateDownKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void RotateLeftKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void RotateRightKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void RotateUpModifierKey(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void RotateDownModifierKey(StateMachineAction *, InteractionEvent *interactionEvent);

    virtual void ScaleDownKey(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
    virtual void ScaleUpKey(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);

    virtual void RestoreNodeProperties();

    /**
     * @brief InitMembers convinience method to avoid code duplication between InitRotate() and InitTranslate().
     * @param interactionEvent
     */
    bool InitMembers(InteractionEvent *interactionEvent);

  private:
    Point3D m_InitialPickedWorldPoint;
    Point2D m_InitialPickedDisplayPoint;

    Geometry3D::Pointer m_OriginalGeometry;
  };
}
#endif
