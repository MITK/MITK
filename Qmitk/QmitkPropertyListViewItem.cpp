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
// Qmitk related includes
#include "QmitkPropertyListViewItem.h"
#include "QmitkPropertyListViewItemFactory.h"
#include "QmitkMaterialEditor.h"

// mitk related includes
#include "mitkConfig.h"
#include "mitkRenderWindow.h"
#include "mitkPropertyList.h"
#include "mitkProperties.h"
#include "mitkColorProperty.h"
#include "mitkPropertyManager.h"
#include "mitkRenderingManager.h"
#include "mitkEnumerationProperty.h"
#include "mitkMaterialProperty.h"


// QT related includes
#include <qapplication.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qcolordialog.h>
#include <qvalidator.h>
#include <qhbox.h>
#include <qslider.h>
#include <qcombobox.h>

#include "enabled.xpm"
#include "disabled.xpm"


QmitkPropertyListViewItem::QmitkPropertyListViewItem(std::string name, mitk::PropertyList* propertyList, QWidget* /*parent*/, bool /*createOnlyControl*/) 
  : m_Name(name), m_PropertyList(propertyList), m_Label(NULL), m_Control(NULL)
{
}

void QmitkPropertyListViewItem::CreateEnabledButton(QWidget* parent)
{
  m_EnabledButton = new QPushButton(parent);
  connect(
      (QObject*)(m_EnabledButton),
      SIGNAL(clicked()),
      (QObject*)(this),
      SLOT(EnabledButtonClicked())
      );
  m_EnabledButton->show();
  UpdateEnabledView();
}


QmitkPropertyListViewItem* QmitkPropertyListViewItem::CreateInstance(mitk::PropertyList *propList, const std::string name, QWidget* parent, bool createOnlyControl)
{
  return QmitkPropertyListViewItemFactory::GetInstance()->CreateQmitkPropertyListViewItem(propList,name,parent,createOnlyControl);
}


void QmitkPropertyListViewItem::CheckBoxControlActivated(bool on)
{
  m_PropertyList->SetProperty(m_Name.c_str(), new mitk::BoolProperty(on));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkPropertyListViewItem::StringControlActivated(const QString &text)
{
  m_PropertyList->SetProperty(m_Name.c_str(), new mitk::StringProperty(text.ascii()));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkPropertyListViewItem::FloatControlActivated(const QString &text)
{
  if (((QLineEdit*)m_Control)->hasAcceptableInput())
  {
    m_Control->setPaletteForegroundColor(QApplication::palette().active().foreground());
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
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkPropertyListViewItem::IntControlActivated(const QString &text)
{
  if (((QLineEdit*)m_Control)->hasAcceptableInput())
  {
    m_Control->setPaletteForegroundColor(QApplication::palette().active().foreground());
    int value = text.toInt();
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
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkPropertyListViewItem::ColorControlActivated()
{
  mitk::ColorProperty* colorProp = dynamic_cast<mitk::ColorProperty*>(m_PropertyList->GetProperty(m_Name.c_str()).GetPointer());
  mitk::Color col = colorProp->GetColor();
  QColor result = QColorDialog::getColor(QColor((int)(col.GetRed() * 255), (int)(col.GetGreen() * 255), (int)(col.GetBlue() * 255)));
  if (result.isValid())
  {
    col.SetRed(result.red() / 255.0);
    col.SetGreen(result.green() / 255.0);
    col.SetBlue(result.blue() / 255.0);
    colorProp->SetColor(col);
    m_PropertyList->InvokeEvent(itk::ModifiedEvent());
    m_Control->setPaletteBackgroundColor(result);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkPropertyListViewItem::UpdateView()
{
  m_Control->blockSignals(true);
  mitk::BaseProperty* baseProp = m_PropertyList->GetProperty(m_Name.c_str());
  if (mitk::BoolProperty* boolProp = dynamic_cast<mitk::BoolProperty*>(baseProp))
  {
    if (QCheckBox* cb = dynamic_cast<QCheckBox*>(m_Control)) {
      cb ->setChecked(boolProp->GetValue());
    } else {
      //std::cout << "warning: non-checkbox control for bool property " << m_Name << std::endl;
    }
  }
  else if (mitk::StringProperty* stringProp = dynamic_cast<mitk::StringProperty*>(baseProp))
  {
    if (QLineEdit* qle = dynamic_cast<QLineEdit*>(m_Control)) {
      qle->setText(QString(stringProp->GetValue()));
    } else {
      //std::cout << "warning: non-lineedit control for string property " << m_Name << std::endl;
    }

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

  else if (mitk::EnumerationProperty* enumerationProp = dynamic_cast<mitk::EnumerationProperty*>(baseProp))
  {
    QComboBox* combo = ( ( QComboBox* ) m_Control );
    std::string enumerationValue = enumerationProp->GetValueAsString();
    for ( int item = 0 ; item < combo->count() ; ++item )
    {
      if ( enumerationValue == combo->text( item ).latin1() )
      {
        combo->setCurrentItem( item );
        break;
      }
    }
  }
  m_Control->blockSignals(false);
}


void QmitkPropertyListViewItem::UpdateEnabledView()
{
  static const QPixmap enabledPix((const char **)enabled_xpm);
  static const QPixmap disabledPix((const char **)disabled_xpm);
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
  //baseProp->SetEnabled(! baseProp->GetEnabled());
  m_PropertyList->SetEnabled(m_Name.c_str(), ! m_PropertyList->IsEnabled(m_Name.c_str()));
  UpdateEnabledView();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkPropertyListViewItem::ComboBoxItemActivated(const QString &item)
{
  mitk::EnumerationProperty* enumProp = dynamic_cast<mitk::EnumerationProperty*>(m_PropertyList->GetProperty(m_Name.c_str()).GetPointer());
  if ( enumProp != NULL )
  {
    std::string activatedItem( item.latin1() );
    if ( activatedItem != enumProp->GetValueAsString() )
    {
      if ( enumProp->IsValidEnumerationValue( activatedItem ) )
      {
        enumProp->SetValue( activatedItem );
        m_PropertyList->InvokeEvent( itk::ModifiedEvent() );
        m_PropertyList->Modified();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
    }
  }
}


void  QmitkPropertyListViewFloatSlider::SliderValueChanged(int value)
{
  m_PropertyList->SetProperty(m_Name.c_str(), new mitk::FloatProperty(value / 100.0f));
  UpdateView();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkPropertyListViewFloatSlider::UpdateView()
{
  m_Slider->blockSignals(true);
  mitk::FloatProperty* floatProp = dynamic_cast<mitk::FloatProperty*>(m_PropertyList->GetProperty(m_Name.c_str()).GetPointer());
  if (floatProp)
  {
    QString text;
    text.setNum(floatProp->GetValue(),'f',2);
    m_ValueLabel->setText(text);
    m_Slider->setValue((int)(floatProp->GetValue() * 100));
  }
  m_Slider->blockSignals(false);
}


void QmitkPropertyListViewItem::MaterialEditorActivated()
{
  if ( mitk::MaterialProperty* materialProperty = dynamic_cast<mitk::MaterialProperty*>(m_PropertyList->GetProperty(m_Name.c_str()).GetPointer()))
  {
    QmitkMaterialEditor* materialEditor = new QmitkMaterialEditor( NULL );
    materialEditor->Initialize( materialProperty );
    if ( materialEditor->exec() == QDialog::Accepted )
    {
      m_PropertyList->InvokeEvent(itk::ModifiedEvent());
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    delete materialEditor;
  }
}

