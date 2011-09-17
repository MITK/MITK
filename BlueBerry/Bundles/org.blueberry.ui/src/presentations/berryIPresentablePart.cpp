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

#include "berryIPresentablePart.h"

#include "../berryIWorkbenchPartConstants.h"

namespace berry
{

const int IPresentablePart::PROP_DIRTY = IWorkbenchPartConstants::PROP_DIRTY;
const int IPresentablePart::PROP_INPUT = IWorkbenchPartConstants::PROP_INPUT;
const int IPresentablePart::PROP_TITLE = IWorkbenchPartConstants::PROP_TITLE;
const int IPresentablePart::PROP_CONTENT_DESCRIPTION =
    IWorkbenchPartConstants::PROP_CONTENT_DESCRIPTION;
const int IPresentablePart::PROP_PART_NAME =
    IWorkbenchPartConstants::PROP_PART_NAME;
const int IPresentablePart::PROP_BUSY = 0x92;
const int IPresentablePart::PROP_TOOLBAR = 0x93;
const int IPresentablePart::PROP_HIGHLIGHT_IF_BACK = 0x94;
const int IPresentablePart::PROP_PANE_MENU = 0x302;
const int IPresentablePart::PROP_PREFERRED_SIZE =
    IWorkbenchPartConstants::PROP_PREFERRED_SIZE;

}
