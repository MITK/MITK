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
#include <mitkNodePredicateOr.h>
#include <mitkSliceNavigationController.h>
#include <mitkStatisticsToImageRelationRule.h>
#include <mitkStatisticsToMaskRelationRule.h>
#include <mitkStatusBar.h>

#include "mitkImageStatisticsContainerManager.h"
#include <mitkPlanarFigureInteractor.h>

const std::string QmitkImageStatisticsView::VIEW_ID = "org.mitk.views.imagestatistics";

QmitkImageStatisticsView::QmitkImageStatisticsView(QObject * /*parent*/, const char * /*name*/)
{
  this->m_CalculationJob = new QmitkImageStatisticsCalculationJob();
}

QmitkImageStatisticsView::~QmitkImageStatisticsView()
{
  if (m_selectedPlanarFigure)
  {
    m_selectedPlanarFigure->RemoveObserver(m_PlanarFigureObserverTag);
  }

  if (!m_CalculationJob->isFinished())
  {
    m_CalculationJob->terminate();
    m_CalculationJob->wait();
  }
  this->m_CalculationJob->deleteLater();
}

void QmitkImageStatisticsView::CreateQtPartControl(QWidget *parent)
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
  m_Controls.sliderWidget_intensityProfile->setPrefix("Time: ");
  m_Controls.sliderWidget_intensityProfile->setDecimals(0);
  m_Controls.sliderWidget_intensityProfile->setVisible(false);
  ResetGUI();

  PrepareDataStorageComboBoxes();
  m_Controls.widget_statistics->SetDataStorage(this->GetDataStorage());
  CreateConnections();
}

void QmitkImageStatisticsView::CreateConnections()
{
  connect(this->m_CalculationJob,
          &QmitkImageStatisticsCalculationJob::finished,
          this,
          &QmitkImageStatisticsView::OnStatisticsCalculationEnds,
          Qt::QueuedConnection);
  connect(this->m_Controls.checkBox_ignoreZero,
          &QCheckBox::stateChanged,
          this,
          &QmitkImageStatisticsView::OnCheckBoxIgnoreZeroStateChanged);
  connect(this->m_Controls.sliderWidget_histogram,
          &ctkSliderWidget::valueChanged,
          this,
          &QmitkImageStatisticsView::OnSliderWidgetHistogramChanged);
  connect(this->m_Controls.sliderWidget_intensityProfile,
          &ctkSliderWidget::valueChanged,
          this,
          &QmitkImageStatisticsView::OnSliderWidgetIntensityProfileChanged);
  connect(this->m_Controls.imageSelector,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this,
          &QmitkImageStatisticsView::OnImageSelectorChanged);
  connect(this->m_Controls.maskImageSelector,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this,
          &QmitkImageStatisticsView::OnMaskSelectorChanged);
}

void QmitkImageStatisticsView::OnCheckBoxIgnoreZeroStateChanged(int state)
{
  m_ForceRecompute = true;
  if (state != Qt::Unchecked)
  {
    this->m_CalculationJob->SetIgnoreZeroValueVoxel(true);
  }
  else
  {
    this->m_CalculationJob->SetIgnoreZeroValueVoxel(false);
  }
  CalculateOrGetStatistics();
}

void QmitkImageStatisticsView::OnSliderWidgetHistogramChanged(double value)
{
  unsigned int timeStep = static_cast<unsigned int>(value);
  auto mask = m_selectedMaskNode ? m_selectedMaskNode->GetData() : nullptr;
  auto imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(
    this->GetDataStorage(), m_selectedImageNode->GetData(), mask);
  HistogramType::ConstPointer histogram = nullptr;
  if (imageStatistics->TimeStepExists(timeStep))
  {
    histogram = imageStatistics->GetStatisticsForTimeStep(timeStep).m_Histogram;
  }

  if(this->m_CalculationJob->GetStatisticsUpdateSuccessFlag())
  {
    if (histogram.IsNotNull())
    {
      this->FillHistogramWidget({histogram}, {m_selectedImageNode->GetName()});
    }
    else {
      HistogramType::Pointer emptyHistogram = HistogramType::New();
      this->FillHistogramWidget({emptyHistogram}, {m_selectedImageNode->GetName()});
    }
  }
}

