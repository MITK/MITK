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

#include "mitkOverlayServiceTracker.h"

// Micro Services
#include <usGetModuleContext.h>

namespace mitk {

OverlayServiceTracker::OverlayServiceTracker(const us::LDAPFilter& filter)
  : Superclass(us::GetModuleContext(),filter)
{
}

OverlayServiceTracker::TrackedType OverlayServiceTracker::AddingService(const OverlayServiceTracker::ServiceReferenceType &reference)
{
  Overlay* overlay = Superclass::AddingService(reference);
  if (overlay)
  {
    m_OverlayServices.push_back(overlay);
  }
  return overlay;
}

void OverlayServiceTracker::RemovedService(const OverlayServiceTracker::ServiceReferenceType &, OverlayServiceTracker::TrackedType tracked)
{
//  tracked->RemoveFromBaseRenderer() TODO19786
  m_OverlayServices.erase(std::remove(m_OverlayServices.begin(),
                                      m_OverlayServices.end(), tracked),
                          m_OverlayServices.end());
}

} // end of namespace mitk
