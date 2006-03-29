/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F
#define SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F

#include "mitkCommon.h"
#include "mitkBaseController.h"
#include "mitkImageToImageFilter.h"
#include "mitkTimeSlicedGeometry.h"
#include <itkEventObject.h>
#include <itkCommand.h>

namespace mitk {

#define mitkTimeSlicedGeometryEventMacro( classname , super ) \
 class classname : public super { \
   public: \
     typedef classname Self; \
     typedef super Superclass; \
     classname(TimeSlicedGeometry* aTimeSlicedGeometry, unsigned int aPos) : Superclass(aTimeSlicedGeometry, aPos) {} \
     virtual ~classname() {} \
     virtual const char * GetEventName() const { return #classname; } \
     virtual bool CheckEvent(const ::itk::EventObject* e) const \
       { return dynamic_cast<const Self*>(e); } \
     virtual ::itk::EventObject* MakeObject() const \
       { return new Self(GetTimeSlicedGeometry(), GetPos()); } \
   private: \
     void operator=(const Self&); \
 }

//##Documentation
//## @brief controls the selection of the slice the associated BaseRenderer
//## will display
//##
//## Subclass of BaseController. Controls the selection of the slice the
//## associated BaseRenderer will display.
//## Example:
//## \code
//## //Initialization
//##   sliceCtrl = mitk::SliceNavigationController::New();
//##   //tell the navigator the geometry to be sliced (with geometry a Geometry3D::ConstPointer)
//##   sliceCtrl->SetInputWorldGeometry(geometry.GetPointer());
//##   //tell the navigator in which direction it shall slice the data
//##   sliceCtrl->SetViewDirection(mitk::SliceNavigationController::Transversal);
//## //Connect one or more BaseRenderer to this navigator, i.e.: events sent 
//##   //by the navigator when stepping through the slices (e.g. by 
//##   //sliceCtrl->GetSlice()->Next()) will be received by the BaseRenderer 
//##   //(in this example only slice-changes, see also ConnectGeometryTimeEvent 
//##   //and ConnectGeometryEvents.)
//##   sliceCtrl->ConnectGeometrySliceEvent(renderer.GetPointer());
//## //create a world geometry and send the information to the connected renderer(s)
//##   sliceCtrl->Update();
//## \endcode
//## You can connect visible navigators to a SliceNavigationController, e.g., a
//## QmitkSliderNavigator (for Qt):
//## \code
//##   //create the visible navigator (a slider with a spin-box)
//##   QmitkSliderNavigator* navigator = new QmitkSliderNavigator(parent, "slidernavigator");
//##   //connect the navigator to the slice-stepper of the SliceNavigationController.
//##   //For initialization (position, mininal and maximal values) the values of the 
//##   //SliceNavigationController are used. Thus, accessing methods of a navigator
//##   //is normally not necessary, since everything can be set via the (Qt-independent)
//##   //SliceNavigationController. The QmitkStepperAdapter converts the Qt-signals
//##   //to Qt-independent itk-events.
//##   new QmitkStepperAdapter(navigator, sliceCtrl->GetSlice(), "navigatoradaptor");
//## \endcode
//## @todo implement for non-evenly-timed geometry!
//## @ingroup NavigationControl
class SliceNavigationController : public BaseController
{
public:
  mitkClassMacro(SliceNavigationController,BaseController);
  //itkNewMacro(Self);
  SliceNavigationController(const char * type = NULL);

  //##Documentation
  //## @brief Possible view directions, \a Original will uses 
  //## the Geometry2D instances in a SlicedGeometry3D provided
  //## as input world geometry (by SetInputWorldGeometry).
  enum ViewDirection{Transversal, Sagittal, Frontal, Original};

  //##Documentation
  //## @brief Set the input world geometry out of which the
  //## geometries for slicing will be created.
  void SetInputWorldGeometry(const mitk::Geometry3D* geometry);
  itkGetConstObjectMacro(InputWorldGeometry, mitk::Geometry3D);

  //##Documentation
  //## @brief Access the created geometry
  itkGetConstObjectMacro(CreatedWorldGeometry, mitk::Geometry3D);

  //##Documentation
  //## @brief Set the desired view directions
  //##
  //## \sa ViewDirection
  //## \sa Update(ViewDirection viewDirection, bool top = true, bool frontside = true, bool rotated = false)
  itkSetMacro(ViewDirection, ViewDirection);
  itkGetMacro(ViewDirection, ViewDirection);

  //##Documentation
  //## @brief Do the actual creation and send it to the connected 
  //## observers (renderers)
  //##
  virtual void Update();

  //##Documentation
  //## @brief Extended version of Update, additionally allowing to
  //## specify the direction/orientation of the created geometry.
  //##
  virtual void Update(ViewDirection viewDirection, bool top = true, bool frontside = true, bool rotated = false);

  //##Documentation
  //## @brief Send the created geometry to the connected
  //## observers (renderers)
  //##
  //## Called by Update().
  virtual void SendCreatedWorldGeometry();

