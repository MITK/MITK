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

#ifndef MITKXNATSESSIONTRACKER_H
#define MITKXNATSESSIONTRACKER_H

#include "usServiceTracker.h"

#include "MitkXNATExports.h"

#include "mitkXnatSession.h"

namespace mitk {

class MITK_XNAT_EXPORT XnatSessionTracker : public QObject, public us::ServiceTracker<ctkXnatSession>
{
  Q_OBJECT

public:
  XnatSessionTracker(us::ModuleContext* context);

signals:
  void Opened(ctkXnatSession*);
  void AboutToBeClosed(ctkXnatSession*);

private:
  typedef us::ServiceTracker<ctkXnatSession> Superclass;

  us::ModuleContext* m_Context;

  virtual TrackedType AddingService(const ServiceReferenceType &reference);
  virtual void RemovedService(const ServiceReferenceType& reference, TrackedType tracked);

  private slots:
    void SessionOpened();
    void SessionAboutToBeClosed();
};

} // end of namespace mitk

#endif // MITKXNATSESSIONTRACKER_H
