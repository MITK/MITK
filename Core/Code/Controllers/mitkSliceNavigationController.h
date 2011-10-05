/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include <MitkExports.h>
#include "mitkBaseController.h"
#include "mitkRenderingManager.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkMessage.h"
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop
#include <itkCommand.h>
#include <mitkIpPicTypeMultiplex.h>
#include <sstream>
#include "mitkRestorePlanePositionOperation.h"


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

/**
 * \brief Controls the selection of the slice the associated BaseRenderer
 * will display
 * 
 * A SliceNavigationController takes a Geometry3D as input world geometry
 * (TODO what are the exact requirements?) and generates a TimeSlicedGeometry
 * as output. The TimeSlicedGeometry holds a number of SlicedGeometry3Ds and
 * these in turn hold a series of Geometry2Ds. One of these Geometry2Ds is 
 * selected as world geometry for the BaseRenderers associated to 2D views.
 *  
 * The SliceNavigationController holds has Steppers (one for the slice, a
 * second for the time step), which control the selection of a single 
 * Geometry2D from the TimeSlicedGeometry. SliceNavigationController generates
 * ITK events to tell observers, like a BaseRenderer,  when the selected slice 
 * or timestep changes.
 * 
 * SliceNavigationControllers are registered as listeners to GlobalInteraction
 * by the QmitkStdMultiWidget. In ExecuteAction, the controllers react to 
 * PositionEvents by setting the steppers to the slice which is nearest to the 
 * point of the PositionEvent.
 *  
 * Example:
 * \code
 * // Initialization
 * sliceCtrl = mitk::SliceNavigationController::New();
 *
 * // Tell the navigator the geometry to be sliced (with geometry a
 * // Geometry3D::ConstPointer)
 * sliceCtrl->SetInputWorldGeometry(geometry.GetPointer());
 *
 * // Tell the navigator in which direction it shall slice the data
 * sliceCtrl->SetViewDirection(mitk::SliceNavigationController::Transversal);
 *
 * // Connect one or more BaseRenderer to this navigator, i.e.: events sent 
 * // by the navigator when stepping through the slices (e.g. by 
 * // sliceCtrl->GetSlice()->Next()) will be received by the BaseRenderer 
 * // (in this example only slice-changes, see also ConnectGeometryTimeEvent
 * // and ConnectGeometryEvents.)
 * sliceCtrl->ConnectGeometrySliceEvent(renderer.GetPointer());
 *
 * //create a world geometry and send the information to the connected renderer(s)
 * sliceCtrl->Update();
 * \endcode
 *
 *
 * You can connect visible navigators to a SliceNavigationController, e.g., a
 * QmitkSliderNavigator (for Qt):
 *
 * \code
 * // Create the visible navigator (a slider with a spin-box)
 * QmitkSliderNavigator* navigator = 
 *   new QmitkSliderNavigator(parent, "slidernavigator");
 *
 * // Connect the navigator to the slice-stepper of the
 * // SliceNavigationController. For initialization (position, mininal and
 * // maximal values) the values of the SliceNavigationController are used.
 * // Thus, accessing methods of a navigator is normally not necessary, since 
 * // everything can be set via the (Qt-independent) SliceNavigationController.
 * // The QmitkStepperAdapter converts the Qt-signals to Qt-independent
 * // itk-events.
 * new QmitkStepperAdapter(navigator, sliceCtrl->GetSlice(), "navigatoradaptor");
 * \endcode
 * 
 * If you do not want that all renderwindows are updated when a new slice is 
 * selected, you can use a specific RenderingManager, which updates only those 
 * renderwindows that should be updated. This is sometimes useful when a 3D view
 * does not need to be updated when the slices in some 2D views are changed.
 * QmitkSliderNavigator (for Qt):
 *
 * \code
 * // create a specific RenderingManager
 * mitk::RenderingManager::Pointer myManager = mitk::RenderingManager::New();
 *
 * // tell the RenderingManager to update only renderwindow1 and renderwindow2
 * myManager->AddRenderWindow(renderwindow1);
 * myManager->AddRenderWindow(renderwindow2);
 *
 * // tell the SliceNavigationController of renderwindow1 and renderwindow2
 * // to use the specific RenderingManager instead of the global one
 * renderwindow1->GetSliceNavigationController()->SetRenderingManager(myManager); 
 * renderwindow2->GetSliceNavigationController()->SetRenderingManager(myManager); 
 * \endcode
 * 
 * \todo implement for non-evenly-timed geometry!
 * \ingroup NavigationControl
 */
