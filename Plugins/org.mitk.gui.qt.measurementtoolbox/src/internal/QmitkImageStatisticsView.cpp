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
  m_Controls.widget_histogram->SetTheme(GetColorTheme());
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

  m_Controls.widget_statistics->SetDataStorage(GetDataStorage());
  CreateConnections();
}

void QmitkImageStatisticsView::CreateConnections()
{
  connect(m_Controls.checkBox_ignoreZero, &QCheckBox::stateChanged,
    this, &QmitkImageStatisticsView::OnCheckBoxIgnoreZeroStateChanged);
  connect(m_Controls.buttonSelection, &QAbstractButton::clicked,
    this, &QmitkImageStatisticsView::OnButtonSelectionPressed);
}

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
    m_selectedPlanarFigure->RemoveObserver(m_PlanarFigureObserverTag);
    m_selectedPlanarFigure = nullptr;
  }

  m_Controls.groupBox_intensityProfile->setVisible(false);
  m_Controls.widget_statistics->setEnabled(m_selectedImageNode.IsNotNull());

  if (nullptr != m_selectedImageNode)
  {
    auto image = dynamic_cast<mitk::Image*>(m_selectedImageNode->GetData());
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
      mask = dynamic_cast<mitk::Image*>(m_selectedMaskNode->GetData());
      if (nullptr == mask)
      {
        maskPlanarFigure = dynamic_cast<mitk::PlanarFigure *>(m_selectedMaskNode->GetData());
      }
    }

    mitk::ImageStatisticsContainer::ConstPointer imageStatistics;
    if (nullptr != mask)
    {
      imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(GetDataStorage(), image, mask);
    }
    else if (nullptr != maskPlanarFigure)
    {
      m_selectedPlanarFigure = maskPlanarFigure;
      ITKCommandType::Pointer changeListener = ITKCommandType::New();
      changeListener->SetCallbackFunction(this, &QmitkImageStatisticsView::CalculateOrGetMultiStatistics);
      m_PlanarFigureObserverTag =
        m_selectedPlanarFigure->AddObserver(mitk::EndInteractionPlanarFigureEvent(), changeListener);
      if (!maskPlanarFigure->IsClosed())
      {
        ComputeAndDisplayIntensityProfile(image, maskPlanarFigure);
      }
      imageStatistics =
        mitk::ImageStatisticsContainerManager::GetImageStatistics(GetDataStorage(), image, maskPlanarFigure);
    }
    else
    {
      imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(GetDataStorage(), image);
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
      if (imageStatistics->TimeStepExists(0))
      {
        auto calculatedBins = imageStatistics->GetStatisticsForTimeStep(0).m_Histogram.GetPointer()->Size();
        if (calculatedBins != 100)
        {
          OnRequestHistogramUpdate(m_Controls.widget_histogram->GetBins());
        }
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
          auto histogramLabelName = GenerateStatisticsNodeName(image, mask);
          FillHistogramWidget({ histogram }, { histogramLabelName });
        }
      }
    }
  }
}