void QmitkImageStatisticsView::OnSliderWidgetIntensityProfileChanged()
{
  // intensity profile is always computed on request, not stored as node in DataStorage
  auto image = dynamic_cast<mitk::Image *>(m_selectedImageNode->GetData());
  auto planarFigure = dynamic_cast<mitk::PlanarFigure *>(m_selectedMaskNode->GetData());

  if (image && planarFigure && this->m_CalculationJob->GetStatisticsUpdateSuccessFlag())
  {
    this->ComputeAndDisplayIntensityProfile(image, planarFigure);
  }
}

void QmitkImageStatisticsView::PartClosed(const berry::IWorkbenchPartReference::Pointer &) {}

void QmitkImageStatisticsView::FillHistogramWidget(const std::vector<const HistogramType*> &histogram,
                                                   const std::vector<std::string> &dataLabels)
{
  m_Controls.groupBox_histogram->setVisible(true);
  m_Controls.widget_histogram->SetTheme(this->GetColorTheme());
  m_Controls.widget_histogram->Reset();
  m_Controls.widget_histogram->SetHistogram(histogram.front(), dataLabels.front());
  connect(m_Controls.widget_histogram,
          &QmitkHistogramVisualizationWidget::RequestHistogramUpdate,
          this,
          &QmitkImageStatisticsView::OnRequestHistogramUpdate);
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

void QmitkImageStatisticsView::OnImageSelectorChanged()
{
  auto selectedImageNode = m_Controls.imageSelector->GetSelectedNode();
  if (selectedImageNode != m_selectedImageNode)
  {
    m_selectedImageNode = selectedImageNode;
    if (m_selectedImageNode.IsNotNull())
    {
      ResetGUIDefault();

      auto isPlanarFigurePredicate = mitk::GetImageStatisticsPlanarFigurePredicate();
      auto isMaskPredicate = mitk::GetImageStatisticsMaskPredicate();
      auto hasSameGeometry = mitk::NodePredicateGeometry::New(m_selectedImageNode->GetData()->GetGeometry());
      hasSameGeometry->SetCheckPrecision(1e-10);
      auto isMaskWithGeometryPredicate = mitk::NodePredicateAnd::New(isMaskPredicate, hasSameGeometry);
      auto isMaskOrPlanarFigureWithGeometryPredicate =
        mitk::NodePredicateOr::New(isPlanarFigurePredicate, isMaskWithGeometryPredicate);
      // prevent triggering of computation as the predicate triggers a signalChanged event
      m_Controls.maskImageSelector->disconnect();
      m_Controls.maskImageSelector->SetPredicate(isMaskOrPlanarFigureWithGeometryPredicate);
      // reset mask to <none>
      m_Controls.maskImageSelector->SetZeroEntryText("<none>");
      m_Controls.checkBox_ignoreZero->setEnabled(true);
      m_selectedMaskNode = nullptr;
      m_Controls.widget_statistics->SetMaskNodes({});
      CalculateOrGetStatistics();
      m_Controls.widget_statistics->SetImageNodes({m_selectedImageNode});
      connect(this->m_Controls.maskImageSelector,
              static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
              this,
              &QmitkImageStatisticsView::OnMaskSelectorChanged);
    }
    else
    {
      m_Controls.widget_statistics->SetImageNodes({});
      m_Controls.widget_statistics->SetMaskNodes({});
      m_Controls.widget_statistics->Reset();
      m_Controls.widget_histogram->Reset();
      ResetGUI();
    }
  }
}

void QmitkImageStatisticsView::OnMaskSelectorChanged()
{
  auto selectedMaskNode = m_Controls.maskImageSelector->GetSelectedNode();
  if (selectedMaskNode != m_selectedMaskNode)
  {
    m_selectedMaskNode = selectedMaskNode;
    if (m_selectedMaskNode.IsNotNull())
    {
      m_Controls.widget_statistics->SetMaskNodes({m_selectedMaskNode});
    }
    else
    {
      m_Controls.widget_statistics->SetMaskNodes({});
    }
    CalculateOrGetStatistics();
  }
}

void QmitkImageStatisticsView::CalculateOrGetStatistics()
{
  if (this->m_selectedPlanarFigure)
  {
    this->m_selectedPlanarFigure->RemoveObserver(this->m_PlanarFigureObserverTag);
    this->m_selectedPlanarFigure = nullptr;
  }

  m_Controls.groupBox_intensityProfile->setVisible(false);
  m_Controls.widget_statistics->setEnabled(m_selectedImageNode.IsNotNull());

  if (m_selectedImageNode != nullptr)
  {
    auto image = dynamic_cast<mitk::Image *>(m_selectedImageNode->GetData());
    mitk::Image *mask = nullptr;
    mitk::PlanarFigure *maskPlanarFigure = nullptr;

    if (image->GetDimension() == 4)
    {
      m_Controls.sliderWidget_histogram->setVisible(true);
      unsigned int maxTimestep = image->GetTimeSteps();
      m_Controls.sliderWidget_histogram->setMaximum(maxTimestep - 1);
    }
    else
    {
      m_Controls.sliderWidget_histogram->setVisible(false);
    }

    if (m_selectedMaskNode != nullptr)
    {
      mask = dynamic_cast<mitk::Image *>(m_selectedMaskNode->GetData());
      if (mask == nullptr)
      {
        maskPlanarFigure = dynamic_cast<mitk::PlanarFigure *>(m_selectedMaskNode->GetData());
      }
    }

    mitk::ImageStatisticsContainer::ConstPointer imageStatistics;
    if (mask)
    {
      imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, mask);
    }
    else if (maskPlanarFigure)
    {
      m_selectedPlanarFigure = maskPlanarFigure;
      ITKCommandType::Pointer changeListener = ITKCommandType::New();
      changeListener->SetCallbackFunction(this, &QmitkImageStatisticsView::CalculateOrGetStatistics);
      this->m_PlanarFigureObserverTag =
        m_selectedPlanarFigure->AddObserver(mitk::EndInteractionPlanarFigureEvent(), changeListener);
      if (!maskPlanarFigure->IsClosed())
      {
        ComputeAndDisplayIntensityProfile(image, maskPlanarFigure);
      }
      imageStatistics =
        mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, maskPlanarFigure);
    }
    else
    {
      imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image);
    }

    bool imageStatisticsOlderThanInputs = false;
    if (imageStatistics &&
        (imageStatistics->GetMTime() < image->GetMTime() || (mask && imageStatistics->GetMTime() < mask->GetMTime()) ||
         (maskPlanarFigure && imageStatistics->GetMTime() < maskPlanarFigure->GetMTime())))
    {
      imageStatisticsOlderThanInputs = true;
    }

    if (imageStatistics)
    {
      // triggers recomputation when switched between images and the newest one has not 100 bins (default)
      auto calculatedBins = imageStatistics->GetStatisticsForTimeStep(0).m_Histogram.GetPointer()->Size();
      if (calculatedBins != 100)
      {
        OnRequestHistogramUpdate(m_Controls.widget_histogram->GetBins());
      }
    }

    // statistics need to be computed
    if (!imageStatistics || imageStatisticsOlderThanInputs || m_ForceRecompute)
    {
      CalculateStatistics(image, mask, maskPlanarFigure);
    }
    // statistics already computed
    else
    {
      // Not an open planar figure: show histogram (intensity profile already shown)
      if (!(maskPlanarFigure && !maskPlanarFigure->IsClosed()))
      {
        if (imageStatistics->TimeStepExists(0))
        {
          auto histogram = imageStatistics->GetStatisticsForTimeStep(0).m_Histogram.GetPointer();
          std::string imageNodeName = m_selectedImageNode->GetName();
          this->FillHistogramWidget({histogram}, {imageNodeName});
        }
      }
    }
  }
  else
  {
    ResetGUI();
  }
  m_ForceRecompute = false;
}

