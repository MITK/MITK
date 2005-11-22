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
  propstring = new mitk::StringProperty("Juhu");
 
  // base property view for the string
  baseview = new QmitkBasePropertyView( propstring, this );
  vl->addWidget(baseview);
 
  // string property view for the string
  stringview = new QmitkStringPropertyView( propstring, this );
  vl->addWidget(stringview);
 
  // string property editor for the string
  stringeditor = new QmitkStringPropertyEditor( propstring, this );
  vl->addWidget(stringeditor);

  // color property
  propcolor = new mitk::ColorProperty(0.5, 1.0, 0.25);

  // color prop view
  colorview = new QmitkColorPropertyView( propcolor, this );
  vl->addWidget(colorview);
 
  // color prop editor
  coloreditor = new QmitkColorPropertyEditor( propcolor, this );
  vl->addWidget(coloreditor);

  // bool property
  propbool = new mitk::BoolProperty(true);

  // bool prop view
  boolview = new QmitkBoolPropertyView( propbool, this );
  vl->addWidget(boolview);
 
  // bool prop editor
  booleditor = new QmitkBoolPropertyEditor( propbool, this );
  vl->addWidget(booleditor);
  
  //propshort = new mitk::GenericProperty<short>(3);  // see comment below
  propint = new mitk::IntProperty(4);
  propfloat = new mitk::FloatProperty(4.5);
  propdouble = new mitk::DoubleProperty(4.75);

  // not possible because lack of: bool mitk::XMLReader::GetAttribute(std::basic_str, short int&)
  // this is nedded in DataStructures/mitkProperties/mitkGenericProperty.h, l. 86
  //numberview = new QmitkNumberPropertyView( propshort, this );
  //vl->addWidget(numberview);
  
  numberview = new QmitkNumberPropertyView( propint, this );
  numberview->setDecimalPlaces(4);
  vl->addWidget(numberview);
  numbereditor = new QmitkNumberPropertyEditor( propint, this );
  numbereditor->setDecimalPlaces(4);
  vl->addWidget(numbereditor);

  numberview = new QmitkNumberPropertyView( propfloat, this );
  numberview->setDecimalPlaces(3);
  vl->addWidget(numberview);
  numbereditor = new QmitkNumberPropertyEditor( propfloat, this );
  numbereditor->setDecimalPlaces(3);
  vl->addWidget(numbereditor);

  numberview = new QmitkNumberPropertyView( propdouble, this );
  vl->addWidget(numberview);
  numbereditor = new QmitkNumberPropertyEditor( propdouble, this );
  vl->addWidget(numbereditor);


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
  
  delete propstring;
    delete propcolor;
    delete propbool;
  }

  void PropertyViewTest::run() 
  {
    assert( baseview->text() == "Juhu" );
    assert( stringview->text() == "Juhu" );
    assert( stringeditor->text() == "Juhu" );

    propstring->SetValue("Huhu Welt");
    assert( baseview->text() == "Huhu Welt" );
    assert( stringview->text() == "Huhu Welt" );
    assert( stringeditor->text() == "Huhu Welt" );

    assert( boolview->isOn() );
    assert( booleditor->isOn() );

    propbool->SetValue(false);
  assert( !boolview->isOn() );
  assert( !booleditor->isOn() );

  if (!m_Stay)
    qApp->quit();
}
    
