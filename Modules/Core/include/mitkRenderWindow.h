/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderWindow_h
#define mitkRenderWindow_h

#include <MitkCoreExports.h>

#include "mitkRenderWindowBase.h"

namespace mitk
{
  class vtkEventProvider;

  /**
   * \brief mitkRenderWindow integrates the MITK rendering mechanism into VTK and
   * is NOT QT dependent
   *
   *
   * \ingroup Renderer
   */
  class MITKCORE_EXPORT RenderWindow : public mitk::RenderWindowBase, public itk::Object
  {
  public:
    mitkClassMacroItkParent(RenderWindow, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    mitkNewMacro1Param(Self, vtkRenderWindow*);
    mitkNewMacro2Param(Self, vtkRenderWindow *, const char *);

    ~RenderWindow() override;

    vtkRenderWindow *GetVtkRenderWindow() override;
    vtkRenderWindowInteractor *GetVtkRenderWindowInteractor() override;

    // Set Layout Index to define the Layout Type
    void SetLayoutIndex(unsigned int layoutIndex);

    // Get Layout Index to define the Layout Type
    unsigned int GetLayoutIndex();

    // MenuWidget need to update the Layout Design List when Layout had changed
    void LayoutDesignListChanged(int layoutDesignIndex);

    void FullScreenMode(bool state);

    /**
    * \brief Convenience method to set the size of an mitkRenderWindow.
    *
    * This method sets the size of the vtkRenderWindow and tells the
    * rendering that the size has changed -> adapts displayGeometry, etc.
    */
    void SetSize(int width, int height);

    /**
    * \brief Initializes the mitkVtkEventProvider to listen to the
    * currently used vtkInteractorStyle.
    *
    * This method makes sure that the internal mitkVtkEventProvider
    * listens to the correct vtkInteractorStyle.
    * This makes sure that VTK-Events are correctly translated into
    * MITK-Events.
    *
    * \warning This method needs to be called MANUALLY as soon as the MapperID
    * for this RenderWindow is changed or the vtkInteractorStyle is modified
    * somehow else!
    */
    void ReinitEventProvider();

  protected:
    RenderWindow(vtkRenderWindow *existingRenderWindow = nullptr,
                 const char *name = "unnamed renderer");

    void ResetView();
    vtkRenderWindow *m_vtkRenderWindow;
    vtkRenderWindowInteractor *m_vtkRenderWindowInteractor;

    vtkEventProvider *m_vtkMitkEventProvider;

  private:
  };
} // namespace

#endif