  //##Documentation
  //## @brief Send the currently selected slice to the connected
  //## observers (renderers)
  //##
  //## Called by Update().
  virtual void SendSlice();

  //##Documentation
  //## @brief Send the currently selected time to the connected
  //## observers (renderers)
  //##
  //## Called by Update().
  virtual void SendTime();

  itkEventMacro( UpdateEvent      , itk::AnyEvent );

  class TimeSlicedGeometryEvent : public itk::AnyEvent 
  { 
  public: 
    typedef TimeSlicedGeometryEvent Self; 
    typedef itk::AnyEvent Superclass; 
    TimeSlicedGeometryEvent(TimeSlicedGeometry* aTimeSlicedGeometry, unsigned int aPos) : 
      m_TimeSlicedGeometry(aTimeSlicedGeometry), m_Pos(aPos) {} 
    virtual ~TimeSlicedGeometryEvent() {} 
    virtual const char * GetEventName() const { return "TimeSlicedGeometryEvent"; } 
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { return dynamic_cast<const Self*>(e); } 
    virtual ::itk::EventObject* MakeObject() const 
    { return new Self(m_TimeSlicedGeometry, m_Pos); } 
    TimeSlicedGeometry* GetTimeSlicedGeometry() const { return m_TimeSlicedGeometry; }
    unsigned int GetPos() const { return m_Pos; }
  private: 
    TimeSlicedGeometry::Pointer m_TimeSlicedGeometry;
    unsigned int m_Pos;
    // TimeSlicedGeometryEvent(const Self&); 
    void operator=(const Self&); 
  };
  mitkTimeSlicedGeometryEventMacro( GeometrySendEvent    , TimeSlicedGeometryEvent );
  mitkTimeSlicedGeometryEventMacro( GeometryTimeEvent    , TimeSlicedGeometryEvent );
  mitkTimeSlicedGeometryEventMacro( GeometrySliceEvent   , TimeSlicedGeometryEvent );
  
  template <typename T> void ConnectGeometrySendEvent(T* receiver)
  {
    typedef typename itk::ReceptorMemberCommand<T>::Pointer ReceptorMemberCommandPointer;
    ReceptorMemberCommandPointer eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
    eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometry);
    AddObserver(GeometrySendEvent(NULL,0), eventReceptorCommand);
  }
  
  template <typename T> void ConnectGeometrySliceEvent(T* receiver, bool connectSendEvent=true)
  {
    typedef typename itk::ReceptorMemberCommand<T>::Pointer ReceptorMemberCommandPointer;
    ReceptorMemberCommandPointer eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
    eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometrySlice);
    AddObserver(GeometrySliceEvent(NULL,0), eventReceptorCommand);
    if(connectSendEvent)
      ConnectGeometrySendEvent(receiver);
  }

  template <typename T> void ConnectGeometryTimeEvent(T* receiver, bool connectSendEvent=true)
  {
    typedef typename itk::ReceptorMemberCommand<T>::Pointer ReceptorMemberCommandPointer;
    ReceptorMemberCommandPointer eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
    eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometryTime);
    AddObserver(GeometryTimeEvent(NULL,0), eventReceptorCommand);
    if(connectSendEvent)
      ConnectGeometrySendEvent(receiver);
  }

  template <typename T> void ConnectGeometryEvents(T* receiver)
  {
    ConnectGeometrySliceEvent(receiver, false); //connect sendEvent only once
    ConnectGeometryTimeEvent(receiver);
  }

  //##Documentation
  //## @brief To connect multiple SliceNavigationController, we can 
  //## act as an observer ourselves: implemented interface
  //##
  //## \warning not implemented
  virtual void SetGeometry(const itk::EventObject & geometrySliceEvent);

  //##Documentation
  //## @brief To connect multiple SliceNavigationController, we can 
  //## act as an observer ourselves: implemented interface
  //##
  virtual void SetGeometrySlice(const itk::EventObject & geometrySliceEvent);

  //##Documentation
  //## @brief To connect multiple SliceNavigationController, we can 
  //## act as an observer ourselves: implemented interface
  //##
  virtual void SetGeometryTime(const itk::EventObject & geometryTimeEvent);

protected:
  //SliceNavigationController();

  //##ModelId=3E189B1D00BF
  virtual ~SliceNavigationController();

	virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent);

  mitk::Geometry3D::ConstPointer m_InputWorldGeometry;

  mitk::TimeSlicedGeometry::Pointer m_CreatedWorldGeometry;

  ViewDirection m_ViewDirection;

  itkSetMacro(Top, bool);
  itkGetMacro(Top, bool);
  itkBooleanMacro(Top);

  itkSetMacro(FrontSide, bool);
  itkGetMacro(FrontSide, bool);
  itkBooleanMacro(FrontSide);

  itkSetMacro(Rotated, bool);
  itkGetMacro(Rotated, bool);
  itkBooleanMacro(Rotated);

  bool m_Top;
  bool m_FrontSide;
  bool m_Rotated;

  bool m_BlockUpdate;
};

} // namespace mitk

#endif /* SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F */