void QmitkImageStatisticsView::CalculateStatistics(const mitk::Image *image, const mitk::Image *mask,
                                                   const mitk::PlanarFigure *maskPlanarFigure)
{
  auto runnable = new QmitkImageStatisticsCalculationRunnable();
  runnable->Initialize(image, mask, maskPlanarFigure);
  runnable->setAutoDelete(false);
  runnable->SetIgnoreZeroValueVoxel(m_IgnoreZeroValueVoxel);
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

void QmitkImageStatisticsView::FillHistogramWidget(const std::vector<const HistogramType*> &histogram,
                                                   const std::vector<std::string> &dataLabels)
{
  m_Controls.groupBox_histogram->setVisible(true);
  m_Controls.widget_histogram->SetTheme(GetColorTheme());
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

void QmitkImageStatisticsView::ResetGUI()
{
  m_Controls.widget_statistics->Reset();
  m_Controls.widget_statistics->setEnabled(false);
  m_Controls.widget_histogram->Reset();
  m_Controls.widget_histogram->setEnabled(false);
}

void QmitkImageStatisticsView::ResetGUIDefault()
{
  m_Controls.widget_histogram->ResetDefault();
  m_Controls.checkBox_ignoreZero->setChecked(false);
  m_IgnoreZeroValueVoxel = false;
}

void QmitkImageStatisticsView::OnStatisticsCalculationEnds()
{
  auto runnable = qobject_cast<QmitkImageStatisticsCalculationRunnable*>(sender());
  if (nullptr == runnable)
  {
    return;
  }

  mitk::StatusBar::GetInstance()->Clear();

  auto it = std::find(m_Runnables.begin(), m_Runnables.end(), runnable);
  if (it != m_Runnables.end())
  {
    m_Runnables.erase(it);
  }

  auto image = runnable->GetStatisticsImage();

  // get mask
  const mitk::BaseData* mask = nullptr;
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
    mitk::ImageStatisticsContainerManager::GetImageStatistics(GetDataStorage(), image, mask);

  if (runnable->GetStatisticsUpdateSuccessFlag()) // case: calculation was successful
  {
    auto statistic = runnable->GetStatisticsData();

    SetupRelationRules(image, mask, statistic);

    // checks for existing statistic, and add it to data manager
    HandleExistingStatistics(image, mask, statistic);

    auto maskPlanarFigure = dynamic_cast<const mitk::PlanarFigure*>(mask);
    if (!maskPlanarFigure || maskPlanarFigure->IsClosed())
    {
      auto histogramLabelName = GenerateStatisticsNodeName(image, mask);
      FillHistogramWidget({ runnable->GetTimeStepHistogram() }, { histogramLabelName });
    }
  }
  else // case: calculation was not successful
  {
    // handle histogram
    const HistogramType* emptyHistogram = HistogramType::New();
    auto histogramLabelName = GenerateStatisticsNodeName(image, mask);
    FillHistogramWidget({ emptyHistogram }, { histogramLabelName });

    // handle statistics
    mitk::ImageStatisticsContainer::Pointer statistic = mitk::ImageStatisticsContainer::New();
    statistic->SetTimeGeometry(const_cast<mitk::TimeGeometry *>(image->GetTimeGeometry()));

    // add empty histogram to statistics for all time steps
    for (unsigned int i = 0; i < image->GetTimeSteps(); ++i)
    {
      auto statisticObject = mitk::ImageStatisticsContainer::ImageStatisticsObject();
      statisticObject.m_Histogram = emptyHistogram;
      statistic->SetStatisticsForTimeStep(i, statisticObject);
    }

    SetupRelationRules(image, mask, statistic);

    HandleExistingStatistics(image, mask, statistic);

    mitk::StatusBar::GetInstance()->DisplayErrorText(runnable->GetLastErrorMessage().c_str());
    m_Controls.widget_histogram->setEnabled(false);
  }

  m_Controls.label_currentlyComputingStatistics->setVisible(false);
}

void QmitkImageStatisticsView::OnRequestHistogramUpdate(unsigned int)
{
  // NEEDS TO BE IMPLEMENTED - SEE T27032
}

void QmitkImageStatisticsView::OnCheckBoxIgnoreZeroStateChanged(int state)
{
  m_ForceRecompute = true;
  m_IgnoreZeroValueVoxel = (state == Qt::Unchecked) ? false : true;

  CalculateOrGetMultiStatistics();

  m_ForceRecompute = false;
}

void QmitkImageStatisticsView::OnButtonSelectionPressed()
{
  m_SelectionDialog = new QmitkNodeSelectionDialog();
  m_SelectionDialog->SetDataStorage(GetDataStorage());
  m_SelectionDialog->SetSelectionCheckFunction(CheckForSameGeometry());

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

void QmitkImageStatisticsView::HandleExistingStatistics(mitk::Image::ConstPointer image,
                                                        mitk::BaseData::ConstPointer mask,
                                                        mitk::ImageStatisticsContainer::Pointer statistic)
{
  auto imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(GetDataStorage(), image, mask);

  // if statistics base data already exist: add to existing node
  if (imageStatistics)
  {
    auto node = GetNodeForStatisticsContainer(imageStatistics);
    node->SetData(statistic);
  }
  // statistics base data does not exist: add new node
  else
  {
    auto statisticsNodeName = GenerateStatisticsNodeName(image, mask);
    auto statisticsNode = mitk::CreateImageStatisticsNode(statistic, statisticsNodeName);
    GetDataStorage()->Add(statisticsNode);
  }
}

std::string QmitkImageStatisticsView::GenerateStatisticsNodeName(mitk::Image::ConstPointer image,
                                                                 mitk::BaseData::ConstPointer mask)
{
  auto statisticsNodeName = image->GetUID();
  if (mask)
  {
    statisticsNodeName += "_" + mask->GetUID();
  }
  statisticsNodeName += "_statistics";

  return statisticsNodeName;
}

void QmitkImageStatisticsView::SetupRelationRules(mitk::Image::ConstPointer image,
                                                  mitk::BaseData::ConstPointer mask,
                                                  mitk::ImageStatisticsContainer::Pointer statistic)
{
  auto imageRule = mitk::StatisticsToImageRelationRule::New();
  imageRule->Connect(statistic, image);

  if (nullptr != mask)
  {
    auto maskRule = mitk::StatisticsToMaskRelationRule::New();
    maskRule->Connect(statistic, mask);
  }
}

mitk::DataNode::Pointer QmitkImageStatisticsView::GetNodeForStatisticsContainer(mitk::ImageStatisticsContainer::ConstPointer container)
{
  if (container.IsNull())
  {
    mitkThrow() << "Given container is null!";
  }

  auto allDataNodes = GetDataStorage()->GetAll()->CastToSTLConstContainer();
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

QmitkNodeSelectionDialog::SelectionCheckFunctionType QmitkImageStatisticsView::CheckForSameGeometry() const
{
  auto lambda = [this](const QmitkNodeSelectionDialog::NodeList& nodes)
  {
    if (nodes.empty())
    {
      return std::string();
    }

    auto leftNode = nodes[0];
    for (int i = 1; i < nodes.size(); ++i)
    {
      auto rightNode = nodes[i];

      bool sameGeometry = CheckForSameGeometry(leftNode, rightNode);
      if (!sameGeometry)
      {
        std::stringstream ss;
        ss << "<font class=\"warning\"><p>Invalid selection: All selected nodes must have the same geometry.</p>";
        return ss.str();
      }
    }

    return std::string();
  };

  return lambda;
}

bool QmitkImageStatisticsView::CheckForSameGeometry(const mitk::DataNode* node1, const mitk::DataNode* node2) const
{
  bool isSameGeometry(true);

  mitk::Image* image1 = dynamic_cast<mitk::Image*>(node1->GetData());
  mitk::Image* image2 = dynamic_cast<mitk::Image*>(node2->GetData());
  if (image1 && image2)
  {
    mitk::BaseGeometry* geo1 = image1->GetGeometry();
    mitk::BaseGeometry* geo2 = image2->GetGeometry();

    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetOrigin(), geo2->GetOrigin());
    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(0), geo2->GetExtent(0));
    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(1), geo2->GetExtent(1));
    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(2), geo2->GetExtent(2));
    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetSpacing(), geo2->GetSpacing());
    isSameGeometry = isSameGeometry && mitk::MatrixEqualElementWise(geo1->GetIndexToWorldTransform()->GetMatrix(), geo2->GetIndexToWorldTransform()->GetMatrix());

    return isSameGeometry;
  }
  else
  {
    return false;
  }
}
