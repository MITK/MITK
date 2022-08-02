/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingShapeInteractor_h
#define mitkBoundingShapeInteractor_h

#include <mitkDataInteractor.h>
#include <mitkEventConfig.h>
#include <mitkGeometry3D.h>

#include <usServiceRegistration.h>

#include <MitkBoundingShapeExports.h>

namespace mitk
{
// create events for interactions
#pragma GCC visibility push(default)
  itkEventMacroDeclaration(BoundingShapeInteractionEvent, itk::AnyEvent);
#pragma GCC visibility pop

  /**
    * @brief Basic interaction methods for mitk::GeometryData
    *
    * Inherit from DataInteratcor, this provides functionality of a state machine and configurable inputs.
    *
    * \ingroup Interaction
    */
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(BoundingShapeInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetDataNode(DataNode *dataNode) override;
    void SetRotationEnabled(bool rotationEnabled);

  protected:
    BoundingShapeInteractor();
    ~BoundingShapeInteractor() override;

    /**
      * Here actions strings from the loaded state machine pattern are mapped to functions of
      * the DataInteractor. These functions are called when an action from the state machine pattern is executed.
      */
    void ConnectActionsAndFunctions() override;

    /**
      * @brief Called when a DataNode has been set/changed.
      */
    void DataNodeChanged() override;

    void HandlePositionChanged(const InteractionEvent *interactionEvent, Point3D &center);

    /**
    * @brief Checks if the mouse pointer is over the object.
    */
    virtual bool CheckOverObject(const InteractionEvent *);

    /**
    * @brief Checks if the mouse pointer is over one of the assigned handles.
    */
    virtual bool CheckOverHandles(const InteractionEvent *interactionEvent);

    /**
    * @brief Called if the mouse pointer is over the object indicated by a color change
    */
    virtual void SelectObject(StateMachineAction *, InteractionEvent *);

    /**
    * @brief Called if the mouse pointer leaves the area of the object
    */
    virtual void DeselectObject(StateMachineAction *, InteractionEvent *);

    /**
    * @brief Called if the mouse pointer is over one of the handles indicated by a color change
    */
    virtual void SelectHandle(StateMachineAction *, InteractionEvent *);

    /**
    * @brief Performs a translation of the object relative to the mouse movement
    */
    virtual void TranslateObject(StateMachineAction *, InteractionEvent *);

    /**
    * @brief Performs a object shape change by influencing the scaling of the initial bounding box
    */
    virtual void ScaleObject(StateMachineAction *, InteractionEvent *);

    /**
    * @brief Initializes the movement, stores starting position
    */

    virtual void InitInteraction(StateMachineAction *, InteractionEvent *interactionEvent);

    /**
    * @brief Deselects all Handles at the end of interaction
    */
    virtual void DeselectHandles(StateMachineAction *, InteractionEvent *interactionEvent);

    /**
    * @brief Restore default properties of bounding box and handles
    */
    virtual void RestoreNodeProperties();

    /**
    * @brief Initializes member variables.
    */
    bool InitMembers(InteractionEvent *interactionEvent);

  private:
    /**
    * @brief Enables default crosshair properties
    */
    void EnableOriginalInteraction();

    /**
    * @brief Sets limited crosshair properties (disable crosshair movement)
    */
    void DisableOriginalInteraction();

    class Impl;
    Impl *m_Impl;
  };
}
#endif
