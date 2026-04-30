/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveInteractor_h
#define mitknnInteractiveInteractor_h

#include <mitknnInteractiveEnums.h>
#include <mitkMessage.h>

namespace mitk
{
  class DataStorage;
  class InteractionEvent;
  class ToolManager;
}

namespace mitk::nnInteractive
{
  /** \brief Base class for all %nnInteractive interactors.
   *
   * This class encapsulates the common logic and control interface for
   * %nnInteractive interactors. Essentially, they can be enabled, disabled,
   * and reset. Derived classes must respond to these events by overriding
   * the corresponding methods: OnEnable(), OnDisable(), and OnReset().
   *
   * %nnInteractive interactors are typically implemented by reusing an
   * existing MITK interactor or by driving a custom EventStateMachine. In
   * the latter case, OnHandleEvent() can be overridden to forward relevant
   * events to the state machine.
   *
   * Utility methods such as GetToolManager(), GetDataStorage(), and
   * GetCurrentPromptType() provide essential functionality for derived classes.
   *
   * Derived classes are pragmatically linked to this base class through their
   * InteractionType, an enum class with enumerators for each interactor.
   * This simplifies handling collections of interactors, such as using
   * InteractionType as a key in maps (see GetType()).
   *
   * This base class blocks global crosshair navigation via left clicks
   * while an %nnInteractive interactor is enabled. This allows derived
   * classes to use plain left clicks in MITK interactor event configurations,
   * which would otherwise require modifier keys.
   *
   * %nnInteractive interactors are mutually exclusive, and the client is
   * responsible for ensuring that no more than one interactor is enabled
   * at a time.
   */
  class MITKPYTHONSEGMENTATION_EXPORT Interactor
  {
  public:
    /** \brief Destructor.
     */
    virtual ~Interactor();

    /** \brief Returns the interaction type associated with this interactor.
     *
     * The interaction type serves as a unique identifier for each interactor,
     * allowing them to be easily distinguished or mapped in data structures.
     *
     * \return The InteractionType that was assigned during construction.
     */
    InteractionType GetType() const;

    /** \brief Sets the ToolManager for this interactor.
     *
     * The ToolManager is responsible for managing segmentation tools
     * potentially used by derived classes. This method should be called as
     * early as possible to ensure that derived classes have access to the
     * tool manager (and data storage) if needed.
     *
     * If the provided ToolManager is the same as the currently set one,
     * this method is a no-op.
     *
     * \param[in] toolManager Pointer to the ToolManager to associate with
     *                        this interactor.
     *
     * \sa OnSetToolManager(), GetToolManager(), GetDataStorage()
     */
    void SetToolManager(ToolManager* toolManager);

    /** \brief Handles an interaction event.
     *
     * This method forwards input events to enabled interactors. If a derived
     * class uses a segmentation tool class, override OnHandleEvent() to
     * call Tool::HandleEvent().
     *
     * Events are only forwarded when the interactor is enabled.
     *
     * \param[in] event The interaction event to handle.
     *
     * \pre The interactor must be enabled for the event to be processed.
     *
     * \sa OnHandleEvent(), IsEnabled()
     */
    void HandleEvent(InteractionEvent* event);

    /** \brief Enables the interactor for the given prompt type.
     *
     * Derived classes must implement OnEnable() to define specific behavior.
     * To switch the prompt type for an enabled interactor, simply call this
     * method again and it will automatically call Disable() before re-enabling
     * the interactor for the other prompt type.
     *
     * Crosshair navigation via left click is blocked while the interactor is
     * enabled and restored on Disable().
     *
     * \param[in] promptType The prompt type (positive or negative) to enable
     *                       this interactor for.
     *
     * \sa OnEnable(), Disable(), GetCurrentPromptType()
     */
    void Enable(PromptType promptType);

    /** \brief Disables the interactor.
     *
     * Derived classes must implement OnDisable() to define specific behavior.
     *
     * Restores left mouse button crosshair navigation that was blocked while
     * the interactor was enabled.
     *
     * \see OnDisable()
     */
    void Disable();

    /** \brief Checks whether the interactor is currently enabled.
     *
     * \return \c true if the interactor has been enabled via Enable() and has
     *         not yet been disabled, \c false otherwise.
     *
     * \sa Enable(), Disable()
     */
    bool IsEnabled() const;

