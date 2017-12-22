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
   * Displays a color gradient in the background
   * of a vtkRenderWindow.
   * The gradient ist faked by displaying a non-interactable
   * smoothly shaded plane in a separate layer behind the
   * scene. After setting the renderwindow, the gradient may be
   * activated by calling Enable()
   * @deprecatedSince{2015_05} not used in renderwindows
   */
  class MITKCORE_EXPORT GradientBackground : public itk::Object
  {
  public:
    mitkClassMacroItkParent(GradientBackground, itk::Object);

    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      /**
       * Sets the renderwindow, in which the gradient background
       * will be shown. Make sure, you have called this function
       * before calling Enable()
       */
      virtual void SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renderWindow);

    /**
     * Returns the vtkRenderWindow, which is used
     * for displaying the gradient background
     */
    virtual vtkSmartPointer<vtkRenderWindow> GetRenderWindow();

    /**
     * Returns the renderer responsible for
     * rendering the color gradient into the
     * vtkRenderWindow
     */
    virtual vtkSmartPointer<vtkRenderer> GetVtkRenderer();

    /**
     * Sets the gradient colors. The gradient
     * will smoothly fade from color1 to color2
     */
    virtual void SetGradientColors(double r1, double g1, double b1, double r2, double g2, double b2);
    virtual void SetGradientColors(Color upper, Color lower);
    virtual void SetUpperColor(double r, double g, double b);
    virtual void SetLowerColor(double r, double g, double b);
    virtual void SetUpperColor(Color upper);
    virtual void SetLowerColor(Color lower);

    /**
     * Enables drawing of the color gradient background.
     * If you want to disable it, call the Disable() function.
     */
    virtual void Enable();

    /**
     * Disables drawing of the color gradient background.
     * If you want to enable it, call the Enable() function.
     */
    virtual void Disable();

    /**
     * Checks, if the gradient background is currently
     * enabled (visible)
     */
    virtual bool IsEnabled();

  protected:
    GradientBackground();
    ~GradientBackground() override;

    vtkSmartPointer<vtkRenderWindow> m_RenderWindow;

    vtkSmartPointer<vtkRenderer> m_Renderer;
  };
} // end of namespace mitk
#endif // mitkGradientBackground_h
