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

#include "QmitkImageStatisticsReloadedView.h"

#include <utility>

// berry includes
#include <berryWorkbenchPlugin.h>
#include <berryIQtStyleManager.h>

#include <QmitkChartWidget.h>
#include <mitkNodePredicateOr.h>
#include <mitkStatusBar.h>
#include <mitkIntensityProfile.h>
#include <mitkImageStatisticsPredicateHelper.h>
#include <mitkImageStatisticsContainerNodeHelper.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkSliceNavigationController.h>
#include <mitkStatisticsToImageRelationRule.h>
#include <mitkStatisticsToMaskRelationRule.h>

#include <mitkPlanarFigureInteractor.h>
#include "mitkImageStatisticsContainerManager.h"


const std::string QmitkImageStatisticsReloadedView::VIEW_ID = "org.mitk.views.imagestatisticsReloaded";

QmitkImageStatisticsReloadedView::QmitkImageStatisticsReloadedView(QObject* /*parent*/, const char* /*name*/)
{
  this->m_CalculationThread = new QmitkImageStatisticsCalculationJob();
}

QmitkImageStatisticsReloadedView::~QmitkImageStatisticsReloadedView()
{
  if (m_selectedPlanarFigure)
    m_selectedPlanarFigure->RemoveObserver(m_PlanarFigureObserverTag);
}

void QmitkImageStatisticsReloadedView::CreateQtPartControl(QWidget *parent)
{
  m_Controls.setupUi(parent);
  m_Controls.widget_histogram->SetTheme(this->GetColorTheme());
  m_Controls.widget_intensityProfile->SetTheme(this->GetColorTheme());
  m_Controls.groupBox_histogram->setVisible(true);
  m_Controls.groupBox_intensityProfile->setVisible(false);
  m_Controls.label_currentlyComputingStatistics->setVisible(false);
  m_Controls.sliderWidget_histogram->setPrefix("Time: ");
  m_Controls.sliderWidget_histogram->setDecimals(0);
  m_Controls.sliderWidget_histogram->setVisible(false);

  PrepareDataStorageComboBoxes();
  m_Controls.widget_statistics->SetDataStorage(this->GetDataStorage());
  CreateConnections();
}

void QmitkImageStatisticsReloadedView::CreateConnections()
{
  connect(this->m_CalculationThread, &QmitkImageStatisticsCalculationJob::finished, this, &QmitkImageStatisticsReloadedView::OnStatisticsCalculationEnds, Qt::QueuedConnection);
  connect(this->m_Controls.checkBox_ignoreZero, &QCheckBox::stateChanged, this, &QmitkImageStatisticsReloadedView::OnCheckBoxIgnoreZeroStateChanged);
  connect(this->m_Controls.sliderWidget_histogram, &ctkSliderWidget::valueChanged, this, &QmitkImageStatisticsReloadedView::OnSliderWidgetHistogramChanged);
  connect(this->m_Controls.imageSelector, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QmitkImageStatisticsReloadedView::OnImageSelectorChanged);
  connect(this->m_Controls.maskImageSelector, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QmitkImageStatisticsReloadedView::OnMaskSelectorChanged);
}

void QmitkImageStatisticsReloadedView::OnCheckBoxIgnoreZeroStateChanged(int state)
{
  m_ForceRecompute = true;
  if (state != Qt::Unchecked)
  {
    this->m_CalculationThread->SetIgnoreZeroValueVoxel(true);
  }
  else
  {
    this->m_CalculationThread->SetIgnoreZeroValueVoxel(false);
  }
  CalculateOrGetStatistics();
}

void QmitkImageStatisticsReloadedView::OnSliderWidgetHistogramChanged(double value)
{
  unsigned int timeStep = static_cast<unsigned int>(value);
  auto mask = m_selectedMaskNode ? m_selectedMaskNode->GetData() : nullptr;
  auto imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), m_selectedImageNode->GetData(), mask);
  HistogramType::ConstPointer histogram = imageStatistics->GetStatisticsForTimeStep(timeStep).m_Histogram;

  if (histogram.IsNotNull() && this->m_CalculationThread->GetStatisticsUpdateSuccessFlag())
  {
    this->FillHistogramWidget({ histogram }, { m_selectedImageNode->GetName() });
  }
}

void QmitkImageStatisticsReloadedView::PartClosed(const berry::IWorkbenchPartReference::Pointer&)
{
}