class MITK_CORE_EXPORT SliceNavigationController : public BaseController
{
  public:
    mitkClassMacro(SliceNavigationController,BaseController);
    itkNewMacro(Self);
    mitkNewMacro1Param(Self, const char *);
  
    /**
     * \brief Possible view directions, \a Original will uses 
     * the Geometry2D instances in a SlicedGeometry3D provided
     * as input world geometry (by SetInputWorldGeometry).
     */
    enum ViewDirection{Transversal, Sagittal, Frontal, Original};

    /**
     * \brief Set the input world geometry out of which the
     * geometries for slicing will be created.
     */
    void SetInputWorldGeometry(const mitk::Geometry3D* geometry);
    itkGetConstObjectMacro(InputWorldGeometry, mitk::Geometry3D);

    /**
     * \brief Access the created geometry
     */
    itkGetConstObjectMacro(CreatedWorldGeometry, mitk::Geometry3D);

    /**
     * \brief Set the desired view directions
     * 
     * \sa ViewDirection
     * \sa Update(ViewDirection viewDirection, bool top = true,
     *     bool frontside = true, bool rotated = false)
     */
    itkSetEnumMacro(ViewDirection, ViewDirection);
    itkGetEnumMacro(ViewDirection, ViewDirection);

    /**
     * \brief Set the default view direction
     *
     * This is used to re-initialize the view direction of the SNC to the
     * default value with SetViewDirectionToDefault()
     * 
     * \sa ViewDirection
     * \sa Update(ViewDirection viewDirection, bool top = true,
     *     bool frontside = true, bool rotated = false)
     */
    itkSetEnumMacro(DefaultViewDirection, ViewDirection);
    itkGetEnumMacro(DefaultViewDirection, ViewDirection);

    virtual void SetViewDirectionToDefault();

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

    /**
     * \brief Set the RenderingManager to be used
     * 
     * If \a NULL, the default RenderingManager will be used.
     */
    itkSetObjectMacro(RenderingManager, RenderingManager);
    mitk::RenderingManager* GetRenderingManager() const;

    #pragma GCC visibility push(default)
    itkEventMacro( UpdateEvent, itk::AnyEvent );
    #pragma GCC visibility pop

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

    Message<> crosshairPositionEvent;

    /**
     * \brief To connect multiple SliceNavigationController, we can 
     * act as an observer ourselves: implemented interface
     * \warning not implemented
     */
    virtual void SetGeometry(const itk::EventObject & geometrySliceEvent);

    /**
     * \brief To connect multiple SliceNavigationController, we can 
     * act as an observer ourselves: implemented interface
     */
    virtual void SetGeometrySlice(const itk::EventObject & geometrySliceEvent);

    /**
     * \brief To connect multiple SliceNavigationController, we can 
     * act as an observer ourselves: implemented interface
     */
    virtual void SetGeometryTime(const itk::EventObject & geometryTimeEvent);


    /** \brief Positions the SNC according to the specified point */
    void SelectSliceByPoint( const mitk::Point3D &point );


    /** \brief Returns the TimeSlicedGeometry created by the SNC. */
    const mitk::TimeSlicedGeometry *GetCreatedWorldGeometry();


    /** \brief Returns the Geometry3D of the currently selected time step. */
    const mitk::Geometry3D *GetCurrentGeometry3D();

    
    /** \brief Returns the currently selected Plane in the current
     * Geometry3D (if existent).
     */
    const mitk::PlaneGeometry *GetCurrentPlaneGeometry();

