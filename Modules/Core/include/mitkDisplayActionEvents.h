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
#include "mitkInteractionEvent.h"

// itk
#include <itkEventObject.h>

#include <mitkVector.h>
#include <mitkPoint.h>

namespace mitk
{
  class MITKCORE_EXPORT DisplayActionEvent : public itk::AnyEvent
  {
  public:
    typedef DisplayActionEvent Self;
    typedef itk::AnyEvent Superclass;

    DisplayActionEvent() : m_InteractionEvent(nullptr) {}
    DisplayActionEvent(InteractionEvent* interactionEvent) : m_InteractionEvent(interactionEvent) {}
    ~DisplayActionEvent() override {}
    const char* GetEventName() const override { return "DisplayActionEvent"; }
    bool CheckEvent(const itk::EventObject* e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(m_InteractionEvent); }
    InteractionEvent* GetInteractionEvent() const { return m_InteractionEvent; }
    BaseRenderer* GetSender() const
    {
      return m_InteractionEvent != nullptr ? m_InteractionEvent->GetSender() : nullptr;
    }
    DisplayActionEvent(const Self& s) : Superclass(s), m_InteractionEvent(s.GetInteractionEvent()) {};

  private:
    InteractionEvent* m_InteractionEvent;
    void operator=(const Self &);
  };

  class MITKCORE_EXPORT DisplayMoveEvent : public DisplayActionEvent
  {
  public:
    typedef DisplayMoveEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplayMoveEvent() : Superclass() {}
    DisplayMoveEvent(InteractionEvent* interactionEvent, const Vector2D& moveVector)
      : Superclass(interactionEvent)
      , m_MoveVector(moveVector)
    {
    }
    ~DisplayMoveEvent() override {}
    const char* GetEventName() const override { return "DisplayMoveEvent"; }
    bool CheckEvent(const itk::EventObject* e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_MoveVector); }
    const Vector2D& GetMoveVector() const { return m_MoveVector; }
    DisplayMoveEvent(const Self& s) : Superclass(s), m_MoveVector(s.GetMoveVector()) {};

  private:
    Vector2D m_MoveVector;
  };

  class MITKCORE_EXPORT DisplaySetCrosshairEvent : public DisplayActionEvent
  {
  public:
    typedef DisplaySetCrosshairEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplaySetCrosshairEvent() : Superclass() {}
    DisplaySetCrosshairEvent(InteractionEvent* interactionEvent, const Point3D& position)
      : Superclass(interactionEvent)
      , m_Position(position)
    {
    }
    ~DisplaySetCrosshairEvent() override {}
    const char* GetEventName() const override { return "DisplaySetCrosshairEvent"; }
    bool CheckEvent(const itk::EventObject* e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_Position); }
    const Point3D& GetPosition() const { return m_Position; }
    DisplaySetCrosshairEvent(const Self& s) : Superclass(s), m_Position(s.GetPosition()) {};

  private:
    Point3D m_Position;
  };

  class MITKCORE_EXPORT DisplayZoomEvent : public DisplayActionEvent
  {
  public:
    typedef DisplayZoomEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplayZoomEvent() : Superclass() {}
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
    float GetZoomFactor() const { return m_ZoomFactor; }
    const Point2D& GetStartCoordinate() const { return m_StartCoordinate; }
    DisplayZoomEvent(const Self& s) : Superclass(s), m_ZoomFactor(s.GetZoomFactor()), m_StartCoordinate(s.GetStartCoordinate()) {};

  private:
    float m_ZoomFactor;
    Point2D m_StartCoordinate;
  };

  class MITKCORE_EXPORT DisplayScrollEvent : public DisplayActionEvent
  {
  public:
    typedef DisplayScrollEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplayScrollEvent() : Superclass() {}
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
    int GetSliceDelta() const { return m_SliceDelta; }
    bool GetAutoRepeat() const { return m_AutoRepeat; }
    DisplayScrollEvent(const Self& s) : Superclass(s), m_SliceDelta(s.GetSliceDelta()), m_AutoRepeat(s.GetAutoRepeat()) {};

  private:
    int m_SliceDelta;
    bool m_AutoRepeat;
  };

  class MITKCORE_EXPORT DisplaySetLevelWindowEvent : public DisplayActionEvent
  {
  public:
    typedef DisplaySetLevelWindowEvent Self;
    typedef DisplayActionEvent Superclass;

    DisplaySetLevelWindowEvent() : Superclass() {}
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
    ScalarType GetLevel() const { return m_Level; }
    ScalarType GetWindow() const { return m_Window; }
    DisplaySetLevelWindowEvent(const Self& s) : Superclass(s), m_Level(s.GetLevel()), m_Window(s.GetWindow()) {};

  private:
    ScalarType m_Level;
    ScalarType m_Window;
  };

} // end namespace

#endif
