/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <QmitkProgressBar.h>
#include <mitkProgressBar.h>

void QmitkControlsLeftFctLayoutTemplate::init()
{
  MainSplitter->setResizeMode(ControlParent,QSplitter::Stretch);
}


void QmitkControlsLeftFctLayoutTemplate::setControlSizeHint(QSize size)
{
  QValueList<int> sizes;
  int w = MainSplitter->handleWidth();
  sizes.append(size.width());
  sizes.append( width() - size.width() - w );
  MainSplitter->setSizes(sizes);
}