void QmitkImageStatisticsReloadedView::FillHistogramWidget(const std::vector<HistogramType::ConstPointer>& histogram, const std::vector<std::string>& dataLabels)
{
  m_Controls.groupBox_histogram->setVisible(true);
  m_Controls.widget_histogram->SetTheme(this->GetColorTheme());
  m_Controls.widget_histogram->Reset();
  m_Controls.widget_histogram->SetHistogram(histogram.front(), dataLabels.front());
  connect(m_Controls.widget_histogram, &QmitkHistogramVisualizationWidget::RequestHistogramUpdate, this, &QmitkImageStatisticsReloadedView::OnRequestHistogramUpdate);
}

QmitkChartWidget::ColorTheme QmitkImageStatisticsReloadedView::GetColorTheme() const
{
  ctkPluginContext* context = berry::WorkbenchPlugin::GetDefault()->GetPluginContext();
  ctkServiceReference styleManagerRef = context->getServiceReference<berry::IQtStyleManager>();
  if (styleManagerRef)
  {
    auto styleManager = context->getService<berry::IQtStyleManager>(styleManagerRef);
    if (styleManager->GetStyle().name == "Dark") {
      return QmitkChartWidget::ColorTheme::darkstyle;
    }
    else {
      return QmitkChartWidget::ColorTheme::lightstyle;
    }
  }
  return QmitkChartWidget::ColorTheme::darkstyle;
}

void QmitkImageStatisticsReloadedView::OnImageSelectorChanged()
{
  auto selectedImageNode = m_Controls.imageSelector->GetSelectedNode();
  if (selectedImageNode != m_selectedImageNode)
  {
    m_selectedImageNode = selectedImageNode;
    if (m_selectedImageNode.IsNotNull()) {
      auto isPlanarFigurePredicate = mitk::GetImageStatisticsPlanarFigurePredicate();
      auto isMaskPredicate = mitk::GetImageStatisticsMaskPredicate();
      auto hasSameGeometry = mitk::NodePredicateGeometry::New(m_selectedImageNode->GetData()->GetGeometry());
      hasSameGeometry->SetCheckPrecision(1e-10);
      auto isMaskWithGeometryPredicate = mitk::NodePredicateAnd::New(isMaskPredicate, hasSameGeometry);
      auto isMaskOrPlanarFigureWithGeometryPredicate = mitk::NodePredicateOr::New(isPlanarFigurePredicate, isMaskWithGeometryPredicate);
      //prevent triggering of computation as the predicate triggers a signalChanged event
      m_Controls.maskImageSelector->disconnect();
      m_Controls.maskImageSelector->SetPredicate(isMaskOrPlanarFigureWithGeometryPredicate);
      //reset mask to <none>
      m_Controls.maskImageSelector->SetZeroEntryText("<none>");
      m_selectedMaskNode = nullptr;
      m_Controls.widget_statistics->SetMaskNodes({});
      CalculateOrGetStatistics();
      m_Controls.widget_statistics->SetImageNodes({ m_selectedImageNode.GetPointer() });
      connect(this->m_Controls.maskImageSelector,
              static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
              this,
              &QmitkImageStatisticsReloadedView::OnMaskSelectorChanged);
    }
    else
    {
      m_Controls.widget_statistics->SetImageNodes({});
      m_Controls.widget_statistics->Reset();
    }
  }
}

void QmitkImageStatisticsReloadedView::OnMaskSelectorChanged()
{
  auto selectedMaskNode = m_Controls.maskImageSelector->GetSelectedNode();
  if (selectedMaskNode != m_selectedMaskNode)
  {
    m_selectedMaskNode = selectedMaskNode;
    if (m_selectedMaskNode.IsNotNull())
    {
      m_Controls.widget_statistics->SetMaskNodes({ m_selectedMaskNode.GetPointer() });
    }
    else
    {
      m_Controls.widget_statistics->SetMaskNodes({});
    }
    CalculateOrGetStatistics();
  }
}

