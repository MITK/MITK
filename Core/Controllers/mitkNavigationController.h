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


#ifndef NAVIGATIONCONTROLLER_H_HEADER_INCLUDED
#define NAVIGATIONCONTROLLER_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkBaseController.h"
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

/**
 * \brief Class providing the basic functionality for both SliceNavigationController and
 * the PointNavigationController
 *
 * Only implements methods that are used by both SliceNavigationController and
 * the PointNavigationController, other methods have to be implemented separately
 *
 */
class MITK_CORE_EXPORT NavigationController : public BaseController
{
  public:
    mitkClassMacro(NavigationController,BaseController);
  
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
    itkSetMacro(ViewDirection, ViewDirection);
    itkGetMacro(ViewDirection, ViewDirection);

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
    itkSetMacro(DefaultViewDirection, ViewDirection);
    itkGetMacro(DefaultViewDirection, ViewDirection);

    virtual void SetViewDirectionToDefault();

    /**
     * \brief Do the actual creation and send it to the connected 
     * observers (renderers). Has to be reimplemented!!!
     * 
     */
    virtual void Update()=0;

    /**
     * \brief Extended version of Update, additionally allowing to
     * specify the direction/orientation of the created geometry.
     * Has to be reimplemented!!!
     * 
     */
    virtual void Update(ViewDirection viewDirection, bool top = true,
      bool frontside = true, bool rotated = false)=0;


    /**
     * \brief Set the RenderingManager to be used
     * 
     * If \a NULL, the default RenderingManager will be used.
     */
    itkSetObjectMacro(RenderingManager, RenderingManager);
    mitk::RenderingManager* GetRenderingManager() const;

    itkEventMacro( UpdateEvent, itk::AnyEvent );
  

    /**
     * \brief To connect multiple NavigationController, we can 
     * act as an observer ourselves: implemented interface
     * \warning not implemented
     */
    virtual void SetGeometry(const itk::EventObject & geometrySliceEvent);


    /** \brief Returns the TimeSlicedGeometry created by the NC. */
    const mitk::TimeSlicedGeometry *GetCreatedWorldGeometry();


    /** \brief Returns the Geometry3D of the currently selected time step. */
    const mitk::Geometry3D *GetCurrentGeometry3D();

    
    /** \brief Returns the currently selected Plane in the current
     * Geometry3D (if existent).
     */
    const mitk::PlaneGeometry *GetCurrentPlaneGeometry();

    /** \brief Sets the BaseRenderer associated with this NC (if any). While
     * the BaseRenderer is not directly used by NC, this is a convenience
     * method to enable BaseRenderer access via the NC. */
    void SetRenderer( BaseRenderer *renderer );

    /** \brief Gets the BaseRenderer associated with this NC (if any). While
     * the BaseRenderer is not directly used by NC, this is a convenience
     * method to enable BaseRenderer access via the NC. Returns NULL if no
     * BaseRenderer has been specified*/
    BaseRenderer *GetRenderer() const;

    virtual bool ExecuteAction( 
      Action* action, mitk::StateEvent const* stateEvent);

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
     * the current geometry orientation of this NC's SlicedGeometry.
     */
    void AdjustSliceStepperRange();

    
    virtual void setSphere(){};


  protected:
    NavigationController(const char * type = NULL);
    virtual ~NavigationController();

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

#endif /* NAVIGATIONCONTROLLER_H_HEADER_INCLUDED */
