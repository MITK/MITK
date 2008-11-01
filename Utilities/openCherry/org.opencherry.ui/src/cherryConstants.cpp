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

#include "cherryConstants.h"

namespace cherry
{

const int Constants::NONE = 0;
const int Constants::DEFAULT = -1;
const int Constants::DRAG = 1;
const int Constants::CENTER = 1 << 24;
const int Constants::HORIZONTAL = 1 << 8;
const int Constants::VERTICAL = 1 << 9;
const int Constants::MIN = 1 << 7;
const int Constants::MAX = 1 << 10;
const int Constants::UP = 1 << 7;
const int Constants::TOP = Constants::UP;
const int Constants::DOWN = 1 << 10;
const int Constants::BOTTOM = Constants::DOWN;
const int Constants::LEAD = 1 << 14;
const int Constants::LEFT = Constants::LEAD;
const int Constants::TRAIL = 1 << 17;
const int Constants::RIGHT = Constants::TRAIL;
const int Constants::FILL = 4;
const int Constants::WRAP = 1 << 6;
const int Constants::BORDER = 1 << 11;
const int Constants::CLOSE = 1 << 6;
const int Constants::RESIZE = 1 << 4;
const int Constants::TITLE = 1 << 5;
const int Constants::SHELL_TRIM = Constants::CLOSE | Constants::TITLE | Constants::MIN | Constants::MAX | Constants::RESIZE;
const int Constants::DIALOG_TRIM = Constants::TITLE | Constants::CLOSE | Constants::BORDER;
const int Constants::MODELESS = 0;
const int Constants::PRIMARY_MODAL = 1 << 15;
const int Constants::APPLICATION_MODAL = 1 << 16;
const int Constants::SYSTEM_MODAL = 1 << 17;

}
