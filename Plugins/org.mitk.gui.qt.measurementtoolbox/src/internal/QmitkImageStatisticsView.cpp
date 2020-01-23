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
#include <mitkNodePredicateOr.h>
#include <mitkSliceNavigationController.h>
#include <mitkStatisticsToImageRelationRule.h>
#include <mitkStatisticsToMaskRelationRule.h>
#include <mitkStatusBar.h>
#include <qthreadpool.h>

#include "mitkImageStatisticsContainerManager.h"
#include <mitkPlanarFigureInteractor.h>

const std::string QmitkImageStatisticsView::VIEW_ID = "org.mitk.views.imagestatistics";

QmitkImageStatisticsView::~QmitkImageStatisticsView()
{
  if (nullptr != m_selectedPlanarFigure)
  {
    m_selectedPlanarFigure->RemoveObserver(m_PlanarFigureObserverTag);
  }
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

  m_Controls.widget_statistics->SetDataStorage(this->GetDataStorage());
  CreateConnections();
}

void QmitkImageStatisticsView::CreateConnections()
{
  connect(this->m_Controls.checkBox_ignoreZero, &QCheckBox::stateChanged,
    this, &QmitkImageStatisticsView::OnCheckBoxIgnoreZeroStateChanged);
  connect(this->m_Controls.buttonSelection, &QAbstractButton::clicked,
    this, &QmitkImageStatisticsView::OnButtonSelectionPressed);
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

  CalculateOrGetMultiStatistics();
}

void QmitkImageStatisticsView::OnButtonSelectionPressed()
{
  m_SelectionDialog = new QmitkNodeSelectionDialog();
  m_SelectionDialog->SetDataStorage(GetDataStorage());

  // set predicates
  auto isPlanarFigurePredicate = mitk::GetImageStatisticsPlanarFigurePredicate();
  auto isMaskPredicate = mitk::GetImageStatisticsMaskPredicate();
  auto isImagePredicate = mitk::GetImageStatisticsImagePredicate();
  auto isMaskOrPlanarFigurePredicate = mitk::NodePredicateOr::New(isPlanarFigurePredicate, isMaskPredicate);
  auto isImageOrMaskOrPlanarFigurePredicate = mitk::NodePredicateOr::New(isMaskOrPlanarFigurePredicate, isImagePredicate);
  m_SelectionDialog->SetNodePredicate(isImageOrMaskOrPlanarFigurePredicate);
  m_SelectionDialog->SetSelectionMode(QAbstractItemView::MultiSelection);
  connect(m_SelectionDialog, &QmitkNodeSelectionDialog::accepted, this, &QmitkImageStatisticsView::OnDialogSelectionChanged);
  m_SelectionDialog->show();
}

void QmitkImageStatisticsView::OnDialogSelectionChanged()
{
  m_selectedImageNodes.resize(0);
  m_selectedMaskNodes.resize(0);

  QmitkNodeSelectionDialog::NodeList selectedNodeList = m_SelectionDialog->GetSelectedNodes();
  auto isImagePredicate = mitk::GetImageStatisticsImagePredicate();
  for (const auto& node : selectedNodeList)
  {
    if (isImagePredicate->CheckNode(node))
    {
      m_selectedImageNodes.push_back(node.GetPointer());
    }
    else
    {
      m_selectedMaskNodes.push_back(node.GetPointer());
    }
  }

  if (!m_selectedImageNodes.empty())
  {
    m_Controls.widget_statistics->SetImageNodes(m_selectedImageNodes);
  }
  if (!m_selectedMaskNodes.empty())
  {
    m_Controls.widget_statistics->SetMaskNodes(m_selectedMaskNodes);
  }

  CalculateOrGetMultiStatistics();
}

void QmitkImageStatisticsView::PartClosed(const berry::IWorkbenchPartReference::Pointer &) {}

