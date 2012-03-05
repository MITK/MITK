/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-09-22 12:01:41 +0200 (Sa, 22 Sep 2007) $
Version:   $Revision: 12241 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKVtkPropRenderer_H_HEADER_INCLUDED_C1C29F6D
#define MITKVtkPropRenderer_H_HEADER_INCLUDED_C1C29F6D

#include <MitkExports.h>
#include "mitkBaseRenderer.h"
#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"

#include <itkCommand.h>

#include <map>
#include <utility>

class vtkRenderWindow;
class vtkLight;
class vtkLightKit;
class vtkWorldPointPicker;
class vtkPointPicker;
class vtkCellPicker;
class vtkTextActor;
class vtkTextProperty;
class vtkAssemblyPath;


namespace mitk 
{

class Mapper;

/*!
\brief VtkPropRenderer

VtkPropRenderer organizes the MITK rendering process. The MITK rendering process is completely integrated into the VTK rendering pipeline.
The vtkMitkRenderProp is a custom vtkProp derived class, which implements the rendering interface between MITK and VTK. It redirects render() calls to the VtkPropRenderer, which is responsible for rendering of the datatreenodes.
VtkPropRenderer replaces the old OpenGLRenderer.

\sa rendering
\ingroup rendering
*/
class MITK_CORE_EXPORT VtkPropRenderer : public BaseRenderer
{
  // Workaround for Displaylistbug
private:
  bool didCount;
  void checkState();

  // Workaround END

public:

  mitkClassMacro(VtkPropRenderer,BaseRenderer);
  mitkNewMacro3Param(VtkPropRenderer, const char*, vtkRenderWindow *, mitk::RenderingManager* );
  
  typedef std::map<int,Mapper*> MappersMapType;

  // Render - called by vtkMitkRenderProp, returns the number of props rendered
  enum RenderType{Opaque,Translucent,Overlay,Volumetric};

  int Render(RenderType type);

  /** \brief This methods contains all method neceassary before a VTK Render() call */
  virtual void PrepareRender();

  // Active current renderwindow
  virtual void MakeCurrent();

  virtual void SetDataStorage( mitk::DataStorage* storage );  ///< set the datastorage that will be used for rendering

  virtual void InitRenderer(vtkRenderWindow* renderwindow);
  virtual void Update(mitk::DataNode* datatreenode);
  virtual void SetMapperID(const MapperSlotId mapperId);

  // Size
  virtual void InitSize(int w, int h);
  virtual void Resize(int w, int h);

  // Picking
  enum PickingMode{ WorldPointPicking, PointPicking };
  itkSetEnumMacro( PickingMode, PickingMode );
  itkGetEnumMacro( PickingMode, PickingMode );

  virtual void PickWorldPoint(const Point2D& displayPoint, Point3D& worldPoint) const;
  virtual mitk::DataNode *PickObject( const Point2D &displayPosition, Point3D &worldPosition ) const;

  // Simple text rendering method
  int WriteSimpleText(std::string text, double posX, double posY, double color1 = 0.0, double color2 = 1.0, double color3 = 0.0);
  vtkTextProperty * GetTextLabelProperty(int text_id);

  // Initialization / geometry handling

  /** This method calculates the bounds of the DataStorage (if it contains any
   * valid data), creates a geometry from these bounds and sets it as world
   * geometry of the renderer.
   *
   * Call this method to re-initialize the renderer to the current DataStorage
   * (e.g. after loading an additional dataset), to ensure that the view is
   * aligned correctly.
   */
  virtual bool SetWorldGeometryToDataStorageBounds();

  /**
   * \brief Used by vtkPointPicker/vtkPicker.
   * This will query a list of all objects in MITK and provide every vtk based mapper to the picker.
   */
  void InitPathTraversal();

  /**
   * \brief Used by vtkPointPicker/vtkPicker.
   * This will query a list of all objects in MITK and provide every vtk based mapper to the picker.
   */
  vtkAssemblyPath* GetNextPath();

  const vtkWorldPointPicker *GetWorldPointPicker() const;
  const vtkPointPicker *GetPointPicker() const;
  const vtkCellPicker *GetCellPicker() const;

  /**
  * \brief Release vtk-based graphics resources. Called by
  * vtkMitkRenderProp::ReleaseGraphicsResources.
  */
  virtual void ReleaseGraphicsResources(vtkWindow *renWin);

