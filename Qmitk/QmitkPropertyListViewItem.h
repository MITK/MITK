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

#ifndef QMITK_PROPERTYLISTVIEWITEM_INCLUDED
#define QMITK_PROPERTYLISTVIEWITEM_INCLUDED
#include <qlabel.h>
#include <qwidget.h>
#include <qobject.h>
#include <string>

namespace mitk {
  class PropertyList;
}

class QmitkPropertyListViewItem : public QObject {
  Q_OBJECT
  public:
    QmitkPropertyListViewItem(std::string name, mitk::PropertyList* propertyList, QLabel* label, QWidget* control) :
      m_Name(name), m_PropertyList(propertyList), m_Label(label), m_Control(control) {}
    std::string m_Name;
    mitk::PropertyList* m_PropertyList;
    QLabel* m_Label;
    QWidget* m_Control;
    static QmitkPropertyListViewItem* CreateInstance(mitk::PropertyList *propList, const std::string name, QWidget* parent);
    // ~QmitkPropertyListViewItem()
  public slots:
    void CheckBoxControlActivated(int state);
    void StringControlActivated(const QString &text);
    void ColorControlActivated();
  protected:
    QmitkPropertyListViewItem() {}

};
#endif

