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
#include "QmitkPropertyListViewItem.h"
#include "mitkPropertyList.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkPropertyManager.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include "mitkRenderWindow.h"
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qcolordialog.h>
#include <qvalidator.h>

QmitkPropertyListViewItem* QmitkPropertyListViewItem::CreateInstance(mitk::PropertyList *propList, const std::string name, QWidget* parent, bool createOnlyControl)
{
  QmitkPropertyListViewItem* newItem = new QmitkPropertyListViewItem(name,propList,NULL,NULL);
  mitk::PropertyList::PropertyMap::const_iterator it = newItem->m_PropertyList->GetMap()->find(newItem->m_Name.c_str());
  mitk::BaseProperty* baseProp = NULL;
  if (it != newItem->m_PropertyList->GetMap()->end())
  {
    baseProp = it->second.first;
  }
  if (!createOnlyControl)
  {
    newItem->m_EnabledButton = new QPushButton(parent);
    connect(
      (QObject*)(newItem->m_EnabledButton),
      SIGNAL(clicked()),
      (QObject*)(newItem),
      SLOT(EnabledButtonClicked())
    );
    newItem->m_Label = new QLabel(name.c_str(),parent);
  }
  if (mitk::BoolProperty* boolProp = dynamic_cast<mitk::BoolProperty*>(baseProp))
  {
    newItem->m_Control = new QCheckBox(parent);
    ((QCheckBox*)(newItem->m_Control))->setChecked(boolProp->GetValue());
    connect((QObject*)(newItem->m_Control),SIGNAL(toggled(bool)),(QObject*)(newItem),SLOT(CheckBoxControlActivated(bool)));
  }
  else if (mitk::StringProperty* stringProp = dynamic_cast<mitk::StringProperty*>(baseProp))
  {
    newItem->m_Control = new QLineEdit(QString(stringProp->GetValue()),parent);
    connect((QObject*)(newItem->m_Control),SIGNAL(textChanged(const QString &)),(QObject*)(newItem),SLOT(StringControlActivated(const QString &)));
  }
  else if (mitk::ColorProperty* colorProp = dynamic_cast<mitk::ColorProperty*>(baseProp))
  {
    newItem->m_Control = new QPushButton(parent);
    QPixmap pm(20,20);
    mitk::Color col = colorProp->GetColor();
    QColor qcol((int)(col.GetRed() * 255), (int)(col.GetGreen() * 255),(int)( col.GetBlue() * 255));
    pm.fill(qcol);
    // newItem->m_Control->setBackgroundPixmap(pm);
    newItem->m_Control->setPaletteBackgroundColor(qcol);
    connect((QObject*)(newItem->m_Control),SIGNAL(clicked()),(QObject*)(newItem),SLOT(ColorControlActivated()));
  }
  else if (mitk::FloatProperty* floatProp = dynamic_cast<mitk::FloatProperty*>(baseProp))
  {

    std::pair<float,float> minMax;
    QString text;
    text.setNum(floatProp->GetValue());
    newItem->m_Control = new QLineEdit(text,parent);
    if (mitk::PropertyManager::GetInstance()->GetDefaultLimits(newItem->m_Name,minMax))
    {
      ((QLineEdit*)(newItem->m_Control))->setValidator(new QDoubleValidator(minMax.first,minMax.second,3,newItem->m_Control));
    }
    else
    {
      ((QLineEdit*)(newItem->m_Control))->setValidator(new QIntValidator(std::numeric_limits<float>::min(),std::numeric_limits<float>::max(),newItem->m_Control));

    }
    connect((QObject*)(newItem->m_Control),SIGNAL(textChanged(const QString &)),(QObject*)(newItem),SLOT(FloatControlActivated(const QString &)));
  }
  else if (mitk::IntProperty* intProp = dynamic_cast<mitk::IntProperty*>(baseProp))
  {
    QString text;
    text.setNum(intProp->GetValue());
    newItem->m_Control = new QLineEdit(text,parent);
    std::pair<float,float> minMax;
    if (mitk::PropertyManager::GetInstance()->GetDefaultLimits(newItem->m_Name,minMax))
    {
      ((QLineEdit*)(newItem->m_Control))->setValidator(new QIntValidator((int)minMax.first,(int)minMax.second,newItem->m_Control));
    }
    else
    {
      ((QLineEdit*)(newItem->m_Control))->
      setValidator(new QIntValidator(std::numeric_limits<int>::min(),std::numeric_limits<int>::max(),newItem->m_Control));
    }
    connect((QObject*)(newItem->m_Control),SIGNAL(textChanged(const QString &)),(QObject*)(newItem),SLOT(IntControlActivated(const QString &)));
  }
  else
  {
    if (baseProp)
    {
      newItem->m_Control = new QLabel(QString(baseProp->GetValueAsString().c_str()),parent);
    }
    else
    {
      newItem->m_Control = new QLabel("n/a",parent);
    }
  }
  if (newItem->m_EnabledButton) { newItem->m_EnabledButton->show();  newItem->UpdateEnabledView(); }
  if (newItem->m_Label) newItem->m_Label->show();
  newItem->m_Control->show();
  return newItem;
}
void QmitkPropertyListViewItem::CheckBoxControlActivated(bool on)
{
  m_PropertyList->SetProperty(m_Name.c_str(), new mitk::BoolProperty(on));
  mitk::RenderWindow::UpdateAllInstances();
}

