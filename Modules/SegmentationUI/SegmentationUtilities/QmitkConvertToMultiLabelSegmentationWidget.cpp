/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkConvertToMultiLabelSegmentationWidget.h"
#include <ui_QmitkConvertToMultiLabelSegmentationWidgetControls.h>

#include <mitkDataStorage.h>
#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkProgressBar.h>
#include <mitkProperties.h>
#include <mitkSurfaceToImageFilter.h>
#include <mitkSurface.h>
#include <mitkContourModel.h>
#include <mitkContourModelSet.h>
#include <mitkContourModelSetToImageFilter.h>
#include <mitkLabelSetImage.h>
#include <mitkMultiLabelPredicateHelper.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateSubGeometry.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkLabelSetImageConverter.h>

#include <QmitkNodeSelectionDialog.h>

mitk::NodePredicateBase::Pointer GetInputPredicate()
{
  auto isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto isNotSeg = mitk::NodePredicateNot::New(mitk::GetMultiLabelSegmentationPredicate());

  auto isSurface = mitk::TNodePredicateDataType<mitk::Surface>::New();
  auto isContourModel = mitk::TNodePredicateDataType<mitk::ContourModel>::New();
  auto isContourModelSet = mitk::TNodePredicateDataType<mitk::ContourModelSet>::New();

  auto isData = mitk::NodePredicateOr::New(isImage, isContourModel, isContourModelSet);
  isData->AddPredicate(isSurface);

  auto isValidInput = mitk::NodePredicateAnd::New(isNotSeg, isData);
  isValidInput->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  isValidInput->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));
  return isValidInput.GetPointer();
}

const mitk::DataNode* GetNodeWithLargestImageGeometry(const QmitkNodeSelectionDialog::NodeList& nodes)
{
  mitk::BaseGeometry::ConstPointer refGeometry;
  mitk::DataNode* result = nullptr;

  for (auto& node : nodes)
  {
    auto castedData = dynamic_cast<const mitk::Image*>(node->GetData());
    if (castedData != nullptr)
    {
      if (refGeometry.IsNull() || mitk::IsSubGeometry(*refGeometry, *(castedData->GetGeometry()), mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION, mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION))
      {
        refGeometry = castedData->GetGeometry();
        result = node;
      }
    }
  }
  return result;
}

QmitkNodeSelectionDialog::NodeList GetNonimageNodes(const QmitkNodeSelectionDialog::NodeList& nodes)
{
  QmitkNodeSelectionDialog::NodeList result;

  for (auto& node : nodes)
  {
    auto castedData = dynamic_cast<const mitk::Image*>(node->GetData());
    if (castedData == nullptr)
    {
      result.push_back(node);
    }
  }
  return result;
}

QmitkNodeSelectionDialog::NodeList GetImageNodes(const QmitkNodeSelectionDialog::NodeList& nodes)
{
  QmitkNodeSelectionDialog::NodeList result;

  for (auto& node : nodes)
  {
    auto castedData = dynamic_cast<const mitk::Image*>(node->GetData());
    if (castedData != nullptr)
    {
      result.push_back(node);
    }
  }
  return result;
}

QmitkNodeSelectionDialog::SelectionCheckFunctionType CheckForSameGeometry(const mitk::DataNode* refNode)
{
  mitk::DataNode::ConstPointer refNodeLambda = refNode;

  auto lambda = [refNodeLambda](const QmitkNodeSelectionDialog::NodeList& nodes)
  {
    if (nodes.empty())
    {
      return std::string();
    }

    mitk::NodePredicateSubGeometry::Pointer geoPredicate;
    bool usedExternalGeo = false;
    std::string refNodeName;

    if (refNodeLambda.IsNotNull() && nullptr != refNodeLambda->GetData())
    {
      geoPredicate = mitk::NodePredicateSubGeometry::New(refNodeLambda->GetData()->GetGeometry());
      usedExternalGeo = true;
      refNodeName = refNodeLambda->GetName();
    }

    if (geoPredicate.IsNull())
    {
      auto imageNode = GetNodeWithLargestImageGeometry(nodes);
      if (nullptr != imageNode)
      {
        geoPredicate = mitk::NodePredicateSubGeometry::New(imageNode->GetData()->GetGeometry());
        refNodeName = imageNode->GetName();
      }
    }

    for (auto& node : nodes)
    {
      auto castedImageData = dynamic_cast<const mitk::Image*>(node->GetData());

      if (nullptr != castedImageData)
      {
        if (!geoPredicate->CheckNode(node))
        {
          std::stringstream ss;
          ss << "<font class=\"warning\"><p>Invalid selection: All selected images must have the same geometry or a sub geometry ";
          if (usedExternalGeo) ss << "of the selected reference/output";
          ss << ".< / p><p>Uses reference data: \"";
          ss << refNodeName << "\"</p>";
          ss << "<p>Differing data selections i.a.: \"";
          ss << node->GetName() << "\"</p></font>";
          return ss.str();
        }
      }
    }

    return std::string();
  };

  return lambda;
}

