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


#ifndef CHERRYINTROCONTENTDETECTOR_H_
#define CHERRYINTROCONTENTDETECTOR_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "../cherryUiDll.h"

namespace cherry {

/**
 * An intro content detector is used when starting the Workbench to determine if
 * new intro content is available. Since calling this method is part of the
 * Workbench start sequence, subclasses should be implemented with care as not
 * to introduce noticeable delay at startup. If an intro content detector
 * reports new available content, the view part showing the content will be
 * opened again even if the user had closed it in a previous session. Because of
 * this, the intro view part should draw the user's attention to the new content
 * to avoid confusion about why the intro view part was opened again without the
 * user requesting it.
 *
 */
struct CHERRY_UI IntroContentDetector : public Object {

  cherryInterfaceMacro(IntroContentDetector, cherry)

  /**
   * Returns <code>true</code> if new intro content is available.
   *
   * @return <code>true</code> if new intro content is available
   */
  virtual bool IsNewContentAvailable() const = 0;

};

}

#endif /* CHERRYINTROCONTENTDETECTOR_H_ */
