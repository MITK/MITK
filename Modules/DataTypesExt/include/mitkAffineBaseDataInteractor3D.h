/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAffineBaseDataInteractor3D_h
#define mitkAffineBaseDataInteractor3D_h

#include <MitkDataTypesExtExports.h>
#include <mitkDataInteractor.h>
#include <mitkGeometry3D.h>

namespace mitk
{
  /** \brief Event fired on any affine interaction (translate, rotate, scale). */
#pragma GCC visibility push(default)
  itkEventMacroDeclaration(AffineInteractionEvent, itk::AnyEvent);
  /** \brief Event fired when a scale operation occurs. */
  itkEventMacroDeclaration(ScaleEvent, AffineInteractionEvent);
  /** \brief Event fired when a rotation operation occurs. */
  itkEventMacroDeclaration(RotateEvent, AffineInteractionEvent);
  /** \brief Event fired when a translation operation occurs. */
  itkEventMacroDeclaration(TranslateEvent, AffineInteractionEvent);
#pragma GCC visibility pop

  /**
   * \brief Affine interaction with mitk::BaseGeometry.
   *
   * Provides translation, rotation, and scaling of data objects in 3D render
   * windows via mouse and keyboard interaction. The interactor is driven by a
   * state machine pattern and supports configurable step sizes through
   * DataNode properties.
   *
   * Relevant DataNode properties:
   * - "AffineBaseDataInteractor3D.Translation Step Size" (float, default 1.0)
   * - "AffineBaseDataInteractor3D.Rotation Step Size" (float, default 1.0)
   * - "AffineBaseDataInteractor3D.Scale Step Size" (float, default 0.1)
   * - "AffineBaseDataInteractor3D.Anchor Point X/Y/Z" (float, default 0.0)
   *
   * \sa DataInteractor, AffineImageCropperInteractor
   * \ingroup Interaction
   */
  class MITKDATATYPESEXT_EXPORT AffineBaseDataInteractor3D : public DataInteractor
  {
  public:
    mitkClassMacro(AffineBaseDataInteractor3D, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Set the DataNode for this interactor.
     *
     * Restores the color of any previously assigned node before setting
     * the new one.
     *
     * \param[in] node The DataNode to interact with.
     */
    void SetDataNode(DataNode *node) override;

    /**
     * \brief Translate the given geometry by the specified vector.
     *
     * Triggers a rendering update after translation.
     *
     * \param[in] translate The translation vector in world coordinates.
     * \param[in,out] geometry The geometry to translate.
     */
    void TranslateGeometry(mitk::Vector3D translate, mitk::BaseGeometry *geometry);

    /**
     * \brief Rotate the given geometry around one of its principal axes.
     *
     * The rotation is performed around an anchor point defined by DataNode
     * properties.
     *
     * \param[in] angle The rotation angle in degrees.
     * \param[in] rotationaxis The axis index to rotate around (0=x, 1=y, 2=z).
     * \param[in,out] geometry The geometry to rotate.
     */
    void RotateGeometry(mitk::ScalarType angle, int rotationaxis, mitk::BaseGeometry *geometry);

    /**
     * \brief Scale the given geometry by the specified factors.
     *
     * Scaling is performed relative to an anchor point defined by DataNode
     * properties.
     *
     * \param[in] newScale The scale factors for each axis.
     * \param[in,out] geometry The geometry to scale.
     */
    void ScaleGeometry(mitk::Point3D newScale, mitk::BaseGeometry *geometry);

    /**
     * \brief Retrieve the geometry for the current time step from the interaction event.
     *
     * \param[in] interactionEvent The event providing time step information.
     * \return Pointer to the geometry for the relevant time step, or nullptr
     *         if the geometry could not be obtained.
     */
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
     * @brief InitMembers convenience method to avoid code duplication between InitRotate() and InitTranslate().
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
