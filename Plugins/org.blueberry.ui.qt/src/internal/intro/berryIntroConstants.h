/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYINTROCONSTANTS_H_
#define BERRYINTROCONSTANTS_H_

#include <QString>

namespace berry {

/**
 * Contains constants used by the intro implementation
 */
struct IntroConstants {

  /**
   * The id of the view that is used as the intro host.
   */
  static const QString INTRO_VIEW_ID; // = "org.blueberry.ui.internal.introview";

  /**
   * The id of the editor that is used as the intro host.
   */
  static const QString INTRO_EDITOR_ID; // = "org.blueberry.ui.internal.introeditor";

  /**
   * Constant defining the realization of the intro part as a view.
   */
  static const int INTRO_ROLE_VIEW; // = 0x01;

  /**
   * Constant defining the realization of the intro part as an editor.
   */
  static const int INTRO_ROLE_EDITOR; // = 0x02;

};


}

#endif /* BERRYINTROCONSTANTS_H_ */
