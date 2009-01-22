/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision: 14610 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>

#include "@VIEW_CLASS_H@"

@BEGIN_NAMESPACE@

void @VIEW_CLASS@::CreateQtPartControl(QWidget* parent)
{
  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0,0,0,0);
  m_ButtonStart = new QPushButton("start", parent);
  layout->addWidget(m_ButtonStart);
}

void @VIEW_CLASS@::SetFocus()
{
	m_ButtonStart->setFocus();
}

@END_NAMESPACE@
