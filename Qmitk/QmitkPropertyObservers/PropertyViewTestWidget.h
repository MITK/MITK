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

#include <QmitkBasePropertyView.h>
#include <QmitkBoolPropertyView.h>
#include <QmitkBoolPropertyEditor.h>
#include <QmitkStringPropertyView.h>
#include <QmitkStringPropertyEditor.h>
#include <QmitkColorPropertyView.h>
#include <QmitkColorPropertyEditor.h>

#include <qtimer.h>

class PropertyViewTest : public QWidget
{
  Q_OBJECT

  public:

    PropertyViewTest(bool stay, QWidget* parent, const char* name = 0);

    virtual ~PropertyViewTest();
    
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
   QmitkColorPropertyView* coloreditor;
   mitk::BoolProperty* propb;
   mitk::StringProperty* props;
   mitk::ColorProperty* propcol;
   QTimer* timer;

   bool m_Stay;
};