void QmitkImageStatisticsReloadedView::CalculateOrGetStatistics()
{
  if (this->m_selectedPlanarFigure)
  {
    this->m_selectedPlanarFigure->RemoveObserver(this->m_PlanarFigureObserverTag);
    this->m_selectedPlanarFigure = nullptr;
  }

  m_Controls.groupBox_intensityProfile->setVisible(false);
  m_Controls.widget_statistics->setEnabled(m_selectedImageNode.IsNotNull());

  if (m_selectedImageNode != nullptr) {
    auto image = dynamic_cast<mitk::Image*>(m_selectedImageNode->GetData());
    mitk::Image::Pointer mask = nullptr;
    mitk::PlanarFigure::Pointer maskPlanarFigure = nullptr;

    if (image->GetDimension() == 4) {
      m_Controls.sliderWidget_histogram->setVisible(true);
      unsigned int maxTimestep = image->GetTimeSteps();
      m_Controls.sliderWidget_histogram->setMaximum(maxTimestep - 1);
    }
    else {
      m_Controls.sliderWidget_histogram->setVisible(false);
    }

    if (m_selectedMaskNode != nullptr) {
      mask = dynamic_cast<mitk::Image*>(m_selectedMaskNode->GetData());
      if (mask == nullptr) {
        maskPlanarFigure = dynamic_cast<mitk::PlanarFigure*>(m_selectedMaskNode->GetData());
      }
    }

    mitk::ImageStatisticsContainer::ConstPointer imageStatistics;
    if (mask) {
      auto imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, mask.GetPointer());
    }
    else if (maskPlanarFigure) {
      m_selectedPlanarFigure = maskPlanarFigure;
      ITKCommandType::Pointer changeListener = ITKCommandType::New();
      changeListener->SetCallbackFunction(this, &QmitkImageStatisticsReloadedView::CalculateOrGetStatistics);
      this->m_PlanarFigureObserverTag =
        m_selectedPlanarFigure->AddObserver(mitk::EndInteractionPlanarFigureEvent(), changeListener);
      if (!maskPlanarFigure->IsClosed()) {
        ComputeAndDisplayIntensityProfile(image, maskPlanarFigure);
      }
      imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, maskPlanarFigure.GetPointer());
    }
    else {
      imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(),image);
    }

    bool imageStatisticsOlderThanInputs = false;
    if (imageStatistics && (imageStatistics->GetMTime() < image->GetMTime() ||
      (mask && imageStatistics->GetMTime() < mask->GetMTime()) ||
      (maskPlanarFigure && imageStatistics->GetMTime() < maskPlanarFigure->GetMTime()))) {
      imageStatisticsOlderThanInputs = true;
    }

    //statistics need to be computed
    if (!imageStatistics || imageStatisticsOlderThanInputs || m_ForceRecompute) {
      CalculateStatistics(image, mask.GetPointer(), maskPlanarFigure.GetPointer());
    }
    //statistics already computed
    else {
      //Not an open planar figure: show histogram (intensity profile already shown)
      if (!(maskPlanarFigure && !maskPlanarFigure->IsClosed())) {
        if (imageStatistics->TimeStepExists(0)) {
          auto histogram = imageStatistics->GetStatisticsForTimeStep(0).m_Histogram.GetPointer();
          std::string imageNodeName = m_selectedImageNode->GetName();
          this->FillHistogramWidget({histogram}, {imageNodeName });
        }
      }
    }

  }
  else {
    ResetGUI();
  }
  m_ForceRecompute = false;
}

void QmitkImageStatisticsReloadedView::ComputeAndDisplayIntensityProfile(mitk::Image * image, mitk::PlanarFigure::Pointer maskPlanarFigure)
{
  auto intensityProfile = mitk::ComputeIntensityProfile(image, maskPlanarFigure);
  //Don't show histogram for intensity profiles
  m_Controls.groupBox_histogram->setVisible(false);
  m_Controls.groupBox_intensityProfile->setVisible(true);
  m_Controls.widget_intensityProfile->Reset();
  m_Controls.widget_intensityProfile->SetIntensityProfile(intensityProfile.GetPointer(), "Intensity Profile of " + m_selectedImageNode->GetName());
}

void QmitkImageStatisticsReloadedView::ResetGUI()
{
  m_Controls.widget_statistics->Reset();
  //m_Controls.widget_statistics->setEnabled(false);
  m_Controls.widget_histogram->Reset();
  m_Controls.widget_histogram->setEnabled(false);
}

