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


#ifndef BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4
#define BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4

#include "mitkDataTree.h"
#include "mitkGeometry2D.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkDisplayGeometry.h"
#include "mitkGeometry2DData.h"
#include "mitkCameraController.h"
#include "mitkEventTypedefs.h"
#include <set>


namespace mitk {

class RenderWindow;
class CameraController;

//##ModelId=3C6E9AA90306
//##Documentation
//## @brief Organizes the rendering process
//##
//## Organizes the rendering process. A Renderer contains a reference to a
//## (sub-) data tree and asks the mappers of the data objects to render
//## the data into the renderwindow it is associated to.
//##
//## #Render() checks if rendering is currently allowed by calling
//## RenderWindow::PrepareRendering(). Initialization of a rendering context
//## can also be performed in this method.
//##
//## The actual rendering code has been moved to #Repaint()
//## Both #Repaint() and #Update() are declared protected now.
//##
//## Note: Separation of the Repaint and Update processes (rendering vs
//## creating a vtk prop tree) still needs to be worked on. The whole
//## rendering process also should be reworked to use VTK based classes for
//## both 2D and 3D rendering.
//## @ingroup Renderer
class BaseRenderer : public itk::Object
{
public:

  itkEventMacro( RendererResetEvent, itk::AnyEvent );

  typedef std::set<BaseRenderer*> RendererSet;
  /** Standard class typedefs. */
  //##ModelId=3E691E0901DB
  mitkClassMacro(BaseRenderer, itk::Object);

  //##ModelId=3E3D2F120050
  BaseRenderer( const char* name = NULL );

  //##Documentation
  //## @brief MapperSlotId defines which kind of mapper (e.g., 2D or 3D) shoud be used.
  typedef int MapperSlotId;

  enum StandardMapperSlot{Standard2D=1, Standard3D=2};

  //##ModelId=3D6A1791038B
  //##Documentation
  //## @brief @a iterator defines which part of the data tree is traversed for renderering.
  virtual void SetData(const mitk::DataTreeIteratorBase* iterator);

  //##ModelId=3E6423D20245
  //##Documentation
  //## @brief Get the DataTreeIteratorClone defining which part of the data tree is traversed for renderering.
  virtual mitk::DataTreeIteratorBase* GetData() const
  {
    return m_DataTreeIterator.GetPointer();
  };
  //##ModelId=3E6423D20264
  //##Documentation
  //## @brief Access the RenderWindow into which this renderer renders.
  mitk::RenderWindow* GetRenderWindow() const
  {
    return m_RenderWindow;
  }

  //##ModelId=3E330B9C02F9
  //##Documentation
  //## @brief Initiates the rendering. The associated RenderWindow is given
  //## the chance to initialize the rendering context, and block rendering if
  //## required (e.g. if the render window is currently invisible).
  //##
  //## Note: Subclasses should implement Repaint(), not Render().
  virtual void Render(bool drawOverlayOnly = false );

  //##ModelId=3EF1627503C4
  //##Documentation
  //## @brief Makes the widget, this renderer is drawing into, the current widget for
  //## (e.g., if the renderer is an OpenGL-based renderer as mitk::OpenGLRenderer)
  //## OpenGL operations, i.e. makes the widget's rendering context the current
  //## OpenGL rendering context.
  virtual void MakeCurrent();

  //##ModelId=3E330C4D0395
  //##Documentation
  //## @brief Default mapper id to use.
  static const MapperSlotId defaultMapper;

  //##ModelId=3E33162C00D0
  //##Documentation
  //## @brief Do the rendering and flush the result.
  virtual void Paint();

  //##ModelId=3E331632031E
  //##Documentation
  //## @brief Initialize the RenderWindow. Should only be called from RenderWindow.
  virtual void Initialize();

  //##ModelId=3E33163703D9
  //##Documentation
  //## @brief Called to inform the renderer that the RenderWindow has been resized.
  virtual void Resize(int w, int h);

  //##ModelId=3E33163A0261
  //##Documentation
  //## @brief Initialize the renderer with a RenderWindow (@a renderwindow).
  virtual void InitRenderer(mitk::RenderWindow* renderwindow);

