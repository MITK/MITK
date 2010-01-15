/*=========================================================================
 
 Program:   openCherry Platform
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


#ifndef CHERRYIAPPLICATIONCONTEXT_H_
#define CHERRYIAPPLICATIONCONTEXT_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

#include <osgi/framework/IBundle.h>

#include <Poco/Util/AbstractConfiguration.h>

namespace cherry {

using namespace osgi::framework;

/**
 * The context used to start an application.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @since 1.0
 */
struct IApplicationContext : public Object {

  osgiInterfaceMacro(cherry::IApplicationContext)

  /**
   * A key used to store arguments for the application.  The content of this argument
   * is unchecked and should conform to the expectations of the application being invoked.
   * Typically this is a <code>String</code> array.
   * <p>
   *
   * If the map used to launch an application {@link ApplicationDescriptor#launch(Map)} does
   * not contain a value for this key then command line arguments used to launch
   * the platform are set in the arguments of the application context.
   */
  //static const std::string APPLICATION_ARGS = "application.args";

  /**
   * The arguments used for the application.  The arguments from
   * {@link ApplicationDescriptor#launch(Map)} are used as the arguments
   * for this context when an application is launched.
   *
   * @return a map of application arguments.
   */
  virtual const Poco::Util::AbstractConfiguration& GetConfiguration() const = 0;

  /**
   * This method should be called once the application is completely initialized and running.
   * This method will perform certain operations that are needed once an application is running.
   * One example is bringing down a splash screen if it exists.
   */
  virtual void ApplicationRunning() = 0;

  /**
   * Returns the application associated with this application context.  This information
   * is used to guide the runtime as to what application extension to create and execute.
   *
   * @return this product's application or <code>null</code> if none
   */
  virtual std::string GetBrandingApplication() const = 0;

  /**
   * Returns the name of the product associated with this application context.
   * The name is typically used in the title bar of UI windows.
   *
   * @return the name of the product or <code>null</code> if none
   */
  virtual std::string GetBrandingName() const = 0;

  /**
   * Returns the text description of the product associated with this application context.
   *
   * @return the description of the product or <code>null</code> if none
   */
  virtual std::string GetBrandingDescription() const = 0;

  /** Returns the unique product id of the product associated with this application context.
   *
   * @return the id of the product
   */
  virtual std::string GetBrandingId() const = 0;

  /**
   * Returns the property with the given key of the product associated with this application context.
   * <code>null</code> is returned if there is no such key/value pair.
   *
   * @param key the name of the property to return
   * @return the value associated with the given key or <code>null</code> if none
   */
  virtual std::string GetBrandingProperty(const std::string& key) const = 0;

  /**
   * Returns the bundle which is responsible for the definition of the product associated with
   * this application context.
   * Typically this is used as a base for searching for images and other files
   * that are needed in presenting the product.
   *
   * @return the bundle which defines the product or <code>null</code> if none
   */
  virtual IBundle::Pointer GetBrandingBundle() const = 0;
};

}

#endif /* CHERRYIAPPLICATIONCONTEXT_H_ */
