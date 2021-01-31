/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageStatisticsView.h"

#include <utility>

// berry includes
#include <berryIQtStyleManager.h>
#include <berryWorkbenchPlugin.h>

#include <QmitkChartWidget.h>
#include <mitkImageStatisticsContainerNodeHelper.h>
#include <mitkImageStatisticsPredicateHelper.h>
#include <mitkImageTimeSelector.h>
#include <mitkIntensityProfile.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkNodePredicateSubGeometry.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateFunction.h>
#include <mitkSliceNavigationController.h>
#include <mitkStatusBar.h>
#include <mitkPlanarFigure.h>
#include "mitkPlanarFigureMaskGenerator.h"

#include "QmitkImageStatisticsDataGenerator.h"

#include "mitkImageStatisticsContainerManager.h"
#include <mitkPlanarFigureInteractor.h>

const std::string QmitkImageStatisticsView::VIEW_ID = "org.mitk.views.imagestatistics";

QmitkImageStatisticsView::~QmitkImageStatisticsView()
{
}

void QmitkImageStatisticsView::CreateQtPartControl(QWidget *parent)
{
  m_Controls.setupUi(parent);
  m_Controls.widget_intensityProfile->SetTheme(GetColorTheme());
  m_Controls.groupBox_histogram->setVisible(true);
  m_Controls.groupBox_intensityProfile->setVisible(false);
  m_Controls.label_currentlyComputingStatistics->setVisible(false);
  m_Controls.sliderWidget_histogram->setPrefix("Time: ");
  m_Controls.sliderWidget_histogram->setDecimals(0);
  m_Controls.sliderWidget_histogram->setVisible(false);
  m_Controls.sliderWidget_intensityProfile->setPrefix("Time: ");
  m_Controls.sliderWidget_intensityProfile->setDecimals(0);
  m_Controls.sliderWidget_intensityProfile->setVisible(false);
  ResetGUI();

  m_DataGenerator = new QmitkImageStatisticsDataGenerator(parent);
  m_DataGenerator->SetDataStorage(this->GetDataStorage());
  m_DataGenerator->SetAutoUpdate(true);
  m_Controls.widget_statistics->SetDataStorage(this->GetDataStorage());

  m_Controls.imageNodesSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.imageNodesSelector->SetNodePredicate(mitk::GetImageStatisticsImagePredicate());
  m_Controls.imageNodesSelector->SetSelectionCheckFunction(this->CheckForSameGeometry());
  m_Controls.imageNodesSelector->SetSelectionIsOptional(false);
  m_Controls.imageNodesSelector->SetInvalidInfo(QStringLiteral("Please select images for statistics"));
  m_Controls.imageNodesSelector->SetPopUpTitel(QStringLiteral("Select input images"));
  m_Controls.roiNodesSelector->SetPopUpHint(QStringLiteral("You may select multiple images for the statistics computation. But all selected images must have the same geometry."));

  m_Controls.roiNodesSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.roiNodesSelector->SetNodePredicate(this->GenerateROIPredicate());
  m_Controls.roiNodesSelector->SetSelectionIsOptional(true);
  m_Controls.roiNodesSelector->SetEmptyInfo(QStringLiteral("Please select ROIs"));
  m_Controls.roiNodesSelector->SetPopUpTitel(QStringLiteral("Select ROIs for statistics computation"));
  m_Controls.roiNodesSelector->SetPopUpHint(QStringLiteral("You may select ROIs (e.g. planar figures, segmentations) that should be used for the statistics computation. The statistics will only computed for the image parts defined by the ROIs."));

  CreateConnections();

  this->m_TimePointChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
  connect(&m_TimePointChangeListener, &QmitkSliceNavigationListener::SelectedTimePointChanged, this, & QmitkImageStatisticsView::OnSelectedTimePointChanged);
}

void QmitkImageStatisticsView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_TimePointChangeListener.RenderWindowPartActivated(renderWindowPart);
}

