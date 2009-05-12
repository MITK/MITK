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

/**
  This is a mini-application to demonstrate and test the PropertyObserver
  classes in one place without the whole lot of SampleApp stuff around.
  */

#include "PropertyViewTestWidget.h"

#include <QmitkBasePropertyView.h>
#include <QmitkBoolPropertyView.h>
#include <QmitkBoolPropertyEditor.h>
#include <QmitkStringPropertyView.h>
#include <QmitkStringPropertyEditor.h>
#include <QmitkColorPropertyView.h>
#include <QmitkColorPropertyEditor.h>
#include <QmitkNumberPropertyView.h>
#include <QmitkNumberPropertyEditor.h>

#include <mitkDataTreeFilterFunctions.h>
#include <QmitkDataTreeListView.h>
#include <QmitkDataTreeComboBox.h>

#include <mitkImage.h>

#include <assert.h>
#include <qlayout.h>
#include <qapplication.h>

#include <qlistview.h>

PropertyViewTest::PropertyViewTest(bool stay, QWidget* parent, const char* name)
:QWidget(parent,name),
 m_Stay(stay)
{
/**/
  QHBoxLayout* hl = new QHBoxLayout(this, QBoxLayout::LeftToRight);
  QVBoxLayout* vl = new QVBoxLayout(hl, QBoxLayout::TopToBottom);
 
  // string property
  propstring = mitk::StringProperty::New("Juhu");
 
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
  propcolor = mitk::ColorProperty::New(0.5, 1.0, 0.25);

  // color prop editor
  coloreditor = new QmitkColorPropertyEditor( propcolor, this );
  vl->addWidget(coloreditor);

  // bool property
  propbool = mitk::BoolProperty::New(true);

  // bool prop view
  boolview = new QmitkBoolPropertyView( propbool, this );
  vl->addWidget(boolview);
 
  // bool prop editor
  booleditor = new QmitkBoolPropertyEditor( propbool, this );
  vl->addWidget(booleditor);
  
  //propshort = new mitk::GenericProperty<short>(3);  // see comment below
  propint = mitk::IntProperty::New(4);
  propfloat = mitk::FloatProperty::New(4.5);
  propdouble = mitk::DoubleProperty::New(4.75);

  // not possible because lack of: bool mitk::XMLReader::GetAttribute(std::basic_str, short int&)
  // this is nedded in DataStructures/mitkProperties/mitkGenericProperty.h, l. 86
  //numberview = new QmitkNumberPropertyView( propshort, this );
  //vl->addWidget(numberview);
  
  numberview1 = new QmitkNumberPropertyView( propint, this );
  numberview1->setDecimalPlaces(4);
  vl->addWidget(numberview1);
  numbereditor1 = new QmitkNumberPropertyEditor( propint, this );
  numbereditor1->setDecimalPlaces(4);
  vl->addWidget(numbereditor1);

  numberview2 = new QmitkNumberPropertyView( propfloat, this );
  numberview2->setDecimalPlaces(3);
  vl->addWidget(numberview2);
  numbereditor2 = new QmitkNumberPropertyEditor( propfloat, this );
  numbereditor2->setDecimalPlaces(3);
  vl->addWidget(numbereditor2);
  numbereditor3 = new QmitkNumberPropertyEditor( propfloat, this );
  numbereditor3->setDecimalPlaces(2);
  vl->addWidget(numbereditor3);
  numbereditor4 = new QmitkNumberPropertyEditor( propfloat, this );
  vl->addWidget(numbereditor4);
  numberview3 = new QmitkNumberPropertyView( propfloat, this );
  numberview3->setDecimalPlaces(2);
  vl->addWidget(numberview3);

  numberview4 = new QmitkNumberPropertyView( propdouble, this );
  vl->addWidget(numberview4);
  numbereditor5 = new QmitkNumberPropertyEditor( propdouble, this );
  numbereditor5->setDecimalPlaces(4);
  vl->addWidget(numbereditor5);
  numbereditor6 = new QmitkNumberPropertyEditor( propdouble, this );
  vl->addWidget(numbereditor6);
 
  // percents
  propfloat2 = mitk::FloatProperty::New(0.43);
  numberview5 = new QmitkNumberPropertyView( propfloat2, this );
  vl->addWidget(numberview5);
  
  numbereditor7 = new QmitkNumberPropertyEditor( propfloat2, this );
  numbereditor7->setShowPercent(true);
  vl->addWidget(numbereditor7);
  numbereditor8 = new QmitkNumberPropertyEditor( propfloat2, this );
  numbereditor8->setShowPercent(true);
  numbereditor8->setDecimalPlaces(1);
  vl->addWidget(numbereditor8);
/**/
  prepare_tree();
  
//QVBoxLayout* vlright = new QVBoxLayout(this, QBoxLayout::TopToBottom);
  
  //treelistview = new QmitkDataTreeListView(tree_filter, this);
  treelistview = new QmitkDataTreeListView(this);
  treelistview->SetFilter(tree_filter);
  // alternative constructor, will create default filter
  //treelistview = new QmitkDataTreeListView(data_tree, this);
  treelistview->setStretchedColumn(2);

//vlright->addWidget(treelistview);
  
/**/
  QVBoxLayout* vlright = new QVBoxLayout(hl, QBoxLayout::TopToBottom);
  //hl->addWidget( vlright );
  
  vlright->addWidget( treelistview );
  
  // color prop view
  colorview = new QmitkColorPropertyView( propcolor, this );
  //vlright->addWidget(colorview, 10);
  vlright->addWidget(colorview);
  
  treecombobox = new QmitkDataTreeComboBox(tree_filter, this);
  vlright->addWidget( treecombobox );

  // finally, a timer that starts some testing
  timer = new QTimer(this);
  connect ( timer, SIGNAL(timeout()), this, SLOT(run()) );
  timer->start(0, TRUE);
/**/
}