void QmitkImageStatisticsView::ComputeAndDisplayIntensityProfile(mitk::Image *image,
                                                                 mitk::PlanarFigure *maskPlanarFigure)
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
  // Don't show histogram for intensity profiles
  m_Controls.groupBox_histogram->setVisible(false);
  m_Controls.groupBox_intensityProfile->setVisible(true);
  m_Controls.widget_intensityProfile->Reset();
  m_Controls.widget_intensityProfile->SetIntensityProfile(intensityProfile.GetPointer(),
                                                          "Intensity Profile of " + m_selectedImageNode->GetName());
}

void QmitkImageStatisticsView::ResetGUI()
{
  m_Controls.widget_statistics->Reset();
  m_Controls.widget_statistics->setEnabled(false);
  m_Controls.widget_histogram->Reset();
  m_Controls.widget_histogram->setEnabled(false);
  m_Controls.checkBox_ignoreZero->setEnabled(false);
}

void QmitkImageStatisticsView::ResetGUIDefault()
{
  m_Controls.widget_histogram->ResetDefault();
  m_Controls.checkBox_ignoreZero->setChecked(false);
}

void QmitkImageStatisticsView::OnStatisticsCalculationEnds()
{
  mitk::StatusBar::GetInstance()->Clear();

  if (this->m_CalculationJob->GetStatisticsUpdateSuccessFlag())
  {
    auto statistic = m_CalculationJob->GetStatisticsData();
    auto image = m_CalculationJob->GetStatisticsImage();
    mitk::BaseData::ConstPointer mask = nullptr;
    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    imageRule->Connect(statistic, image);

    if (m_CalculationJob->GetMaskImage())
    {
      auto maskRule = mitk::StatisticsToMaskRelationRule::New();
      mask = m_CalculationJob->GetMaskImage();
      maskRule->Connect(statistic, mask);
    }
    else if (m_CalculationJob->GetPlanarFigure())
    {
      auto planarFigureRule = mitk::StatisticsToMaskRelationRule::New();
      mask = m_CalculationJob->GetPlanarFigure();
      planarFigureRule->Connect(statistic, mask);
    }

    auto imageStatistics =
      mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, mask);

    // if statistics base data already exist: add to existing node
    if (imageStatistics)
    {
      auto allDataNodes = this->GetDataStorage()->GetAll()->CastToSTLConstContainer();
      for (auto node : allDataNodes)
      {
        auto nodeData = node->GetData();
        if (nodeData && nodeData->GetUID() == imageStatistics->GetUID())
        {
          node->SetData(statistic);
        }
      }
    }
    // statistics base data does not exist: add new node
    else
    {
      auto statisticsNodeName = m_selectedImageNode->GetName();
      if (m_selectedMaskNode)
      {
        statisticsNodeName += "_" + m_selectedMaskNode->GetName();
      }
      statisticsNodeName += "_statistics";
      auto statisticsNode = mitk::CreateImageStatisticsNode(statistic, statisticsNodeName);
      this->GetDataStorage()->Add(statisticsNode);
    }

    if (!m_selectedPlanarFigure || m_selectedPlanarFigure->IsClosed())
    {
      this->FillHistogramWidget({m_CalculationJob->GetTimeStepHistogram()}, {m_selectedImageNode->GetName()});
    }
  }
  else
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText(m_CalculationJob->GetLastErrorMessage().c_str());
    m_Controls.widget_histogram->setEnabled(false);
  }
  m_Controls.label_currentlyComputingStatistics->setVisible(false);
}

