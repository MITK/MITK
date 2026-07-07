/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSingleApplication_h
#define QmitkSingleApplication_h

#include <MitkAppUtilExports.h>

#include <qtsingleapplication.h>

#include <QmitkSafeNotify.h>

/**
 * \brief A single-instance application with optional exception-safe event dispatching.
 *
 * This class extends QtSingleApplication (which ensures only one instance of the application
 * runs at a time) with the same safe-mode behavior as QmitkSafeApplication. When safe mode is
 * enabled, unhandled exceptions thrown during Qt event processing are caught and reported to
 * the user instead of crashing the application.
 *
 * This class is used by mitk::BaseApplication when single-instance mode is requested.
 *
 * \sa QmitkSafeApplication, mitk::BaseApplication
 */
class MITKAPPUTIL_EXPORT QmitkSingleApplication : public QtSingleApplication
{
public:
  /**
   * \brief Construct a QmitkSingleApplication.
   * \param[in,out] argc Reference to the application argument count (passed to QtSingleApplication).
   * \param[in] argv Array of command line argument strings (passed to QtSingleApplication).
   * \param[in] safeMode If \c true, exception-safe event dispatching is enabled. Defaults to \c true.
   */
  QmitkSingleApplication(int &argc, char **argv, bool safeMode = true);

  /**
   * \brief Reimplements QtSingleApplication::notify() to catch unhandled exceptions.
   *
   * When safe mode is enabled, exceptions thrown during event processing are caught and
   * an error message dialog is displayed. When safe mode is disabled, this method delegates
   * directly to QtSingleApplication::notify().
   *
   * \param[in] receiver The target QObject that should receive the event.
   * \param[in] event The QEvent to be delivered to the receiver.
   * \return \c true if the event was recognized and processed by the receiver; \c false otherwise.
   */
  bool notify(QObject *receiver, QEvent *event) override;

  /**
   * \brief Enable or disable safe mode.
   * \param[in] safeMode If \c true, exception-catching event dispatching is enabled.
   */
  void setSafeMode(bool safeMode);

  /**
   * \brief Query whether safe mode is currently enabled.
   * \return \c true if safe mode is enabled; \c false otherwise.
   */
  bool getSafeMode() const;

private:
  bool m_SafeMode;
};

#endif
