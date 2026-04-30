/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRegistrationManipulationInteractor_h
#define mitkRegistrationManipulationInteractor_h

#include <mitkDataInteractor.h>
#include <mitkEventConfig.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkPoint.h>
#include <mitkVector.h>

#include <usServiceReference.h>

#include <MitkMatchPointRegistrationExports.h>

#include <map>

namespace mitk
{
  /** Event carrying a translation delta (Vector3D). */
  itkEventMacroDeclaration(RegistrationTranslationEvent, itk::AnyEvent);

  /** Event carrying a rotation delta (axis as Vector3D and angle in degrees). */
  itkEventMacroDeclaration(RegistrationRotationEvent, itk::AnyEvent);

  /** Event carrying a uniform scale factor. */
  itkEventMacroDeclaration(RegistrationScaleEvent, itk::AnyEvent);

  /** Event fired when the user clicks without a modifier to select a navigator position. */
  itkEventMacroDeclaration(RegistrationSelectPositionEvent, itk::AnyEvent);

  /**
   * \brief Data interactor for mouse-based registration manipulation in 2D render windows.
   *
   * The interactor supports in-plane translation, in-plane rotation (around the
   * view plane normal), and (optionally) uniform scaling via modifier+mouse-drag
   * combinations, plus a plain click for navigator position selection:
   *   - Shift + Left-drag:        In-plane translation
   *   - Ctrl + Left-drag:         In-plane rotation around the center of rotation
   *   - Shift + Ctrl + Left-drag: Uniform scaling (only active when explicitly enabled
   *                               via SetScalingEnabled(true))
   *   - Plain Left-click:         Selects the clicked world position as navigator
   *                               position
   *
   * While a modifier is held without a pressed mouse button, the interactor
   * foreshadows the upcoming gesture by switching the cursor.
   *
   * The interactor computes transform deltas and notifies observers via ITK events:
   *   - RegistrationTranslationEvent
   *   - RegistrationRotationEvent
   *   - RegistrationScaleEvent
   *   - RegistrationSelectPositionEvent
   *
   * It does NOT modify any data directly; the receiving widget/plugin is
   * responsible for reading the corresponding delta/position via the matching
   * getter and applying it to the registration transform.
   *
   * \pre The center of rotation must be set before rotation/scaling interactions.
   * \pre The interactor should be registered on a DataNode that is visible in the
   *      2D render windows where interaction is desired.
   */
  class MITKMATCHPOINTREGISTRATION_EXPORT RegistrationManipulationInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(RegistrationManipulationInteractor, DataInteractor);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Set the center of rotation in world coordinates.
     * Used for rotation angle computation and scaling. */
    void SetCenterOfRotation(const Point3D& center);

    /** Enable or disable the scaling interaction mode. */
    void SetScalingEnabled(bool enabled);

    /** @return true if scaling interaction is enabled. */
    bool GetScalingEnabled() const;

    /** Struct to transport the rotation delta via ITK event. */
    struct RotationDelta
    {
      Vector3D Axis;
      double AngleDeg;
    };

    /** @return The last computed translation delta. Valid after RegistrationTranslationEvent. */
    const Vector3D& GetTranslationDelta() const;

    /** @return The last computed rotation delta. Valid after RegistrationRotationEvent. */
    const RotationDelta& GetRotationDelta() const;

    /** @return The last computed scale factor. Valid after RegistrationScaleEvent. */
    double GetScaleFactor() const;

    /** @return The world position where the user clicked for position selection.
     *  Valid after RegistrationSelectPositionEvent. */
    const Point3D& GetSelectPosition() const;

    /** Disable the default display interactions (pan, zoom, etc.) to avoid conflicts
     *  with this interactor's modifier+mouse-drag combinations.
     *  Saves the current DisplayActionEventBroadcast configs for later restoration.
     *  Safe to call multiple times; double-disabling is a no-op.
     *  @sa EnableOriginalInteraction */
    void DisableOriginalInteraction();

    /** Restore the display interactions that were saved by DisableOriginalInteraction().
     *  @sa DisableOriginalInteraction */
    void EnableOriginalInteraction();

    /** Push the base manipulation cursor onto the application cursor stack.
     *  Call this when the interaction tool is activated.
     *  @sa PopManipulationCursor */
    void PushManipulationCursor();

    /** Pop the base manipulation cursor from the application cursor stack.
     *  Call this when the interaction tool is deactivated.
     *  @sa PushManipulationCursor */
    void PopManipulationCursor();

  private:
    RegistrationManipulationInteractor();
    ~RegistrationManipulationInteractor() override;

    void ConnectActionsAndFunctions() override;

    // State machine conditions
    bool ScalingEnabled(const InteractionEvent*);

    // State machine actions: hover cursor foreshadowing
    void HintTranslate(StateMachineAction*, InteractionEvent*);
    void HintRotate(StateMachineAction*, InteractionEvent*);
    void HintNeutral(StateMachineAction*, InteractionEvent*);

    // State machine actions: drag gestures
    void InitTranslation(StateMachineAction*, InteractionEvent*);
    void Translate(StateMachineAction*, InteractionEvent*);
    void EndTranslation(StateMachineAction*, InteractionEvent*);

    void InitRotation(StateMachineAction*, InteractionEvent*);
    void Rotate(StateMachineAction*, InteractionEvent*);
    void EndRotation(StateMachineAction*, InteractionEvent*);

    void InitScaling(StateMachineAction*, InteractionEvent*);
    void Scale(StateMachineAction*, InteractionEvent*);
    void EndScaling(StateMachineAction*, InteractionEvent*);

    void SelectPosition(StateMachineAction*, InteractionEvent*);

    // Interaction state
    Point2D m_InitialClickPosition2D;
    double m_InitialClickPosition2DZ = 0.0;
    Point3D m_InitialClickPosition3D;

    Point2D m_CenterOfRotation2D;
    Point3D m_CenterOfRotation;

    bool m_ScalingEnabled = false;

    // Output deltas / positions (populated before emitting events)
    Vector3D m_TranslationDelta;
    RotationDelta m_RotationDelta;
    double m_ScaleFactor = 1.0;
    Point3D m_SelectPosition;

    // Cursor management helpers
    void PushCursorFromResource(const std::string& resourceName);
    void PopCursorSafe();
    int m_NumPushedCursors = 0;

    // Hover hint cursor tracking
    enum class HintMode { None, Translate, Rotate };
    HintMode m_CurrentHintMode = HintMode::None;

    // Saved DisplayActionEventBroadcast configs for restore on EnableOriginalInteraction()
    bool m_OriginalInteractionDisabled = false;
    std::map<us::ServiceReferenceU, EventConfig> m_DisplayInteractionConfigs;
  };

} // namespace mitk

#endif