  //##ModelId=3E3799250397
  //##Documentation
  //## @brief Set the initial size. Called by RenderWindow after it has become
  //## visible for the first time.
  virtual void InitSize(int w, int h);

  //##Documentation
  //## @brief Draws a point on the widget.
  //## Should be used during conferences to show the position of the remote mouse
  virtual void DrawOverlayMouse(Point2D& p2d);

  //##Documentation
  //## @brief Set/Get the WorldGeometry (m_WorldGeometry) for 3D and 2D rendering, that describing the
  //## (maximal) area to be rendered.
  //##
  //## Depending of the type of the passed Geometry3D more or less information can be extracted:
  //## \li if it is a Geometry2D (which is a sub-class of Geometry3D), m_CurrentWorldGeometry2D is
  //## also set to point to it. m_TimeSlicedWorldGeometry is set to NULL.
  //## \li if it is a TimeSlicedGeometry, m_TimeSlicedWorldGeometry is also set to point to it.
  //## If m_TimeSlicedWorldGeometry contains instances of SlicedGeometry3D, m_CurrentWorldGeometry2D is set to
  //## one of geometries stored in the SlicedGeometry3D according to the value of m_Slice;  otherwise
  //## a PlaneGeometry describing the top of the bounding-box of the Geometry3D is set as the
  //## m_CurrentWorldGeometry2D.
  //## \li otherwise a PlaneGeometry describing the top of the bounding-box of the Geometry3D
  //## is set as the m_CurrentWorldGeometry2D. m_TimeSlicedWorldGeometry is set to NULL.
  //## @todo add calculation of PlaneGeometry describing the top of the bounding-box of the Geometry3D
  //## when the passed Geometry3D is not sliced.
  //## \sa m_WorldGeometry
  //## \sa m_TimeSlicedWorldGeometry
  //## \sa m_CurrentWorldGeometry2D
  virtual void SetWorldGeometry(mitk::Geometry3D* geometry);
  itkGetConstObjectMacro(WorldGeometry, mitk::Geometry3D);
  //##Documentation
  //## @brief Get the current 3D-worldgeometry (m_CurrentWorldGeometry) used for 3D-rendering
  itkGetConstObjectMacro(CurrentWorldGeometry, mitk::Geometry3D);
  //##Documentation
  //## @brief Get the current 2D-worldgeometry (m_CurrentWorldGeometry2D) used for 2D-rendering
  itkGetConstObjectMacro(CurrentWorldGeometry2D, mitk::Geometry2D);

  //##Documentation
  //## @brief Set/Get the DisplayGeometry (for 2D rendering)
  //##
  //## The DisplayGeometry describes which part of the Geometry2D m_CurrentWorldGeometry2D
  //## is displayed.
  virtual void SetDisplayGeometry(mitk::DisplayGeometry* geometry2d);
  itkGetConstObjectMacro(DisplayGeometry, mitk::DisplayGeometry);
  itkGetObjectMacro(DisplayGeometry, mitk::DisplayGeometry);

  //##Documentation
  //## @brief Set/Get m_Slice which defines together with m_TimeStep the 2D geometry
  //## stored in m_TimeSlicedWorldGeometry used as m_CurrentWorldGeometry2D
  //##
  //## \sa m_Slice
  virtual void SetSlice(unsigned int slice);
  itkGetConstMacro(Slice, unsigned int);
  //##Documentation
  //## @brief Set/Get m_TimeStep which defines together with m_Slice the 2D geometry
  //## stored in m_TimeSlicedWorldGeometry used as m_CurrentWorldGeometry2D
  //##
  //## \sa m_TimeStep
  virtual void SetTimeStep(unsigned int timeStep);
  itkGetConstMacro(TimeStep, unsigned int);

  //##Documentation
  //## @brief Get the time-step of a BaseData object which
  //## exists at the time of the currently displayed content
  //##
  //## Returns -1 or mitk::BaseData::m_TimeSteps if there
  //## is no data at the current time.
  //## \sa GetTimeStep, m_TimeStep
  int GetTimeStep(mitk::BaseData* data) const;

  //##Documentation
  //## @brief Get the time in ms of the currently displayed content
  //##
  //## \sa GetTimeStep, m_TimeStep
  mitk::ScalarType GetTime() const;

