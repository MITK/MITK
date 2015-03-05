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

#include "QmitkDataSelectionWidget.h"
#include "../mitkPluginActivator.h"

#include <berryIWorkbench.h>

#include <mitkContourModel.h>
#include <mitkContourModelSet.h>
#include <mitkIDataStorageService.h>
#include <mitkImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkSurface.h>
#include <QmitkDataStorageComboBox.h>
#include <QLabel>
#include <algorithm>
#include <cassert>
#include <iterator>

static mitk::NodePredicateBase::Pointer CreatePredicate(QmitkDataSelectionWidget::Predicate predicate)
{
  switch(predicate)
  {
    case QmitkDataSelectionWidget::ImagePredicate:
      return mitk::NodePredicateAnd::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true))),
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();

    case QmitkDataSelectionWidget::SegmentationPredicate:
      return mitk::NodePredicateAnd::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)),
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();

    case QmitkDataSelectionWidget::SurfacePredicate:
      return mitk::NodePredicateAnd::New(
        mitk::TNodePredicateDataType<mitk::Surface>::New(),
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();

    case QmitkDataSelectionWidget::ImageAndSegmentationPredicate:
      return mitk::NodePredicateAnd::New(
        mitk::TNodePredicateDataType<mitk::Image>::New(),
        mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();
    case QmitkDataSelectionWidget::ContourModelPredicate:
      return mitk::NodePredicateAnd::New(
            mitk::NodePredicateOr::New( mitk::TNodePredicateDataType<mitk::ContourModelSet>::New(),
            mitk::TNodePredicateDataType<mitk::ContourModel>::New()),
            mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))).GetPointer();
    default:
      assert(false && "Unknown predefined predicate!");
      return NULL;
  }
}

QmitkDataSelectionWidget::QmitkDataSelectionWidget(QWidget* parent)
  : QWidget(parent)
{
  m_Controls.setupUi(this);
  m_Controls.helpLabel->hide();
}

QmitkDataSelectionWidget::~QmitkDataSelectionWidget()
{
}

unsigned int QmitkDataSelectionWidget::AddDataStorageComboBox(QmitkDataSelectionWidget::Predicate predicate)
{
  return this->AddDataStorageComboBox("", predicate);
}

unsigned int QmitkDataSelectionWidget::AddDataStorageComboBox(mitk::NodePredicateBase* predicate)
{
  return this->AddDataStorageComboBox("", predicate);
}

unsigned int QmitkDataSelectionWidget::AddDataStorageComboBox(const QString &labelText, QmitkDataSelectionWidget::Predicate predicate)
{
  return this->AddDataStorageComboBox(labelText, CreatePredicate(predicate));
}

unsigned int QmitkDataSelectionWidget::AddDataStorageComboBox(const QString &labelText, mitk::NodePredicateBase* predicate)
{
  int row = m_Controls.gridLayout->rowCount();

  if (!labelText.isEmpty())
  {
    QLabel* label = new QLabel(labelText, m_Controls.dataSelectionWidget);
    label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_Controls.gridLayout->addWidget(label, row, 0);
  }

  QmitkDataStorageComboBox* comboBox = new QmitkDataStorageComboBox(this->GetDataStorage(), predicate, m_Controls.dataSelectionWidget);
  connect(comboBox, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnSelectionChanged(const mitk::DataNode *)));
  m_Controls.gridLayout->addWidget(comboBox, row, 1);

  m_DataStorageComboBoxes.push_back(comboBox);
  return static_cast<unsigned int>(m_DataStorageComboBoxes.size() - 1);
}

mitk::DataStorage::Pointer QmitkDataSelectionWidget::GetDataStorage() const
{
  ctkServiceReference ref = mitk::PluginActivator::getContext()->getServiceReference<mitk::IDataStorageService>();
  assert(ref == true);

  mitk::IDataStorageService* service = mitk::PluginActivator::getContext()->getService<mitk::IDataStorageService>(ref);

  assert(service);

  return service->GetDefaultDataStorage()->GetDataStorage();
}

mitk::DataNode::Pointer QmitkDataSelectionWidget::GetSelection(unsigned int index)
{
  assert(index < m_DataStorageComboBoxes.size());
  return m_DataStorageComboBoxes[index]->GetSelectedNode();
}

void QmitkDataSelectionWidget::SetPredicate(unsigned int index, Predicate predicate)
{
  this->SetPredicate(index, CreatePredicate(predicate));
}

void QmitkDataSelectionWidget::SetPredicate(unsigned int index, mitk::NodePredicateBase* predicate)
{
  assert(index < m_DataStorageComboBoxes.size());
  m_DataStorageComboBoxes[index]->SetPredicate(predicate);
}

void QmitkDataSelectionWidget::SetHelpText(const QString& text)
{
  if (!text.isEmpty())
  {
    m_Controls.helpLabel->setText(text);

    if (!m_Controls.helpLabel->isVisible())
      m_Controls.helpLabel->show();
  }
  else
  {
    m_Controls.helpLabel->hide();
  }
}

void QmitkDataSelectionWidget::OnSelectionChanged(const mitk::DataNode* selection)
{
  std::vector<QmitkDataStorageComboBox*>::iterator it = std::find(m_DataStorageComboBoxes.begin(), m_DataStorageComboBoxes.end(), sender());
  assert(it != m_DataStorageComboBoxes.end());

  emit SelectionChanged(std::distance(m_DataStorageComboBoxes.begin(), it), selection);
}
