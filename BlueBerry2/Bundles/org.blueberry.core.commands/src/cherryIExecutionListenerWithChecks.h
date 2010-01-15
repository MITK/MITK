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


#ifndef CHERRYIEXECUTIONLISTENERWITHCHECKS_H_
#define CHERRYIEXECUTIONLISTENERWITHCHECKS_H_

#include "cherryIExecutionListener.h"
#include "common/cherryCommandExceptions.h"

namespace cherry {

/**
 * <p>
 * A listener to the execution of commands. This listener will be notified if
 * someone tries to execute a command and it is not enabled or not defined. The
 * listener also be notified if a command is about to execute, and when that
 * execution completes. It is not possible for the listener to prevent the
 * execution, only to respond to it in some way.
 * </p>
 * <p>
 * Clients may implement.
 * </p>
 *
 * @since 3.2
 */
struct IExecutionListenerWithChecks : public IExecutionListener {

  osgiInterfaceMacro(cherry::IExecutionListenerWithChecks)

  struct Events : public IExecutionListener::Events {

    Message2<const std::string&, const NotDefinedException*> notDefined;
    Message2<const std::string&, const NotEnabledException*> notEnabled;

    void AddListener(IExecutionListener::Pointer listener);
    void RemoveListener(IExecutionListener::Pointer listener);
    bool HasListeners() const;
    bool IsEmpty() const;

    typedef MessageDelegate2<IExecutionListenerWithChecks, const std::string&, const NotDefinedException* > NotDefinedDelegate;
    typedef MessageDelegate2<IExecutionListenerWithChecks, const std::string&, const NotEnabledException*> NotEnabledDelegate;
  };

  /**
   * Notifies the listener that an attempt was made to execute a command that
   * is not defined.
   *
   * @param commandId
   *            The identifier of command that is not defined; never
   *            <code>null</code>
   * @param exception
   *            The exception that occurred; never <code>null</code>.
   */
  virtual void NotDefined(const std::string& commandId, const NotDefinedException* exception) = 0;

  /**
   * Notifies the listener that an attempt was made to execute a command that
   * is disabled.
   *
   * @param commandId
   *            The identifier of command that is not enabled; never
   *            <code>null</code>
   * @param exception
   *            The exception that occurred; never <code>null</code>.
   */
  virtual void NotEnabled(const std::string& commandId, const NotEnabledException* exception) = 0;
};

}

#endif /* CHERRYIEXECUTIONLISTENERWITHCHECKS_H_ */
