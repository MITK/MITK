/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSafeApplication_h
#define QmitkSafeApplication_h

#include <MitkAppUtilExports.h>

#include <QApplication>
#include <QMessageBox>

/**
 * \brief A QApplication subclass that optionally catches unhandled exceptions in the Qt event loop.
 *
 * When safe mode is enabled, this class reimplements QApplication::notify() to wrap all
 * event dispatching in a try/catch block. If an unhandled exception is thrown during event
 * processing, it is caught and an error message is displayed to the user rather than
 * crashing the application. When safe mode is disabled, standard QApplication::notify()
 * behavior is used.
 *
 * \sa QmitkSingleApplication, mitk::BaseApplication
 */
class MITKAPPUTIL_EXPORT QmitkSafeApplication : public QApplication
{
public:
  /**
   * \brief Construct a QmitkSafeApplication.
   * \param[in,out] argc Reference to the application argument count (passed to QApplication).
   * \param[in] argv Array of command line argument strings (passed to QApplication).
   * \param[in] safeMode If \c true, exception-safe event dispatching is enabled. Defaults to \c true.
   */
  QmitkSafeApplication(int &argc, char **argv, bool safeMode = true);

  /**
   * \brief Reimplements QApplication::notify() to catch unhandled exceptions.
   *
   * When safe mode is enabled, exceptions thrown during event processing are caught and
   * an error message dialog is displayed. When safe mode is disabled, this method delegates
   * directly to QApplication::notify().
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
