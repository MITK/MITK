/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkCommon.h"
#include "mitkBaseRenderer.h"
#include "mitkDataTree.h"
#include "mitkDataStorage.h"

#include <itkCommand.h>

#include <map>
#include <utility>

class vtkRenderWindow;
class vtkLight;
class vtkLightKit;
class vtkWorldPointPicker;
class vtkPointPicker;
class vtkTextActor;
class vtkTextProperty;
class vtkAssemblyPath;

namespace mitk {

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
public:

  mitkClassMacro(VtkPropRenderer,BaseRenderer);
  mitkNewMacro2Param(VtkPropRenderer, const char*, vtkRenderWindow *);
  
  typedef std::map<int,Mapper*> MappersMapType;

  // Render - called by vtkMitkRenderProp, returns the number of props rendered
  #if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
    enum RenderType{Opaque,Translucent,Overlay,Volumetric};
  #else
    enum RenderType{Opaque,Translucent,Overlay};
  #endif

  int Render(RenderType type);

  // Active current renderwindow
  virtual void MakeCurrent();

  // Data handling
  using BaseRenderer::SetData;
  virtual void SetData(const mitk::DataTreeIteratorBase* iterator);
  virtual void InitRenderer(vtkRenderWindow* renderwindow);
  virtual void Update(mitk::DataTreeNode* datatreenode);
  virtual void SetMapperID(const MapperSlotId mapperId);

  // Size
  virtual void InitSize(int w, int h);
  virtual void Resize(int w, int h);

  // Picking
  enum PickingMode{ WorldPointPicking, PointPicking };
  itkSetMacro( PickingMode, PickingMode );
  itkGetMacro( PickingMode, PickingMode );
  virtual void PickWorldPoint(const Point2D& displayPoint, Point3D& worldPoint) const;

  // Simple text rendering method
  int WriteSimpleText(std::string text, double posX, double posY, double color1 = 0.0, double color2 = 1.0, double color3 = 0.0);
	vtkTextProperty * GetTextLabelProperty(int text_id);

  // Initialization / geometry handling

  /** This method calculates the bounds of the DataTree (if it contains any
   * valid data), creates a geometry from these bounds and sets it as world
   * geometry of the renderer.
   *
   * Call this method to re-initialize the renderer to the current DataTree
   * (e.g. after loading an additional dataset), to ensure that the view is
   * aligned correctly.
   */
  virtual bool SetWorldGeometryToDataTreeBounds();

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

  vtkWorldPointPicker* GetWorldPointPicker();
  vtkPointPicker* GetPointPicker();

  /**
  * \brief Release vtk-based graphics resources. Called by
  * vtkMitkRenderProp::ReleaseGraphicsResources.
  */
  virtual void ReleaseGraphicsResources(vtkWindow *renWin);

  #if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
  MappersMapType GetMappersMap() const;
  #endif
protected:
  VtkPropRenderer( const char* name = "VtkPropRenderer", vtkRenderWindow * renWin = NULL);
  virtual ~VtkPropRenderer();
  virtual void Update();

private:

  // switch between orthogonal opengl projection (2D rendering via mitk::GLMapper2D) and perspective projection (3D rendering)
  void Enable2DOpenGL();
  void Disable2DOpenGL();

  // prepare all mitk::mappers for rendering
  void PrepareMapperQueue();

  bool m_InitNeeded;
  bool m_ResizeNeeded;
  bool m_VtkMapperPresent;
  bool m_NewRenderer;

  // Picking
  vtkWorldPointPicker     * m_WorldPointPicker;
  vtkPointPicker          * m_PointPicker;
  PickingMode               m_PickingMode;

  mitk::Mapper::Pointer m_CurrentWorldGeometry2DMapper;

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
