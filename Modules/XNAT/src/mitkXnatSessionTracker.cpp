/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkXnatSessionTracker.h"

#include "ctkXnatLoginProfile.h"

namespace mitk
{
  XnatSessionTracker::XnatSessionTracker(us::ModuleContext *context) : Superclass(context), m_Context(context) {}
  void XnatSessionTracker::SessionOpened()
  {
    ctkXnatSession *session = dynamic_cast<ctkXnatSession *>(QObject::sender());
    emit Opened(session);
  }

  void XnatSessionTracker::SessionAboutToBeClosed()
  {
    ctkXnatSession *session = dynamic_cast<ctkXnatSession *>(QObject::sender());
    emit AboutToBeClosed(session);
  }

  ctkXnatSession *XnatSessionTracker::AddingService(const ServiceReferenceType &reference)
  {
    ctkXnatSession *session = Superclass::AddingService(reference);
    if (session->isOpen())
    {
      emit Opened(session);
    }
    connect(session, SIGNAL(sessionOpened()), this, SLOT(SessionOpened()));
    connect(session, SIGNAL(sessionAboutToBeClosed()), this, SLOT(SessionAboutToBeClosed()));

    return session;
  }

  void XnatSessionTracker::RemovedService(const ServiceReferenceType &reference, ctkXnatSession *tracked)
  {
    emit AboutToBeClosed(tracked);
    Superclass::RemovedService(reference, tracked);
  }

} // end of namespace mitk
