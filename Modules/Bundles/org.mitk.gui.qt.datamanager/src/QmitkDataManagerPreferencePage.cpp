/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-07-07 16:57:15 +0200 (Di, 07 Jul 2009) $
 Version:   $Revision: 18019 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkDataManagerPreferencePage.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

QmitkDataManagerPreferencePage::QmitkDataManagerPreferencePage( QWidget* parent, Qt::WindowFlags f )
: IQtPreferencePage(parent, f)
{
}

bool QmitkDataManagerPreferencePage::PerformOk()
{
  return true;
}

void QmitkDataManagerPreferencePage::PerformCancel()
{

}

void QmitkDataManagerPreferencePage::Update()
{

}