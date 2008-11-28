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


#ifndef CHERRYSHELLEVENT_H_
#define CHERRYSHELLEVENT_H_

#include "cherryUiDll.h"

#include <cherryMacros.h>

namespace cherry {

class Shell;

/**
 * Instances of this class are sent as a result of
 * operations being performed on shells.
 *
 * @see ShellListener
 * @see <a href="http://www.opencherry.org/swt/">Sample code and further information</a>
 */

class CHERRY_UI ShellEvent : public Object {

public:

  cherryClassMacro(ShellEvent);

  /**
   * A flag indicating whether the operation should be allowed.
   * Setting this field to <code>false</code> will cancel the operation.
   */
  bool doit;

  /**
   * Constructs a new instance of this class based on the
   * information in the given untyped event.
   *
   * @param e the untyped event containing the information
   */
  ShellEvent(SmartPointer<Shell> source) ;

  SmartPointer<Shell> GetSource();

private:

  SmartPointer<Shell> source;

};

}

#endif /* CHERRYSHELLEVENT_H_ */