void QmitkImageStatisticsView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  this->m_TimePointChangeListener.RenderWindowPartDeactivated(renderWindowPart);
}

void QmitkImageStatisticsView::CreateConnections()
{
  connect(m_Controls.checkBox_ignoreZero, &QCheckBox::stateChanged,
    this, &QmitkImageStatisticsView::OnCheckBoxIgnoreZeroStateChanged);
  connect(m_Controls.buttonSelection, &QAbstractButton::clicked,
    this, &QmitkImageStatisticsView::OnButtonSelectionPressed);

  connect(m_Controls.widget_histogram, &QmitkHistogramVisualizationWidget::RequestHistogramUpdate,
    this, &QmitkImageStatisticsView::OnRequestHistogramUpdate);

  connect(m_DataGenerator, &QmitkImageStatisticsDataGenerator::DataGenerationStarted,
    this, &QmitkImageStatisticsView::OnGenerationStarted);
  connect(m_DataGenerator, &QmitkImageStatisticsDataGenerator::GenerationFinished,
    this, &QmitkImageStatisticsView::OnGenerationFinished);
  connect(m_DataGenerator, &QmitkImageStatisticsDataGenerator::JobError,
    this, &QmitkImageStatisticsView::OnJobError);

  connect(m_Controls.imageNodesSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkImageStatisticsView::OnImageSelectionChanged);
  connect(m_Controls.roiNodesSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkImageStatisticsView::OnROISelectionChanged);

  connect(m_Controls.sliderWidget_intensityProfile, &ctkSliderWidget::valueChanged, this, &QmitkImageStatisticsView::UpdateIntensityProfile);
}

void QmitkImageStatisticsView::UpdateIntensityProfile()
{
  m_Controls.groupBox_intensityProfile->setVisible(false);

  const auto selectedImageNodes = m_Controls.imageNodesSelector->GetSelectedNodes();
  const auto selectedROINodes = m_Controls.roiNodesSelector->GetSelectedNodes();

  if (selectedImageNodes.size()==1 && selectedROINodes.size()==1)
  { //only supported for one image and roi currently
    auto image = dynamic_cast<mitk::Image*>(selectedImageNodes.front()->GetData());

    auto maskPlanarFigure = dynamic_cast<mitk::PlanarFigure*>(selectedROINodes.front()->GetData());

    if (maskPlanarFigure != nullptr)
    {
      if (!maskPlanarFigure->IsClosed())
      {
        mitk::Image::Pointer inputImage;
        if (image->GetDimension() == 4)
        {
          m_Controls.sliderWidget_intensityProfile->setVisible(true);
          unsigned int maxTimestep = image->GetTimeSteps();
          m_Controls.sliderWidget_intensityProfile->setMaximum(maxTimestep - 1);
          // Intensity profile can only be calculated on 3D, so extract if 4D
          mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
          int currentTimestep = static_cast<int>(m_Controls.sliderWidget_intensityProfile->value());
          timeSelector->SetInput(image);
          timeSelector->SetTimeNr(currentTimestep);
          timeSelector->Update();

          inputImage = timeSelector->GetOutput();
        }
        else
        {
          m_Controls.sliderWidget_intensityProfile->setVisible(false);
          inputImage = image;
        }

        auto intensityProfile = mitk::ComputeIntensityProfile(inputImage, maskPlanarFigure);
        m_Controls.groupBox_intensityProfile->setVisible(true);
        m_Controls.widget_intensityProfile->Reset();
        m_Controls.widget_intensityProfile->SetIntensityProfile(intensityProfile.GetPointer(),
          "Intensity Profile of " + selectedImageNodes.front()->GetName());
      }
    }
  }
}

