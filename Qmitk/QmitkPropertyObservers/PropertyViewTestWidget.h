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
#include <qwidget.h>
#include <qtimer.h>

#include <mitkProperties.h>
#include <mitkDataTree.h>
#include "mitkDataTreeFilter.h"

class QmitkBasePropertyView;
class QmitkBoolPropertyView;
class QmitkBoolPropertyEditor;
class QmitkStringPropertyView;
class QmitkStringPropertyEditor;
class QmitkColorPropertyView;
class QmitkColorPropertyEditor;
class QmitkNumberPropertyView;
class QmitkNumberPropertyEditor;
class QmitkDataTreeListView;
class QmitkDataTreeComboBox;

class QMITK_EXPORT PropertyViewTest : public QWidget
{
  Q_OBJECT

  public:

    PropertyViewTest(bool stay, QWidget* parent, const char* name = 0);

    virtual ~PropertyViewTest();

    void prepare_tree();
    
  public slots:
    
    void run();
    
  protected:

  private:
  
   QmitkBasePropertyView* baseview;
   QmitkBoolPropertyView* boolview;
   QmitkBoolPropertyEditor* booleditor;
   QmitkStringPropertyView* stringview;
   QmitkStringPropertyEditor* stringeditor;
   QmitkColorPropertyView* colorview;
   QmitkColorPropertyEditor* coloreditor;
   QmitkNumberPropertyView* numberview1;
   QmitkNumberPropertyView* numberview2;
   QmitkNumberPropertyView* numberview3;
   QmitkNumberPropertyView* numberview4;
   QmitkNumberPropertyView* numberview5;
   QmitkNumberPropertyView* numberview6;
   QmitkNumberPropertyEditor* numbereditor1;
   QmitkNumberPropertyEditor* numbereditor2;
   QmitkNumberPropertyEditor* numbereditor3;
   QmitkNumberPropertyEditor* numbereditor4;
   QmitkNumberPropertyEditor* numbereditor5;
   QmitkNumberPropertyEditor* numbereditor6;
   QmitkNumberPropertyEditor* numbereditor7;
   QmitkNumberPropertyEditor* numbereditor8;
   
   mitk::BoolProperty* propbool;
   mitk::StringProperty* propstring;
   mitk::ColorProperty* propcolor;
   mitk::FloatProperty* propfloat;
   mitk::FloatProperty* propfloat2;
   mitk::DoubleProperty* propdouble;
   mitk::IntProperty* propint;
   //mitk::GenericProperty<short>* propshort;

   mitk::DataTree::Pointer data_tree;
   mitk::DataTreeFilter::Pointer tree_filter;

   QmitkDataTreeListView* treelistview;
   QmitkDataTreeComboBox* treecombobox;

   QTimer* timer;

   bool m_Stay;
};