    /** \brief Resets the interactor to its initial state.
     *
     * This function clears any ongoing interactions and restores the interactor
     * to a clean state.
     *
     * Derived classes must implement OnReset() for specific behavior.
     *
     * \see OnReset()
     */
    void Reset();

    /** \brief Retrieves the SVG icon as a string from module resources.
     *
     * The icon path is automatically determined based on the InteractionType,
     * e.g., "nnInteractive/Point.svg".
     *
     * \return The SVG icon data as a string, or an empty string if the icon
     *         is not found in the resources.
     */
    std::string GetIcon() const;

    /** \brief Retrieves the SVG cursor as a string from module resources based on PromptType.
     *
     * The cursor path is automatically determined using the InteractionType
     * and the specified PromptType, e.g., "nnInteractive/PositivePointCursor.svg".
     *
     * \param[in] promptType The prompt type used to determine the cursor variant.
     *
     * \return The SVG cursor data as a string, or an empty string if the
     *         cursor is not found in the resources.
     */
    std::string GetCursor(PromptType promptType) const;

    /** \brief Checks whether any interactions have occurred.
     *
     * Derived classes must implement this method to determine whether the
     * interactor has processed any interactions.
     *
     * \return \c true if at least one interaction has been recorded,
     *         \c false otherwise.
     */
    virtual bool HasInteractions() const = 0;

    /** \brief Event triggered after an interaction has occurred.
     *
     * Observers of this event are notified whenever a new interaction is
     * completed (e.g., a point is placed, a box is drawn). The boolean
     * parameter signature matches SegWithPreviewTool::UpdatePreview().
     *
     * \sa mitk::nnInteractiveTool
     */
    Message1<bool> UpdatePreviewEvent;

  protected:
    /** \brief Constructs an interactor with a given type.
     *
     * \note Enumerators of InteractionType must not be reused across different
     * derived classes. Each interactor must be associated with a unique
     * interaction type.
     *
     * \param type The interaction type associated with this interactor.
     */
    explicit Interactor(InteractionType type);

    /** \brief Called when a ToolManager is assigned.
     *
     * Derived classes can override this method to configure tools and set up
     * necessary connections when a ToolManager is assigned.
     *
     * \see SetToolManager(), GetToolManager()
     */
    virtual void OnSetToolManager();

    /** \brief Called when an interaction event is received.
     *
     * Invoked only when the interactor is enabled.
     *
     * Derived classes can override this method to forward the event to an
     * internal state machine.
     *
     * \param[in] event The interaction event to handle.
     *
     * \see HandleEvent()
     */
    virtual void OnHandleEvent(InteractionEvent* event);

    /** \brief Called when the interactor is enabled.
     *
     * Derived classes must implement this method to define specific behavior
     * when the interactor is activated.
     *
     * \see Enable()
     */
    virtual void OnEnable() = 0;

    /** \brief Called when the interactor is disabled.
     *
     * Derived classes must implement this method to define specific behavior
     * when the interactor is deactivated.
     *
     * \see Disable()
     */
    virtual void OnDisable() = 0;

    /** \brief Called when the interactor is reset.
     *
     * Derived classes must implement this method to handle reset logic,
     * ensuring the interactor returns to a clean state.
     *
     * \note Consider calling this method also from a derived class destructor
     * to perform clean up.
     *
     * \see Reset()
     */
    virtual void OnReset() = 0;

    /** \brief Returns the ToolManager associated with this interactor.
     *
     * \pre SetToolManager() must have been called beforehand.
     *
     * \return Pointer to the associated ToolManager, or \c nullptr if none
     *         has been set.
     *
     * \sa SetToolManager()
     */
    ToolManager* GetToolManager() const;

    /** \brief Convenience method for accessing the DataStorage.
     *
     * Retrieves the DataStorage from the associated ToolManager.
     *
     * \pre SetToolManager() must have been called beforehand.
     *
     * \return Pointer to the DataStorage, or \c nullptr if no ToolManager
     *         has been set.
     *
     * \sa SetToolManager(), GetToolManager()
     */
    DataStorage* GetDataStorage() const;

    /** \brief Returns the currently active prompt type.
     *
     * The active prompt type is determined by the most recent call to Enable().
     * Before the first call to Enable(), the default is PromptType::Positive.
     *
     * \return The currently active PromptType.
     *
     * \sa Enable()
     */
    PromptType GetCurrentPromptType() const;

  private:
    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