void QmitkImageStatisticsView::UpdateHistogramWidget()
{
  bool visibility = false;

  const auto selectedImageNodes = m_Controls.imageNodesSelector->GetSelectedNodes();
  const auto selectedMaskNodes = m_Controls.roiNodesSelector->GetSelectedNodes();

  if (selectedImageNodes.size() == 1 && selectedMaskNodes.size()<=1)
  { //currently only supported for one image and roi due to histogram widget limitations.
    auto imageNode = selectedImageNodes.front();
    const mitk::DataNode* roiNode = nullptr;
    const mitk::PlanarFigure* planarFigure = nullptr;
    if (!selectedMaskNodes.empty())
    {
      roiNode = selectedMaskNodes.front();
      planarFigure = dynamic_cast<const mitk::PlanarFigure*>(roiNode->GetData());
    }

    if ((planarFigure == nullptr || planarFigure->IsClosed())
        && imageNode->GetData()->GetTimeGeometry()->IsValidTimePoint(m_TimePointChangeListener.GetCurrentSelectedTimePoint()))
    { //if a planar figure is not closed, we show the intensity profile instead of the histogram.
      auto statisticsNode = m_DataGenerator->GetLatestResult(imageNode, roiNode, true);

      if (statisticsNode.IsNotNull())
      {
        auto statistics = dynamic_cast<const mitk::ImageStatisticsContainer*>(statisticsNode->GetData());

        if (statistics)
        {
          const auto timeStep = imageNode->GetData()->GetTimeGeometry()->TimePointToTimeStep(m_TimePointChangeListener.GetCurrentSelectedTimePoint());

          std::stringstream label;
          label << imageNode->GetName();
          if (imageNode->GetData()->GetTimeSteps() > 1)
          {
            label << "["<< timeStep <<"]";
          }

          if (roiNode)
          {
            label << " with " << roiNode->GetName();
          }

          //Hardcoded labels are currently needed because the current histogram widget (and ChartWidget)
          //do not allow correct removal or sound update/insertion of serveral charts.
          //only thing that works for now is always to update/overwrite the same data label
          //This is a quick fix for T28223 and T28221
          m_Controls.widget_histogram->SetHistogram(statistics->GetHistogramForTimeStep(timeStep), "histogram");

          visibility = true;
        }
      }
    }
  }

  if (visibility != m_Controls.groupBox_histogram->isVisible())
  {
    m_Controls.groupBox_histogram->setVisible(visibility);
  }

}

QmitkChartWidget::ColorTheme QmitkImageStatisticsView::GetColorTheme() const
{
  ctkPluginContext *context = berry::WorkbenchPlugin::GetDefault()->GetPluginContext();
  ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
  if (styleManagerRef)
  {
    auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
    if (styleManager->GetStyle().name == "Dark")
    {
      return QmitkChartWidget::ColorTheme::darkstyle;
    }
    else
    {
      return QmitkChartWidget::ColorTheme::lightstyle;
    }
  }
  return QmitkChartWidget::ColorTheme::darkstyle;
}

void QmitkImageStatisticsView::ResetGUI()
{
  m_Controls.widget_statistics->Reset();
  m_Controls.widget_statistics->setEnabled(false);
  m_Controls.widget_histogram->Reset();
  m_Controls.widget_histogram->setEnabled(false);
  m_Controls.widget_histogram->SetTheme(GetColorTheme());
}

void QmitkImageStatisticsView::OnGenerationStarted(const mitk::DataNode* /*imageNode*/, const mitk::DataNode* /*roiNode*/, const QmitkDataGenerationJobBase* /*job*/)
{
  m_Controls.label_currentlyComputingStatistics->setVisible(true);
}

void QmitkImageStatisticsView::OnGenerationFinished()
{
  m_Controls.label_currentlyComputingStatistics->setVisible(false);

  mitk::StatusBar::GetInstance()->Clear();

  this->UpdateIntensityProfile();
  this->UpdateHistogramWidget();
}

void QmitkImageStatisticsView::OnSelectedTimePointChanged(const mitk::TimePointType& /*newTimePoint*/)
{
  this->UpdateHistogramWidget();
}

