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
   * %nnInteractive interactors are typically implemented by reusing either
   * existing MITK interactors or manual segmentation tools. For the latter,
   * methods like OnSetToolManager() and OnHandleEvent() can be overridden
   * to properly configure tools and forward relevant events to them.
   *
   * Utility methods such as GetToolManager(), GetDataStorage(), and
   * GetCurrentPromptType() provide essential functionality for derived classes.
   *
   * Derived classes are pragmatically linked to this base class through their
   * InteractionType, an enum class with enumerators for each interactor.
   * This simplifies handling collections of interactors, such as using
   * InteractionType as a key in maps (see GetType()).
   *
   * This base class optionally blocks global crosshair navigation via left
   * clicks while an %nnInteractive interactor is enabled. This allows derived
   * classes to use plain left clicks in MITK interactor event configurations,
   * which would otherwise require modifier keys (see InteractionMode).
   *
   * %nnInteractive interactors are mutually exclusive, and the client is
   * responsible for ensuring that no more than one interactor is enabled
   * at a time.
   */
  class MITKSEGMENTATION_EXPORT Interactor
  {
  public:
    virtual ~Interactor();

    /** \brief Returns the interaction type associated with this interactor.
     *
     * The interaction type serves as a unique identifier for each interactor,
     * allowing them to be easily distinguished or mapped in data structures.
     */
    InteractionType GetType() const;

    /** \brief Sets the ToolManager for this interactor.
     *
     * The ToolManager is responsible for managing segmentation tools
     * potentially used by derived classes. This method should be called as
     * early as possible to ensure that derived classes have access to the
     * tool manager (and data storage) if needed.
     */
    void SetToolManager(ToolManager* toolManager);

    /** \brief Handles an interaction event.
     *
     * This method forwards input events to enabled interactors. If a derived
     * class uses a segmentation tool class, override OnHandleEvent() to
     * call Tool::HandleEvent().
     */
    void HandleEvent(InteractionEvent* event);

    /** \brief Enables the interactor for the given prompt type.
     *
     * Derived classes must implement OnEnable() to define specific behavior.
     * To switch the prompt type for an enabled interactor, simply call this
     * method again and it will automatically call Disable() before re-enabling
     * the interactor for the other prompt type.
     *
     * If a derived class uses InteractionMode::BlockLMBDisplayInteraction,
     * the crosshair navigation via left click will be blocked while the
     * interactor is enabled.
     *
     * \see OnEnable()
     */
    void Enable(PromptType promptType);

    /** \brief Disables the interactor.
     *
     * Derived classes must implement OnDisable() to define specific behavior.
     *
     * If crosshair navigation via left click was previously blocked, it will be
     * unblocked after this method is called.
     *
     * \see OnDisable()
     */
    void Disable();

    /** \brief Checks whether the interactor is currently enabled.
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
     * e.g., "nnInteractive/Point.svg". Returns an empty string if the icon is
     * not found in the resources.
     */
    std::string GetIcon() const;

    /** \brief Retrieves the SVG cursor as a string from module resources based on PromptType.
     *
     * The cursor path is automatically determined using the InteractionType
     * and the specified PromptType, e.g., "nnInteractive/PositivePointCursor.svg".
     * Returns an empty string if the cursor is not found in the resources.
     */
    std::string GetCursor(PromptType promptType) const;

    /** \brief Checks whether any interactions have occurred.
     *
     * Derived classes must implement this method to determine whether the
     * interactor has processed any interactions.
     */
    virtual bool HasInteractions() const = 0;

    /** \brief Event triggered after an interaction has occurred.
     *
     * The signature matches SegWithPreviewTool::UpdatePreview().
     */
    Message1<bool> UpdatePreviewEvent;

  protected:
    /** \brief Constructs an interactor with a given type and mode.
     *
     * If a derived class uses InteractionMode::BlockLMBDisplayInteraction,
     * the crosshair navigation via left click will be blocked while the
     * interactor is enabled.
     *
     * \note Enumerators of InteractionType must not be reused across different
     * derived classes. Each interactor must be associated with a unique
     * interaction type.
     *
     * \param type The interaction type associated with this interactor.
     * \param mode The interaction mode, defaulting to InteractionMode::Default.
     */
    explicit Interactor(InteractionType type, InteractionMode mode = InteractionMode::Default);

    /** \brief Called when a ToolManager is assigned.
     *
     * Derived classes can override this method to configure tools and set up
     * necessary connections when a ToolManager is assigned.
     *
     * \see SetToolManager(), GetToolManager()
     */
    virtual void OnSetToolManager() {};

    /** \brief Called when an interaction event for a tool is received.
     *
     * Invoked only when the interactor is enabled.
     *
     * Derived classes can override this method to forward the event to their
     * tool.
     *
     * \see HandleEvent(), Tool::HandleEvent()
     */
    virtual void OnHandleEvent(InteractionEvent* /*event*/) {};

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
     * SetToolManager() must be called beforehand.
     *
     * \see SetToolManager()
     */
    ToolManager* GetToolManager() const;

    /** \brief Convenience method for accessing the DataStorage.
     *
     * SetToolManager() must be called beforehand.
     *
     * \see SetToolManager()
     */
    DataStorage* GetDataStorage() const;

    /** \brief Returns the currently active prompt type.
     *
     * The active prompt type is determined by the most recent call to Enable().
     *
     * \see Enable()
     */
    PromptType GetCurrentPromptType() const;

  private:
    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