void QmitkImageStatisticsReloadedView::OnStatisticsCalculationEnds()
{
  mitk::StatusBar::GetInstance()->Clear();

  if (this->m_CalculationThread->GetStatisticsUpdateSuccessFlag()) {
    auto statistic = m_CalculationThread->GetStatisticsData();
    mitk::PropertyRelations::RuleResultVectorType rulesForCurrentStatistic;
    auto statisticNonConst = statistic->Clone();
    auto statisticsNodeName = m_selectedImageNode->GetName();
    if (m_selectedMaskNode) {
      statisticsNodeName += "_" + m_selectedMaskNode->GetName();
    }
    statisticsNodeName += "_statistics";
    auto statisticsNode = mitk::CreateImageStatisticsNode(statisticNonConst, statisticsNodeName);
    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    imageRule->Connect(statisticNonConst.GetPointer(), m_CalculationThread->GetStatisticsImage().GetPointer());
    rulesForCurrentStatistic.push_back(imageRule.GetPointer());

    if (m_CalculationThread->GetMaskImage()) {
      auto maskRule = mitk::StatisticsToMaskRelationRule::New();
      maskRule->Connect(statisticNonConst.GetPointer(), m_CalculationThread->GetMaskImage().GetPointer());
      rulesForCurrentStatistic.push_back(maskRule.GetPointer());
    }
    else if (m_CalculationThread->GetPlanarFigure()) {
      auto planarFigureRule = mitk::StatisticsToMaskRelationRule::New();
      planarFigureRule->Connect(statisticNonConst.GetPointer(), m_CalculationThread->GetPlanarFigure().GetPointer());
      rulesForCurrentStatistic.push_back(planarFigureRule.GetPointer());
    }

    m_statisticContainerRules.push_back(rulesForCurrentStatistic);

    this->GetDataStorage()->Add(statisticsNode);

    if (!m_selectedPlanarFigure || m_selectedPlanarFigure->IsClosed()) {
      this->FillHistogramWidget({ m_CalculationThread->GetTimeStepHistogram() }, { m_selectedImageNode->GetName() });
    }
  }
  else {
    mitk::StatusBar::GetInstance()->DisplayErrorText(m_CalculationThread->GetLastErrorMessage().c_str());
    m_Controls.widget_histogram->setEnabled(false);
  }
  m_Controls.label_currentlyComputingStatistics->setVisible(false);
}

void QmitkImageStatisticsReloadedView::OnRequestHistogramUpdate(unsigned int nBins)
{
  m_CalculationThread->SetHistogramNBins(nBins);
  m_CalculationThread->start();
}

void QmitkImageStatisticsReloadedView::CalculateStatistics(mitk::Image::ConstPointer image, mitk::Image::ConstPointer mask, mitk::PlanarFigure::ConstPointer maskPlanarFigure)
{
  this->m_StatisticsUpdatePending = true;
  auto renderPart = this->GetRenderWindowPart();
  unsigned int timeStep = renderPart->GetTimeNavigationController()->GetTime()->GetPos();
  this->m_CalculationThread->Initialize(image, mask, maskPlanarFigure);
  this->m_CalculationThread->SetTimeStep(timeStep);

  try
  {
    // Compute statistics
    this->m_CalculationThread->start();
    m_Controls.label_currentlyComputingStatistics->setVisible(true);
  }
  catch (const mitk::Exception& e)
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText(e.GetDescription());
    this->m_StatisticsUpdatePending = false;
    m_Controls.label_currentlyComputingStatistics->setVisible(false);
  }
  catch (const std::runtime_error &e)
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText(e.what());
    this->m_StatisticsUpdatePending = false;
    m_Controls.label_currentlyComputingStatistics->setVisible(false);
  }
  catch (const std::exception &e)
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText(e.what());
    this->m_StatisticsUpdatePending = false;
    m_Controls.label_currentlyComputingStatistics->setVisible(false);
  }
}

void QmitkImageStatisticsReloadedView::OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes)
{
  Q_UNUSED(part);
  Q_UNUSED(nodes);
}

void QmitkImageStatisticsReloadedView::PrepareDataStorageComboBoxes()
{
  auto isPlanarFigurePredicate = mitk::GetImageStatisticsPlanarFigurePredicate();
  auto isMaskPredicate = mitk::GetImageStatisticsMaskPredicate();
  auto isImagePredicate = mitk::GetImageStatisticsImagePredicate();
  auto isMaskOrPlanarFigurePredicate = mitk::NodePredicateOr::New(isPlanarFigurePredicate, isMaskPredicate);

  m_Controls.imageSelector->SetDataStorage(GetDataStorage());
  m_Controls.imageSelector->SetPredicate(isImagePredicate);

  m_Controls.maskImageSelector->SetDataStorage(GetDataStorage());

  m_Controls.maskImageSelector->SetPredicate(isMaskOrPlanarFigurePredicate);
  m_Controls.maskImageSelector->SetZeroEntryText("<none>");
}

void QmitkImageStatisticsReloadedView::Activated()
{
}

void QmitkImageStatisticsReloadedView::Deactivated()
{
}

void QmitkImageStatisticsReloadedView::Visible()
{
  connect(this->m_Controls.imageSelector, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QmitkImageStatisticsReloadedView::OnImageSelectorChanged);
  connect(this->m_Controls.maskImageSelector, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QmitkImageStatisticsReloadedView::OnMaskSelectorChanged);
  OnImageSelectorChanged();
  OnMaskSelectorChanged();
}

void QmitkImageStatisticsReloadedView::Hidden()
{
  m_Controls.imageSelector->disconnect();
  m_Controls.maskImageSelector->disconnect();
}

void QmitkImageStatisticsReloadedView::SetFocus()
{
}