  //##Documentation
  //## @brief SetWorldGeometry is called according to the geometrySliceEvent,
  //## which is supposed to be a SliceNavigationController::GeometrySendEvent
  virtual void SetGeometry(const itk::EventObject & geometrySliceEvent);
  
  //##Documentation
  //## @brief UpdateWorldGeometry is called to re-read the 2D geometry from the
  //## slice navigation controller
  virtual void UpdateGeometry(const itk::EventObject & geometrySliceEvent);

  //##Documentation
  //## @brief SetSlice is called according to the geometrySliceEvent,
  //## which is supposed to be a SliceNavigationController::GeometrySliceEvent
  virtual void SetGeometrySlice(const itk::EventObject & geometrySliceEvent);

  //##Documentation
  //## @brief SetTimeStep is called according to the geometrySliceEvent,
  //## which is supposed to be a SliceNavigationController::GeometryTimeEvent
  virtual void SetGeometryTime(const itk::EventObject & geometryTimeEvent);

  //##Documentation
  //## @brief Get a data object containing the DisplayGeometry (for 2D rendering)
  itkGetObjectMacro(DisplayGeometryData, mitk::Geometry2DData);
  //##Documentation
  //## @brief Get a data object containing the WorldGeometry (for 2D rendering)
  itkGetObjectMacro(WorldGeometryData, mitk::Geometry2DData);

  //##Documentation
  //## @brief Get a data tree node pointing to a data object containing the WorldGeometry (3D and 2D rendering)
  itkGetObjectMacro(WorldGeometryNode, mitk::DataTreeNode);
  //##Documentation
  //## @brief Get a data tree node pointing to a data object containing the DisplayGeometry (for 2D rendering)
  itkGetObjectMacro(DisplayGeometryNode, mitk::DataTreeNode);
  //##Documentation
  //## @brief Get a data tree node pointing to a data object containing the current 2D-worldgeometry m_CurrentWorldGeometry2D (for 2D rendering)
  itkGetObjectMacro(CurrentWorldGeometry2DNode, mitk::DataTreeNode);

  //##Documentation
  //## @brief Get timestamp of last call of SetCurrentWorldGeometry2D
  unsigned long GetCurrentWorldGeometry2DUpdateTime() { return m_CurrentWorldGeometry2DUpdateTime; };
  //##Documentation
  //## @brief Get timestamp of last call of SetDisplayGeometry
  unsigned long GetDisplayGeometryUpdateTime() { return m_CurrentWorldGeometry2DUpdateTime; };
  //##Documentation
  //## @brief Get timestamp of last change of current TimeStep
  unsigned long GetTimeStepUpdateTime() { return m_TimeStepUpdateTime; };

  //##Documentation
  //## @brief Perform a picking: find the x,y,z world coordinate of a
  //## display x,y coordinate.
  //## @warning Has to be overwritten in subclasses for the 3D-case.
  //##
  //## Implemented here only for 2D-rendering by using
  //## m_DisplayGeometry
  virtual void PickWorldPoint(const Point2D& diplayPosition, Point3D& worldPosition) const;

  //##Documentation
  //## @brief Get the MapperSlotId to use.
  itkGetMacro(MapperID, MapperSlotId);
  //##Documentation
  //## @brief Set the MapperSlotId to use.
  itkSetMacro(MapperID, MapperSlotId);

  //##Documentation
  //## @brief Has the renderer the focus?
  itkGetMacro(Focused, bool);
  //##Documentation
  //## @brief Tell the renderer that it is focused. The caller is responsible for focus management,
  //## not the renderer itself.
  itkSetMacro(Focused, bool);

  itkGetMacro(Size, int*);

  itkGetObjectMacro(CameraController, mitk::CameraController);

