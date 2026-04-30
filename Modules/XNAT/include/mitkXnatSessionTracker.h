/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkXnatSessionTracker_h
#define mitkXnatSessionTracker_h

#include <usServiceTracker.h>

#include <MitkXNATExports.h>

#include <mitkXnatSession.h>

namespace mitk
{
  /**
   * \brief Tracks ctkXnatSession services in the CppMicroServices registry and emits
   *        signals when sessions are opened or about to be closed.
   *
   * XnatSessionTracker monitors the CppMicroServices service registry for ctkXnatSession
   * instances. When a session service is registered, the tracker connects to its opened and
   * aboutToBeClosed signals and re-emits corresponding Qt signals. This allows plugins and
   * modules to react to XNAT session lifecycle events without directly depending on the
   * service registry.
   *
   * \sa mitk::XnatSession, ctkXnatSession, us::ServiceTracker
   */
  class MITKXNAT_EXPORT XnatSessionTracker : public QObject, public us::ServiceTracker<ctkXnatSession>
  {
    Q_OBJECT

  public:
    /**
     * \brief Construct a session tracker for the given module context.
     *
     * \param[in] context The CppMicroServices module context used to track ctkXnatSession services.
     */
    XnatSessionTracker(us::ModuleContext *context);

  signals:
    /**
     * \brief Emitted when a tracked XNAT session has been opened.
     *
     * \param[in] session Pointer to the ctkXnatSession that was opened.
     */
    void Opened(ctkXnatSession * session);

    /**
     * \brief Emitted when a tracked XNAT session is about to be closed.
     *
     * \param[in] session Pointer to the ctkXnatSession that is about to close.
     */
    void AboutToBeClosed(ctkXnatSession * session);

  private:
    typedef us::ServiceTracker<ctkXnatSession> Superclass;

    us::ModuleContext *m_Context;

    TrackedType AddingService(const ServiceReferenceType &reference) override;
    void RemovedService(const ServiceReferenceType &reference, TrackedType tracked) override;

  private slots:
    void SessionOpened();
    void SessionAboutToBeClosed();
  };

} // end of namespace mitk

#endif
