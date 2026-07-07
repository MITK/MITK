/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderWindowFrame_h
#define mitkRenderWindowFrame_h

#include <MitkCoreExports.h>
#include <itkObject.h>
#include <mitkCommon.h>
#include <vtkSmartPointer.h>

class vtkRenderer;
class vtkRenderWindow;

namespace mitk
{
  /**
   * \brief Renders a colored rectangle frame at the borders of a vtkRenderWindow.
   *
   * The rectangle rendering is performed by a vtkMitkRectangleProp. This class
   * instantiates the vtkProp and a corresponding vtkRenderer instance.
   *
   * \sa vtkMitkRectangleProp
   */
  class MITKCORE_EXPORT RenderWindowFrame : public itk::Object
  {
  public:
    mitkClassMacroItkParent(RenderWindowFrame, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * \brief Set the render window in which the colored rectangle border will be shown.
       *
       * \pre Must be called before calling Enable().
       *
       * \param[in] renderWindow  The vtkRenderWindow to render the frame into.
       */
      virtual void SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renderWindow);

    /**
     * \brief Enable drawing of the colored rectangle frame.
     *
     * If you want to disable it, call the Disable() function.
     *
     * \param[in] col1  Red color component (0.0 to 1.0).
     * \param[in] col2  Green color component (0.0 to 1.0).
     * \param[in] col3  Blue color component (0.0 to 1.0).
     */
    virtual void Enable(float col1, float col2, float col3);

    /**
     * \brief Disable drawing of the colored rectangle frame.
     *
     * If you want to enable it, call the Enable() function.
     */
    virtual void Disable();

    /**
     * \brief Check whether the frame is currently enabled (visible).
     *
     * \return True if the frame is enabled, false otherwise.
     */
    virtual bool IsEnabled();

    /**
     * \brief Get the vtkRenderWindow used for displaying the frame.
     *
     * \return The associated vtkRenderWindow, or nullptr if not set.
     */
    virtual vtkSmartPointer<vtkRenderWindow> GetRenderWindow();

    /**
     * \brief Get the renderer responsible for rendering the frame.
     *
     * \return The internal vtkRenderer instance.
     */
    virtual vtkSmartPointer<vtkRenderer> GetVtkRenderer();

  protected:
    /** \brief Constructor. */
    RenderWindowFrame();

    /** \brief Destructor. */
    ~RenderWindowFrame() override;

    vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
    vtkSmartPointer<vtkRenderer> m_RectangleRenderer;
    bool m_IsEnabled;
  };
} // end of namespace mitk
#endif
