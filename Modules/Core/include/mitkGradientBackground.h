/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGradientBackground_h
#define mitkGradientBackground_h

#include <itkObject.h>
#include <mitkColorProperty.h>
#include <mitkCommon.h>
#include <vtkSmartPointer.h>

class vtkRenderer;
class vtkRenderWindow;

namespace mitk
{
  class RenderWindow;
  /**
   * \brief Displays a color gradient in the background of a vtkRenderWindow.
   *
   * The gradient is faked by displaying a non-interactable, smoothly shaded
   * plane in a separate layer behind the scene. After setting the render
   * window, the gradient may be activated by calling Enable().
   *
   * \deprecated Since 2015_05. Not used in render windows.
   * \ingroup Rendering
   */
  class MITKCORE_EXPORT GradientBackground : public itk::Object
  {
  public:
    mitkClassMacroItkParent(GradientBackground, itk::Object);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Set the render window in which the gradient background will be shown.
       *
       * \pre Must be called before calling Enable().
       *
       * \param[in] renderWindow  The vtkRenderWindow to render the gradient into.
       */
      virtual void SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renderWindow);

    /**
     * \brief Get the vtkRenderWindow used for displaying the gradient background.
     *
     * \return The associated vtkRenderWindow, or nullptr if not set.
     */
    virtual vtkSmartPointer<vtkRenderWindow> GetRenderWindow();

    /**
     * \brief Get the renderer responsible for rendering the color gradient.
     *
     * \return The internal vtkRenderer instance.
     */
    virtual vtkSmartPointer<vtkRenderer> GetVtkRenderer();

    /**
     * \brief Set the gradient colors using individual RGB components.
     *
     * The gradient will smoothly fade from the upper color to the lower color.
     *
     * \param[in] r1  Red component of the upper color.
     * \param[in] g1  Green component of the upper color.
     * \param[in] b1  Blue component of the upper color.
     * \param[in] r2  Red component of the lower color.
     * \param[in] g2  Green component of the lower color.
     * \param[in] b2  Blue component of the lower color.
     */
    virtual void SetGradientColors(double r1, double g1, double b1, double r2, double g2, double b2);

    /**
     * \brief Set the gradient colors using Color objects.
     *
     * \param[in] upper  The upper color of the gradient.
     * \param[in] lower  The lower color of the gradient.
     */
    virtual void SetGradientColors(Color upper, Color lower);

    /**
     * \brief Set the upper color of the gradient using RGB components.
     *
     * \param[in] r  Red component.
     * \param[in] g  Green component.
     * \param[in] b  Blue component.
     */
    virtual void SetUpperColor(double r, double g, double b);

    /**
     * \brief Set the lower color of the gradient using RGB components.
     *
     * \param[in] r  Red component.
     * \param[in] g  Green component.
     * \param[in] b  Blue component.
     */
    virtual void SetLowerColor(double r, double g, double b);

    /**
     * \brief Set the upper color of the gradient using a Color object.
     *
     * \param[in] upper  The upper color.
     */
    virtual void SetUpperColor(Color upper);

    /**
     * \brief Set the lower color of the gradient using a Color object.
     *
     * \param[in] lower  The lower color.
     */
    virtual void SetLowerColor(Color lower);

    /**
     * \brief Enable drawing of the color gradient background.
     *
     * If you want to disable it, call the Disable() function.
     */
    virtual void Enable();

    /**
     * \brief Disable drawing of the color gradient background.
     *
     * If you want to enable it, call the Enable() function.
     */
    virtual void Disable();

    /**
     * \brief Check whether the gradient background is currently enabled (visible).
     *
     * \return True if the gradient is enabled, false otherwise.
     */
    virtual bool IsEnabled();

  protected:
    /** \brief Constructor. */
    GradientBackground();

    /** \brief Destructor. */
    ~GradientBackground() override;

    vtkSmartPointer<vtkRenderWindow> m_RenderWindow;

    vtkSmartPointer<vtkRenderer> m_Renderer;
  };
} // end of namespace mitk
#endif