void QmitkImageStatisticsView::FillHistogramWidget(const std::vector<const HistogramType*> &histogram,
                                                   const std::vector<std::string> &dataLabels)
{
  m_Controls.groupBox_histogram->setVisible(true);
  m_Controls.widget_histogram->SetTheme(this->GetColorTheme());
  m_Controls.widget_histogram->Reset();
  m_Controls.widget_histogram->SetHistogram(histogram.front(), dataLabels.front());
  connect(m_Controls.widget_histogram, &QmitkHistogramVisualizationWidget::RequestHistogramUpdate,
          this, &QmitkImageStatisticsView::OnRequestHistogramUpdate);
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

/*
void QmitkImageStatisticsView::CalculateOrGetStatistics()
{
  if (nullptr != m_selectedPlanarFigure)
  {
    m_selectedPlanarFigure->RemoveObserver(this->m_PlanarFigureObserverTag);
    m_selectedPlanarFigure = nullptr;
  }

  m_Controls.groupBox_intensityProfile->setVisible(false);
  m_Controls.widget_statistics->setEnabled(!m_selectedImageNodes.empty());

  std::vector<mitk::ImageStatisticsContainer::ConstPointer> statisticsVector;
  if (m_selectedImageNodes.empty())
  {
    ResetGUI();
    m_ForceRecompute = false;
  }

  for (auto imageNode : m_selectedImageNodes)
  {
    m_selectedImageNode = imageNode;
    if (nullptr == m_selectedImageNode)
    {
      continue;
    }

    auto image = dynamic_cast<mitk::Image*>(m_selectedImageNode->GetData());
    mitk::ImageStatisticsContainer::ConstPointer imageStatistics;

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

    if (m_selectedMaskNodes.empty())
    {
      // compute image statistics solely for the image
      imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image);
    }

    for (auto maskNode : m_selectedMaskNodes)
    {
      m_selectedMaskNode = maskNode;
      mitk::Image* mask = nullptr;
      mitk::PlanarFigure* maskPlanarFigure = nullptr;

      if (nullptr == m_selectedMaskNode)
      {
        // compute image statistics solely for the image
        imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image);
      }
      else
      {
        // a mask exists; check if it contains real data
        mask = dynamic_cast<mitk::Image*>(m_selectedMaskNode->GetData());
        // mask is an image; calculate statistics
        if (nullptr != mask)
        {
          imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, mask);
        }
        else
        {
          // mask is not an image, could be a planar figure
          maskPlanarFigure = dynamic_cast<mitk::PlanarFigure*>(maskNode->GetData());
          if (nullptr != maskPlanarFigure)
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
        }
      }

      bool imageStatisticsOlderThanInputs = false;
      if (nullptr != imageStatistics &&
         (nullptr != image && imageStatistics->GetMTime() < image->GetMTime() ||
         (nullptr != mask && imageStatistics->GetMTime() < mask->GetMTime()) ||
         (nullptr != maskPlanarFigure && imageStatistics->GetMTime() < maskPlanarFigure->GetMTime())))
      {
        imageStatisticsOlderThanInputs = true;
      }

      if (nullptr != imageStatistics)
      {
        // triggers re-computation when switched between images and the newest one has not 100 bins (default)
        auto calculatedBins = imageStatistics->GetStatisticsForTimeStep(0).m_Histogram.GetPointer()->Size();
        if (calculatedBins != 100)
        {
          OnRequestHistogramUpdate(m_Controls.widget_histogram->GetBins());
        }
      }

      // statistics need to be computed
      if (nullptr == imageStatistics || imageStatisticsOlderThanInputs || m_ForceRecompute)
      {
        CalculateStatistics(image, mask, maskPlanarFigure);
        m_selectedMaskNode = nullptr;
        m_selectedPlanarFigure = nullptr;
      }
      else // statistics already computed
      {
        // Not an open planar figure: show histogram (intensity profile already shown)
        if (!(maskPlanarFigure && !maskPlanarFigure->IsClosed()))
        {
          if (imageStatistics->TimeStepExists(0))
          {
            auto histogram = imageStatistics->GetStatisticsForTimeStep(0).m_Histogram.GetPointer();
            std::string imageNodeName = imageNode->GetName();
            //this->FillHistogramWidget({ histogram }, { imageNodeName });
          }
        }
      }
    }
  }
}
*/

void QmitkImageStatisticsView::CalculateOrGetMultiStatistics()
{
  m_selectedImageNode = nullptr;
  m_selectedMaskNode = nullptr;

  if (m_selectedImageNodes.empty())
  {
    // no images selected; reset everything
    ResetGUI();
  }

  for (auto imageNode : m_selectedImageNodes)
  {
    m_selectedImageNode = imageNode;

    if (m_selectedMaskNodes.empty())
    {
      CalculateOrGetStatistics();
    }
    else
    {
      for (auto maskNode : m_selectedMaskNodes)
      {
        m_selectedMaskNode = maskNode;
        CalculateOrGetStatistics();
      }
    }
  }
}

void QmitkImageStatisticsView::CalculateOrGetStatistics()
{
  if (nullptr != m_selectedPlanarFigure)
  {
    this->m_selectedPlanarFigure->RemoveObserver(this->m_PlanarFigureObserverTag);
    this->m_selectedPlanarFigure = nullptr;
  }

  m_Controls.groupBox_intensityProfile->setVisible(false);
  m_Controls.widget_statistics->setEnabled(m_selectedImageNode.IsNotNull());

  if (nullptr != m_selectedImageNode)
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

    if (nullptr != m_selectedMaskNode)
    {
      mask = dynamic_cast<mitk::Image *>(m_selectedMaskNode->GetData());
      if (nullptr == mask)
      {
        maskPlanarFigure = dynamic_cast<mitk::PlanarFigure *>(m_selectedMaskNode->GetData());
      }
    }

    mitk::ImageStatisticsContainer::ConstPointer imageStatistics;
    if (nullptr != mask)
    {
      imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, mask);
    }
    else if (nullptr != maskPlanarFigure)
    {
      m_selectedPlanarFigure = maskPlanarFigure;
      ITKCommandType::Pointer changeListener = ITKCommandType::New();
      changeListener->SetCallbackFunction(this, &QmitkImageStatisticsView::CalculateOrGetMultiStatistics);
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
    if (nullptr != imageStatistics &&
       ((nullptr != image && imageStatistics->GetMTime() < image->GetMTime()) ||
        (nullptr != mask && imageStatistics->GetMTime() < mask->GetMTime()) ||
        (nullptr != maskPlanarFigure && imageStatistics->GetMTime() < maskPlanarFigure->GetMTime())))
    {
      imageStatisticsOlderThanInputs = true;
    }

    if (nullptr != imageStatistics)
    {
      // triggers re-computation when switched between images and the newest one has not 100 bins (default)
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
          this->FillHistogramWidget({ histogram }, { imageNodeName });
        }
      }
    }
  }
  else
  {
    ResetGUI();
  }
}

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

