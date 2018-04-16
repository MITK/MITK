/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKDISPLAYACTIONEVENTS_H
#define MITKDISPLAYACTIONEVENTS_H

#include <MitkCoreExports.h>

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
    virtual ~DisplayActionEvent() {}
    virtual const char* GetEventName() const override { return "DisplayActionEvent"; }
    virtual bool CheckEvent(const itk::EventObject* e) const override
                                                     { return dynamic_cast<const Self*>(e) != nullptr; }
    virtual itk::EventObject* MakeObject() const override { return new Self(m_InteractionEvent); }
    InteractionEvent* GetInteractionEvent() const { return m_InteractionEvent; }
    DisplayActionEvent(const Self& s) : Superclass(s), m_InteractionEvent(s.m_InteractionEvent) {};

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
    virtual ~DisplayMoveEvent() {}
    virtual const char* GetEventName() const override { return "DisplayMoveEvent"; }
    virtual bool CheckEvent(const itk::EventObject* e) const override
                                                     { return dynamic_cast<const Self*>(e) != nullptr; }
    virtual itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_MoveVector); }
    const Vector2D& GetMoveVector() const { return m_MoveVector; }
    DisplayMoveEvent(const Self& s) : Superclass(s), m_MoveVector(s.m_MoveVector) {};

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
    virtual ~DisplaySetCrosshairEvent() {}
    virtual const char* GetEventName() const override { return "DisplaySetCrosshairEvent"; }
    virtual bool CheckEvent(const itk::EventObject* e) const override
                                                     { return dynamic_cast<const Self*>(e) != nullptr; }
    virtual itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_Position); }
    const Point3D& GetPosition() const { return m_Position; }
    DisplaySetCrosshairEvent(const Self& s) : Superclass(s), m_Position(s.m_Position) {};

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
    virtual ~DisplayZoomEvent() {}
    virtual const char* GetEventName() const override { return "DisplayZoomEvent"; }
    virtual bool CheckEvent(const itk::EventObject* e) const override
                                                     { return dynamic_cast<const Self*>(e) != nullptr; }
    virtual itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_ZoomFactor, m_StartCoordinate); }
    float GetZoomFactor() const { return m_ZoomFactor; }
    const Point2D& GetStartCoordinate() const { return m_StartCoordinate; }
    DisplayZoomEvent(const Self& s) : Superclass(s), m_ZoomFactor(s.m_ZoomFactor), m_StartCoordinate(s.m_StartCoordinate) {};

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
    DisplayScrollEvent(InteractionEvent* interactionEvent, int sliceDelta)
      : Superclass(interactionEvent)
      , m_SliceDelta(sliceDelta)
    {
    }
    virtual ~DisplayScrollEvent() {}
    virtual const char* GetEventName() const override { return "DisplayScrollEvent"; }
    virtual bool CheckEvent(const itk::EventObject* e) const override
                                                     { return dynamic_cast<const Self*>(e) != nullptr; }
    virtual itk::EventObject* MakeObject() const override { return new Self(GetInteractionEvent(), m_SliceDelta); }
    int GetSliceDelta() const { return m_SliceDelta; }
    DisplayScrollEvent(const Self& s) : Superclass(s), m_SliceDelta(s.m_SliceDelta) {};

  private:
    int m_SliceDelta;
  };
} // end namespace

#endif // MITKDISPLAYACTIONEVENTS_H
