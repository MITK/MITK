/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-21 11:55:15 +0200 (Mo, 21 Apr 2008) $
Version:   $Revision: 14134 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef POINTNAVIGATIONCONTROLLER_H_HEADER_INCLUDED
#define POINTNAVIGATIONCONTROLLER_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkNavigationController.h"
#include "mitkRenderingManager.h"
#include "mitkTimeSlicedGeometry.h"
#include <itkEventObject.h>
#include <itkCommand.h>

namespace mitk {

#define mitkTimeSlicedGeometryEventMacro( classname , super ) \
 class MITK_CORE_EXPORT classname : public super { \
   public: \
     typedef classname Self; \
     typedef super Superclass; \
     classname(TimeSlicedGeometry* aTimeSlicedGeometry, unsigned int aPos) \
     : Superclass(aTimeSlicedGeometry, aPos) {} \
     virtual ~classname() {} \
     virtual const char * GetEventName() const { return #classname; } \
     virtual bool CheckEvent(const ::itk::EventObject* e) const \
       { return dynamic_cast<const Self*>(e); } \
     virtual ::itk::EventObject* MakeObject() const \
       { return new Self(GetTimeSlicedGeometry(), GetPos()); } \
   private: \
     void operator=(const Self&); \
 }

class PlaneGeometry;
class Geometry3D;
class BaseRenderer;
class Ellipsoid;

/**
 * \brief similar to SliceNavigationController, at the moment only used by QmitkLocalizerDialogBar
 * 
 *  compared to the SliceNavigationController, the only thing that has really changed is that another 
 *  action is performed when an event is emited. Instead of SelectSlicebyPoint(), setSphere is called. 
 *  This method sets the origin of the sphere, saved in the node "Localizer" to the point in which all
 *  3 planes meet.
 *
 * \todo implement for non-evenly-timed geometry!
 * \ingroup NavigationControl
 */
class MITK_CORE_EXPORT PointNavigationController : public NavigationController
{
  public:
    mitkClassMacro(PointNavigationController,NavigationController);
    itkNewMacro(Self);
    mitkNewMacro1Param(Self, const char *);
  
    /**
     * \brief Do the actual creation and send it to the connected 
     * observers (renderers)
     * 
     */
    virtual void Update();

    /**
     * \brief Extended version of Update, additionally allowing to
     * specify the direction/orientation of the created geometry.
     * 
     */
    virtual void Update(ViewDirection viewDirection, bool top = true,
      bool frontside = true, bool rotated = false);

    /**
     * \brief Send the created geometry to the connected
     * observers (renderers)
     * 
     * Called by Update().
     */
    virtual void SendCreatedWorldGeometry();
   
    /**
     * \brief Tell observers to re-read the currently selected 2D geometry
     * 
     * Called by mitk::SlicesRotator during rotation.
     */
    virtual void SendCreatedWorldGeometryUpdate();

    /**
     * \brief Send the currently selected slice to the connected
     * observers (renderers)
     * 
     * Called by Update().
     */
    virtual void SendSlice();

    /**
     * \brief Send the currently selected time to the connected
     * observers (renderers)
     * 
     * Called by Update().
     */
    virtual void SendTime();

    class MITK_CORE_EXPORT TimeSlicedGeometryEvent : public itk::AnyEvent 
    { 
      public: 
        typedef TimeSlicedGeometryEvent Self; 
        typedef itk::AnyEvent Superclass; 

        TimeSlicedGeometryEvent(
          TimeSlicedGeometry* aTimeSlicedGeometry, unsigned int aPos) 
          : m_TimeSlicedGeometry(aTimeSlicedGeometry), m_Pos(aPos) 
          {} 

        virtual ~TimeSlicedGeometryEvent() 
          {} 

        virtual const char * GetEventName() const 
          { return "TimeSlicedGeometryEvent"; } 

        virtual bool CheckEvent(const ::itk::EventObject* e) const 
          { return dynamic_cast<const Self*>(e); } 

        virtual ::itk::EventObject* MakeObject() const 
          { return new Self(m_TimeSlicedGeometry, m_Pos); } 

        TimeSlicedGeometry* GetTimeSlicedGeometry() const 
          { return m_TimeSlicedGeometry; }

