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

#ifndef MITK_WIIMOTEADDON_H_
#define MITK_WIIMOTEADDON_H_

#include <mitkVector.h>
#include <mitkCommon.h>

#include <mitkEventMapperAddon.h>
#include <mitkInputDevicesExports.h>

namespace mitk {

  /**
  * Documentation in the interface.
  *
  * @brief EventMapper addon for a WiiRemote
  * @ingroup Interaction
  */
  class mitkInputDevices_EXPORT WiiMoteAddOn : public EventMapperAddOn
  {

  public:

    // Singleton
    static WiiMoteAddOn* GetInstance();

    /**
    * @see mitk::EventMapperAddOn::ForwardEvent(const mitk::StateEvent* e)
    */
    void ForwardEvent(const mitk::StateEvent* e);

  protected:

  private:

  }; // end class WiiMoteAddOn
} // end namespace mitk

#endif /* MITK_WIIMOTEADDON_H_ */
