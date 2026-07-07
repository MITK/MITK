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

#include <mitkRenderWindowBase.h>

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

    /** \brief Destructor. Cleans up VTK render window, interactor, and event provider. */
    ~RenderWindow() override;

    /** \brief Returns the underlying vtkRenderWindow.
     *
     * \return The VTK render window instance.
     */
    vtkRenderWindow *GetVtkRenderWindow() override;

    /** \brief Returns the underlying vtkRenderWindowInteractor.
     *
     * \return The VTK render window interactor instance.
     */
    vtkRenderWindowInteractor *GetVtkRenderWindowInteractor() override;

    /** \brief Sets the layout index to define the layout type.
     *
     * \param layoutIndex the layout index to set.
     */
    void SetLayoutIndex(unsigned int layoutIndex);

    /** \brief Returns the layout index that defines the layout type.
     *
     * \return The current layout index.
     */
    unsigned int GetLayoutIndex();

    /** \brief Notifies that the layout design list has changed.
     *
     * Called when the MenuWidget needs to update the Layout Design List
     * after a layout change.
     *
     * \param layoutDesignIndex the new layout design index.
     */
    void LayoutDesignListChanged(int layoutDesignIndex);

    /** \brief Toggles full screen mode.
     *
     * \param state if true, enters full screen mode; if false, leaves it.
     */
    void FullScreenMode(bool state);

    /**
    * \brief Convenience method to set the size of an mitkRenderWindow.
    *
    * This method sets the size of the vtkRenderWindow and tells the
    * rendering that the size has changed, which adapts displayGeometry, etc.
    *
    * \param width the new width in pixels.
    * \param height the new height in pixels.
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
