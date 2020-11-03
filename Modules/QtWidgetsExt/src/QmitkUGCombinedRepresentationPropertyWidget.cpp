/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUGCombinedRepresentationPropertyWidget.h"

#include <mitkGridRepresentationProperty.h>
#include <mitkGridVolumeMapperProperty.h>

#include <mitkProperties.h>
#include <mitkPropertyObserver.h>

class _UGCombinedBoolPropEditor : public mitk::PropertyEditor
{
public:
  _UGCombinedBoolPropEditor(mitk::BoolProperty *boolProp, QmitkUGCombinedRepresentationPropertyWidget *combo)
    : PropertyEditor(boolProp), m_BoolProperty(boolProp), m_ComboBox(combo)
  {
    PropertyChanged();
  }

  ~_UGCombinedBoolPropEditor() override {}
  bool IsEnabled() const { return enabled; }
  void SetEnabled(bool enable)
  {
    this->BeginModifyProperty();
    m_BoolProperty->SetValue(enable);
    this->EndModifyProperty();
  }

protected:
  void PropertyChanged() override
  {
    if (m_BoolProperty)
      enabled = m_BoolProperty->GetValue();
    else
      enabled = false;

    m_ComboBox->IsVolumeChanged(enabled);
  }

  void PropertyRemoved() override
  {
    m_Property = nullptr;
    m_BoolProperty = nullptr;
    enabled = false;
  }

  mitk::BoolProperty *m_BoolProperty;
  QmitkUGCombinedRepresentationPropertyWidget *m_ComboBox;
  bool enabled;
};

class _UGCombinedEnumPropEditor : public mitk::PropertyEditor
{
public:
  _UGCombinedEnumPropEditor(mitk::EnumerationProperty *property,
                            QmitkUGCombinedRepresentationPropertyWidget *combo,
                            bool isVolumeProp)
    : PropertyEditor(property), m_EnumerationProperty(property), m_ComboBox(combo), m_IsVolumeProp(isVolumeProp)
  {
  }

  ~_UGCombinedEnumPropEditor() override { m_EnumerationProperty = nullptr; }
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
      if (m_IsVolumeProp)
      {
        m_ComboBox->SetGridVolumeId(m_EnumerationProperty->GetValueAsId());
      }
      else
      {
        m_ComboBox->SetGridRepresentationId(m_EnumerationProperty->GetValueAsId());
      }
    }
  }

  void PropertyRemoved() override
  {
    m_Property = nullptr;
    m_EnumerationProperty = nullptr;
  }

protected:
  mitk::EnumerationProperty *m_EnumerationProperty;
  QmitkUGCombinedRepresentationPropertyWidget *m_ComboBox;
  QHash<int, int> m_EnumIdToItemIndex;
  bool m_IsVolumeProp;
};

QmitkUGCombinedRepresentationPropertyWidget::QmitkUGCombinedRepresentationPropertyWidget(QWidget *parent)
  : QComboBox(parent),
    gridRepPropEditor(nullptr),
    volumeMapperPropEditor(nullptr),
    volumePropEditor(nullptr),
    m_GridRepIndex(0),
    m_GridVolIndex(0),
    m_FirstVolumeRepId(0)
{
  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(OnIndexChanged(int)));
}

QmitkUGCombinedRepresentationPropertyWidget::~QmitkUGCombinedRepresentationPropertyWidget()
{
  delete gridRepPropEditor;
  delete volumeMapperPropEditor;
  delete volumePropEditor;
}

void QmitkUGCombinedRepresentationPropertyWidget::SetProperty(mitk::GridRepresentationProperty *gridRepProp,
                                                              mitk::GridVolumeMapperProperty *gridVolProp,
                                                              mitk::BoolProperty *volumeProp)
{
  if (gridRepPropEditor)
  {
    delete gridRepPropEditor;
    gridRepPropEditor = nullptr;
  }

  if (volumeMapperPropEditor)
  {
    delete volumeMapperPropEditor;
    volumeMapperPropEditor = nullptr;
  }

  if (volumePropEditor)
  {
    delete volumePropEditor;
    volumePropEditor = nullptr;
  }

  this->clear();
  this->setEnabled(true);
  m_FirstVolumeRepId = 0;
  m_MapRepEnumToIndex.clear();
  m_MapVolEnumToIndex.clear();

  if (!gridRepProp && !gridVolProp)
    return;

  int i = 0;
  if (gridRepProp)
  {
    const mitk::EnumerationProperty::EnumStringsContainerType &repStrings = gridRepProp->GetEnumStrings();

    for (auto it = repStrings.begin(); it != repStrings.end(); ++it, ++i)
    {
      m_MapRepEnumToIndex.insert(it->second, i);
      this->addItem(QString::fromStdString(it->first), it->second);
    }
    m_FirstVolumeRepId = i;
  }
  if (gridVolProp)
  {
    const mitk::EnumerationProperty::EnumStringsContainerType &volStrings = gridVolProp->GetEnumStrings();

    for (auto it = volStrings.begin(); it != volStrings.end(); ++it, ++i)
    {
      m_MapVolEnumToIndex.insert(it->second, i);
      this->addItem(QString("Volume (") + QString::fromStdString(it->first) + ")", it->second);
    }
  }

  if (gridRepProp)
  {
    gridRepPropEditor = new _UGCombinedEnumPropEditor(gridRepProp, this, false);
  }
  if (gridVolProp)
  {
    volumeMapperPropEditor = new _UGCombinedEnumPropEditor(gridVolProp, this, true);
  }
  if (volumeProp)
  {
    volumePropEditor = new _UGCombinedBoolPropEditor(volumeProp, this);
  }

  if (gridRepProp)
  {
    this->SetGridRepresentationId(gridRepProp->GetValueAsId());
  }
  if (gridVolProp)
  {
    this->SetGridVolumeId(gridVolProp->GetValueAsId());
  }
}

void QmitkUGCombinedRepresentationPropertyWidget::OnIndexChanged(int index)
{
  int enumIndex = this->itemData(index, Qt::UserRole).toInt();
  if (index < m_FirstVolumeRepId && gridRepPropEditor)
  {
    gridRepPropEditor->IndexChanged(enumIndex);
    if (volumePropEditor)
    {
      volumePropEditor->SetEnabled(false);
    }
  }
  else if (volumeMapperPropEditor)
  {
    volumeMapperPropEditor->IndexChanged(enumIndex);
    if (volumePropEditor)
    {
      volumePropEditor->SetEnabled(true);
    }
  }
}

void QmitkUGCombinedRepresentationPropertyWidget::SetGridRepresentationId(int enumId)
{
  m_GridRepIndex = enumId;
  if (volumePropEditor && volumePropEditor->IsEnabled())
  {
    return;
  }
  else
  {
    this->setCurrentIndex(m_MapRepEnumToIndex[enumId]);
  }
}

void QmitkUGCombinedRepresentationPropertyWidget::SetGridVolumeId(int enumId)
{
  m_GridVolIndex = enumId;
  if (volumePropEditor && volumePropEditor->IsEnabled())
  {
    this->setCurrentIndex(m_MapVolEnumToIndex[enumId]);
  }
  else
  {
    return;
  }
}

void QmitkUGCombinedRepresentationPropertyWidget::IsVolumeChanged(bool volume)
{
  if (volume)
  {
    this->SetGridVolumeId(m_GridVolIndex);
  }
  else
  {
    this->SetGridRepresentationId(m_GridRepIndex);
  }
}
