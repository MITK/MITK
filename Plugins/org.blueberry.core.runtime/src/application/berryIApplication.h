/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIAPPLICATION_H_
#define BERRYIAPPLICATION_H_

#include <org_blueberry_core_runtime_Export.h>

#include <QObject>

namespace berry {

struct IApplicationContext;

/**
 * Bootstrap type for an application.  An IApplication represent executable
 * entry points into an application.  An IApplication can be configured into
 * the Platform's <code>org.blueberry.osgi.applications</code> extension-point.
 *
 * <p>
 * Clients may implement this interface.
 * </p>
 */
struct org_blueberry_core_runtime_EXPORT IApplication
{

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
     * this is used to relaunch BlueBerry with different command line arguments.  When the executable is
     * relaunched the command line will be retrieved from the <code>BlueBerry.exitdata</code> system property.
     */
    static const int EXIT_RELAUNCH;

    virtual ~IApplication();
    /**
     * Starts this application with the given context and returns a result.  This
     * method must not exit until the application is finished and is ready to exit.
     * The content of the context is unchecked and should conform to the expectations of
     * the application being invoked.<p>
     *
     * Applications can return any object they like.  If an <code>Integer</code> is returned
     * it is treated as the program exit code if BlueBerry is exiting.
     * <p>
     * Note: This method is called by the platform; it is not intended
     * to be called directly by clients.
     * </p>
     * @return the return value of the application
     * @see #EXIT_OK
     * @see #EXIT_RESTART
     * @see #EXIT_RELAUNCH
     * @param context the application context to pass to the application
     * @exception std::exception if there is a problem running this application.
     */
    virtual QVariant Start(IApplicationContext* context) = 0;

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

Q_DECLARE_INTERFACE(berry::IApplication, "org.blueberry.IApplication")

#endif /*BERRYIAPPLICATION_H_*/
