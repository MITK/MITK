#ifndef SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F
#define SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F

#include "mitkCommon.h"
#include "mitkBaseController.h"
#include "mitkImageToImageFilter.h"
#include "mitkTimeSlicedGeometry.h"
#include <itkEventObject.h>

namespace mitk {

#define mitkTimeSlicedGeometryEventMacro( classname , super ) \
 class classname : public super { \
   public: \
     typedef classname Self; \
     typedef super Superclass; \
     classname(const TimeSlicedGeometry* aTimeSlicedGeometry, unsigned int aPos) : Superclass(aTimeSlicedGeometry, aPos) {} \
     virtual ~classname() {} \
     virtual const char * GetEventName() const { return #classname; } \
     virtual bool CheckEvent(const ::itk::EventObject* e) const \
       { return dynamic_cast<const Self*>(e); } \
     virtual ::itk::EventObject* MakeObject() const \
       { return new Self(GetTimeSlicedGeometry(), GetPos()); } \
   private: \
     classname(const Self&); \
     void operator=(const Self&); \
 }

//##ModelId=3DD523E00048
//##Documentation
//## @brief controls the selection of the slice the associated BaseRenderer
//## will display
//## @ingroup NavigationControl
//## Subclass of BaseController. Controls the selection of the slice the
//## associated BaseRenderer will display.
//## @todo implement for non-evenly-timed geometry!
class SliceNavigationController : public BaseController
{
public:
  mitkClassMacro(SliceNavigationController,BaseController);
  itkNewMacro(Self);

  enum ViewDirection{Transversal, Sagittal, Frontal, Original};

  itkSetObjectMacro(WorldGeometry, mitk::Geometry3D);
  itkGetConstObjectMacro(WorldGeometry, mitk::Geometry3D);

  itkSetMacro(ViewDirection, ViewDirection);
  itkGetMacro(ViewDirection, ViewDirection);

  virtual bool AddRenderer(mitk::BaseRenderer* renderer);

  virtual void Update();

  class TimeSlicedGeometryEvent : public itk::AnyEvent 
  { 
  public: 
    typedef TimeSlicedGeometryEvent Self; 
    typedef itk::AnyEvent Superclass; 
    TimeSlicedGeometryEvent(const TimeSlicedGeometry* aTimeSlicedGeometry, unsigned int aPos) : 
      m_TimeSlicedGeometry(aTimeSlicedGeometry), m_Pos(aPos) {} 
    virtual ~TimeSlicedGeometryEvent() {} 
    virtual const char * GetEventName() const { return "TimeSlicedGeometryEvent"; } 
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { return dynamic_cast<const Self*>(e); } 
    virtual ::itk::EventObject* MakeObject() const 
    { return new Self(m_TimeSlicedGeometry, m_Pos); } 
    const TimeSlicedGeometry* GetTimeSlicedGeometry() const { return m_TimeSlicedGeometry; }
    unsigned int GetPos() const { return m_Pos; }
  private: 
    TimeSlicedGeometry::ConstPointer m_TimeSlicedGeometry;
    unsigned int m_Pos;
    TimeSlicedGeometryEvent(const Self&); 
    void operator=(const Self&); 
  };
  mitkTimeSlicedGeometryEventMacro( GeometryTimeEvent    , TimeSlicedGeometryEvent );
  mitkTimeSlicedGeometryEventMacro( GeometrySliceEvent   , TimeSlicedGeometryEvent );
  
  template <typename T> void ConnectGeometrySliceEvent(T* receiver)
  {
    itk::ReceptorMemberCommand<T>::Pointer eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
  #ifdef WIN32
    eventReceptorCommand->SetCallbackFunction(receiver, T::SetGeometryTime);
  #else
    eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometryTime);
  #endif
    AddObserver(GeometrySliceEvent(NULL,0), eventReceptorCommand);
  }

  template <typename T> void ConnectGeometryTimeEvent(T* receiver)
  {
    itk::ReceptorMemberCommand<T>::Pointer eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
  #ifdef WIN32
    eventReceptorCommand->SetCallbackFunction(receiver, T::SetGeometryTime);
  #else
    eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometryTime);
  #endif
    AddObserver(GeometryTimeEvent(NULL,0), eventReceptorCommand);
  }

  template <typename T> void ConnectGeometryEvents(T* receiver)
  {
    ConnectGeometrySliceEvent(receiver);
    ConnectGeometryTimeEvent(receiver);
  }

  virtual void SetGeometryTime(const itk::EventObject & geometryTimeEvent);

  virtual void SetGeometrySlice(const itk::EventObject & geometryTimeEvent);

protected:
  //##ModelId=3E189B1D008D
  SliceNavigationController();

  //##ModelId=3E189B1D00BF
  virtual ~SliceNavigationController();

  virtual void SliceStepperChanged();

  virtual void TimeStepperChanged();

  mitk::Geometry3D::ConstPointer m_WorldGeometry;

  mitk::TimeSlicedGeometry::Pointer m_TimeSlicedWorldGeometry;

  ViewDirection m_ViewDirection;

  bool m_BlockUpdate;
};

} // namespace mitk

#endif /* SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F */
