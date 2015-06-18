/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYINTROCONTENTDETECTOR_H_
#define BERRYINTROCONTENTDETECTOR_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

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
struct BERRY_UI_QT IntroContentDetector : public Object
{

  berryObjectMacro(berry::IntroContentDetector)

  /**
   * Returns <code>true</code> if new intro content is available.
   *
   * @return <code>true</code> if new intro content is available
   */
  virtual bool IsNewContentAvailable() const = 0;

};

}

Q_DECLARE_INTERFACE(berry::IntroContentDetector, "org.blueberry.ui.IntroContentDetector")

#endif /* BERRYINTROCONTENTDETECTOR_H_ */
