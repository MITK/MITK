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


#ifndef CHERRYHANDLEREVENT_H_
#define CHERRYHANDLEREVENT_H_

#include "common/cherryAbstractBitSetEvent.h"
#include "cherryCommandsDll.h"

namespace cherry {

struct IHandler;

/**
 * An instance of this class describes changes to an instance of
 * <code>IHandler</code>.
 * <p>
 * This class is not intended to be extended by clients.
 * </p>
 *
 * @see IHandlerListener#HandlerChanged(HandlerEvent)
 */
class CHERRY_COMMANDS HandlerEvent : public AbstractBitSetEvent {

public:

  osgiObjectMacro(HandlerEvent)

  /**
   * Creates a new instance of this class.
   *
   * @param handler
   *            the instance of the interface that changed; must not be
     *            <code>null</code>.
   * @param enabledChanged
   *            Whether the enabled state of the handler has changed.
   * @param handledChanged
   *            Whether the handled state of the handler has changed.
   */
  HandlerEvent(const SmartPointer<IHandler> handler, bool enabledChanged,
      bool handledChanged);

  /**
   * Returns the instance of the interface that changed.
   *
   * @return the instance of the interface that changed. Guaranteed not to be
   *         <code>null</code>.
   */
  SmartPointer<IHandler> GetHandler() const;

  /**
   * Returns whether or not the enabled property changed.
   *
   * @return <code>true</code>, iff the enabled property changed.
   */
  bool IsEnabledChanged() const;

  /**
   * Returns whether or not the handled property changed.
   *
   * @return <code>true</code>, iff the handled property changed.
   */
  bool IsHandledChanged() const ;


private:

  /**
   * The bit used to represent whether the handler has changed its enabled
   * state.
   */
  static const int CHANGED_ENABLED; // = 1;

  /**
   * The bit used to represent whether the handler has changed its handled
   * state.
   */
  static const int CHANGED_HANDLED; // = 1 << 1;

  /**
   * The handler that changed; this value is never <code>null</code>.
   */
  const SmartPointer<IHandler> handler;


};

}

#endif /* CHERRYHANDLEREVENT_H_ */
