/*=========================================================================
 
 Program:   BlueBerry Platform
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


#ifndef BERRYINTROCONSTANTS_H_
#define BERRYINTROCONSTANTS_H_

#include <string>

namespace berry {

/**
 * Contains constants used by the intro implementation
 *
 * @since 3.0
 */
struct IntroConstants {

  /**
   * The id of the view that is used as the intro host.
   */
  static const std::string INTRO_VIEW_ID; // = "org.blueberry.ui.internal.introview";

  /**
   * The id of the editor that is used as the intro host.
   */
  static const std::string INTRO_EDITOR_ID; // = "org.blueberry.ui.internal.introeditor";

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
