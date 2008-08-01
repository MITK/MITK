/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkComplexExternalToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
  
QmitkComplexExternalToolGUI::QmitkComplexExternalToolGUI()
:QmitkToolGUI()
{
  // create the visible widgets
  QBoxLayout* layout = new QHBoxLayout( this );

  QLabel* label = new QLabel( "External tool's GUI", this );
  QFont f = label->font();
  f.setBold(false);
  label->setFont( f );
  layout->addWidget(label);
}

QmitkComplexExternalToolGUI::~QmitkComplexExternalToolGUI()
{

}

