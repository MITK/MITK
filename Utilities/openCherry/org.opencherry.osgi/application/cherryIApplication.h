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

#ifndef CHERRYIAPPLICATION_H_
#define CHERRYIAPPLICATION_H_

#include "../cherryOSGiDll.h"

namespace cherry {

/**
 * Bootstrap type for an application.  An IApplication represent executable 
 * entry points into an application.  An IApplication can be configured into 
 * the Platform's <code>org.opencherry.osgi.applications</code> extension-point.
 *
 * <p>
 * Clients may implement this interface.
 * </p>
 * 
 * @since 1.0
 */
struct CHERRY_OSGI IApplication {

    /**
     * Exit object indicating normal termination
     */
    static const int EXIT_OK;

    /**
     * Exit object requesting platform restart
     */
    static const int EXIT_RESTART;

    /**
     * Exit object requesting that the command passed back be executed.  Typically
     * this is used to relaunch Eclipse with different command line arguments.  When the executable is
     * relaunched the command line will be retrieved from the <code>eclipse.exitdata</code> system property.  
     */
    static const int EXIT_RELAUNCH;

    /**
     * Starts this application with the given context and returns a result.  This 
     * method must not exit until the application is finished and is ready to exit.
     * The content of the context is unchecked and should conform to the expectations of
     * the application being invoked.<p>
     * 
     * Applications can return any object they like.  If an <code>Integer</code> is returned
     * it is treated as the program exit code if Eclipse is exiting.
     * <p>
     * Note: This method is called by the platform; it is not intended
     * to be called directly by clients.
     * </p>
     * @return the return value of the application
     * @see #EXIT_OK
     * @see #EXIT_RESTART
     * @see #EXIT_RELAUNCH
     * @param context the application context to pass to the application
     * @exception Exception if there is a problem running this application.
     */
    virtual int Start() = 0;

    /**
     * Forces this running application to exit.  This method should wait until the 
     * running application is ready to exit.  The {@link #start()} 
     * should already have exited or should exit very soon after this method exits<p>
     * 
     * This method is only called to force an application to exit.
     * This method will not be called if an application exits normally from 
     * the {@link #start()} method.
     * <p>
     * Note: This method is called by the platform; it is not intended
     * to be called directly by clients.
     * </p>
     */
    virtual void Stop() = 0;
  
};

}

#endif /*CHERRYIAPPLICATION_H_*/