        unsigned int GetPos() const 
          { return m_Pos; }

      private: 
        TimeSlicedGeometry::Pointer m_TimeSlicedGeometry;
        unsigned int m_Pos;
        // TimeSlicedGeometryEvent(const Self&); 
        void operator=(const Self&); //just hide
    };
  
    mitkTimeSlicedGeometryEventMacro( 
      GeometrySendEvent,TimeSlicedGeometryEvent );
    mitkTimeSlicedGeometryEventMacro( 
      GeometryUpdateEvent, TimeSlicedGeometryEvent );
    mitkTimeSlicedGeometryEventMacro( 
      GeometryTimeEvent, TimeSlicedGeometryEvent );
    mitkTimeSlicedGeometryEventMacro( 
      GeometrySliceEvent, TimeSlicedGeometryEvent );
  
    template <typename T> 
    void ConnectGeometrySendEvent(T* receiver)
    {
      typedef typename itk::ReceptorMemberCommand<T>::Pointer
        ReceptorMemberCommandPointer;
      ReceptorMemberCommandPointer eventReceptorCommand = 
        itk::ReceptorMemberCommand<T>::New();
      eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometry);
      AddObserver(GeometrySendEvent(NULL,0), eventReceptorCommand);
    }
    
    template <typename T> 
    void ConnectGeometryUpdateEvent(T* receiver)
    {
      typedef typename itk::ReceptorMemberCommand<T>::Pointer 
        ReceptorMemberCommandPointer;
      ReceptorMemberCommandPointer eventReceptorCommand = 
        itk::ReceptorMemberCommand<T>::New();
      eventReceptorCommand->SetCallbackFunction(receiver, &T::UpdateGeometry);
      AddObserver(GeometryUpdateEvent(NULL,0), eventReceptorCommand);
    }
  
    template <typename T> 
    void ConnectGeometrySliceEvent(T* receiver, bool connectSendEvent=true)
    {
      typedef typename itk::ReceptorMemberCommand<T>::Pointer 
        ReceptorMemberCommandPointer;
      ReceptorMemberCommandPointer eventReceptorCommand = 
        itk::ReceptorMemberCommand<T>::New();
      eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometrySlice);
      AddObserver(GeometrySliceEvent(NULL,0), eventReceptorCommand);
      if(connectSendEvent)
        ConnectGeometrySendEvent(receiver);
    }

    template <typename T>
    void ConnectGeometryTimeEvent(T* receiver, bool connectSendEvent=true)
    {
      typedef typename itk::ReceptorMemberCommand<T>::Pointer 
        ReceptorMemberCommandPointer;
      ReceptorMemberCommandPointer eventReceptorCommand = 
        itk::ReceptorMemberCommand<T>::New();
      eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometryTime);
      AddObserver(GeometryTimeEvent(NULL,0), eventReceptorCommand);
      if(connectSendEvent)
        ConnectGeometrySendEvent(receiver);
    }

    template <typename T>
    void ConnectGeometryEvents(T* receiver)
    {
      //connect sendEvent only once
      ConnectGeometrySliceEvent(receiver, false);
      ConnectGeometryTimeEvent(receiver);
    }

    /**
     * \brief To connect multiple SliceNavigationController, we can 
     * act as an observer ourselves: implemented interface
     */
    virtual void SetGeometrySlice(const itk::EventObject & geometrySliceEvent);

    /** \brief Positions the SNC according to the specified point */
    //void SelectSliceByPoint( const mitk::Point3D &point );

    //void setSphere();

    void setSphere(const Point3D &point);

    /** \brief Re-orients the slice stack to include the plane specified by
     * the given point an normal vector. 
     */
    void ReorientSlices( 
      const mitk::Point3D &point, const mitk::Vector3D &normal );


    virtual bool ExecuteAction( 
      Action* action, mitk::StateEvent const* stateEvent);

    void ExecuteOperation(Operation* operation);

  protected:
    PointNavigationController(const char * type = NULL);
    virtual ~PointNavigationController();

};

} // namespace mitk

#endif /* POINTNAVIGATIONCONTROLLER_H_HEADER_INCLUDED */
