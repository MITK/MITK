/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDisplayActionEvents_h
#define mitkDisplayActionEvents_h

#include <MitkCoreExports.h>

// mitk core
#include <mitkInteractionEvent.h>

// itk
#include <itkEventObject.h>

#include <mitkVector.h>
#include <mitkPoint.h>

namespace mitk
{
  /**
   * \brief Base class for display action events broadcast by DisplayActionEventBroadcast.
   *
   * Wraps an InteractionEvent and provides access to the sender BaseRenderer.
   * Subclasses carry additional data specific to each action (move vector,
   * zoom factor, etc.).
   *
   * \ingroup Interaction
   * \sa DisplayActionEventBroadcast DisplayActionEventHandler
   */
  class MITKCORE_EXPORT DisplayActionEvent : public itk::AnyEvent
  {
  public:
    typedef DisplayActionEvent Self;
    typedef itk::AnyEvent Superclass;

    /** \brief Default constructor with no interaction event. */
    DisplayActionEvent() : m_InteractionEvent(nullptr) {}

    /**
     * \brief Construct with the originating interaction event.
     * \param interactionEvent The interaction event that triggered this display action.
     */
    DisplayActionEvent(InteractionEvent* interactionEvent) : m_InteractionEvent(interactionEvent) {}
    ~DisplayActionEvent() override {}
    const char* GetEventName() const override { return "DisplayActionEvent"; }
    bool CheckEvent(const itk::EventObject* e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(m_InteractionEvent); }

    /**
     * \brief Get the originating interaction event.
     * \return The InteractionEvent, or nullptr if not set.
     */
    InteractionEvent* GetInteractionEvent() const { return m_InteractionEvent; }

    /**
     * \brief Get the BaseRenderer that sent the interaction event.
     * \return The sender renderer, or nullptr if no interaction event is set.
     */
    BaseRenderer* GetSender() const
    {
      return m_InteractionEvent != nullptr ? m_InteractionEvent->GetSender() : nullptr;
    }

    /** \brief Copy constructor. */
    DisplayActionEvent(const Self& s) : Superclass(s), m_InteractionEvent(s.GetInteractionEvent()) {};

  private:
    InteractionEvent* m_InteractionEvent;
    void operator=(const Self &);
  };

  /**
   * \brief Display action event carrying a 2D move vector for camera panning.
   * \sa DisplayActionEvent
   */
  class MITKCORE_EXPORT DisplayMoveEvent : public DisplayActionEvent
  {
  public:
    typedef DisplayMoveEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplayMoveEvent() : Superclass() {}

    /**
     * \brief Construct with the originating event and move vector.
     * \param interactionEvent The interaction event that triggered this action.
     * \param moveVector       The 2D translation vector in mm.
     */
    DisplayMoveEvent(InteractionEvent* interactionEvent, const Vector2D& moveVector)
      : Superclass(interactionEvent)
      , m_MoveVector(moveVector)
    {
    }
    ~DisplayMoveEvent() override {}
    const char* GetEventName() const override { return "DisplayMoveEvent"; }
    bool CheckEvent(const itk::EventObject* e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_MoveVector); }

    /**
     * \brief Get the 2D move vector.
     * \return A const reference to the move vector in mm.
     */
    const Vector2D& GetMoveVector() const { return m_MoveVector; }

    /** \brief Copy constructor. */
    DisplayMoveEvent(const Self& s) : Superclass(s), m_MoveVector(s.GetMoveVector()) {};

