/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 17:34:48 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21985 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
