/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKTRACKEDSERVICELISTENER_H
#define MITKTRACKEDSERVICELISTENER_H

#include <itkLightObject.h>

#include "mitkServiceEvent.h"

namespace mitk {

/**
 * This class is not intended to be used directly. It is exported to support
 * the MITK module system.
 */
struct TrackedServiceListener : public itk::LightObject
{

  /**
   * Slot connected to service events for the
   * <code>ServiceTracker</code> class. This method must NOT be
   * synchronized to avoid deadlock potential.
   *
   * @param event <code>ServiceEvent</code> object from the framework.
   */
  virtual void ServiceChanged(const ServiceEvent& event) = 0;

};

}

#endif // MITKTRACKEDSERVICELISTENER_H