  private:
    Vector2D m_MoveVector;
  };

  /**
   * \brief Display action event carrying a 3D world position for crosshair placement.
   * \sa DisplayActionEvent
   */
  class MITKCORE_EXPORT DisplaySetCrosshairEvent : public DisplayActionEvent
  {
  public:
    typedef DisplaySetCrosshairEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplaySetCrosshairEvent() : Superclass() {}

    /**
     * \brief Construct with the originating event and crosshair position.
     * \param interactionEvent The interaction event that triggered this action.
     * \param position         The 3D world position for the crosshair.
     */
    DisplaySetCrosshairEvent(InteractionEvent* interactionEvent, const Point3D& position)
      : Superclass(interactionEvent)
      , m_Position(position)
    {
    }
    ~DisplaySetCrosshairEvent() override {}
    const char* GetEventName() const override { return "DisplaySetCrosshairEvent"; }
    bool CheckEvent(const itk::EventObject* e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_Position); }

    /**
     * \brief Get the 3D crosshair position.
     * \return A const reference to the position in world coordinates.
     */
    const Point3D& GetPosition() const { return m_Position; }

    /** \brief Copy constructor. */
    DisplaySetCrosshairEvent(const Self& s) : Superclass(s), m_Position(s.GetPosition()) {};

  private:
    Point3D m_Position;
  };

  /**
   * \brief Display action event carrying a zoom factor and start coordinate.
   * \sa DisplayActionEvent
   */
  class MITKCORE_EXPORT DisplayZoomEvent : public DisplayActionEvent
  {
  public:
    typedef DisplayZoomEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplayZoomEvent() : Superclass() {}

    /**
     * \brief Construct with the originating event, zoom factor, and start coordinate.
     * \param interactionEvent The interaction event that triggered this action.
     * \param zoomFactor       The zoom factor (>1 zooms in, <1 zooms out).
     * \param startCoordinate  The 2D point around which to zoom (in mm).
     */
    DisplayZoomEvent(InteractionEvent* interactionEvent, float zoomFactor, const Point2D& startCoordinate)
      : Superclass(interactionEvent)
      , m_ZoomFactor(zoomFactor)
      , m_StartCoordinate(startCoordinate)
    {
    }
    ~DisplayZoomEvent() override {}
    const char* GetEventName() const override { return "DisplayZoomEvent"; }
    bool CheckEvent(const itk::EventObject* e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_ZoomFactor, m_StartCoordinate); }

    /**
     * \brief Get the zoom factor.
     * \return The zoom factor.
     */
    float GetZoomFactor() const { return m_ZoomFactor; }

    /**
     * \brief Get the zoom start coordinate.
     * \return A const reference to the 2D start coordinate in mm.
     */
    const Point2D& GetStartCoordinate() const { return m_StartCoordinate; }

    /** \brief Copy constructor. */
    DisplayZoomEvent(const Self& s) : Superclass(s), m_ZoomFactor(s.GetZoomFactor()), m_StartCoordinate(s.GetStartCoordinate()) {};

  private:
    float m_ZoomFactor;
    Point2D m_StartCoordinate;
  };

  /**
   * \brief Display action event carrying a slice scroll delta and auto-repeat flag.
   * \sa DisplayActionEvent
   */
  class MITKCORE_EXPORT DisplayScrollEvent : public DisplayActionEvent
  {
  public:
    typedef DisplayScrollEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplayScrollEvent() : Superclass() {}

    /**
     * \brief Construct with the originating event, slice delta, and auto-repeat flag.
     * \param interactionEvent The interaction event that triggered this action.
     * \param sliceDelta       Number of slices to scroll (positive or negative).
     * \param autoRepeat       Whether scrolling wraps around at the data set boundary.
     */
    DisplayScrollEvent(InteractionEvent* interactionEvent, int sliceDelta, bool autoRepeat)
      : Superclass(interactionEvent)
      , m_SliceDelta(sliceDelta)
      , m_AutoRepeat(autoRepeat)
    {
    }
    ~DisplayScrollEvent() override {}
    const char* GetEventName() const override { return "DisplayScrollEvent"; }
    bool CheckEvent(const itk::EventObject *e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_SliceDelta, m_AutoRepeat); }

    /**
     * \brief Get the number of slices to scroll.
     * \return The slice delta (positive = forward, negative = backward).
     */
    int GetSliceDelta() const { return m_SliceDelta; }

    /**
     * \brief Get whether auto-repeat (wrap-around) is enabled.
     * \return \c true if scrolling wraps around at data set boundaries.
     */
    bool GetAutoRepeat() const { return m_AutoRepeat; }

    /** \brief Copy constructor. */
    DisplayScrollEvent(const Self& s) : Superclass(s), m_SliceDelta(s.GetSliceDelta()), m_AutoRepeat(s.GetAutoRepeat()) {};

  private:
    int m_SliceDelta;
    bool m_AutoRepeat;
  };

  /**
   * \brief Display action event carrying level and window values for level-window adjustment.
   * \sa DisplayActionEvent
   */
  class MITKCORE_EXPORT DisplaySetLevelWindowEvent : public DisplayActionEvent
  {
  public:
    typedef DisplaySetLevelWindowEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplaySetLevelWindowEvent() : Superclass() {}

    /**
     * \brief Construct with the originating event, level, and window values.
     * \param interactionEvent The interaction event that triggered this action.
     * \param level            The level (center) value for the level-window.
     * \param window           The window (width) value for the level-window.
     */
    DisplaySetLevelWindowEvent(InteractionEvent* interactionEvent, ScalarType level, ScalarType window)
      : Superclass(interactionEvent)
      , m_Level(level)
      , m_Window(window)
    {
    }
    ~DisplaySetLevelWindowEvent() override {}
    const char* GetEventName() const override { return "DisplaySetLevelWindowEvent"; }
    bool CheckEvent(const itk::EventObject *e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_Level, m_Window); }

    /**
     * \brief Get the level (center) value.
     * \return The level value.
     */
    ScalarType GetLevel() const { return m_Level; }

    /**
     * \brief Get the window (width) value.
     * \return The window value.
     */
    ScalarType GetWindow() const { return m_Window; }

    /** \brief Copy constructor. */
    DisplaySetLevelWindowEvent(const Self& s) : Superclass(s), m_Level(s.GetLevel()), m_Window(s.GetWindow()) {};

  private:
    ScalarType m_Level;
    ScalarType m_Window;
  };

} // end namespace

#endif
