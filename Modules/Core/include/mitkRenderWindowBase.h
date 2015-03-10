/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKRENDERWINDOWBASE_H_HEADER_INCLUDED_C1C40D66ASDF
#define MITKRENDERWINDOWBASE_H_HEADER_INCLUDED_C1C40D66ASDF

#include <MitkCoreExports.h>

#include "mitkVtkPropRenderer.h"
#include "vtkMitkRenderProp.h"
#include "mitkSliceNavigationController.h"
#include "mitkCameraRotationController.h"

#include "mitkInteractionEvent.h"

namespace mitk
{
/**
 * \brief Base class of MITK RenderWindows
 *
 * This class sets up the MITK rendering mechanism and it's integration into VTK.
 *
 * Currently, there are two specific implementations of this abstract class:
 * QmitkRenderWindow, inerhits from the QVTKWidget and is the matured way for MITK rendering
 * mitkRenderWindow is a new, QT-independent RenderWindow implementation
 *
 * \ingroup Renderer
 */

class MITKCORE_EXPORT RenderWindowBase
{
public:

  //mitkClassMacro(RenderWindowBase,itk::Object);
  //itkFactorylessNewMacro(Self)
  //itkCloneMacro(Self)

  virtual ~RenderWindowBase();

  void InitRenderer();

  virtual mitk::SliceNavigationController * GetSliceNavigationController();
  virtual mitk::CameraRotationController * GetCameraRotationController();
  virtual mitk::BaseController * GetController();
  virtual mitk::VtkPropRenderer* GetRenderer();
  virtual vtkRenderWindow* GetVtkRenderWindow() = 0;
  virtual vtkRenderWindowInteractor* GetVtkRenderWindowInteractor() = 0;

  /** \deprecatedSince{2013_03} Use InteractionEvent instead and pass them to HandleEvent() */
  DEPRECATED(virtual void mousePressMitkEvent(mitk::MouseEvent *me));
  /** \deprecatedSince{2013_03} Use InteractionEvent instead and pass them to HandleEvent() */
  DEPRECATED(virtual void mouseReleaseMitkEvent(mitk::MouseEvent *me));
  /** \deprecatedSince{2013_03} Use InteractionEvent instead and pass them to HandleEvent() */
  DEPRECATED(virtual void mouseMoveMitkEvent(mitk::MouseEvent *me));
  /** \deprecatedSince{2013_03} Use InteractionEvent instead and pass them to HandleEvent() */
  DEPRECATED(virtual void wheelMitkEvent(mitk::WheelEvent *we));
  /** \deprecatedSince{2013_03} Use InteractionEvent instead and pass them to HandleEvent() */
  DEPRECATED(virtual void keyPressMitkEvent(mitk::KeyEvent* mke));
  /** \deprecatedSince{2013_03} Use InteractionEvent instead and pass them to HandleEvent() */
  virtual void resizeMitkEvent(int width, int height);

  virtual bool HandleEvent(InteractionEvent* interactionEvent);

protected:
  RenderWindowBase();

  // helper functions: within constructors and destructors classes are not polymorph.
  void Initialize( mitk::RenderingManager* renderingManager = NULL, const char* name = "unnamed renderer",mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard );
  void Destroy();

  mitk::VtkPropRenderer::Pointer m_Renderer;

  vtkMitkRenderProp*             m_RenderProp;

  bool                           m_InResize;

  private:
};
}

#endif /* MITKRENDERWINDOWBASE_H_HEADER_INCLUDED_C1C40D66ASDF */
