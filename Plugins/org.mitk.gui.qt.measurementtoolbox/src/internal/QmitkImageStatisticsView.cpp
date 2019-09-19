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
#include <qthreadpool.h>

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

  //PrepareDataStorageComboBoxes();
  m_Controls.widget_statistics->SetDataStorage(this->GetDataStorage());
  CreateConnections();
}

void QmitkImageStatisticsView::CreateConnections()
{
  connect(this->m_Controls.checkBox_ignoreZero,
          &QCheckBox::stateChanged,
          this,
          &QmitkImageStatisticsView::OnCheckBoxIgnoreZeroStateChanged);
  connect(this->m_Controls.buttonSelection, 
	      &QAbstractButton::clicked, 
	      this, 
	      &QmitkImageStatisticsView::OnButtonSelectionPressed);
}

void QmitkImageStatisticsView::OnButtonSelectionPressed()
{
	m_SelectionDialog = new QmitkNodeSelectionDialog();
	m_SelectionDialog->SetDataStorage(GetDataStorage());
	
	//set predicates
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
	QList<mitk::DataNode::Pointer> selectedNodeList = m_SelectionDialog->GetSelectedNodes();
	std::vector<mitk::DataNode::Pointer> selectedNodes(selectedNodeList.toVector().toStdVector());
	auto isImagePredicate = mitk::GetImageStatisticsImagePredicate();
	for (int i = 0; i < selectedNodes.size(); i++)
	{
		if (isImagePredicate->CheckNode(selectedNodes[i]))
		{
			m_selectedImageNodes.push_back(selectedNodes[i]);
		}
		else
		{
			m_selectedMaskNodes.push_back(selectedNodes[i]);
		}
	}
	std::vector<mitk::DataNode::ConstPointer> selectedImageNodesConst(m_selectedImageNodes.size(), 0);
	for (int i = 0; i < m_selectedImageNodes.size(); i++)
	{
		selectedImageNodesConst[i] = m_selectedImageNodes[i];
	}
	std::vector<mitk::DataNode::ConstPointer> selectedMaskNodesConst(m_selectedMaskNodes.size(), 0);
	for (int i = 0; i < m_selectedMaskNodes.size(); i++)
	{
		selectedMaskNodesConst[i] = m_selectedMaskNodes[i];
	}
	if(!selectedImageNodesConst.empty())
	m_Controls.widget_statistics->SetImageNodes(selectedImageNodesConst);
	if (!selectedMaskNodesConst.empty())
		m_Controls.widget_statistics->SetMaskNodes(selectedMaskNodesConst);
	CalculateOrGetStatisticsNew();
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
  CalculateOrGetStatisticsNew();
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

void QmitkImageStatisticsView::CalculateOrGetStatisticsNew()
{
	MITK_INFO << "Into the function check";
	if (this->m_selectedPlanarFigure)
	{
		this->m_selectedPlanarFigure->RemoveObserver(this->m_PlanarFigureObserverTag);
		this->m_selectedPlanarFigure = nullptr;
	}
	m_Controls.groupBox_intensityProfile->setVisible(false);
	m_Controls.widget_statistics->setEnabled(!m_selectedImageNodes.empty());

	std::vector<mitk::ImageStatisticsContainer::ConstPointer> statisticsVector;
	if (!m_selectedImageNodes.empty())
	{
		for (auto imageNode : m_selectedImageNodes)
		{
			m_selectedImageNode = imageNode;
			auto image = dynamic_cast<mitk::Image*>(imageNode->GetData());
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

			for (int i = 0; i < m_selectedMaskNodes.size() + 1; i++)
			{
				mitk::Image* mask = nullptr;
				mitk::PlanarFigure* maskPF = nullptr;
				mitk::DataNode::Pointer maskNode;
				mitk::BaseGeometry* imageGeometry;
				if (m_selectedMaskNodes.size())
				{
					maskNode = m_selectedMaskNodes[i];
					mask = dynamic_cast<mitk::Image*>(maskNode->GetData());
					imageGeometry = image->GetGeometry();
					if ((i+1) == m_selectedMaskNodes.size())
						i++;
				}

				if (m_selectedMaskNodes.size()&&mask == nullptr)
				{
					maskPF = dynamic_cast<mitk::PlanarFigure*>(maskNode->GetData());
					auto maskPFGeometry = maskPF->GetGeometry();
					//if (imageGeometry == maskPFGeometry) //doesnt activate
					//{
						m_selectedPlanarFigure = maskPF;
						ITKCommandType::Pointer changeListener = ITKCommandType::New();
						changeListener->SetCallbackFunction(this, &QmitkImageStatisticsView::CalculateOrGetStatisticsNew);
						this->m_PlanarFigureObserverTag =
							m_selectedPlanarFigure->AddObserver(mitk::EndInteractionPlanarFigureEvent(), changeListener);
						if (!maskPF->IsClosed())
						{
							ComputeAndDisplayIntensityProfile(image, maskPF);
						}
						imageStatistics =
							mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, maskPF);
					//}
				}
				else if (mask) //do this if the mask is a binary mask (no need for selected: either is PF(case above) or still nullptr from before)
				{
					m_selectedMaskNode = maskNode;
					auto maskGeometry = mask->GetGeometry();
					//if (imageGeometry == maskGeometry) //doesnt activate
					//{
						imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image, mask);
					//}
				}
				else
				{
					imageStatistics = mitk::ImageStatisticsContainerManager::GetImageStatistics(this->GetDataStorage(), image);
				}

				bool imageStatisticsOlderThanInputs = false;
				if (imageStatistics &&
					(imageStatistics->GetMTime() < image->GetMTime() || (mask && imageStatistics->GetMTime() < mask->GetMTime()) ||
					(maskPF && imageStatistics->GetMTime() < maskPF->GetMTime())))
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
					CalculateStatistics(image, mask, maskPF);
					m_selectedMaskNode = nullptr;
					m_selectedPlanarFigure = nullptr;
				}
				// statistics already computed
				else
				{
					// Not an open planar figure: show histogram (intensity profile already shown)
					if (!(maskPF && !maskPF->IsClosed()))
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
  int i = 0;
  bool finishedThreadFound = false;
  QmitkImageStatisticsCalculationRunnable* runnable;
  while(!finishedThreadFound && i < m_Runnables.size())
  {
	  if (m_Runnables[i]->GetStatisticsUpdateSuccessFlag())
	  {
		  runnable = m_Runnables[i];
		  finishedThreadFound = true;
		  i--;
	  }
	  i++;
  }
  m_Runnables.erase(m_Runnables.begin()+i);
  if (runnable->GetStatisticsUpdateSuccessFlag())
  {
    auto statistic = runnable->GetStatisticsData();
    auto image = runnable->GetStatisticsImage();
    mitk::BaseData::ConstPointer mask = nullptr;
    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    imageRule->Connect(statistic, image);

    if (runnable->GetMaskImage())
    {
      auto maskRule = mitk::StatisticsToMaskRelationRule::New();
      mask = runnable->GetMaskImage();
      maskRule->Connect(statistic, mask);
    }
    else if (runnable->GetPlanarFigure())
    {
      auto planarFigureRule = mitk::StatisticsToMaskRelationRule::New();
      mask = runnable->GetPlanarFigure();
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
      //this->FillHistogramWidget({m_CalculationJob->GetTimeStepHistogram()}, {m_selectedImageNode->GetName()});
    }
  }

  {
    mitk::StatusBar::GetInstance()->DisplayErrorText(runnable->GetLastErrorMessage().c_str());
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
	auto runnable = new QmitkImageStatisticsCalculationRunnable();
	runnable->Initialize(image, mask, maskPlanarFigure);
	runnable->setAutoDelete(false);
	m_Runnables.push_back(runnable);
	connect(runnable, &QmitkImageStatisticsCalculationRunnable::finished, this, &QmitkImageStatisticsView::OnStatisticsCalculationEnds, Qt::QueuedConnection);
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

void QmitkImageStatisticsView::OnSelectionChanged(berry::IWorkbenchPart::Pointer part,
                                                  const QList<mitk::DataNode::Pointer> &nodes)
{
  Q_UNUSED(part);
  Q_UNUSED(nodes);
}

void QmitkImageStatisticsView::Activated() {}

void QmitkImageStatisticsView::Deactivated() {}

void QmitkImageStatisticsView::Visible()
{
  /*connect(this->m_Controls.imageSelector,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this,
          &QmitkImageStatisticsView::OnImageSelectorChanged);
  connect(this->m_Controls.maskImageSelector,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          this,
          &QmitkImageStatisticsView::OnMaskSelectorChanged);
  OnImageSelectorChanged();
  OnMaskSelectorChanged();*/
}

void QmitkImageStatisticsView::Hidden()
{
  //m_Controls.imageSelector->disconnect();
  //m_Controls.maskImageSelector->disconnect();
}

void QmitkImageStatisticsView::SetFocus() {}