std::string QmitkImageStatisticsView::GenerateStatisticsNodeName()
{
  auto statisticsNodeName = m_selectedImageNode->GetName();
  if (m_selectedMaskNode)
  {
    statisticsNodeName += "_" + m_selectedMaskNode->GetName();
  }
  statisticsNodeName += "_statistics";

  return statisticsNodeName;
}

mitk::DataNode::Pointer QmitkImageStatisticsView::GetNodeForStatisticsContainer(
  mitk::ImageStatisticsContainer::ConstPointer container)
{
  if (!container)
  {
    mitkThrow() << "Given container is null!";
  }

  auto allDataNodes = this->GetDataStorage()->GetAll()->CastToSTLConstContainer();
  for (auto node : allDataNodes)
  {
    auto nodeData = node->GetData();
    if (nodeData && nodeData->GetUID() == container->GetUID())
    {
      return node;
    }
  }
  mitkThrow() << "No DataNode is found which holds the given statistics container!";
}

void QmitkImageStatisticsView::HandleExistingStatistics(mitk::Image::ConstPointer image,
                                                        mitk::BaseData::ConstPointer mask,
                                                        mitk::ImageStatisticsContainer::Pointer statistic)
{
  auto imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(
    this->GetDataStorage(), image, mask);

  // if statistics base data already exist: add to existing node
  if (imageStatistics)
  {
    auto node = GetNodeForStatisticsContainer(imageStatistics);
    node->SetData(statistic);
  }
  // statistics base data does not exist: add new node
  else
  {
    auto statisticsNodeName = GenerateStatisticsNodeName();
    auto statisticsNode = mitk::CreateImageStatisticsNode(statistic, statisticsNodeName);
    this->GetDataStorage()->Add(statisticsNode);
  }
}

