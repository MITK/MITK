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

#ifndef CHERRYLOGVIEW_H_
#define CHERRYLOGVIEW_H_

#include <QtGui/QWidget>

#include "../cherryQtViewPart.h"

namespace cherry {

class LogView : public QtViewPart
{
public:

  void SetFocus();

protected:

  void CreateQtPartControl(QWidget* parent);

};

} // namespace cherry

#endif /*CHERRYLOGVIEW_H_*/
