/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkEnumerationPropertyWidget.h"

#include <mitkPropertyObserver.h>
#include <mitkEnumerationProperty.h>

class _EnumPropEditorImpl : public mitk::PropertyEditor
{

public:

  _EnumPropEditorImpl(mitk::EnumerationProperty* property,
                                     QComboBox* combo, const QHash<int, int>& enumIdToItemIndex)
    : PropertyEditor(property), m_EnumerationProperty(property), m_ComboBox(combo),
    m_EnumIdToItemIndex(enumIdToItemIndex)
  {

  }

  ~_EnumPropEditorImpl()
  {
    m_EnumerationProperty = 0;
  }

  void IndexChanged(int enumId)
  {
    this->BeginModifyProperty();
    m_EnumerationProperty->SetValue(enumId);
    this->EndModifyProperty();
  }

  virtual void PropertyChanged()
  {
    if (m_EnumerationProperty)
    {
      m_ComboBox->setCurrentIndex(m_EnumIdToItemIndex[m_EnumerationProperty->GetValueAsId()]);
    }
  }

  virtual void PropertyRemoved()
  {
    m_Property = 0;
    m_EnumerationProperty = 0;
    m_ComboBox->setEnabled(false);
  }

protected:

  mitk::EnumerationProperty* m_EnumerationProperty;
  QComboBox* m_ComboBox;
  QHash<int, int> m_EnumIdToItemIndex;
};


QmitkEnumerationPropertyWidget::QmitkEnumerationPropertyWidget(QWidget* parent)
: QComboBox(parent), propView(0)
{
  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(OnIndexChanged(int)));
}

QmitkEnumerationPropertyWidget::~QmitkEnumerationPropertyWidget()
{
  delete propView;
}

void QmitkEnumerationPropertyWidget::SetProperty(mitk::EnumerationProperty* property)
{
  if(propView)
  {
    delete propView;
    propView = 0;
  }

  this->clear();

  if(!property)
  {
    return;
  }

  this->setEnabled(true);

  QHash<int,int> enumIdToItemIndex;

  const mitk::EnumerationProperty::EnumStringsContainerType& strings = property->GetEnumStrings();
  int index = 0;
  for (mitk::EnumerationProperty::EnumStringsContainerType::const_iterator it = strings.begin();
       it != strings.end(); ++it, ++index)
  {
    enumIdToItemIndex.insert(it->second, index);
    this->addItem(QString::fromStdString(it->first), it->second);
  }

  propView = new _EnumPropEditorImpl(property, this, enumIdToItemIndex);
  propView->PropertyChanged();
}

 void QmitkEnumerationPropertyWidget::OnIndexChanged(int index)
 {
   if (propView)
   {
     int enumIndex = this->itemData(index, Qt::UserRole).toInt();
     propView->IndexChanged(enumIndex);
   }
 }