void QmitkImageStatisticsView::SetupRelationRules(mitk::ImageStatisticsContainer::Pointer statistic,
                                                  mitk::BaseData::ConstPointer mask)
{
  auto imageRule = mitk::StatisticsToImageRelationRule::New();
  imageRule->Connect(statistic, m_CalculationJob->GetStatisticsImage());

  if (mask)
  {
    auto maskRule = mitk::StatisticsToMaskRelationRule::New();
    maskRule->Connect(statistic, mask);
  }
}

void QmitkImageStatisticsView::OnStatisticsCalculationEnds()
{
  mitk::StatusBar::GetInstance()->Clear();
  auto runnable = m_Runnables[0];
  m_Runnables.erase(m_Runnables.begin());

  auto image = runnable->GetStatisticsImage();

  // get mask
  mitk::BaseData::ConstPointer mask = nullptr;
  if (runnable->GetMaskImage())
  {
    mask = runnable->GetMaskImage();
  }
  else if (runnable->GetPlanarFigure())
  {
    mask = runnable->GetPlanarFigure();
  }

  // get current statistics
  auto currentImageStatistics =
    mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, mask);

  if (runnable->GetStatisticsUpdateSuccessFlag()) // case: calculation was successful
  {
    auto statistic = runnable->GetStatisticsData();

    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    imageRule->Connect(statistic, image);

    SetupRelationRules(statistic, mask);

    // checks for existing statistic, and add it to data manager
    HandleExistingStatistics(image, mask, statistic);

    /* HISTOGRAM CURRENTLY NOT WORKING - SEE T26269
    if (!m_selectedPlanarFigure || m_selectedPlanarFigure->IsClosed())
    {
      FillHistogramWidget({ m_CalculationJob->GetTimeStepHistogram() }, { m_selectedImageNode->GetName() });
    }
    */
  }
  else // case: calculation was not successful
  {
    // handle histogram
    const HistogramType* emptyHistogram = HistogramType::New();
    // HISTOGRAM CURRENTLY NOT WORKING - SEE T26269
    //FillHistogramWidget({ emptyHistogram }, { m_selectedImageNode->GetName() });

    // handle statistics
    mitk::ImageStatisticsContainer::Pointer statistic = mitk::ImageStatisticsContainer::New();
    statistic->SetTimeGeometry(const_cast<mitk::TimeGeometry *>(image->GetTimeGeometry()));

    /* HISTOGRAM CURRENTLY NOT WORKING - SEE T26269
    // add empty histogram to statistics for all time steps
    for (unsigned int i = 0; i < image->GetTimeSteps(); ++i)
    {
      auto statisticObject = mitk::ImageStatisticsContainer::ImageStatisticsObject();
      statisticObject.m_Histogram = emptyHistogram;
      statistic->SetStatisticsForTimeStep(i, statisticObject);
    }
    */

    SetupRelationRules(statistic, mask);

    HandleExistingStatistics(image, mask, statistic);

    mitk::StatusBar::GetInstance()->DisplayErrorText(runnable->GetLastErrorMessage().c_str());
    m_Controls.widget_histogram->setEnabled(false);
  }

  m_Controls.label_currentlyComputingStatistics->setVisible(false);
}

void QmitkImageStatisticsView::OnRequestHistogramUpdate(unsigned int nBins)
{
}

void QmitkImageStatisticsView::CalculateStatistics(const mitk::Image *image,
                                                   const mitk::Image *mask,
                                                   const mitk::PlanarFigure *maskPlanarFigure)
{
	auto runnable = new QmitkImageStatisticsCalculationRunnable();
	runnable->Initialize(image, mask, maskPlanarFigure);
	runnable->setAutoDelete(false);
	m_Runnables.push_back(runnable);
	connect(runnable, &QmitkImageStatisticsCalculationRunnable::finished,
    this, &QmitkImageStatisticsView::OnStatisticsCalculationEnds, Qt::QueuedConnection);

  try
  {
    // Compute statistics
    QThreadPool::globalInstance()->start(runnable);
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

{
}