void QmitkPropertyListViewItem::StringControlActivated(const QString &text)
{
  std::cout << "setting string property to:" << text.ascii() << std::endl;
  m_PropertyList->SetProperty(m_Name.c_str(), new mitk::StringProperty(text.ascii()));
  mitk::RenderWindow::UpdateAllInstances();
}
void QmitkPropertyListViewItem::FloatControlActivated(const QString &text)
{
  if (((QLineEdit*)m_Control)->hasAcceptableInput())
  {
    m_Control->setPaletteForegroundColor(Qt::black);
    float value = text.toFloat();
    mitk::FloatProperty* floatProp = dynamic_cast<mitk::FloatProperty*>(m_PropertyList->GetProperty(m_Name.c_str()).GetPointer());
    if (value != floatProp->GetValue())
    {
      m_PropertyList->SetProperty(m_Name.c_str(), new mitk::FloatProperty(value));
    }
  }
  else
  {
    m_Control->setPaletteForegroundColor(Qt::red);
  }
  mitk::RenderWindow::UpdateAllInstances();
}
void QmitkPropertyListViewItem::IntControlActivated(const QString &text)
{
  if (((QLineEdit*)m_Control)->hasAcceptableInput())
  {
    m_Control->setPaletteForegroundColor(Qt::black);
    float value = text.toInt();
    mitk::IntProperty* intProp = dynamic_cast<mitk::IntProperty*>(m_PropertyList->GetProperty(m_Name.c_str()).GetPointer());
    if (value != intProp->GetValue())
    {
      m_PropertyList->SetProperty(m_Name.c_str(), new mitk::IntProperty(value));
    }
  }
  else
  {
    m_Control->setPaletteForegroundColor(Qt::red);
  }
  mitk::RenderWindow::UpdateAllInstances();
}
void QmitkPropertyListViewItem::ColorControlActivated()
{
  std::cout << "color control" << std::endl;
  mitk::ColorProperty* colorProp = dynamic_cast<mitk::ColorProperty*>(m_PropertyList->GetProperty(m_Name.c_str()).GetPointer());
  mitk::Color col = colorProp->GetColor();
  QColor result = QColorDialog::getColor(QColor((int)(col.GetRed() * 255), (int)(col.GetGreen() * 255), (int)(col.GetBlue() * 255)));
  if (result.isValid())
  {
    col.SetRed(result.red() / 255.0);
    col.SetGreen(result.green() / 255.0);
    col.SetBlue(result.blue() / 255.0);
    colorProp->SetColor(col);
    m_PropertyList->SetProperty(m_Name.c_str(), colorProp);
    m_Control->setPaletteBackgroundColor(result);
    mitk::RenderWindow::UpdateAllInstances();
  }
}
void QmitkPropertyListViewItem::UpdateView()
{
  m_Control->blockSignals(true);
  mitk::BaseProperty* baseProp = m_PropertyList->GetProperty(m_Name.c_str());
  if (mitk::BoolProperty* boolProp = dynamic_cast<mitk::BoolProperty*>(baseProp))
  {
    ((QCheckBox*)(m_Control))->setChecked(boolProp->GetValue());
  }
  else if (mitk::StringProperty* stringProp = dynamic_cast<mitk::StringProperty*>(baseProp))
  {
    ((QLineEdit*)(m_Control))->setText(QString(stringProp->GetValue()));
  }
  else if (mitk::FloatProperty* floatProp = dynamic_cast<mitk::FloatProperty*>(baseProp))
  {
    QString text;
    text.setNum(floatProp->GetValue());
    ((QLineEdit*)(m_Control))->setText(text);
  }

  else if (mitk::ColorProperty* colorProp = dynamic_cast<mitk::ColorProperty*>(baseProp))
  {
    mitk::Color col = colorProp->GetColor();
    QColor qcol((int)(col.GetRed() * 255), (int)(col.GetGreen() * 255),(int)( col.GetBlue() * 255));
    ((QPushButton*)(m_Control))->setPaletteBackgroundColor(qcol);
  }
  m_Control->blockSignals(false);
}
void QmitkPropertyListViewItem::UpdateEnabledView()
{
  static const QPixmap enabledPix((const char **)enabled_xpm);
  static const QPixmap disabledPix((const char **)disabled_xpm);
  mitk::BaseProperty* baseProp = m_PropertyList->GetProperty(m_Name.c_str());
  if (m_PropertyList->IsEnabled(m_Name.c_str())) /* baseProp->GetEnabled()) */
  {
    m_EnabledButton->setPixmap(enabledPix);
    if (m_Control) {m_Control->setEnabled(true);}
    if (m_Label) {m_Label->setEnabled(true);}
  }
  else
  {
    m_EnabledButton->setPixmap(disabledPix);
    if (m_Control) {m_Control->setEnabled(false);}
    if (m_Label) {m_Label->setEnabled(false);}
  }
}
void QmitkPropertyListViewItem::EnabledButtonClicked()
{
  mitk::BaseProperty* baseProp = m_PropertyList->GetProperty(m_Name.c_str());
  //baseProp->SetEnabled(! baseProp->GetEnabled());
  m_PropertyList->SetEnabled(m_Name.c_str(), ! m_PropertyList->IsEnabled(m_Name.c_str()));
  UpdateEnabledView();
  mitk::RenderWindow::UpdateAllInstances();
}