QmitkConvertToMultiLabelSegmentationWidget::QmitkConvertToMultiLabelSegmentationWidget(mitk::DataStorage* dataStorage, QWidget* parent)
  : QWidget(parent), m_DataStorage(dataStorage)
{
  m_Controls = new Ui::QmitkConvertToMultiLabelSegmentationWidgetControls;
  m_Controls->setupUi(this);

  m_Controls->inputNodesSelector->SetDataStorage(dataStorage);
  m_Controls->inputNodesSelector->SetNodePredicate(GetInputPredicate());
  m_Controls->inputNodesSelector->SetSelectionCheckFunction(CheckForSameGeometry(nullptr));
  m_Controls->inputNodesSelector->SetSelectionIsOptional(false);
  m_Controls->inputNodesSelector->SetInvalidInfo(QStringLiteral("Please select inputs (images, surfaces or contours) for conversion"));
  m_Controls->inputNodesSelector->SetPopUpTitel(QStringLiteral("Select inputs"));
  m_Controls->inputNodesSelector->SetPopUpHint(QStringLiteral("You may select multiple inputs for conversion. But all selected images must have the same geometry or a sub geometry."));

  m_Controls->outputSegSelector->SetDataStorage(dataStorage);
  m_Controls->outputSegSelector->SetNodePredicate(mitk::GetMultiLabelSegmentationPredicate());
  m_Controls->outputSegSelector->SetSelectionIsOptional(false);
  m_Controls->outputSegSelector->SetInvalidInfo(QStringLiteral("Please select the target segmentation"));
  m_Controls->outputSegSelector->SetPopUpTitel(QStringLiteral("Select target segmentation"));
  m_Controls->outputSegSelector->SetPopUpHint(QStringLiteral("Select the segmentation to which the converted inputs should be added."));
  m_Controls->outputSegSelector->SetAutoSelectNewNodes(true);

  m_Controls->refNodeSelector->SetDataStorage(dataStorage);
  m_Controls->refNodeSelector->SetNodePredicate(mitk::NodePredicateOr::New(GetInputPredicate(),mitk::GetMultiLabelSegmentationPredicate()));
  m_Controls->refNodeSelector->SetSelectionIsOptional(false);
  m_Controls->refNodeSelector->SetInvalidInfo(QStringLiteral("Please select a reference image or segmentation"));
  m_Controls->refNodeSelector->SetPopUpTitel(QStringLiteral("Select a reference image or segmentation"));
  m_Controls->refNodeSelector->SetPopUpHint(QStringLiteral("Select the image or segmentation that defines the geometry of the conversion result."));


  this->ConfigureWidgets();

  connect (m_Controls->btnConvert, &QAbstractButton::clicked, this, &QmitkConvertToMultiLabelSegmentationWidget::OnConvertPressed);
  connect(m_Controls->inputNodesSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkConvertToMultiLabelSegmentationWidget::OnInputSelectionChanged);
  connect(m_Controls->refNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkConvertToMultiLabelSegmentationWidget::OnRefSelectionChanged);
  connect(m_Controls->outputSegSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkConvertToMultiLabelSegmentationWidget::OnOutputSelectionChanged);
  auto widget = this;
  connect(m_Controls->radioAddToSeg, &QRadioButton::toggled,
    m_Controls->outputSegSelector, [widget](bool) {widget->ConfigureWidgets(); });
  connect(m_Controls->checkMultipleOutputs, &QCheckBox::toggled,
    m_Controls->outputSegSelector, [widget](bool) {widget->ConfigureWidgets(); });
}

QmitkConvertToMultiLabelSegmentationWidget::~QmitkConvertToMultiLabelSegmentationWidget()
{
}

