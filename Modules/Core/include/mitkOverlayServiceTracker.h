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

#ifndef MITKOVERLAYSERVICETRACKER_H
#define MITKOVERLAYSERVICETRACKER_H

#include "usServiceTracker.h"
#include "mitkOverlay.h"

namespace mitk {

class OverlayServiceTracker : public us::ServiceTracker<Overlay>
{

public:

  OverlayServiceTracker(const us::LDAPFilter &filter);

private:

  typedef us::ServiceTracker<mitk::Overlay> Superclass;

  TrackedType AddingService(const ServiceReferenceType &reference) override;

  void RemovedService(const ServiceReferenceType& /*reference*/, TrackedType tracked) override;

  std::vector<Overlay*> m_OverlayServices;

};

} // end of namespace mitk

#endif // MITKOVERLAYSERVICETRACKER_H
