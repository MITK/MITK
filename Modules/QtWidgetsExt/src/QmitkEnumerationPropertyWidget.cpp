/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkEnumerationPropertyWidget.h"

#include <mitkEnumerationProperty.h>
#include <mitkPropertyObserver.h>

class _EnumPropEditorImpl : public mitk::PropertyEditor
{
public:
  _EnumPropEditorImpl(mitk::EnumerationProperty *property, QComboBox *combo, const QHash<int, int> &enumIdToItemIndex)
    : PropertyEditor(property),
      m_EnumerationProperty(property),
      m_ComboBox(combo),
      m_EnumIdToItemIndex(enumIdToItemIndex)
  {
  }

  ~_EnumPropEditorImpl() override { m_EnumerationProperty = nullptr; }
  void IndexChanged(int enumId)
  {
    this->BeginModifyProperty();
    m_EnumerationProperty->SetValue(enumId);
    this->EndModifyProperty();
  }

  void PropertyChanged() override
  {
    if (m_EnumerationProperty)
    {
      m_ComboBox->setCurrentIndex(m_EnumIdToItemIndex[m_EnumerationProperty->GetValueAsId()]);
    }
  }

  void PropertyRemoved() override
  {
    m_Property = nullptr;
    m_EnumerationProperty = nullptr;
    m_ComboBox->setEnabled(false);
  }

protected:
  mitk::EnumerationProperty *m_EnumerationProperty;
  QComboBox *m_ComboBox;
  QHash<int, int> m_EnumIdToItemIndex;
};

QmitkEnumerationPropertyWidget::QmitkEnumerationPropertyWidget(QWidget *parent) : QComboBox(parent), propView(nullptr)
{
  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(OnIndexChanged(int)));
}

QmitkEnumerationPropertyWidget::~QmitkEnumerationPropertyWidget()
{
  delete propView;
}

void QmitkEnumerationPropertyWidget::SetProperty(mitk::EnumerationProperty *property)
{
  if (propView)
  {
    delete propView;
    propView = nullptr;
  }

  this->clear();

  if (!property)
  {
    return;
  }

  this->setEnabled(true);

  QHash<int, int> enumIdToItemIndex;

  const mitk::EnumerationProperty::EnumStringsContainerType &strings = property->GetEnumStrings();
  int index = 0;
  for (auto it = strings.begin(); it != strings.end(); ++it, ++index)
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