void QmitkConvertToMultiLabelSegmentationWidget::ConfigureWidgets()
{
  m_InternalEvent = true;

  if (m_Controls->radioAddToSeg->isChecked())
  {
    auto selectedNode = m_Controls->outputSegSelector->GetSelectedNode();
    m_Controls->inputNodesSelector->SetSelectionCheckFunction(CheckForSameGeometry(selectedNode));
  }
  else
  {
    m_Controls->inputNodesSelector->SetSelectionCheckFunction(CheckForSameGeometry(nullptr));
  }
  m_Controls->outputSegSelector->setVisible(m_Controls->radioAddToSeg->isChecked());

  if (m_Controls->radioNewSeg->isChecked())
  {
    auto selectedNode = m_Controls->refNodeSelector->GetSelectedNode();
  }
  m_Controls->checkMultipleOutputs->setVisible(m_Controls->radioNewSeg->isChecked());
  bool refNeeded = m_Controls->radioNewSeg->isChecked() && !m_Controls->inputNodesSelector->GetSelectedNodes().empty() && nullptr == GetNodeWithLargestImageGeometry(m_Controls->inputNodesSelector->GetSelectedNodes());
  m_Controls->refNodeSelector->setVisible(refNeeded);

  if (refNeeded) m_Controls->inputNodesSelector->SetSelectionCheckFunction(CheckForSameGeometry(m_Controls->refNodeSelector->GetSelectedNode()));

  m_Controls->groupGrouping->setVisible(m_Controls->radioAddToSeg->isChecked() || !(m_Controls->checkMultipleOutputs->isChecked()));

  bool inputIsOK = !m_Controls->inputNodesSelector->GetSelectedNodes().empty() &&
    !m_Controls->inputNodesSelector->CurrentSelectionViolatesCheckFunction();
  bool outputIsOK = !m_Controls->radioAddToSeg->isChecked() || m_Controls->outputSegSelector->GetSelectedNode().IsNotNull();
  bool refIsOK = !m_Controls->radioNewSeg->isChecked() || !m_Controls->refNodeSelector->isVisible() || m_Controls->refNodeSelector->GetSelectedNode().IsNotNull();

  m_Controls->btnConvert->setEnabled(inputIsOK && outputIsOK && refIsOK);
  m_InternalEvent = false;
}

void QmitkConvertToMultiLabelSegmentationWidget::OnInputSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  if (!m_InternalEvent) this->ConfigureWidgets();
}

void QmitkConvertToMultiLabelSegmentationWidget::OnOutputSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  if (!m_InternalEvent) this->ConfigureWidgets();
}

void QmitkConvertToMultiLabelSegmentationWidget::OnRefSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  if (!m_InternalEvent) this->ConfigureWidgets();
}


void QmitkConvertToMultiLabelSegmentationWidget::OnConvertPressed()
{
  auto dataStorage = m_DataStorage.Lock();
  if (dataStorage.IsNull())
  {
    mitkThrow() << "QmitkConvertToMultiLabelSegmentationWidget is in invalid state. No datastorage is set.";
  }

  auto nodes = m_Controls->inputNodesSelector->GetSelectedNodes();
  mitk::ProgressBar::GetInstance()->Reset();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(3 * nodes.size() + 1);

  if (m_Controls->radioNewSeg->isChecked() && m_Controls->checkMultipleOutputs->isChecked())
  {
    for (auto& node : nodes)
    {
      this->ConvertNodes({ node });
    }
  }
  else
  {
    this->ConvertNodes(nodes);
  }
}


mitk::Image::Pointer ConvertSurfaceToImage(const mitk::Image* refImage, const mitk::Surface* surface)
{
  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();
  surfaceToImageFilter->MakeOutputBinaryOn();
  surfaceToImageFilter->UShortBinaryPixelTypeOn();
  surfaceToImageFilter->SetInput(surface);
  surfaceToImageFilter->SetImage(refImage);
  try
  {
    surfaceToImageFilter->Update();
  }
  catch (itk::ExceptionObject& excpt)
  {
    MITK_ERROR << excpt.GetDescription();
    return nullptr;
  }

  return surfaceToImageFilter->GetOutput();
}

mitk::Image::Pointer ConvertContourModelSetToImage(mitk::Image* refImage, mitk::ContourModelSet* contourSet)
{
  // Use mitk::ContourModelSetToImageFilter to fill the ContourModelSet into the image
  mitk::ContourModelSetToImageFilter::Pointer contourFiller = mitk::ContourModelSetToImageFilter::New();
  contourFiller->SetImage(refImage);
  contourFiller->SetInput(contourSet);
  contourFiller->MakeOutputLabelPixelTypeOn();

  try
  {
    contourFiller->Update();
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Error while converting contour model. " << e.what();
  }
  catch (...)
  {
    MITK_ERROR << "Unknown error while converting contour model.";
  }

  return contourFiller->GetOutput();
}

