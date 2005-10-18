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
#include "mitkRenderingManager.h"
#include "mitkEnumerationProperty.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include "mitkRenderWindow.h"
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qcolordialog.h>
#include <qvalidator.h>
#include <qhbox.h>
#include <qslider.h>
#include <qcombobox.h>


QmitkPropertyListViewItem::QmitkPropertyListViewItem(std::string name, mitk::PropertyList* propertyList, QWidget* parent, bool createOnlyControl) : m_Name(name), m_PropertyList(propertyList), m_Control(NULL), m_Label(NULL)
{
  if (!createOnlyControl)
  {
    CreateEnabledButton(parent);
    m_Label = new QLabel(name.c_str(),parent);
    m_Label->show();
  }
};

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
  QmitkPropertyListViewItem* newItem = NULL;

  mitk::PropertyList::PropertyMap::const_iterator it = propList->GetMap()->find(name.c_str());
  mitk::BaseProperty* baseProp = NULL;
  if (it != propList->GetMap()->end())
  {
    baseProp = it->second.first;
  }
  if (mitk::BoolProperty* boolProp = dynamic_cast<mitk::BoolProperty*>(baseProp))
  {
    newItem = new QmitkPropertyListViewItem(name,propList,parent,createOnlyControl);
    newItem->m_Control = new QCheckBox(parent);
    ((QCheckBox*)(newItem->m_Control))->setChecked(boolProp->GetValue());
    connect((QObject*)(newItem->m_Control),SIGNAL(toggled(bool)),(QObject*)(newItem),SLOT(CheckBoxControlActivated(bool)));
  }
  else if (mitk::StringProperty* stringProp = dynamic_cast<mitk::StringProperty*>(baseProp))
  {
    newItem = new QmitkPropertyListViewItem(name,propList,parent,createOnlyControl);
    newItem->m_Control = new QLineEdit(QString(stringProp->GetValue()),parent);
    connect((QObject*)(newItem->m_Control),SIGNAL(textChanged(const QString &)),(QObject*)(newItem),SLOT(StringControlActivated(const QString &)));
  }
  else if (mitk::ColorProperty* colorProp = dynamic_cast<mitk::ColorProperty*>(baseProp))
  {
    newItem = new QmitkPropertyListViewItem(name,propList,parent,createOnlyControl);
    newItem->m_Control = new QPushButton(parent);
    QPixmap pm(20,20);
    mitk::Color col = colorProp->GetColor();
    QColor qcol((int)(col.GetRed() * 255), (int)(col.GetGreen() * 255),(int)( col.GetBlue() * 255));
    pm.fill(qcol);
    // newItem->m_Control->setBackgroundPixmap(pm);
    newItem->m_Control->setPaletteBackgroundColor(qcol);
    connect((QObject*)(newItem->m_Control),SIGNAL(clicked()),(QObject*)(newItem),SLOT(ColorControlActivated()));
  }
  else if (mitk::EnumerationProperty* enumProp = dynamic_cast<mitk::EnumerationProperty*>(baseProp))
  {
    newItem = new QmitkPropertyListViewItem( name,propList,parent,createOnlyControl );
    newItem->m_Control = new QComboBox( parent );
    
    //
    // Fill the combo box with the possible enumeration values.
    // Keep track of the index of the current value set in the enumeration property
    // to set it as current item in the  combobox
    //
    int indexOfCurrentValue = -1;
    int index = 0;
    for ( mitk::EnumerationProperty::EnumConstIterator it = enumProp->Begin(); it !=enumProp->End(); ++it, ++index )
    {
      if ( enumProp->GetValueAsString() == it->second )
      {
        indexOfCurrentValue = index;
      }
      ( ( QComboBox* )( newItem->m_Control ) )->insertItem( it->second.c_str() );  
    }
    
    //
    // select the current item of the combo box to the value defined in the
    // enumeration property
    //
    assert( indexOfCurrentValue != -1 );
    ( ( QComboBox* )( newItem->m_Control ) )->setCurrentItem( indexOfCurrentValue );
    
    connect((QObject*)(newItem->m_Control),SIGNAL( activated(const QString&) ),(QObject*)(newItem),SLOT(ComboBoxItemActivated(const QString&)));
  }
  else if (mitk::FloatProperty* floatProp = dynamic_cast<mitk::FloatProperty*>(baseProp))
  {
    std::pair<float,float> minMax;
    QString text;
    text.setNum(floatProp->GetValue());

    if (mitk::PropertyManager::GetInstance()->GetDefaultLimits(name,minMax) && (minMax.first == 0.0f && minMax.second == 1.0f))
    {
      // condition to use slider instead of lineedit

      QmitkPropertyListViewFloatSlider* sliderItem = new QmitkPropertyListViewFloatSlider(name,propList,parent,createOnlyControl);
      sliderItem->m_Control =  new QHBox(parent);
      sliderItem->m_Slider = new QSlider(Qt::Horizontal,sliderItem->m_Control);
      sliderItem->m_ValueLabel = new QLabel(text,sliderItem->m_Control);
      sliderItem->m_Slider->setMinValue(0);
      sliderItem->m_Slider->setMaxValue(100);
      sliderItem->UpdateView();
      newItem = sliderItem;
      connect((QObject*)(sliderItem->m_Slider),SIGNAL(valueChanged(int)),(QObject*)(sliderItem),SLOT(SliderValueChanged(int)));
    }
    else
    {
      newItem = new QmitkPropertyListViewItem(name,propList,parent,createOnlyControl);
      newItem->m_Control = new QLineEdit(text,parent);
      ((QLineEdit*)(newItem->m_Control))->setValidator(new QDoubleValidator(minMax.first,minMax.second,3,newItem->m_Control));
      connect((QObject*)(newItem->m_Control),SIGNAL(textChanged(const QString &)),(QObject*)(newItem),SLOT(FloatControlActivated(const QString &)));
    }
  }
  else if (mitk::IntProperty* intProp = dynamic_cast<mitk::IntProperty*>(baseProp))
  {
    newItem = new QmitkPropertyListViewItem(name,propList,parent,createOnlyControl);
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
    newItem = new QmitkPropertyListViewItem(name,propList,parent,createOnlyControl);
    if (baseProp)
    {
      newItem->m_Control = new QLabel(QString(baseProp->GetValueAsString().c_str()),parent);
    }
    else
    {
      newItem->m_Control = new QLabel("n/a",parent);
    }
  }
  newItem->m_Control->show();
  return newItem;
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
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
void QmitkPropertyListViewItem::IntControlActivated(const QString &text)
{
  if (((QLineEdit*)m_Control)->hasAcceptableInput())
  {
    m_Control->setPaletteForegroundColor(Qt::black);
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
       std::cout << "warning: non-checkbox control for bool property " << m_Name << std::endl;
     }
  }
  else if (mitk::StringProperty* stringProp = dynamic_cast<mitk::StringProperty*>(baseProp))
  {
    if (QLineEdit* qle = dynamic_cast<QLineEdit*>(m_Control)) {
    qle->setText(QString(stringProp->GetValue()));
} else {
       std::cout << "warning: non-lineedit control for string property " << m_Name << std::endl;
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

