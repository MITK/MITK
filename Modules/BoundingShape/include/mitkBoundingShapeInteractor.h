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
#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
  itkEventMacroDeclaration(BoundingShapeInteractionEvent, itk::AnyEvent);
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

  /** \brief Interactor for manipulating bounding shapes (GeometryData).
   *
   * Provides interactive translation and scaling of a bounding box through
   * mouse-based handle manipulation. Inherits from DataInteractor, providing
   * state machine and configurable input support.
   *
   * The interactor manages handles that can be dragged to resize the bounding
   * box: face handles resize one axis; in 2D render windows, additional handles
   * on the corners of the rendered cross-section resize two axes at once (the
   * two the crossed box edge does not run along, which are the in-plane axes as
   * long as the box is aligned with the slice). In 2D render windows the entire
   * box can be translated by dragging its body. Visual feedback is provided
   * through color changes on hover and selection.
   *
   * \sa BoundingShapeCropper, BoundingShapeVtkMapper2D, BoundingShapeVtkMapper3D, DataInteractor
   * \ingroup Interaction
   */
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(BoundingShapeInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Set the data node this interactor operates on.
     *
     * Initializes default bounding shape properties on the node and sets up
     * the interaction geometry.
     *
     * \param[in] dataNode The data node containing GeometryData to interact with.
     */
    void SetDataNode(DataNode *dataNode) override;

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

    /**
     * @brief Recomputes the handles visible in the render window of the given event.
     *
     * In a 2D render window handles sit on the sides and corners of the rendered cross-section
     * (only where the box crosses the current slice); in the 3D render window they sit at the
     * centers of the six box faces.
     *
     * @return The center of the bounding box.
     */
    Point3D UpdateHandles(const InteractionEvent *interactionEvent);

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