void PropertyViewTest::prepare_tree()
{
  data_tree = mitk::DataTree::New();
  // create some test data
  mitk::Image::Pointer image;
  mitk::PixelType pt(typeid(int));
  unsigned int dim[]={10,10,20}; // image dimensions

  image = mitk::Image::New();
  image->Initialize(mitk::PixelType(typeid(int)), 3, dim);
  int *p = (int*)image->GetData(); // pointer to pixel data
  int size = dim[0]*dim[1]*dim[2];
  for(int i=0; i<size; ++i, ++p) *p=i; // fill image
        
  {
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
    node->SetData(image);
  }

  mitk::DataTreePreOrderIterator it(data_tree);
/**/
  {
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); node->SetData(image);
    node->SetProperty("name", mitk::StringProperty::New("Opa"));
//    node->SetProperty("opacity", mitk::FloatProperty::New(0.8));
    it.Set(node);
  }
/**/
  {
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); node->SetData(image);
    node->SetProperty("name", mitk::StringProperty::New("Bruder"));
    node->SetProperty("visible", mitk::BoolProperty::New(false));
    node->SetProperty("color", mitk::ColorProperty::New(1.0,0.0,0.0));
    node->SetProperty("opacity", mitk::FloatProperty::New(0.8));
    it.Add(node);
  }
  {
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); node->SetData(image);
    node->SetProperty("name", mitk::StringProperty::New("Ich"));
    node->SetProperty("color", mitk::ColorProperty::New(0.0,0.0,1.0));
    node->SetProperty("opacity", mitk::FloatProperty::New(0.8));
    it.Add(node);
  }
  {
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); node->SetData(image);
    node->SetProperty("name", mitk::StringProperty::New("Schwester"));
    node->SetProperty("color", mitk::ColorProperty::New(0.0,1.0,0.0));
    node->SetProperty("opacity", mitk::FloatProperty::New(0.8));
    it.Add(node);
  }

  it.GoToChild(1);
  
  {
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); node->SetData(image);
    node->SetProperty("name", mitk::StringProperty::New("Sohn"));
//    node->SetProperty("opacity", mitk::FloatProperty::New(0.8));
    it.Add(node);
  }
  {
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); node->SetData(image);
    node->SetProperty("name", mitk::StringProperty::New("Tochter"));
    node->SetProperty("visible", mitk::BoolProperty::New(false));
    node->SetProperty("opacity", mitk::FloatProperty::New(0.8));
    it.Add(node);
  }

  mitk::DataTreeFilter::PropertyList visible_props;
  visible_props.push_back("visible");
  visible_props.push_back("name");
  visible_props.push_back("color");
  visible_props.push_back("opacity");
  mitk::DataTreeFilter::PropertyList editable_props;
  editable_props.push_back("visible");
//  editable_props.push_back("name");
  editable_props.push_back("opacity");
  editable_props.push_back("color");
  
  tree_filter = mitk::DataTreeFilter::New(data_tree);
  tree_filter->SetSelectionMode(mitk::DataTreeFilter::SINGLE_SELECT);
  tree_filter->SetHierarchyHandling(mitk::DataTreeFilter::FLATTEN_HIERARCHY);
  tree_filter->SetFilter(mitk::IsGoodDataTreeNode());
  tree_filter->SetVisibleProperties(visible_props);
  tree_filter->SetEditableProperties(editable_props);
          
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

  delete numberview1;
  delete numberview2;
  delete numberview3;
  delete numberview4;
  delete numberview5;
  delete numbereditor1;
  delete numbereditor2;
  delete numbereditor3;
  delete numbereditor4;
  delete numbereditor5;
  delete numbereditor6;
  delete numbereditor7;
  delete numbereditor8;
  
  delete propstring;
  delete propcolor;
  delete propbool;
  delete propfloat;
  delete propfloat2;

  delete treelistview;
  delete treecombobox;
  
  tree_filter = 0;
  data_tree = 0;
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

    //propbool->SetValue(false);
    //assert( !boolview->isOn() );
    //assert( !booleditor->isOn() );

  if (!m_Stay)
    qApp->quit();
}
    
