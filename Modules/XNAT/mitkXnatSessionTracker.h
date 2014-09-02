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

#include "mitkXnatSession.h"
#include <usServiceTracker.h>

namespace mitk {

class MITK_XNAT_EXPORT XnatSessionTracker : public us::ServiceTracker<XnatSession>, public QObject
{
  Q_OBJECT

public:
  XnatSessionTracker(us::ModuleContext* context);

  virtual void Open();
  virtual void Close();

  public signals:
    void Opened(XnatSession*);
    void AboutToBeClosed(XnatSession*);

  private slots:
    void SessionOpened();
    void SessionAboutToBeClosed():

private:
  typedef us::ServiceTracker<XnatSession> Superclass;

  us::ModuleContext* m_Context;

  virtual TrackedType AddingService(const ServiceReferenceType &reference);
  virtual void RemovedService(const ServiceReferenceType& /*reference*/, TrackedType tracked);

};

} // end of namespace mitk

#endif // MITKXNATSESSIONTRACKER_H