  //##ModelId=3E6D5DD30322
  //##Documentation
  //## @brief Mouse event dispatchers
  //## @note for internal use only. preliminary.
  virtual void MousePressEvent(mitk::MouseEvent*);
  //##ModelId=3E6D5DD30372
  //##Documentation
  //## @brief Mouse event dispatchers
  //## @note for internal use only. preliminary.
  virtual void MouseReleaseEvent(mitk::MouseEvent*);
  //##ModelId=3E6D5DD303C2
  //##Documentation
  //## @brief Mouse event dispatchers
  //## @note for internal use only. preliminary.
  virtual void MouseMoveEvent(mitk::MouseEvent*);
  //##Documentation
  //## @brief Wheel event dispatcher
  //## @note for internal use only. preliminary.
  virtual void WheelEvent(mitk::WheelEvent*);
  //##ModelId=3E6D5DD4002A
  //##Documentation
  //## @brief Key event dispatcher
  //## @note for internal use only. preliminary.
  virtual void KeyPressEvent(mitk::KeyEvent*);

  //##Documentation
  //## @brief get the name of the Renderer
  //## @note
  const char * GetName() const
  {
    return m_Name.c_str();
  }

  //##Documentation
  //## @brief get the Renderer by the name
  //## @note
  static const BaseRenderer* GetByName( const std::string& name )
  {
    for (RendererSet::const_iterator iter = instances.begin();iter != instances.end();iter++) {
      if (name == (*iter)->m_Name) {
        return *iter;
      }
    }
    return NULL;
  }

  //##Documentation
  //## @brief get the x_size of the RendererWindow
  //## @note
  const int GetSizeX() const
  {
    return m_Size[0];
  }

  //##Documentation
  //## @brief get the y_size of the RendererWindow
  //## @note
  const int GetSizeY() const
  {
    return m_Size[1];
  }

protected:

  //##ModelId=3E3D2F12008C
  virtual ~BaseRenderer();

  //##ModelId=3E330B9C02F9
  //##Documentation
  //## @brief Do the rendering. To be implemented in subclasses.
  virtual void Repaint(bool overlay=false) = 0;

  //##ModelId=3E330B930328
  //##Documentation
  //## @brief Call update of all mappers. To be implemented in subclasses.
  virtual void Update() = 0;

  //##ModelId=3E3D381A027D
  //##Documentation
  //## @brief MapperSlotId to use. Defines which kind of mapper (e.g., 2D or 3D) shoud be used.
  MapperSlotId m_MapperID;

  //##ModelId=3E330D6902E8
  //##Documentation
  //## @brief The DataTreeIteratorClone defining which part of the data tree is traversed for renderering.
  mitk::DataTreeIteratorClone m_DataTreeIterator;

  //##ModelId=3E3D1FCA0272
  //##Documentation
  //## @brief The RenderWindow to render into.
  RenderWindow* m_RenderWindow;

  //##ModelId=3E6423D20213
  //##Documentation
  //## @brief Timestamp of last call of Update().
  unsigned long m_LastUpdateTime;

  //##ModelId=3E6D5DD30232
  //##Documentation
  //## @brief CameraController for 3D rendering
  //## @note preliminary.
  mitk::CameraController::Pointer m_CameraController;

  //##ModelId=3E6D5DD302E6
  //##Documentation
  //## @brief Size of the RenderWindow.
  int m_Size[2];

  //##ModelId=3ED91D0400D3
  //##Documentation
  //## @brief Contains whether the renderer that it is focused. The caller of
  //## SetFocused is responsible for focus management, not the renderer itself.
  //## is doubled because of mitk::FocusManager in GlobalInteraction!!! (ingmar)
  bool m_Focused;

  //##Documentation
  //## @brief Sets m_CurrentWorldGeometry2D
  virtual void SetCurrentWorldGeometry2D(mitk::Geometry2D* geometry2d);
private:
  //##Documentation
  //## Pointer to the worldgeometry, describing the maximal area to be rendered
  //## (3D as well as 2D).
  //## It is const, since we are not allowed to change it (it may be taken
  //## directly from the geometry of an image-slice and thus it would be
  //## very strange when suddenly the image-slice changes its geometry).
  //## \sa SetWorldGeometry
  Geometry3D::Pointer m_WorldGeometry;

  //##Documentation
  //## m_TimeSlicedWorldGeometry is set by SetWorldGeometry if the passed Geometry3D is a
  //## TimeSlicedGeometry (or a sub-class of it). If it contains instances of SlicedGeometry3D,
  //## m_Slice and m_TimeStep (set via SetSlice and SetTimeStep, respectively) define
  //## which 2D geometry stored in m_TimeSlicedWorldGeometry (if available)
  //## is used as m_CurrentWorldGeometry2D.
  //## \sa m_CurrentWorldGeometry2D
  TimeSlicedGeometry::Pointer m_TimeSlicedWorldGeometry;

