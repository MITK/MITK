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

#ifndef CHERRYIHANDLER_H_
#define CHERRYIHANDLER_H_

#include <org.opencherry.osgi/cherryMacros.h>


namespace cherry {

class ExecutionEvent;

/**
 * A handler is the pluggable piece of a command that handles execution. Each
 * command can have zero or more handlers associated with it (in general), of
 * which only one will be active at any given moment in time. When the command
 * is asked to execute, it will simply pass that request on to its active
 * handler, if any.
 * 
 * @see AbstractHandler
 * @since 3.1
 */
struct CHERRY_API IHandler : public Object {
  
  cherryClassMacro(IHandler)
  
  /**
   * Registers an instance of <code>IHandlerListener</code> to listen for
   * changes to properties of this instance.
   * 
   * @param handlerListener
   *            the instance to register. Must not be <code>null</code>. If
   *            an attempt is made to register an instance which is already
   *            registered with this instance, no operation is performed.
   */
  //virtual void AddHandlerListener(IHandlerListener handlerListener) = 0;

  /**
   * Disposes of this handler. This can be used as an opportunity to unhook listeners
   * from other objects.
   */
  virtual ~IHandler() {}

  /**
   * Executes with the map of parameter values by name.
   * 
   * @param event
   *            An event containing all the information about the current
   *            state of the application; must not be <code>null</code>.
   * @return the result of the execution. Reserved for future use, must be
   *         <code>null</code>.
   * @throws ExecutionException
   *             if an exception occurred during execution.
   */
  virtual Object::Pointer Execute(const SmartPointer<ExecutionEvent> event) = 0;

  /**
     * Called by the framework to allow the handler to update its enabled state.
     * 
     * @param evaluationContext
     *            the state to evaluate against. May be <code>null</code>
     *            which indicates that the handler can query whatever model that
     *            is necessary. This context must not be cached.
     */
  virtual void SetEnabled(Object::Pointer evaluationContext) = 0;
    
  /**
   * Returns whether this handler is capable of executing at this moment in
   * time. If the enabled state is other than true clients should also
   * consider implementing IHandler2 so they can be notified about framework
   * execution contexts.
   * 
   * @return <code>true</code> if the command is enabled; <code>false</code>
   *         otherwise.
   * @see IHandler2#setEnabled(Object)
   */
  virtual bool IsEnabled() const = 0;

  /**
   * Returns whether this handler is really capable of handling delegation. In
   * the case of a handler that is a composition of other handlers, this reply
   * is intended to indicate whether the handler is truly capable of receiving
   * delegated responsibilities at this time.
   * 
   * @return <code>true</code> if the handler is handled; <code>false</code>
   *         otherwise.
   */
  virtual bool IsHandled() const = 0;

  /**
   * Unregisters an instance of <code>IHandlerListener</code> listening for
   * changes to properties of this instance.
   * 
   * @param handlerListener
   *            the instance to unregister. Must not be <code>null</code>.
   *            If an attempt is made to unregister an instance which is not
   *            already registered with this instance, no operation is
   *            performed.
   */
  //virtual void RemoveHandlerListener(IHandlerListener handlerListener);
  
  
};

}

#endif /*CHERRYIHANDLER_H_*/
