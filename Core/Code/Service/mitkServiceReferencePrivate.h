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


#ifndef MITKSERVICEREFERENCEPRIVATE_H
#define MITKSERVICEREFERENCEPRIVATE_H

#include "mitkAtomicInt.h"

#include "mitkServiceProperties.h"

namespace itk {
  class LightObject;
}

namespace mitk {

class Module;
class ServiceRegistrationPrivate;
class ServiceReferencePrivate;


/**
 * \ingroup MicroServices
 */
class ServiceReferencePrivate
{
public:

  ServiceReferencePrivate(ServiceRegistrationPrivate* reg);

  virtual ~ServiceReferencePrivate() {}

  /**
    * Get the service object.
    *
    * @param module requester of service.
    * @return Service requested or null in case of failure.
    */
  itk::LightObject* GetService(Module* module);

  /**
   * Unget the service object.
   *
   * @param module Module who wants remove service.
   * @param checkRefCounter If true decrement refence counter and remove service
   *                        if we reach zero. If false remove service without
   *                        checking refence counter.
   * @return True if service was remove or false if only refence counter was
   *         decremented.
   */
  bool UngetService(Module* module, bool checkRefCounter);

  /**
   * Get all properties registered with this service.
   *
   * @return A ServiceProperties object containing properties or being empty
   *         if service has been removed.
   */
  ServiceProperties GetProperties() const;

  /**
   * Returns the property value to which the specified property key is mapped
   * in the properties <code>ServiceProperties</code> object of the service
   * referenced by this <code>ServiceReference</code> object.
   *
   * <p>
   * Property keys are case-insensitive.
   *
   * <p>
   * This method must continue to return property values after the service has
   * been unregistered. This is so references to unregistered services can
   * still be interrogated.
   *
   * @param key The property key.
   * @param lock If <code>true</code>, access of the properties of the service
   * referenced by this <code>ServiceReference</code> object will be
   * synchronized.
   * @return The property value to which the key is mapped; an invalid Any
   * if there is no property named after the key.
   */
  Any GetProperty(const std::string& key, bool lock) const;

  /**
   * Reference count for implicitly shared private implementation.
   */
  AtomicInt ref;

  /**
   * Link to registration object for this reference.
   */
  ServiceRegistrationPrivate* const registration;

private:

  // disable copy constructor and assignment operator
  ServiceReferencePrivate(const ServiceReferencePrivate&);
  ServiceReferencePrivate& operator=(const ServiceReferencePrivate&);
};

}

#endif // MITKSERVICEREFERENCEPRIVATE_H