  //##Documentation
  //## Pointer to the current 3D-worldgeometry.
  Geometry3D::Pointer m_CurrentWorldGeometry;

  //##ModelId=3EDD039F00A9
  //##Documentation
  //## Pointer to the current 2D-worldgeometry. The 2D-worldgeometry
  //## describes the maximal area (2D manifold) to be rendered in case we
  //## are doing 2D-rendering. More precisely, a subpart of this according
  //## to m_DisplayGeometry is displayed.
  //## It is const, since we are not allowed to change it (it may be taken
  //## directly from the geometry of an image-slice and thus it would be
  //## very strange when suddenly the image-slice changes its geometry).
  Geometry2D::Pointer m_CurrentWorldGeometry2D;

  //##ModelId=3EDD039F002C
  //##Documentation
  //## Pointer to the displaygeometry. The displaygeometry describes the
  //## geometry of the \em visible area in the window controlled by the renderer
  //## in case we are doing 2D-rendering.
  //## It is const, since we are not allowed to change it.
  DisplayGeometry::Pointer m_DisplayGeometry;

  //##Documentation
  //## Defines together with m_Slice which 2D geometry stored in m_TimeSlicedWorldGeometry
  //## is used as m_CurrentWorldGeometry2D: m_TimeSlicedWorldGeometry->GetGeometry2D(m_Slice, m_TimeStep).
  //## \sa m_TimeSlicedWorldGeometry
  unsigned int m_Slice;
  //##Documentation
  //## Defines together with m_TimeStep which 2D geometry stored in m_TimeSlicedWorldGeometry
  //## is used as m_CurrentWorldGeometry2D: m_TimeSlicedWorldGeometry->GetGeometry2D(m_Slice, m_TimeStep).
  //## \sa m_TimeSlicedWorldGeometry
  unsigned int m_TimeStep;

  //##Documentation
  //## @brief timestamp of last call of SetWorldGeometry
  itk::TimeStamp m_CurrentWorldGeometry2DUpdateTime;
  
  //##Documentation
  //## @brief timestamp of last call of SetDisplayGeometry
  itk::TimeStamp m_DisplayGeometryUpdateTime;

  //##Documentation
  //## @brief timestamp of last change of the current time step
  itk::TimeStamp m_TimeStepUpdateTime;

protected:
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  //##Documentation
  //## Data object containing the m_WorldGeometry defined above.
  Geometry2DData::Pointer m_WorldGeometryData;
  //##ModelId=3E66BDF000F4
  //##Documentation
  //## Data object containing the m_DisplayGeometry defined above.
  Geometry2DData::Pointer m_DisplayGeometryData;
  //##ModelId=3E66CC5901C1
  //##Documentation
  //## Data object containing the m_CurrentWorldGeometry2D defined above.
  Geometry2DData::Pointer m_CurrentWorldGeometry2DData;

  //##Documentation
  //## DataTreeNode objects containing the m_WorldGeometryData defined above.
  DataTreeNode::Pointer m_WorldGeometryNode;
  //##ModelId=3ED91D04020B
  //##Documentation
  //## DataTreeNode objects containing the m_DisplayGeometryData defined above.
  DataTreeNode::Pointer m_DisplayGeometryNode;
  //##ModelId=3ED91D040288
  //##Documentation
  //## DataTreeNode objects containing the m_CurrentWorldGeometry2DData defined above.
  DataTreeNode::Pointer m_CurrentWorldGeometry2DNode;

  //##ModelId=3ED91D040305
  //##Documentation
  //## @brief test only
  unsigned long m_DisplayGeometryTransformTime;
  //##ModelId=3ED91D040382
  //##Documentation
  //## @brief test only
  unsigned long m_CurrentWorldGeometry2DTransformTime;

  std::string m_Name;
  static RendererSet instances;
};

} // namespace mitk

#endif /* BASERENDERER_H_HEADER_INCLUDED_C1CCA0F4 */
