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
#include "enabled.xpm"
#include "disabled.xpm"

class QPushButton;
class QTextEdit;
class QHBox;
class QSlider;

namespace mitk
{
  class PropertyList;
}

class QmitkPropertyListViewItem : public QObject
{
  Q_OBJECT
public:
  QmitkPropertyListViewItem(std::string name, mitk::PropertyList* propertyList, QLabel* label, QWidget* control) :
  m_Name(name), m_PropertyList(propertyList), m_Label(label), m_Control(control),m_EnabledButton(NULL)  {}
  std::string m_Name;
  mitk::PropertyList* m_PropertyList;
  QPushButton* m_EnabledButton;
  QLabel* m_Label;
  QWidget* m_Control;
  static QmitkPropertyListViewItem* CreateInstance(mitk::PropertyList *propList, const std::string name, QWidget* parent, bool createOnlyControl);
  virtual void UpdateView();
  void QmitkPropertyListViewItem::UpdateEnabledView();
  // ~QmitkPropertyListViewItem()
public slots:
  void CheckBoxControlActivated(bool on);
  void StringControlActivated(const QString &text);
  void ColorControlActivated();
  void FloatControlActivated(const QString &text);
  void IntControlActivated(const QString &text);
protected:
  QmitkPropertyListViewItem() {}
  void CreateEnabledButton(QWidget* parent);
protected slots:
  void EnabledButtonClicked();
};

class QmitkPropertyListViewFloatSlider : public QmitkPropertyListViewItem
{
  Q_OBJECT
public:
  QmitkPropertyListViewFloatSlider(std::string name, mitk::PropertyList* propertyList, QLabel* label, QWidget* control) :
  QmitkPropertyListViewItem(name,propertyList,label,control),m_Slider(NULL), m_ValueLabel(NULL)  {}
  void UpdateView();
  QSlider* m_Slider;
  QLabel* m_ValueLabel;
protected slots:
  void SliderValueChanged(int value);
};
#endif

