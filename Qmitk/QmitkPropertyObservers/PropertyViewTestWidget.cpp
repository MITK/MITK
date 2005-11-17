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
#include <qapplication.h>

PropertyViewTest::PropertyViewTest(bool stay, QWidget* parent, const char* name)
:QWidget(parent,name),
 m_Stay(stay)
{
  QVBoxLayout* vl = new QVBoxLayout(this, QBoxLayout::TopToBottom);
 
  // string property
  props = new mitk::StringProperty("Juhu");
 
  // base property view for the string
  baseview = new QmitkBasePropertyView( props, this );
  vl->addWidget(baseview);
 
  // string property view for the string
  stringview = new QmitkStringPropertyView( props, this );
  vl->addWidget(stringview);
 
  // string property editor for the string
  stringeditor = new QmitkStringPropertyEditor( props, this );
  vl->addWidget(stringeditor);

  // color property
  propcol = new mitk::ColorProperty(0.5, 1.0, 0.25);

  // color prop view
  colorview = new QmitkColorPropertyView( propcol, this );
  vl->addWidget(colorview);
 
  // color prop editor
  coloreditor = new QmitkColorPropertyEditor( propcol, this );
  vl->addWidget(coloreditor);

  // bool property
  propb = new mitk::BoolProperty(true);

  // bool prop view
  boolview = new QmitkBoolPropertyView( propb, this );
  vl->addWidget(boolview);
 
  // bool prop editor
  booleditor = new QmitkBoolPropertyEditor( propb, this );
  vl->addWidget(booleditor);
  

  // finally, a timer that starts some testing
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
  delete booleditor;
  delete boolview;
  delete baseview;
  
  delete props;
  delete propcol;
  delete propb;
}

void PropertyViewTest::run() 
{
  assert( baseview->text() == "Juhu" );
  assert( stringview->text() == "Juhu" );
  assert( stringeditor->text() == "Juhu" );

  props->SetValue("Huhu Welt");
  assert( baseview->text() == "Huhu Welt" );
  assert( stringview->text() == "Huhu Welt" );
  assert( stringeditor->text() == "Huhu Welt" );

  assert( boolview->isOn() );
  assert( booleditor->isOn() );

  propb->SetValue(false);
  assert( !boolview->isOn() );
  assert( !booleditor->isOn() );

  if (!m_Stay)
    qApp->quit();
}
    
