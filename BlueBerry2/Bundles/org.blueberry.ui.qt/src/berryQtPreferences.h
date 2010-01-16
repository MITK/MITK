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


#ifndef BERRYQTPREFERENCES_H_
#define BERRYQTPREFERENCES_H_

#include "berryUiQtDll.h"

#include <string>

namespace berry {

struct BERRY_UI_QT QtPreferences
{
  static const std::string QT_STYLES_NODE; // = "qtstyles";
  static const std::string QT_STYLE_NAME; // = "stylename";
  static const std::string QT_STYLE_SEARCHPATHS; // = "searchpaths";
};
}

#endif /* BERRYQTPREFERENCES_H_ */
