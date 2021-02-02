/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataSelectionWidget.h"
#include "internal/mitkPluginActivator.h"

#include <berryPlatform.h>

#include <mitkContourModel.h>
#include <mitkContourModelSet.h>
#include <mitkIDataStorageService.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkSurface.h>
#include <QmitkSingleNodeSelectionWidget.h>
#include <QLabel>
#include <algorithm>
#include <cassert>
#include <iterator>

#include <ctkServiceReference.h>

static mitk::NodePredicateBase::Pointer CreatePredicate(QmitkDataSelectionWidget::PredicateType predicateType)
{
  auto nonHelperObject = mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(
    mitk::NodePredicateProperty::New("helper object"),
    mitk::NodePredicateProperty::New("hidden object")));

  auto imageType = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto contourModelType = mitk::TNodePredicateDataType<mitk::ContourModel>::New();
  auto contourModelSetType = mitk::TNodePredicateDataType<mitk::ContourModelSet>::New();

  auto segmentationPredicate = mitk::TNodePredicateDataType<mitk::LabelSetImage>::New();

  auto maskPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));

  auto isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  auto isDti = mitk::NodePredicateDataType::New("TensorImage");
  auto isOdf = mitk::NodePredicateDataType::New("OdfImage");
  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();

  auto validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate(isImage);
  validImages->AddPredicate(isDwi);
  validImages->AddPredicate(isDti);
  validImages->AddPredicate(isOdf);

  auto imagePredicate = mitk::NodePredicateAnd::New();
  imagePredicate->AddPredicate(validImages);
  imagePredicate->AddPredicate(mitk::NodePredicateNot::New(segmentationPredicate));
  imagePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true))));

  auto surfacePredicate = mitk::TNodePredicateDataType<mitk::Surface>::New();

  mitk::NodePredicateBase::Pointer result;

  switch(predicateType)
  {
    case QmitkDataSelectionWidget::ImagePredicate:
      result = imagePredicate.GetPointer();
      break;

    case QmitkDataSelectionWidget::SegmentationPredicate:
      result = segmentationPredicate.GetPointer();
      break;

    case QmitkDataSelectionWidget::SurfacePredicate:
      result = surfacePredicate.GetPointer();
      break;

    case QmitkDataSelectionWidget::ImageAndSegmentationPredicate:
      result = imageType.GetPointer();
      break;

    case QmitkDataSelectionWidget::ContourModelPredicate:
      result = mitk::NodePredicateOr::New(
        contourModelSetType,
        contourModelSetType).GetPointer();
      break;

    case QmitkDataSelectionWidget::MaskPredicate:
      result = maskPredicate.GetPointer();
      break;

    default:
      assert(false && "Unknown predefined predicate!");
      return nullptr;
  }

  return mitk::NodePredicateAnd::New(result, nonHelperObject).GetPointer();
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

unsigned int QmitkDataSelectionWidget::AddDataSelection(QmitkDataSelectionWidget::PredicateType predicate)
{
  QString hint = "Select node";

  switch (predicate)
  {
  case QmitkDataSelectionWidget::ImagePredicate:
    hint = "Select an image";
    break;

  case QmitkDataSelectionWidget::MaskPredicate:
    hint = "Select a binary mask";
    break;

  case QmitkDataSelectionWidget::SegmentationPredicate:
    hint = "Select an ML segmentation";
    break;

  case QmitkDataSelectionWidget::SurfacePredicate:
    hint = "Select a surface";
    break;

  case QmitkDataSelectionWidget::ImageAndSegmentationPredicate:
    hint = "Select an image or segmentation";
    break;

  case QmitkDataSelectionWidget::ContourModelPredicate:
    hint = "Select a contour model";
    break;
  }

  return this->AddDataSelection("", hint, hint, "", predicate);
}

unsigned int QmitkDataSelectionWidget::AddDataSelection(mitk::NodePredicateBase* predicate)
{
  return this->AddDataSelection("", "Select a node", "Select a node", "", predicate);
}

unsigned int QmitkDataSelectionWidget::AddDataSelection(const QString &labelText, const QString &info, const QString &popupTitel, const QString &popupHint, QmitkDataSelectionWidget::PredicateType predicate)
{
  return this->AddDataSelection(labelText, info, popupHint, popupTitel, CreatePredicate(predicate));
}

unsigned int QmitkDataSelectionWidget::AddDataSelection(const QString &labelText, const QString &info, const QString &popupTitel, const QString &popupHint, mitk::NodePredicateBase* predicate)
{
  int row = m_Controls.gridLayout->rowCount();

  if (!labelText.isEmpty())
  {
    QLabel* label = new QLabel(labelText, m_Controls.dataSelectionWidget);
    label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    m_Controls.gridLayout->addWidget(label, row, 0);
  }

  QmitkSingleNodeSelectionWidget* nodeSelection = new QmitkSingleNodeSelectionWidget(m_Controls.dataSelectionWidget);

  nodeSelection->SetSelectionIsOptional(false);
  nodeSelection->SetAutoSelectNewNodes(false);
  nodeSelection->SetInvalidInfo(info);
  nodeSelection->SetPopUpTitel(popupTitel);
  nodeSelection->SetPopUpHint(popupHint);
  nodeSelection->SetDataStorage(this->GetDataStorage());
  nodeSelection->SetNodePredicate(predicate);
  nodeSelection->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  nodeSelection->setMinimumSize(0, 40);

  connect(nodeSelection, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged, this, &QmitkDataSelectionWidget::OnSelectionChanged);

  m_Controls.gridLayout->addWidget(nodeSelection, row, 1);

  m_NodeSelectionWidgets.push_back(nodeSelection);
  return static_cast<unsigned int>(m_NodeSelectionWidgets.size() - 1);
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
  assert(index < m_NodeSelectionWidgets.size());
  return m_NodeSelectionWidgets[index]->GetSelectedNode();
}

void QmitkDataSelectionWidget::SetPredicate(unsigned int index, PredicateType predicate)
{
  this->SetPredicate(index, CreatePredicate(predicate));
}

void QmitkDataSelectionWidget::SetPredicate(unsigned int index, mitk::NodePredicateBase* predicate)
{
  assert(index < m_NodeSelectionWidgets.size());
  m_NodeSelectionWidgets[index]->SetNodePredicate(predicate);
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

void QmitkDataSelectionWidget::OnSelectionChanged(QList<mitk::DataNode::Pointer> selection)
{
  std::vector<QmitkSingleNodeSelectionWidget*>::iterator it = std::find(m_NodeSelectionWidgets.begin(), m_NodeSelectionWidgets.end(), sender());
  assert(it != m_NodeSelectionWidgets.end());

  const mitk::DataNode* result = nullptr;
  if (!selection.empty())
  {
    result = selection.front();
  }
  emit SelectionChanged(std::distance(m_NodeSelectionWidgets.begin(), it), result);
}
