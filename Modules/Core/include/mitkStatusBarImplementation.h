/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStatusBarImplementation_h
#define mitkStatusBarImplementation_h
#include <MitkCoreExports.h>
#include <mitkCommon.h>

namespace mitk
{
  /**
   * \brief GUI-independent interface for all GUI-dependent implementations of a status bar.
   *
   * Subclass this to provide a platform-specific status bar implementation.
   */
  class MITKCORE_EXPORT StatusBarImplementation
  {
  public:
    mitkClassMacroNoParent(StatusBarImplementation)

    /** \brief Constructor. */
    StatusBarImplementation(){};

    /** \brief Destructor. */
    virtual ~StatusBarImplementation(){};

    /**
     * \brief Send a string to the application's status bar.
     *
     * \param[in] t the text to display.
     */
    virtual void DisplayText(const char *t) = 0;

    /**
     * \brief Send a string with a time delay to the application's status bar.
     *
     * \param[in] t the text to display.
     * \param[in] ms the time in milliseconds before the text is cleared.
     */
    virtual void DisplayText(const char *t, int ms) = 0;

    /**
     * \brief Display an error message in the status bar.
     *
     * \param[in] t the error text to display.
     */
    virtual void DisplayErrorText(const char *t) = 0;

    /**
     * \brief Display a warning message in the status bar.
     *
     * \param[in] t the warning text to display.
     */
    virtual void DisplayWarningText(const char *t) = 0;

    /**
     * \brief Display a warning message with a time delay in the status bar.
     *
     * \param[in] t the warning text to display.
     * \param[in] ms the time in milliseconds before the text is cleared.
     */
    virtual void DisplayWarningText(const char *t, int ms) = 0;

    /**
     * \brief Display a generic output message in the status bar.
     *
     * \param[in] t the text to display.
     */
    virtual void DisplayGenericOutputText(const char *t) = 0;

    /**
     * \brief Display a debug message in the status bar.
     *
     * \param[in] t the debug text to display.
     */
    virtual void DisplayDebugText(const char *t) = 0;

    /**
     * \brief Display a grey value text in the status bar.
     *
     * \param[in] t the grey value text to display.
     */
    virtual void DisplayGreyValueText(const char *t) = 0;

    /**
     * \brief Remove any temporary message being shown.
     */
    virtual void Clear() = 0;

    /**
     * \brief Set the size grip of the window to enabled or disabled.
     *
     * The size grip is the triangle in the lower right window corner
     * for changing the window size.
     *
     * \param[in] enable true to enable the size grip, false to disable it.
     */
    virtual void SetSizeGripEnabled(bool enable) = 0;
  };

} // end namespace mitk
#endif
