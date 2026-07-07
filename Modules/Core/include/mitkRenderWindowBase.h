/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderWindowBase_h
#define mitkRenderWindowBase_h

#include <MitkCoreExports.h>

#include <mitkCameraRotationController.h>
#include <mitkSliceNavigationController.h>
#include <mitkVtkPropRenderer.h>
#include <vtkMitkRenderProp.h>

#include <mitkInteractionEvent.h>

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
    // mitkClassMacroItkParent(RenderWindowBase,itk::Object);
    // itkFactorylessNewMacro(Self)
    // itkCloneMacro(Self)

    /** \brief Virtual destructor. */
    virtual ~RenderWindowBase();

    /** \brief Initializes the renderer associated with this render window. */
    void InitRenderer();

    /** \brief Returns the SliceNavigationController for navigating through slices.
     *
     * \return The SliceNavigationController associated with this render window.
     */
    virtual mitk::SliceNavigationController *GetSliceNavigationController();

    /** \brief Returns the CameraRotationController for 3D camera rotation.
     *
     * \return The CameraRotationController associated with this render window.
     */
    virtual mitk::CameraRotationController *GetCameraRotationController();

    /** \brief Returns the appropriate controller for the current mapper mode.
     *
     * Returns the SliceNavigationController for 2D rendering or
     * the CameraRotationController for 3D rendering.
     *
     * \return The active BaseController for this render window.
     */
    virtual mitk::BaseController *GetController();

    /** \brief Returns the VtkPropRenderer associated with this render window.
     *
     * \return The VtkPropRenderer instance.
     */
    virtual mitk::VtkPropRenderer *GetRenderer();

    /** \brief Returns the underlying vtkRenderWindow. Must be implemented by subclasses.
     *
     * \return The VTK render window instance.
     */
    virtual vtkRenderWindow *GetVtkRenderWindow() = 0;

    /** \brief Returns the underlying vtkRenderWindowInteractor. Must be implemented by subclasses.
     *
     * \return The VTK render window interactor instance.
     */
    virtual vtkRenderWindowInteractor *GetVtkRenderWindowInteractor() = 0;

    /** \brief Dispatches an interaction event to the renderer's event dispatcher.
     *
     * \param interactionEvent the interaction event to handle.
     * \return True if the event was processed successfully.
     */
    virtual bool HandleEvent(InteractionEvent *interactionEvent);

  protected:
    /** \brief Constructor. */
    RenderWindowBase();

    /** \brief Initializes the renderer and adds it to the RenderingManager.
     *
     * This helper must be called from subclass constructors because virtual functions
     * are not polymorphic during construction.
     *
     * \param name a descriptive name for the renderer.
     */
    void Initialize(const char *name = "unnamed renderer");

    /** \brief Tears down the renderer and removes it from the RenderingManager.
     *
     * This helper must be called from subclass destructors because virtual functions
     * are not polymorphic during destruction.
     */
    void Destroy();

    mitk::VtkPropRenderer::Pointer m_Renderer;

    vtkMitkRenderProp *m_RenderProp;

    bool m_InResize;

  private:
  };
}

#endif