void QmitkImageStatisticsView::OnJobError(QString error, const QmitkDataGenerationJobBase* /*failedJob*/)
{
  mitk::StatusBar::GetInstance()->DisplayErrorText(error.toStdString().c_str());
  MITK_WARN << "Error when calculating statistics: " << error;
}

void QmitkImageStatisticsView::OnRequestHistogramUpdate(unsigned int nbins)
{
  m_Controls.widget_statistics->SetHistogramNBins(nbins);
  m_DataGenerator->SetHistogramNBins(nbins);
  this->UpdateIntensityProfile();
  this->UpdateHistogramWidget();
}

void QmitkImageStatisticsView::OnCheckBoxIgnoreZeroStateChanged(int state)
{
  auto ignoreZeroValueVoxel = (state == Qt::Unchecked) ? false : true;
  m_Controls.widget_statistics->SetIgnoreZeroValueVoxel(ignoreZeroValueVoxel);
  m_DataGenerator->SetIgnoreZeroValueVoxel(ignoreZeroValueVoxel);
  this->UpdateIntensityProfile();
  this->UpdateHistogramWidget();
}

void QmitkImageStatisticsView::OnImageSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  auto images = m_Controls.imageNodesSelector->GetSelectedNodesStdVector();
  m_Controls.widget_statistics->SetImageNodes(images);

  m_Controls.widget_statistics->setEnabled(!images.empty());

  m_Controls.roiNodesSelector->SetNodePredicate(this->GenerateROIPredicate());

  m_DataGenerator->SetAutoUpdate(false);
  m_DataGenerator->SetImageNodes(images);
  m_DataGenerator->Generate();
  m_DataGenerator->SetAutoUpdate(true);

  this->UpdateHistogramWidget();
  this->UpdateIntensityProfile();
}

void QmitkImageStatisticsView::OnROISelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  auto rois = m_Controls.roiNodesSelector->GetSelectedNodesStdVector();

  m_Controls.widget_statistics->SetMaskNodes(rois);

  m_DataGenerator->SetAutoUpdate(false);
  m_DataGenerator->SetROINodes(rois);
  m_DataGenerator->Generate();
  m_DataGenerator->SetAutoUpdate(true);

  this->UpdateHistogramWidget();
  this->UpdateIntensityProfile();
}


void QmitkImageStatisticsView::OnButtonSelectionPressed()
{
  QmitkNodeSelectionDialog* dialog = new QmitkNodeSelectionDialog(nullptr, "Select input for the statistic","You may select images and ROIs to compute their statistic. ROIs may be segmentations or planar figures.");
  dialog->SetDataStorage(GetDataStorage());
  dialog->SetSelectionCheckFunction(CheckForSameGeometry());

  // set predicates
  auto isPlanarFigurePredicate = mitk::GetImageStatisticsPlanarFigurePredicate();
  auto isMaskPredicate = mitk::GetImageStatisticsMaskPredicate();
  auto isImagePredicate = mitk::GetImageStatisticsImagePredicate();
  auto isMaskOrPlanarFigurePredicate = mitk::NodePredicateOr::New(isPlanarFigurePredicate, isMaskPredicate);
  auto isImageOrMaskOrPlanarFigurePredicate = mitk::NodePredicateOr::New(isMaskOrPlanarFigurePredicate, isImagePredicate);
  dialog->SetNodePredicate(isImageOrMaskOrPlanarFigurePredicate);
  dialog->SetSelectionMode(QAbstractItemView::MultiSelection);
  dialog->SetCurrentSelection(m_Controls.imageNodesSelector->GetSelectedNodes()+m_Controls.roiNodesSelector->GetSelectedNodes());

  if (dialog->exec())
  {
    auto selectedNodeList = dialog->GetSelectedNodes();

    m_Controls.imageNodesSelector->SetCurrentSelection(selectedNodeList);
    m_Controls.roiNodesSelector->SetCurrentSelection(selectedNodeList);
  }

  delete dialog;
}

