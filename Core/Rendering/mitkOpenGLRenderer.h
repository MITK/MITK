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


#ifndef MITKOPENGLRENDERER_H_HEADER_INCLUDED_C1C29F6D
#define MITKOPENGLRENDERER_H_HEADER_INCLUDED_C1C29F6D

#include "mitkCameraController.h"
#include "mitkCommon.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderWindow.h"
#include "mitkDataTree.h"
#include "mitkImageSliceSelector.h"
#include "mitkRenderingManager.h"
#include "mitkSimpleTextRendering.h"
#include <mitkGL.h>

#include <itkCommand.h>

class vtkRenderer;
class vtkRenderWindow;
class VtkRenderWindow;
class vtkLight;
class vtkLightKit;
class vtkWorldPointPicker;
class vtkPointPicker;

namespace mitk {
//##ModelId=3C6E9AD90215
//##Documentation
//## @brief OpenGL-based implementation of BaseRenderer
//## @ingroup Renderer
class OpenGLRenderer : public BaseRenderer
{
public:
  /** Standard class typedefs. */
  //##ModelId=3E691CC70250
  mitkClassMacro(OpenGLRenderer,BaseRenderer);

  //##ModelId=3E33ECF301AD
  OpenGLRenderer( const char* name = "OpenGLRenderer" );

  //##ModelId=3E3D28AB0018
  void SetData(const mitk::DataTreeIteratorBase* iterator);

  /*!
  \brief set the controller of this renderer's camera. By default it is VtkInteractorCameraController
  \param cameraController instance of the active camera controller
  */
  void SetCameraController(CameraController* cameraController);

  int WriteSimpleText(int posX, int posY, std::string text);

  virtual void Update(mitk::DataTreeNode* datatreenode);

  //##ModelId=3EF59AD20235
  virtual void SetMapperID(const MapperSlotId mapperId);

  //##ModelId=3EF162760271
  virtual void MakeCurrent();

  //##ModelId=3E3799420227
  virtual void InitSize(int w, int h);

  //##ModelId=3ED91D060305
  //##Documentation
  //## @brief Update the vtk-based-mappers by adding the produced actors to the m_VtkRenderer.
  //##
  //## @warning Always call Update() before, which checks, whether there are vtk-based-mappers.
  virtual void UpdateIncludingVtkActors();

  inline VtkRenderWindow* GetVtkRenderWindow() const
  {
    if(m_RenderWindow!=NULL)
      return m_RenderWindow->GetVtkRenderWindow();
    else
      return NULL;
  }

  //##Documentation
  //## @brief returns the vtk-renderer associated with the OpenGL renderer.
  inline vtkRenderer * GetVtkRenderer() const
  {
    return m_VtkRenderer;
  };

  //##Documentation
  //## @brief returns the vtkLightKit which is used 
  //## by default
  inline vtkLightKit * GetLightKit() const
  {
    return m_LightKit;
  }
  
  //##Documentation
  //## @brief defines if a vtkWordPointPicker or vtkPointPicker is used for picking in 3D
  //## by default WorldPointPicking is used
  enum PickingMode{ WorldPointPicking, PointPicking };
  
  //##Documentation
  //## Set/Get wether a vtkWorldPointPicker or vtkPointPicker is used for
  //## picking a point in the 3D window.
  itkSetMacro( PickingMode, PickingMode );
  itkGetMacro( PickingMode, PickingMode );
  
  virtual void PickWorldPoint(const Point2D& displayPoint, Point3D& worldPoint) const;

  //##Documentation
  //## @brief Save a point to
  //## Should be used during conferences to hand the postion of a mouse curser
  virtual void DrawOverlayMouse(Point2D& p2d);
  
  //##Documentation
  //## @brief Draws a mouse primitive to a specified point on the widget.
  //## will be requested during Repaint()
  virtual void DrawOverlay();

protected:
  //##ModelId=3E330D260255
  virtual void Update();

  //##ModelId=3E330D2903CC
  //##Documentation
  //## @brief Do the rendering.
  //## If necessary, Update() and UpdateVtkActors() is called first.
  //## The order of rendering is: First update the VTK Actors and render them,
  //## then prepare 2D-rendering informations with layers and render them.
  //## Due to that order, 2D-Layers or e.g. a video can be manually put in the background
  virtual void Repaint(bool overlay=false );

  //##ModelId=3E33145B0096
  virtual void Initialize();

  //##ModelId=3E33145B00D2
  virtual void Resize(int w, int h);

  //##ModelId=3E33145B0226
  virtual void InitRenderer(mitk::RenderWindow* renderwindow);

  //##ModelId=3E33ECF301B7
  virtual ~OpenGLRenderer();

  virtual void DataChangedEvent(const itk::Object *caller, const itk::EventObject &event);

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  itk::MemberCommand<mitk::OpenGLRenderer>::Pointer m_DataChangedCommand;

private:
  //##ModelId=3E33145A0315
  bool m_InitNeeded;

  //##ModelId=3E33145A0347
  bool m_ResizeNeeded;

  //##ModelId=3EF1627601DB
  bool m_VtkMapperPresent;

  //##ModelId=3E33145A0383
  vtkRenderer* m_VtkRenderer;

  // will disable rendering - instead using only a copy
  bool m_RequestOverlay;

  bool m_NewRenderer;

  // keep the position for an overlay mouse curser
  mitk::Point2D m_DrawOverlayPosition;

  //##ModelId=3E33145B001E
  vtkLight* m_Light;

  //##ModelId=3ED91D0602E6
  vtkLightKit* m_LightKit;

  //##Documentation
  //## @brief Timestamp of last call of UpdateVtkActors().
  unsigned long m_LastUpdateVtkActorsTime;

  vtkWorldPointPicker *m_WorldPointPicker;
  vtkPointPicker *m_PointPicker;

  mitk::Mapper::Pointer m_CurrentWorldGeometry2DMapper;

  mitk::SimpleTextRendering::Pointer m_SimpleTextRenderer;

  // save a copy of the rendered window.
  GLbyte *m_PixelMapGL;

  //##Documentation
  //## @brief Keep track whether m_PixelMapGL contains valid data
  bool m_PixelMapGLValid;
  
  // describes wether world point picking or point picking is used.
  PickingMode m_PickingMode;
};

} // namespace mitk

#endif /* MITKOPENGLRENDERER_H_HEADER_INCLUDED_C1C29F6D */