void CheckForLabelCollision(const QmitkNodeSelectionDialog::NodeList& nodes,
  const std::map<const mitk::DataNode*, mitk::LabelSetImage::LabelValueVectorType>& foundLabelsMap,
  mitk::LabelSetImage::LabelValueVectorType& usedLabelValues,
  std::map<const mitk::DataNode*, mitk::LabelValueMappingVector>& labelsMappingMap)
{
  for (const auto& node : nodes)
  {
    mitk::ProgressBar::GetInstance()->Progress();

    const auto& foundLabels = foundLabelsMap.at(node);
    mitk::LabelSetImage::LabelValueVectorType correctedLabelValues;
    mitk::CheckForLabelValueConflictsAndResolve(foundLabels, usedLabelValues, correctedLabelValues);

    mitk::LabelValueMappingVector mapping;
    std::transform(foundLabels.begin(), foundLabels.end(), correctedLabelValues.begin(), std::back_inserter(mapping),
      [](mitk::LabelSetImage::LabelValueType a, mitk::LabelSetImage::LabelValueType b) { return std::make_pair(a, b); });
    labelsMappingMap.emplace(node, mapping);
  }
}

void QmitkConvertToMultiLabelSegmentationWidget::ConvertNodes(const QmitkNodeSelectionDialog::NodeList& nodes)
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  auto nonimageNodes = GetNonimageNodes(nodes);
  auto imageNodes = GetImageNodes(nodes);

  mitk::LabelSetImage::Pointer outputSeg;
  mitk::Image::Pointer refImage;
  const mitk::DataNode* refNode;

  if (m_Controls->radioAddToSeg->isChecked())
  {
    outputSeg = dynamic_cast<mitk::LabelSetImage*>(m_Controls->outputSegSelector->GetSelectedNode()->GetData());

    if (outputSeg->GetNumberOfLayers() > 0)
    {
      refImage = outputSeg->GetGroupImage(0);
    }
    else
    {
      //in case we work with a output seg, we need to generate a template image
      //reason is that the conversion filters used for surfaces or contours need images
      //as reference and MultiLabelSegmentations is currently empty.
      refImage = mitk::Image::New();
      refImage->Initialize(mitk::MakePixelType<mitk::LabelSetImage::LabelValueType, mitk::LabelSetImage::LabelValueType, 1>(), *(outputSeg->GetTimeGeometry()));
    }
  }
  else
  {
    outputSeg = mitk::LabelSetImage::New();

    auto inputNode = GetNodeWithLargestImageGeometry(m_Controls->inputNodesSelector->GetSelectedNodes());
    if (nullptr != inputNode)
    {
      refNode = inputNode;
      refImage = dynamic_cast<mitk::Image*>(inputNode->GetData());
      outputSeg->Initialize(refImage);
    }
    else
    {
      refNode = m_Controls->refNodeSelector->GetSelectedNode();
      refImage = dynamic_cast<mitk::Image*>(refNode->GetData());
      outputSeg->Initialize(refImage);
    }
  }

  //convert non-image nodes to images
  std::map<const mitk::DataNode*, mitk::Image::Pointer> preparedImageMap;
  std::map<const mitk::DataNode*, mitk::LabelSetImage::LabelValueVectorType> foundLabelsMap;
  for (const auto& node : nonimageNodes)
  {
    mitk::ProgressBar::GetInstance()->Progress();
    mitk::Image::Pointer convertedImage;

    auto surface = dynamic_cast<mitk::Surface*>(node->GetData());
    auto contourModel = dynamic_cast<mitk::ContourModel*>(node->GetData());
    auto contourModelSet = dynamic_cast<mitk::ContourModelSet*>(node->GetData());
    if (nullptr != surface)
    {
      convertedImage = ConvertSurfaceToImage(refImage, surface);
    }
    else if (nullptr != contourModelSet)
    {
      convertedImage = ConvertContourModelSetToImage(refImage, contourModelSet);
    }
    else if (nullptr != contourModel)
    {
      auto contourModelSet = mitk::ContourModelSet::New();
      contourModelSet->AddContourModel(contourModel);
      convertedImage = ConvertContourModelSetToImage(refImage, contourModelSet);
    }
    else
    {
      mitkThrow() << "Invalid state of QmitkConvertToMultiLabelSegmentationWidget. At least one input is of invalid type, that should not be possible to select. Invalid node: " << *(node.GetPointer());
    }

    if (convertedImage.IsNotNull())
    {
      preparedImageMap.emplace(node, convertedImage);
      //all non-image data is converted to binary maps
      foundLabelsMap.emplace(node, mitk::LabelSetImage::LabelValueVectorType({ 1 }));
    }
    else
    {
      mitkThrow() << "Failed to convert an input. Failed node: " << *(node.GetPointer());
    }
  }

  //prepare image nodes and get contained labels
  for (const auto& node : imageNodes)
  {
    mitk::ProgressBar::GetInstance()->Progress();
    mitk::LabelSetImage::LabelValueVectorType foundLabels;

    mitk::ProgressBar::GetInstance()->Progress();
    mitk::Image::Pointer convertedImage = mitk::ConvertImageToGroupImage(dynamic_cast<mitk::Image*>(node->GetData()), foundLabels);
    preparedImageMap.emplace(node, convertedImage);
    foundLabelsMap.emplace(node, foundLabels);
  }

  //check for label collision and fix if needed
  mitk::LabelSetImage::LabelValueVectorType usedLabelValues = outputSeg->GetAllLabelValues();
  std::map<const mitk::DataNode*, mitk::LabelValueMappingVector> labelsMappingMap;
  CheckForLabelCollision(imageNodes, foundLabelsMap, usedLabelValues, labelsMappingMap);
  CheckForLabelCollision(nonimageNodes, foundLabelsMap, usedLabelValues, labelsMappingMap);

  //Ensure that we have the first layer to add
  mitk::LabelSetImage::GroupIndexType currentGroupIndex = 0;
  if (m_Controls->radioAddToSeg->isChecked() || 0 == outputSeg->GetNumberOfLayers())
  {
    currentGroupIndex = outputSeg->AddLayer();
  }

  //Transfer content and add labels
  for (const auto& node : imageNodes)
  {
    mitk::ProgressBar::GetInstance()->Progress();

    if (m_Controls->radioSingleGroup->isChecked() && node != imageNodes.front())
      currentGroupIndex = outputSeg->AddLayer();

    const auto& labelsMapping = labelsMappingMap.at(node);
    for (auto [oldV, correctedV] : labelsMapping)
    {
      std::string name = "Value " + std::to_string(oldV);
      if (m_Controls->radioMergeGroup->isChecked())
        name = node->GetName() + " " + name;

      auto label = mitk::LabelSetImageHelper::CreateNewLabel(outputSeg, name, true);
      label->SetValue(correctedV);

      outputSeg->AddLabel(label, currentGroupIndex, false, false);
    }

    mitk::TransferLabelContent(preparedImageMap.at(node), outputSeg->GetGroupImage(currentGroupIndex),
      outputSeg->GetConstLabelsByValue(outputSeg->GetLabelValuesByGroup(currentGroupIndex)),
      mitk::LabelSetImage::UNLABELED_VALUE, mitk::LabelSetImage::UNLABELED_VALUE, false, labelsMapping);
  }

  for (const auto& node : nonimageNodes)
  {
    mitk::ProgressBar::GetInstance()->Progress();

    if (m_Controls->radioSingleGroup->isChecked() && (node != nonimageNodes.front() || !imageNodes.empty()))
      currentGroupIndex = outputSeg->AddLayer();

    const auto& labelsMapping = labelsMappingMap.at(node);
    for (auto [oldV, correctedV] : labelsMapping)
    {
      auto label = mitk::LabelSetImageHelper::CreateNewLabel(outputSeg, node->GetName(), true);
      label->SetValue(correctedV);
      mitk::ColorProperty::ConstPointer colorProp = dynamic_cast<const mitk::ColorProperty*>(node->GetConstProperty("color").GetPointer());
      if (colorProp.IsNotNull())
        label->SetColor(colorProp->GetColor());

      outputSeg->AddLabel(label, currentGroupIndex, false, false);
    }

    mitk::TransferLabelContent(preparedImageMap.at(node), outputSeg->GetGroupImage(currentGroupIndex),
      outputSeg->GetConstLabelsByValue(outputSeg->GetLabelValuesByGroup(currentGroupIndex)),
      mitk::LabelSetImage::UNLABELED_VALUE, mitk::LabelSetImage::UNLABELED_VALUE, false, labelsMapping);
  }

  if (m_Controls->radioAddToSeg->isChecked())
  {
    m_Controls->outputSegSelector->GetSelectedNode()->Modified();
  }
  else
  {
    mitk::DataNode::Pointer outNode = mitk::DataNode::New();
    std::stringstream stream;
    stream << "ConvertedSeg";
    if (nodes.size() == 1)
    {
      stream << "_" << nodes.front()->GetName();
    }
    outNode->SetName(stream.str());
    outNode->SetData(outputSeg);

    auto dataStorage = m_DataStorage.Lock();
    dataStorage->Add(outNode);
  }
  mitk::ProgressBar::GetInstance()->Reset();
  QApplication::restoreOverrideCursor();
}