QmitkNodeSelectionDialog::SelectionCheckFunctionType QmitkImageStatisticsView::CheckForSameGeometry() const
{
  auto isMaskPredicate = mitk::GetImageStatisticsMaskPredicate();

  auto lambda = [isMaskPredicate](const QmitkNodeSelectionDialog::NodeList& nodes)
  {
    if (nodes.empty())
    {
      return std::string();
    }

    const mitk::Image* imageNodeData = nullptr;
    for (auto& node : nodes)
    {
      auto castedData = dynamic_cast<const mitk::Image*>(node->GetData());
      if (castedData != nullptr && !isMaskPredicate->CheckNode(node))
      {
        imageNodeData = castedData;
        break;
      }
    }

    if (imageNodeData == nullptr)
    {
      std::stringstream ss;
      ss << "<font class=\"warning\"><p>Select at least one image.</p></font>";
      return ss.str();
    }

    auto imageGeoPredicate = mitk::NodePredicateGeometry::New(imageNodeData->GetGeometry());
    auto maskGeoPredicate = mitk::NodePredicateSubGeometry::New(imageNodeData->GetGeometry());

    for (auto& rightNode : nodes)
    {
      if (imageNodeData != rightNode->GetData())
      {
        bool validGeometry = true;

        if (isMaskPredicate->CheckNode(rightNode))
        {
          validGeometry = maskGeoPredicate->CheckNode(rightNode);
        }
        else if (dynamic_cast<const mitk::Image*>(rightNode->GetData()))
        {
          validGeometry = imageGeoPredicate->CheckNode(rightNode);
        }
        else
        {
          const mitk::PlanarFigure* planar2 = dynamic_cast<const mitk::PlanarFigure*>(rightNode->GetData());
          if (planar2)
          {
            validGeometry = mitk::PlanarFigureMaskGenerator::CheckPlanarFigureIsNotTilted(planar2->GetPlaneGeometry(), imageNodeData->GetGeometry());
          }
        }

        if (!validGeometry)
        {
          std::stringstream ss;
          ss << "<font class=\"warning\"><p>Invalid selection: All selected nodes must have the same geometry.</p><p>Differing node i.a.: \"";
          ss << rightNode->GetName() <<"\"</p></font>";
          return ss.str();
        }
      }
    }

    return std::string();
  };

  return lambda;
}

mitk::NodePredicateBase::Pointer QmitkImageStatisticsView::GenerateROIPredicate() const
{
  auto isPlanarFigurePredicate = mitk::GetImageStatisticsPlanarFigurePredicate();
  auto isMaskPredicate = mitk::GetImageStatisticsMaskPredicate();
  auto isMaskOrPlanarFigurePredicate = mitk::NodePredicateOr::New(isPlanarFigurePredicate, isMaskPredicate);

  mitk::NodePredicateBase::Pointer result = isMaskOrPlanarFigurePredicate.GetPointer();

  if(!m_Controls.imageNodesSelector->GetSelectedNodes().empty())
  {
    auto image = m_Controls.imageNodesSelector->GetSelectedNodes().front()->GetData();
    auto imageGeoPredicate = mitk::NodePredicateSubGeometry::New(image->GetGeometry());

    auto lambda = [image, imageGeoPredicate](const mitk::DataNode* node)
    {
      bool sameGeometry = true;

      if (dynamic_cast<const mitk::Image*>(node->GetData()) != nullptr)
      {
        sameGeometry = imageGeoPredicate->CheckNode(node);
      }
      else
      {
        const auto planar2 = dynamic_cast<const mitk::PlanarFigure*>(node->GetData());
        if (planar2)
        {
          sameGeometry = mitk::PlanarFigureMaskGenerator::CheckPlanarFigureIsNotTilted(planar2->GetPlaneGeometry(), image->GetGeometry());
        }
      }

      return sameGeometry;
    };

    result = mitk::NodePredicateAnd::New(isMaskOrPlanarFigurePredicate, mitk::NodePredicateFunction::New(lambda)).GetPointer();
  }

  return result;
}
