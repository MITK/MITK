/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include "PropertyViewTestWidget.h"
#include <assert.h>
#include <qlayout.h>

PropertyViewTest::PropertyViewTest(QWidget* parent, const char* name)
:QWidget(parent,name)
{
  QVBoxLayout* vl = new QVBoxLayout(this, QBoxLayout::TopToBottom);
  props = new mitk::StringProperty::StringProperty("Juhu");
  
  baseview = new QmitkBasePropertyView( props, this );
  vl->addWidget(baseview);
  
  stringview = new QmitkStringPropertyView( props, this );
  vl->addWidget(stringview);
  
  propcol = new mitk::ColorProperty::ColorProperty(0.5, 1.0, 0.25);
  colorview = new QmitkColorPropertyView( propcol, this );
  vl->addWidget(colorview);
  
  coloreditor = new QmitkColorPropertyEditor( propcol, this );
  vl->addWidget(coloreditor);
  
  stringeditor = new QmitkStringPropertyEditor( props, this );
  vl->addWidget(stringeditor);
  
  timer = new QTimer(this);
  connect ( timer, SIGNAL(timeout()), this, SLOT(run()) );
  timer->start(0, TRUE);
}

PropertyViewTest::~PropertyViewTest() 
{
  delete timer;
  delete colorview;
  delete coloreditor;
  delete stringeditor;
  delete stringview;
  delete baseview;
  delete props;
  delete propcol;
}

void PropertyViewTest::run() 
{
  assert( baseview->text() == "Juhu" );

  props->SetValue("Huhu Welt");
  assert( baseview->text() == "Huhu Welt" );
  assert( stringview->text() == "Huhu Welt" );
  assert( stringeditor->text() == "Huhu Welt" );

}
    
