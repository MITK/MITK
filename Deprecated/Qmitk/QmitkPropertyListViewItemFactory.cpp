/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include <QmitkPropertyListViewItemFactory.h>
#include <QmitkPropertyListViewItem.h>
#include <mitkPropertyList.h>
#include <mitkPropertyManager.h>
#include <mitkProperties.h>
#include <mitkBaseProperty.h>
#include <mitkEnumerationProperty.h>
#include <mitkMaterialProperty.h>

// QT related includes
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

#include <iostream>

QmitkPropertyListViewItemFactory* QmitkPropertyListViewItemFactory::GetInstance() {
  static QmitkPropertyListViewItemFactory* instance = new QmitkPropertyListViewItemFactory();
  return instance;
}
QmitkPropertyListViewItem* QmitkPropertyListViewItemFactory::CreateQmitkPropertyListViewItem(mitk::PropertyList *propList, const std::string name, QWidget* parent, bool createOnlyControl) const {

  QmitkPropertyListViewItem* newItem = NULL;
  QLabel* itemLabel = NULL;
  if ( !createOnlyControl )
  {
    itemLabel = new QLabel ( name.c_str(),parent );
    itemLabel->show();
  }
  if (m_Extension) {
    // try the registered extension
    newItem = m_Extension->CreateQmitkPropertyListViewItem(propList,name,parent,createOnlyControl);
  }
  if (!newItem) { 

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
      QObject::connect((QObject*)(newItem->m_Control),SIGNAL(toggled(bool)),(QObject*)(newItem),SLOT(CheckBoxControlActivated(bool)));
    }
    else if (mitk::StringProperty* stringProp = dynamic_cast<mitk::StringProperty*>(baseProp))
    {
      newItem = new QmitkPropertyListViewItem(name,propList,parent,createOnlyControl);
      newItem->m_Control = new QLineEdit(QString(stringProp->GetValue()),parent);
      QObject::connect((QObject*)(newItem->m_Control),SIGNAL(textChanged(const QString &)),(QObject*)(newItem),SLOT(StringControlActivated(const QString &))); 
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
      QObject::connect((QObject*)(newItem->m_Control),SIGNAL(clicked()),(QObject*)(newItem),SLOT(ColorControlActivated()));
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

      QObject::connect((QObject*)(newItem->m_Control),SIGNAL( activated(const QString&) ),(QObject*)(newItem),SLOT(ComboBoxItemActivated(const QString&)));
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
        QObject::connect((QObject*)(sliderItem->m_Slider),SIGNAL(valueChanged(int)),(QObject*)(sliderItem),SLOT(SliderValueChanged(int)));
      }
      else
      {
        newItem = new QmitkPropertyListViewItem(name,propList,parent,createOnlyControl);
        newItem->m_Control = new QLineEdit(text,parent);
        ((QLineEdit*)(newItem->m_Control))->setValidator(new QDoubleValidator(minMax.first,minMax.second,3,newItem->m_Control));
        QObject::connect((QObject*)(newItem->m_Control),SIGNAL(textChanged(const QString &)),(QObject*)(newItem),SLOT(FloatControlActivated(const QString &)));
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
      QObject::connect((QObject*)(newItem->m_Control),SIGNAL(textChanged(const QString &)),(QObject*)(newItem),SLOT(IntControlActivated(const QString &)));
    }
    else if (dynamic_cast<mitk::MaterialProperty*>(baseProp))
    {
      newItem = new QmitkPropertyListViewItem(name,propList,parent,createOnlyControl);
      newItem->m_Control = new QPushButton("Edit",parent);
      QObject::connect((QObject*)(newItem->m_Control),SIGNAL(clicked()),(QObject*)(newItem),SLOT(MaterialEditorActivated()));
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
  }
  if (!createOnlyControl) {
    newItem->m_Label = itemLabel;
    newItem->CreateEnabledButton(parent);
  }

  if (newItem && newItem->m_Control) {
    newItem->m_Control->setName((name + "_" + "QmitkPropertyListViewItem_control").c_str());
    newItem->m_Control->show();
  }
  return newItem;
}

