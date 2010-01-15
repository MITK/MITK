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


#ifndef CHERRYQTPREFERENCES_H_
#define CHERRYQTPREFERENCES_H_

#include "cherryUiQtDll.h"

#include <string>

namespace cherry {

struct CHERRY_UI_QT QtPreferences
{
  static const std::string QT_STYLES_NODE; // = "qtstyles";
  static const std::string QT_STYLE_NAME; // = "stylename";
  static const std::string QT_STYLE_SEARCHPATHS; // = "searchpaths";
};
}

#endif /* CHERRYQTPREFERENCES_H_ */
