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

#ifndef MITKEVENTMAPPERADDON_H_
#define MITKEVENTMAPPERADDON_H_

#include <mitkStateEvent.h>
#include <mitkGlobalInteraction.h>

namespace mitk {

  /**
  * The event mapper addon resembles an interface, that allows addons, which are not stored in the core <br>
  * to be used inside the core. For an additional input device it is mandatory to inherit from this class.
  *
  * @noimplement This interface is not intended to be implemented by clients.
  * @ingroup Interaction
  */
  class MITK_CORE_EXPORT EventMapperAddOn : public itk::Object
  {
  public:
    /**
    *  Forwards an event fired by the driver of device to the <code>mitk::GlobalInteraction</code>
    *
    *  @param e the event, who should be processed
    */
    virtual void ForwardEvent(const mitk::StateEvent* e)
    {
      mitk::GlobalInteraction::GetInstance()->HandleEvent(e);
    }

  }; // end struct EventMapperAddOn
} // end namespace mitk

#endif /* MITKEVENTMAPPERADDON_H_ */