void QmitkImageStatisticsView::OnRequestHistogramUpdate(unsigned int nBins)
{
  m_CalculationJob->SetHistogramNBins(nBins);
  m_CalculationJob->start();
}

void QmitkImageStatisticsView::CalculateStatistics(const mitk::Image *image,
                                                   const mitk::Image *mask,
                                                   const mitk::PlanarFigure *maskPlanarFigure)
{
  this->m_CalculationJob->Initialize(image, mask, maskPlanarFigure);

  try
  {
    // Compute statistics
    this->m_CalculationJob->start();
    m_Controls.label_currentlyComputingStatistics->setVisible(true);
  }
  catch (const mitk::Exception &e)
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText(e.GetDescription());
    m_Controls.label_currentlyComputingStatistics->setVisible(false);
  }
  catch (const std::runtime_error &e)
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText(e.what());
    m_Controls.label_currentlyComputingStatistics->setVisible(false);
  }
  catch (const std::exception &e)
  {
    mitk::StatusBar::GetInstance()->DisplayErrorText(e.what());
    m_Controls.label_currentlyComputingStatistics->setVisible(false);
  }
}

void QmitkImageStatisticsView::OnSelectionChanged(berry::IWorkbenchPart::Pointer part,
                                                  const QList<mitk::DataNode::Pointer> &nodes)
{
  Q_UNUSED(part);
  Q_UNUSED(nodes);
}

void QmitkImageStatisticsView::PrepareDataStorageComboBoxes()
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

void QmitkImageStatisticsView::Activated() {}

void QmitkImageStatisticsView::Deactivated() {}

void QmitkImageStatisticsView::Visible()
{
  connect(this->m_Controls.imageSelector,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this,
          &QmitkImageStatisticsView::OnImageSelectorChanged);
  connect(this->m_Controls.maskImageSelector,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this,
          &QmitkImageStatisticsView::OnMaskSelectorChanged);
  OnImageSelectorChanged();
  OnMaskSelectorChanged();
}

void QmitkImageStatisticsView::Hidden()
{
  m_Controls.imageSelector->disconnect();
  m_Controls.maskImageSelector->disconnect();
}

void QmitkImageStatisticsView::SetFocus() {}
