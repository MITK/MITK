/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkWidgetRendering_h
#define mitkVtkWidgetRendering_h

#include <mitkBaseData.h>

class vtkRenderer;
class vtkRenderWindow;
class vtkInteractorObserver;

namespace mitk
{
  class RenderWindow;

  /**
   * \brief Mechanism for rendering a vtkWidget in the foreground of a RenderWindow.
   *
   * To use this class, specify the vtkRenderWindow of the window into which the
   * vtkWidget shall be placed, and set the vtkWidget using SetVtkWidget().
   * After enabling the vtkWidget and calling Enable() of this class, the widget
   * should be rendered.
   *
   * \note This class only provides a basic mechanism for adding a widget; all widget
   * configuration such as placement, size, and en-/disabling of interaction
   * mechanisms need to be done in the vtkWidget object.
   */
  class MITKCORE_EXPORT VtkWidgetRendering : public BaseData
  {
  public:
    mitkClassMacro(VtkWidgetRendering, BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Set the render window in which the widget will be shown.
       *
       * \pre Must be called before calling Enable().
       *
       * \param[in] renderWindow  The vtkRenderWindow into which the widget is placed.
       */
      virtual void SetRenderWindow(vtkRenderWindow *renderWindow);

    /**
     * \brief Enable drawing of the widget.
     *
     * If you want to disable it, call the Disable() function.
     *
     * \pre SetRenderWindow() and SetVtkWidget() must have been called.
     */
    virtual void Enable();

    /**
     * \brief Disable drawing of the widget.
     *
     * If you want to enable it, call the Enable() function.
     */
    virtual void Disable();

    /**
     * \brief Check whether the widget is currently enabled (visible).
     *
     * \return True if the widget is enabled, false otherwise.
     */
    virtual bool IsEnabled();

    /**
     * \brief Empty implementation; VtkWidgetRendering does not support the
     *        requested region concept.
     */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Empty implementation; VtkWidgetRendering does not support the
     *        requested region concept.
     *
     * \return Always false.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Empty implementation; VtkWidgetRendering does not support the
     *        requested region concept.
     *
     * \return Always true.
     */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Empty implementation; VtkWidgetRendering does not support the
     *        requested region concept.
     */
    void SetRequestedRegion(const itk::DataObject *) override;

    /**
     * \brief Get the vtkRenderWindow used for displaying the widget.
     *
     * \return The associated vtkRenderWindow, or nullptr if not yet set.
     */
    virtual vtkRenderWindow *GetRenderWindow();

    /**
     * \brief Get the renderer responsible for rendering the widget into the
     *        vtkRenderWindow.
     *
     * \return The internal vtkRenderer instance.
     */
    virtual vtkRenderer *GetVtkRenderer();

    /**
     * \brief Set the vtkWidget to be rendered.
     *
     * \param[in] widget  The vtkInteractorObserver (widget) to render.
     */
    void SetVtkWidget(vtkInteractorObserver *widget);

    /**
     * \brief Get the vtkWidget that is being rendered.
     *
     * \return The currently set vtkInteractorObserver (widget), or nullptr.
     */
    vtkInteractorObserver *GetVtkWidget() const;

  protected:
    /** \brief Constructor. Creates an internal vtkRenderer. */
    VtkWidgetRendering();

    /** \brief Destructor. Disables the widget if still enabled. */
    ~VtkWidgetRendering() override;

    vtkRenderWindow *m_RenderWindow;
    vtkRenderer *m_Renderer;

    vtkInteractorObserver *m_VtkWidget;

    bool m_IsEnabled;
  };

} // end of namespace mitk
#endif