    /** \brief Sets the BaseRenderer associated with this SNC (if any). While
     * the BaseRenderer is not directly used by SNC, this is a convenience
     * method to enable BaseRenderer access via the SNC. */
    void SetRenderer( BaseRenderer *renderer );

    /** \brief Gets the BaseRenderer associated with this SNC (if any). While
     * the BaseRenderer is not directly used by SNC, this is a convenience
     * method to enable BaseRenderer access via the SNC. Returns NULL if no
     * BaseRenderer has been specified*/
    BaseRenderer *GetRenderer() const;

    /** \brief Re-orients the slice stack to include the plane specified by
     * the given point an normal vector. 
     */
    void ReorientSlices( 
      const mitk::Point3D &point, const mitk::Vector3D &normal );

    virtual bool ExecuteAction( 
      Action* action, mitk::StateEvent const* stateEvent);

    void ExecuteOperation(Operation* operation);

    /**
     * \brief Feature option to lock planes during mouse interaction.
     * This option flag disables the mouse event which causes the center
     * cross to move near by.
     */
    itkSetMacro(SliceLocked, bool);
    itkGetMacro(SliceLocked, bool);
    itkBooleanMacro(SliceLocked);
    
    /**
     * \brief Feature option to lock slice rotation.
     *
     * This option flag disables separately the rotation of a slice which is 
     * implemented in mitkSliceRotator.
     */
    itkSetMacro(SliceRotationLocked, bool);
    itkGetMacro(SliceRotationLocked, bool);
    itkBooleanMacro(SliceRotationLocked);

    /** 
     * \brief Adjusts the numerical range of the slice stepper according to
     * the current geometry orientation of this SNC's SlicedGeometry.
     */
    void AdjustSliceStepperRange();


  protected:
    SliceNavigationController(const char * type = NULL);
    virtual ~SliceNavigationController();

    template <class T>
    static void buildstring( mitkIpPicDescriptor *pic, itk::Point<int, 3> p, std::string &s, T = 0)
    {
      std::string value;
      std::stringstream stream;
      stream.imbue(std::locale::classic());
      stream<<s<<"; Pixelvalue: ";

      if ( (p[0]>=0 && p[1] >=0 && p[2]>=0) && (unsigned int)p[0] < pic->n[0] && (unsigned int)p[1] < pic->n[1] && (unsigned int)p[2] < pic->n[2] )
      {
        if(pic->bpe!=24)
        {
          stream<<(((T*) pic->data)[ p[0] + p[1]*pic->n[0] + p[2]*pic->n[0]*pic->n[1] ]);
        }
        else
        {
          stream<<(((T*) pic->data)[p[0]*3 + 0 + p[1]*pic->n[0]*3 + p[2]*pic->n[0]*pic->n[1]*3 ]);
          stream<<(((T*) pic->data)[p[0]*3 + 1 + p[1]*pic->n[0]*3 + p[2]*pic->n[0]*pic->n[1]*3 ]);
          stream<<(((T*) pic->data)[p[0]*3 + 2 + p[1]*pic->n[0]*3 + p[2]*pic->n[0]*pic->n[1]*3 ]);
        }
        
        s = stream.str();
      }
      else
      {
        s+= "point out of data";
      }
    };

    mitk::Geometry3D::ConstPointer m_InputWorldGeometry;
    mitk::Geometry3D::Pointer m_ExtendedInputWorldGeometry;

    mitk::TimeSlicedGeometry::Pointer m_CreatedWorldGeometry;

    ViewDirection m_ViewDirection;
    ViewDirection m_DefaultViewDirection;

    mitk::RenderingManager::Pointer m_RenderingManager;

    mitk::BaseRenderer *m_Renderer;

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

    bool m_SliceLocked;
    bool m_SliceRotationLocked;
    unsigned int m_OldPos;
};

} // namespace mitk

#endif /* SLICENAVIGATIONCONTROLLER_H_HEADER_INCLUDED_C1C55A2F */
