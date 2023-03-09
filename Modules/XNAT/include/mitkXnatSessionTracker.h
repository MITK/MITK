/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkXnatSessionTracker_h
#define mitkXnatSessionTracker_h

#include "usServiceTracker.h"

#include "MitkXNATExports.h"

#include "mitkXnatSession.h"

namespace mitk
{
  class MITKXNAT_EXPORT XnatSessionTracker : public QObject, public us::ServiceTracker<ctkXnatSession>
  {
    Q_OBJECT

  public:
    XnatSessionTracker(us::ModuleContext *context);

  signals:
    void Opened(ctkXnatSession *);
    void AboutToBeClosed(ctkXnatSession *);

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
