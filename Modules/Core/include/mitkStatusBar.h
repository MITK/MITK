/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStatusBar_h
#define mitkStatusBar_h
#include <mitkStatusBarImplementation.h>
#include <MitkCoreExports.h>
#include <itkObject.h>
#include <mitkPoint.h>
#include <mitkTimeGeometry.h>
#include <itkIndex.h>

namespace mitk
{
  /**
   * \brief Sends messages to the application's status bar.
   *
   * Holds a GUI-dependent StatusBarImplementation and forwards text to it.
   * Similar to itk::OutputWindow, but displays a single line of text with
   * optional timed clearing. All MITK classes use this class to display
   * text on the GUI status bar. The main application must set the internal
   * StatusBarImplementation using SetImplementation().
   *
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT StatusBar : public itk::Object
  {
  public:
    itkTypeMacro(StatusBar, itk::Object);

    /**
     * \brief Get the singleton StatusBar instance.
     *
     * No reference counting due to decentralized static use.
     *
     * \return Pointer to the StatusBar singleton.
     */
    static StatusBar *GetInstance();

    /**
     * \brief Supply a GUI-dependent StatusBarImplementation.
     *
     * Must be set by the application to connect the application-dependent
     * subclass of StatusBarImplementation. If you create an instance, then
     * call ->Delete() on the supplied instance after setting it.
     *
     * \param[in] instance the GUI-dependent status bar implementation.
     */
    static void SetImplementation(StatusBarImplementation *instance);

    /**
     * \brief Send a string to the application's status bar.
     *
     * \param[in] t the text to display.
     */
    void DisplayText(const char *t);

    /**
     * \brief Send a string with a time delay to the application's status bar.
     *
     * \param[in] t the text to display.
     * \param[in] ms the time in milliseconds before the text is cleared.
     */
    void DisplayText(const char *t, int ms);

    /**
     * \brief Display an error message in the status bar.
     *
     * \param[in] t the error text to display.
     */
    void DisplayErrorText(const char *t);

    /**
     * \brief Display a warning message in the status bar.
     *
     * \param[in] t the warning text to display.
     */
    void DisplayWarningText(const char *t);

    /**
     * \brief Display a warning message with a time delay in the status bar.
     *
     * \param[in] t the warning text to display.
     * \param[in] ms the time in milliseconds before the text is cleared.
     */
    void DisplayWarningText(const char *t, int ms);

    /**
     * \brief Display a generic output message in the status bar.
     *
     * \param[in] t the text to display.
     */
    void DisplayGenericOutputText(const char *t);

    /**
     * \brief Display a debug message in the status bar.
     *
     * \param[in] t the debug text to display.
     */
    void DisplayDebugText(const char *t);

    /**
     * \brief Display a grey value text in the status bar.
     *
     * \param[in] t the grey value text to display.
     */
    void DisplayGreyValueText(const char *t);

    /**
     * \brief Display position and time information for a renderer.
     *
     * \param[in] point the 3D world position.
     * \param[in] time the current time point.
     */
    void DisplayRendererInfo(Point3D point, TimePointType time);

    /**
     * \brief Display position, index, time and numeric pixel value.
     *
     * \param[in] point the 3D world position.
     * \param[in] index the voxel index in the image.
     * \param[in] time the current time point.
     * \param[in] pixelValue the numeric pixel value at the position.
     */
    void DisplayImageInfo(Point3D point, itk::Index<3> index, TimePointType time, ScalarType pixelValue);

    /**
     * \brief Display position, index, time and custom pixel value string.
     *
     * \param[in] point the 3D world position.
     * \param[in] index the voxel index in the image.
     * \param[in] time the current time point.
     * \param[in] pixelValue the pixel value as a formatted string.
     */
    void DisplayImageInfo(Point3D point, itk::Index<3> index, TimePointType time, const char* pixelValue);

    /**
     * \brief Display placeholder text for invalid image information.
     */
    void DisplayImageInfoInvalid();

    /**
     * \brief Remove any temporary message being shown.
     */
    void Clear();

    /**
     * \brief Set the size grip of the window to enabled or disabled.
     *
     * The size grip is the triangle in the lower right window corner
     * for changing the window size.
     *
     * \param[in] enable true to enable the size grip, false to disable it.
     */
    void SetSizeGripEnabled(bool enable);

  protected:
    StatusBar();
    ~StatusBar() override;

    static StatusBarImplementation *m_Implementation;
    static StatusBar *m_Instance;
  };

} // end namespace mitk
#endif