  MappersMapType GetMappersMap() const;

  static bool useImmediateModeRendering();

protected:
  VtkPropRenderer( const char* name = "VtkPropRenderer", vtkRenderWindow * renWin = NULL, mitk::RenderingManager* rm = NULL );
  virtual ~VtkPropRenderer();
  virtual void Update();

private:

  /** \brief This method sets up the camera on the actor (e.g. an image) of all
  * 2D vtkRenderWindows. The view is centered; zooming and panning of VTK are called inside.
  *
  * \image html ImageMapperdisplayGeometry.png
  *
  * Similar to the textured plane of an image
  * (cf. void mitkImageVtkMapper2D::GeneratePlane(mitk::BaseRenderer* renderer,
  * vtkFloatingPointType planeBounds[6])), the mitkDisplayGeometry defines a view plane (or
  * projection plane). This plane is used to set the camera parameters. The view plane
  * center (VC) is important for camera positioning (cf. the image above).
  *
  * The following figure shows the combination of the textured plane and the view plane.
  *
  * \image html cameraPositioning.png
  *
  * The view plane center (VC) is the center of the textured plane (C) and the focal point
  * (FP) at the same time. The FP defines the direction the camera faces. Since
  * the textured plane is always in the XY-plane and orthographic projection is applied, the
  * distance between camera and plane is theoretically irrelevant (because in the orthographic
  * projection the center of projection is at infinity and the size of objects depends only on
  * a scaling parameter). As a consequence, the direction of projection (DOP) is (0; 0; -1).
  * The camera up vector is always defined as (0; 1; 0).
  *
  * \warning Due to a VTK clipping bug the distance between textured plane and camera is really huge.
  * Otherwise, VTK would clip off some slices. Same applies for the clipping range size.
  *
  * \note The camera position is defined through the mitkDisplayGeometry.
  * This facilitates zooming and panning, because the display
  * geometry changes and the textured plane does not.
  *
  * \image html scaling.png
  *
  * The textured plane is scaled to fill the render window via
  * camera->SetParallelScale( imageHeightInMM / 2). In the orthographic projection all extends,
  * angles and sizes are preserved. Therefore, the image is scaled by one parameter which defines
  * the size of the rendered image. A higher value will result in smaller images. In order to render
  * just the whole image, the scale is set to half of the image height in worldcoordinates
  * (cf. the picture above).
  *
  * For zooming purposes, a factor is computed as follows:
  * factor = image height / display height (in worldcoordinates).
  * When the display geometry gets smaller (zoom in), the factor becomes bigger. When the display
  * geometry gets bigger (zoom out), the factor becomes smaller. The used VTK method
  * camera->Zoom( factor ) also works with an inverse scale.
  */
  void AdjustCameraToScene();

  // switch between orthogonal opengl projection (2D rendering via mitk::GLMapper2D) and perspective projection (3D rendering)
  void Enable2DOpenGL();
  void Disable2DOpenGL();

  // prepare all mitk::mappers for rendering
  void PrepareMapperQueue();

  /** \brief Set parallel projection, remove the interactor and the lights of VTK. */
  bool Initialize2DvtkCamera();

  bool m_InitNeeded;
  bool m_ResizeNeeded;
  bool m_VtkMapperPresent;
  bool m_NewRenderer;
  MapperSlotId m_CameraInitializedForMapperID;

  // Picking
  vtkWorldPointPicker     * m_WorldPointPicker;
  vtkPointPicker          * m_PointPicker;
  vtkCellPicker           * m_CellPicker;

  PickingMode               m_PickingMode;

  // Explicit use of SmartPointer to avoid circular #includes
  itk::SmartPointer< mitk::Mapper > m_CurrentWorldGeometry2DMapper;

  vtkLightKit* m_LightKit;

  // sorted list of mappers
  MappersMapType m_MappersMap;

  // rendering of text
  vtkRenderer * m_TextRenderer;
  typedef std::map<unsigned int,vtkTextActor*> TextMapType;
  TextMapType m_TextCollection;

  DataStorage::SetOfObjects::ConstPointer m_PickingObjects;
  DataStorage::SetOfObjects::const_iterator m_PickingObjectsIterator;
};

} // namespace mitk

#endif /* MITKVtkPropRenderer_H_HEADER_INCLUDED_C1C29F6D */
