/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCallbackFromGUIThread_h
#define QmitkCallbackFromGUIThread_h

#include <MitkQtWidgetsExtExports.h>
#include <mitkCallbackFromGUIThread.h>

#include <QObject>

/**
 * \brief Qt-specific implementation of mitk::CallbackFromGUIThreadImplementation.
 *
 * Allows executing itk::Command objects from the Qt GUI thread by posting custom
 * QEvents into the Qt event queue. This is necessary for thread-safe GUI updates
 * from background threads in MITK.
 *
 * On construction, this object registers itself as the global callback implementation
 * via mitk::CallbackFromGUIThread::RegisterImplementation().
 *
 * \sa mitk::CallbackFromGUIThread, mitk::CallbackFromGUIThreadImplementation
 */
class MITKQTWIDGETSEXT_EXPORT QmitkCallbackFromGUIThread : public QObject,
                                                           public mitk::CallbackFromGUIThreadImplementation
{
  Q_OBJECT

public:
  /**
   * \brief Post the given command for execution on the GUI thread.
   *
   * The command will be executed asynchronously when the Qt event loop
   * processes the posted event.
   *
   * \param[in] cmd The itk::Command to execute.
   * \param[in] e The itk::EventObject to pass to the command. Ownership is transferred.
   */
  void CallThisFromGUIThread(itk::Command *cmd, itk::EventObject *e) override;

  /**
   * \brief Construct and register as the global GUI thread callback implementation.
   */
  QmitkCallbackFromGUIThread();

  /** \brief Destructor. */
  ~QmitkCallbackFromGUIThread() override;

  /**
   * \brief Process posted callback events.
   * \param[in] e The QEvent to handle.
   * \return True if the event was a callback event and was handled.
   */
  bool event(QEvent *e) override;

protected:
private:
};

#endif
