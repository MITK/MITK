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

// Qt includes
#include <QClipboard>
#include <QScrollBar>
#include <QVector>

// berry includes
#include <berryIWorkbenchPage.h>

// mitk includes
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkImageTimeSelector.h>
#include <QmitkRenderWindow.h>
#include <QmitkChartWidget.h>
#include <mitkImageCast.h>
#include <mitkImageStatisticsHolder.h>

// itk includes
#include "itksys/SystemTools.hxx"
#include "itkImageRegionConstIteratorWithIndex.h"

#include <limits>

//blueberry includes
#include <berryWorkbenchPlugin.h>
#include <berryQtPreferences.h>

const std::string QmitkImageStatisticsView::VIEW_ID = "org.mitk.views.imagestatistics";
const int QmitkImageStatisticsView::STAT_TABLE_BASE_HEIGHT = 180;

QmitkImageStatisticsView::QmitkImageStatisticsView(QObject* /*parent*/, const char* /*name*/)
: m_Controls( nullptr ),
  m_SelectedImage( nullptr ),
  m_SelectedImageMask( nullptr ),
  m_SelectedPlanarFigure( nullptr ),
  m_ImageObserverTag( -1 ),
  m_ImageMaskObserverTag( -1 ),
  m_PlanarFigureObserverTag( -1 ),
  m_TimeObserverTag( -1 ),
  m_CurrentStatisticsValid( false ),
  m_StatisticsUpdatePending( false ),
  m_DataNodeSelectionChanged ( false ),
  m_Visible(false)
{
  this->m_CalculationThread = new QmitkImageStatisticsCalculationThread;
}

QmitkImageStatisticsView::~QmitkImageStatisticsView()
{
  if ( m_SelectedImage != nullptr )
    m_SelectedImage->RemoveObserver( m_ImageObserverTag );
  if ( m_SelectedImageMask != nullptr )
    m_SelectedImageMask->RemoveObserver( m_ImageMaskObserverTag );
  if ( m_SelectedPlanarFigure != nullptr )
    m_SelectedPlanarFigure->RemoveObserver( m_PlanarFigureObserverTag );

  while(this->m_CalculationThread->isRunning()) // wait until thread has finished
  {
    itksys::SystemTools::Delay(100);
  }
  delete this->m_CalculationThread;
}


void QmitkImageStatisticsView::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == nullptr)
  {
    m_Controls = new Ui::QmitkImageStatisticsViewControls;
    m_Controls->setupUi(parent);
    CreateConnections();

    m_Controls->m_ErrorMessageLabel->hide();
    m_Controls->m_StatisticsWidgetStack->setCurrentIndex(0);
    m_Controls->m_BinSizeFrame->setEnabled(false);
  }
}

void QmitkImageStatisticsView::OnPageSuccessfullyLoaded()
{
  berry::IPreferencesService* prefService = berry::WorkbenchPlugin::GetDefault()->GetPreferencesService();
  m_StylePref = prefService->GetSystemPreferences()->Node(berry::QtPreferences::QT_STYLES_NODE);

  QString styleName = m_StylePref->Get(berry::QtPreferences::QT_STYLE_NAME, "");

  if (styleName == ":/org.blueberry.ui.qt/darkstyle.qss")
  {
    this->m_Controls->m_JSHistogram->SetTheme(QmitkChartWidget::ChartStyle::darkstyle);
  }
  else
  {
    this->m_Controls->m_JSHistogram->SetTheme(QmitkChartWidget::ChartStyle::lightstyle);
  }
}

void QmitkImageStatisticsView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(this->m_Controls->m_ButtonCopyHistogramToClipboard), SIGNAL(clicked()),(QObject*) this, SLOT(OnClipboardHistogramButtonClicked()) );
    connect( (QObject*)(this->m_Controls->m_ButtonCopyStatisticsToClipboard), SIGNAL(clicked()),(QObject*) this, SLOT(OnClipboardStatisticsButtonClicked()) );
    connect( (QObject*)(this->m_Controls->m_IgnoreZerosCheckbox), SIGNAL(clicked()),(QObject*) this, SLOT(OnIgnoreZerosCheckboxClicked()) );
    connect( (QObject*) this->m_CalculationThread, SIGNAL(finished()),this, SLOT( OnThreadedStatisticsCalculationEnds()),Qt::QueuedConnection);
    connect( (QObject*) this, SIGNAL(StatisticsUpdate()),this, SLOT( RequestStatisticsUpdate()), Qt::QueuedConnection);
    connect( (QObject*) this->m_Controls->m_StatisticsTable, SIGNAL(cellDoubleClicked(int,int)),this, SLOT( JumpToCoordinates(int,int)) );
    connect((QObject*)(this->m_Controls->m_barRadioButton), SIGNAL(clicked()), (QObject*)(this), SLOT(OnBarRadioButtonSelected()));
    connect((QObject*)(this->m_Controls->m_lineRadioButton), SIGNAL(clicked()), (QObject*)(this), SLOT(OnLineRadioButtonSelected()));
    connect( (QObject*) (this->m_Controls->m_HistogramNBinsSpinbox), SIGNAL(editingFinished()), this, SLOT(OnHistogramNBinsCheckBoxValueChanged()));
    connect((QObject*)(this->m_Controls->m_UseDefaultNBinsCheckBox), SIGNAL(clicked()), (QObject*) this, SLOT(OnDefaultNBinsSpinBoxChanged()));
    connect((QObject*)(this->m_Controls->m_ShowSubchartCheckBox), SIGNAL(clicked()), (QObject*) this, SLOT(OnShowSubchartBoxChanged()));
    connect((QObject*)(this->m_Controls->m_JSHistogram), SIGNAL(PageSuccessfullyLoaded()), (QObject*) this, SLOT(OnPageSuccessfullyLoaded()));
  }
}

void QmitkImageStatisticsView::OnDefaultNBinsSpinBoxChanged()
{
  if (this->m_Controls->m_UseDefaultNBinsCheckBox->isChecked()) {
    m_Controls->m_HistogramNBinsSpinbox->setValue(100);
    this->m_CalculationThread->SetHistogramNBins(m_Controls->m_HistogramNBinsSpinbox->value());
    m_HistogramNBins = m_Controls->m_HistogramNBinsSpinbox->value();
  }
  m_Controls->m_BinSizeFrame->setEnabled(!m_Controls->m_UseDefaultNBinsCheckBox->isChecked());

  this->UpdateStatistics();

}

void QmitkImageStatisticsView::OnShowSubchartBoxChanged()
{
  bool showSubchart = this->m_Controls->m_ShowSubchartCheckBox->isChecked();
  this->m_Controls->m_JSHistogram->Reload(showSubchart);
}


void QmitkImageStatisticsView::OnBarRadioButtonSelected()
{
  this->m_Controls->m_JSHistogram->SetChartTypeForAllDataAndReload(QmitkChartWidget::ChartType::bar);
}

void QmitkImageStatisticsView::OnLineRadioButtonSelected()
{
  this->m_Controls->m_JSHistogram->SetChartTypeForAllDataAndReload(QmitkChartWidget::ChartType::line);
}

void QmitkImageStatisticsView::PartClosed(const berry::IWorkbenchPartReference::Pointer& )
{
}

void QmitkImageStatisticsView::OnTimeChanged(const itk::EventObject& e)
{
  if (this->m_SelectedDataNodes.isEmpty() || this->m_SelectedImage == nullptr)
    return;

  const mitk::SliceNavigationController::GeometryTimeEvent* timeEvent =
      dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent*>(&e);
  assert(timeEvent != nullptr);
  int timestep = timeEvent->GetPos();

  if (this->m_SelectedImage->GetTimeSteps() > 1)
  {
    for (int x = 0; x < this->m_Controls->m_StatisticsTable->columnCount(); x++)
    {
      for (int y = 0; y < this->m_Controls->m_StatisticsTable->rowCount(); y++)
      {
        QTableWidgetItem* item = this->m_Controls->m_StatisticsTable->item(y, x);
        if (item == nullptr)
          break;

        if (x == timestep)
        {
          item->setBackgroundColor(Qt::yellow);
        }
        else
        {
          if (y % 2 == 0)
            item->setBackground(this->m_Controls->m_StatisticsTable->palette().base());
          else
            item->setBackground(this->m_Controls->m_StatisticsTable->palette().alternateBase());
        }
      }
    }

    this->m_Controls->m_StatisticsTable->viewport()->update();
  }

  if ((this->m_SelectedImage->GetTimeSteps() == 1 && timestep == 0) ||
      this->m_SelectedImage->GetTimeSteps() > 1)
  {
    m_Controls->m_JSHistogram->Clear();
    QmitkImageStatisticsCalculationThread::HistogramType::ConstPointer histogram =
        (QmitkImageStatisticsCalculationThread::HistogramType::ConstPointer)this->m_CalculationThread->GetTimeStepHistogram(timestep);

    if (histogram.IsNotNull())
    {
      bool statisticsUpdateSuccessful = this->m_CalculationThread->GetStatisticsUpdateSuccessFlag();

      if (statisticsUpdateSuccessful)
      {

        auto imageNameLabel = m_Controls->m_SelectedFeatureImageLabel->text().toStdString();
        this->m_Controls->m_JSHistogram->AddData2D(ConvertHistogramToMap(histogram), imageNameLabel);
        if (this->m_Controls->m_lineRadioButton->isChecked())
        {
          this->m_Controls->m_JSHistogram->SetChartType(imageNameLabel, QmitkChartWidget::ChartType::line);
        }
        else
        {
          this->m_Controls->m_JSHistogram->SetChartType(imageNameLabel, QmitkChartWidget::ChartType::bar);
        }
        this->m_Controls->m_JSHistogram->SetXAxisLabel("Grey value");
        this->m_Controls->m_JSHistogram->SetYAxisLabel("Frequency");
        this->m_Controls->m_JSHistogram->Show(this->m_Controls->m_ShowSubchartCheckBox->isChecked());
      }
    }
  }
}

void QmitkImageStatisticsView::JumpToCoordinates(int row ,int col)
{
  if(m_SelectedDataNodes.isEmpty())
  {
    MITK_WARN("QmitkImageStatisticsView") << "No data node selected for statistics calculation." ;
    return;
  }

  mitk::Point3D world;
  if (row==5 && !m_WorldMinList.empty())
    world = m_WorldMinList[col];
  else if (row==4 && !m_WorldMaxList.empty())
    world = m_WorldMaxList[col];
  else
    return;

  mitk::IRenderWindowPart* part = this->GetRenderWindowPart();
  if (part)
  {
    part->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->SelectSliceByPoint(world);
    part->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController()->SelectSliceByPoint(world);
    part->GetQmitkRenderWindow("coronal")->GetSliceNavigationController()->SelectSliceByPoint(world);

    mitk::SliceNavigationController::GeometryTimeEvent timeEvent(this->m_SelectedImage->GetTimeGeometry(), col);
    part->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->SetGeometryTime(timeEvent);
  }
}

void QmitkImageStatisticsView::OnIgnoreZerosCheckboxClicked()
{
  emit StatisticsUpdate();
}

void QmitkImageStatisticsView::OnClipboardHistogramButtonClicked()
{
  if (!m_CurrentStatisticsValid)
  {
    QApplication::clipboard()->clear();
  }

  if (m_SelectedPlanarFigure == nullptr)
  {
    const unsigned int t = this->GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();

    typedef mitk::ImageStatisticsCalculator::HistogramType HistogramType;
    const HistogramType *histogram = this->m_CalculationThread->GetTimeStepHistogram(t).GetPointer();

    QString clipboard("Measurement \t Frequency\n");
    for (HistogramType::ConstIterator it = histogram->Begin();
      it != histogram->End();
      ++it)
    {
      clipboard = clipboard.append("%L1 \t %L2\n")
        .arg(it.GetMeasurementVector()[0], 0, 'f', 2)
        .arg(it.GetFrequency());
    }

    QApplication::clipboard()->setText(
      clipboard, QClipboard::Clipboard);
  }
  //If a (non-closed) PlanarFigure is selected, display a line profile widget
  else if (m_SelectedPlanarFigure != nullptr)
  {
    QString clipboard("Pixel \t Intensity\n");
    for (unsigned int i = 0; i < m_IntensityProfileList.size(); i++)
    {
      clipboard =
        clipboard.append("%L1 \t %L2\n").arg(QString::number(i)).arg(QString::number(m_IntensityProfileList.at(i)));
    }
    QApplication::clipboard()->setText(clipboard, QClipboard::Clipboard);
  }
}

void QmitkImageStatisticsView::OnClipboardStatisticsButtonClicked()
{
  QLocale tempLocal;
  QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
  if ( m_CurrentStatisticsValid && !( m_SelectedPlanarFigure != nullptr))
   {
    const std::vector<mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer> &statistics =
      this->m_CalculationThread->GetStatisticsData();

    // Set time borders for for loop ;)
    unsigned int startT, endT;
    if(this->m_Controls->m_CheckBox4dCompleteTable->checkState()==Qt::CheckState::Unchecked)
    {
        startT = this->GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->
          GetPos();
        endT = startT+1;
    }
    else
    {
        startT = 0;
        endT = statistics.size();
    }
    QVector< QVector<QString> > statisticsTable;
    QStringList headline{ "Timestep", "Mean", "Median", "StdDev", "RMS", "Max", "Min", "NumberOfVoxels", "Skewness", "Kurtosis", "Uniformity", "Entropy", "MPP", "UPP", "V [mm³]" };

    for(int i=0;i<headline.size();i++)
    {
        QVector<QString> row;
        row.append(headline.at(i));
        statisticsTable.append(row);
    }

    // Fill Table
    for(unsigned int t=startT;t<endT;t++)
    {
        // Copy statistics to clipboard ("%Ln" will use the default locale for
        // number formatting)
        QStringList value;
        value << QString::number(t)
              << QString::number(statistics[t]->GetMean())
              << QString::number(statistics[t]->GetMedian())
              << QString::number(statistics[t]->GetStd())
              << QString::number(statistics[t]->GetRMS())
              << QString::number(statistics[t]->GetMax())
              << QString::number(statistics[t]->GetMin())
              << QString::number(statistics[t]->GetN())
              << QString::number(statistics[t]->GetSkewness())
              << QString::number(statistics[t]->GetKurtosis())
              << QString::number(statistics[t]->GetUniformity())
              << QString::number(statistics[t]->GetEntropy())
              << QString::number(statistics[t]->GetMPP())
              << QString::number(statistics[t]->GetUPP())
              << QString::number(m_Controls->m_StatisticsTable->item(7, 0)->data(Qt::DisplayRole).toDouble());

         for(int z=0;z<value.size();z++)
         {
             statisticsTable[z].append(value.at(z));
         }
    }

    // Create output string
    QString clipboard;
    for(int i=0;i<statisticsTable.size();i++)
    {
        for(int t=0;t<statisticsTable.at(i).size();t++)
        {
            clipboard.append(statisticsTable.at(i).at(t));
            clipboard.append("\t");
        }
        clipboard.append("\n");
    }
    QApplication::clipboard()->setText(clipboard, QClipboard::Clipboard);
  }
  else
  {
    QApplication::clipboard()->clear();
  }
  QLocale::setDefault(tempLocal);
}

void QmitkImageStatisticsView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*part*/,
    const QList<mitk::DataNode::Pointer> &nodes )
{
  if (this->m_Visible)
  {
    this->SelectionChanged( nodes );
  }
  else
  {
    this->m_DataNodeSelectionChanged = true;
  }
}

void QmitkImageStatisticsView::SelectionChanged(const QList<mitk::DataNode::Pointer> &selectedNodes)
{
  m_Controls->m_JSHistogram->Clear();

  if( this->m_StatisticsUpdatePending )
  {
    this->m_DataNodeSelectionChanged = true;
    return; // not ready for new data now!
  }

  if (selectedNodes.size() == this->m_SelectedDataNodes.size())
  {
    int i = 0;
    for (; i < selectedNodes.size(); ++i)
    {
      if (selectedNodes.at(i) != this->m_SelectedDataNodes.at(i))
      {
        break;
      }
    }
    // node selection did not change
    if (i == selectedNodes.size()) return;
  }

  //reset the feature image and image mask field
  m_Controls->m_SelectedFeatureImageLabel->setText("None");
  m_Controls->m_SelectedMaskLabel->setText("None");

  this->ReinitData();
  if (selectedNodes.isEmpty())
  {
    DisableHistogramGUIElements();
  }
  else
  {
    EnableHistogramGUIElements();
    ResetHistogramGUIElementsToDefault();
  }
  if(selectedNodes.size() == 1 || selectedNodes.size() == 2)
  {
    bool isBinary = false;
    selectedNodes.value(0)->GetBoolProperty("binary",isBinary);
    mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
    isBinary |= isLabelSet->CheckNode(selectedNodes.value(0));
    if(isBinary)
    {
      EnableHistogramGUIElements();
      m_Controls->m_InfoLabel->setText("");
    }
    for (int i= 0; i< selectedNodes.size(); ++i)
    {
      this->m_SelectedDataNodes.push_back(selectedNodes.at(i));
    }
    this->m_DataNodeSelectionChanged = false;
    this->m_Controls->m_ErrorMessageLabel->setText( "" );
    this->m_Controls->m_ErrorMessageLabel->hide();
    emit StatisticsUpdate();
  }
  else
  {
    this->m_DataNodeSelectionChanged = false;
  }
}

void QmitkImageStatisticsView::DisableHistogramGUIElements()
{
  m_Controls->m_InfoLabel->setText("");
  m_Controls->groupBox_histogram->setEnabled(false);
  m_Controls->groupBox_statistics->setEnabled(false);
}

void QmitkImageStatisticsView::ResetHistogramGUIElementsToDefault()
{
  m_Controls->m_barRadioButton->setChecked(true);
  m_Controls->m_HistogramNBinsSpinbox->setValue(100);
  m_HistogramNBins = m_Controls->m_HistogramNBinsSpinbox->value();
  m_Controls->m_UseDefaultNBinsCheckBox->setChecked(true);
  m_Controls->m_ShowSubchartCheckBox->setChecked(true);
  m_Controls->m_BinSizeFrame->setEnabled(false);
  m_Controls->m_barRadioButton->setEnabled(true);
  m_Controls->m_lineRadioButton->setEnabled(true);
  m_Controls->m_HistogramNBinsSpinbox->setEnabled(true);
  this->m_CalculationThread->SetHistogramNBins(m_Controls->m_HistogramNBinsSpinbox->value());
}

void QmitkImageStatisticsView::EnableHistogramGUIElements()
{
  m_Controls->groupBox_histogram->setEnabled(true);
  m_Controls->groupBox_plot->setEnabled(true);
  m_Controls->groupBox_statistics->setEnabled(true);
}

void QmitkImageStatisticsView::ReinitData()
{
  while( this->m_CalculationThread->isRunning()) // wait until thread has finished
  {
    itksys::SystemTools::Delay(100);
  }

  if(this->m_SelectedImage != nullptr)
  {
    this->m_SelectedImage->RemoveObserver( this->m_ImageObserverTag);
    this->m_SelectedImage = nullptr;
  }
  if(this->m_SelectedImageMask != nullptr)
  {
    this->m_SelectedImageMask->RemoveObserver( this->m_ImageMaskObserverTag);
    this->m_SelectedImageMask = nullptr;
  }
  if(this->m_SelectedPlanarFigure != nullptr)
  {
    this->m_SelectedPlanarFigure->RemoveObserver( this->m_PlanarFigureObserverTag);
    this->m_SelectedPlanarFigure = nullptr;
  }
  this->m_SelectedDataNodes.clear();
  this->m_StatisticsUpdatePending = false;

  m_Controls->m_ErrorMessageLabel->setText( "" );
  m_Controls->m_ErrorMessageLabel->hide();
  this->InvalidateStatisticsTableView();
  m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
}

void QmitkImageStatisticsView::OnThreadedStatisticsCalculationEnds()
{
  m_Controls->m_ErrorMessageLabel->setText("");
  m_Controls->m_ErrorMessageLabel->hide();
  this->WriteStatisticsToGUI();
}

void QmitkImageStatisticsView::UpdateStatistics()
{
  mitk::IRenderWindowPart* renderPart = this->GetRenderWindowPart();
  if ( renderPart == nullptr )
  {
    this->m_StatisticsUpdatePending =  false;
    return;
  }
  m_WorldMinList.clear();
  m_WorldMaxList.clear();

  // classify selected nodes
  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
  mitk::NodePredicateDataType::Pointer isLabelSet = mitk::NodePredicateDataType::New("LabelSetImage");
  mitk::NodePredicateOr::Pointer imagePredicate = mitk::NodePredicateOr::New(isImage, isLabelSet);

  std::string maskName;
  std::string maskType;
  std::string featureImageName;
  unsigned int maskDimension = 0;

  // reset data from last run
  ITKCommandType::Pointer changeListener = ITKCommandType::New();
  changeListener->SetCallbackFunction( this, &QmitkImageStatisticsView::SelectedDataModified );

  mitk::DataNode::Pointer planarFigureNode;
  for( int i= 0 ; i < this->m_SelectedDataNodes.size(); ++i)
  {
    mitk::PlanarFigure::Pointer planarFig = dynamic_cast<mitk::PlanarFigure*>(this->m_SelectedDataNodes.at(i)->GetData());
    if( imagePredicate->CheckNode(this->m_SelectedDataNodes.at(i)) )
    {
      bool isMask = false;
      this->m_SelectedDataNodes.at(i)->GetPropertyValue("binary", isMask);
      isMask |= isLabelSet->CheckNode(this->m_SelectedDataNodes.at(i));

      if( this->m_SelectedImageMask == nullptr && isMask)
      {
        this->m_SelectedImageMask = dynamic_cast<mitk::Image*>(this->m_SelectedDataNodes.at(i)->GetData());
        this->m_ImageMaskObserverTag = this->m_SelectedImageMask->AddObserver(itk::ModifiedEvent(), changeListener);

        maskName = this->m_SelectedDataNodes.at(i)->GetName();
        maskType = m_SelectedImageMask->GetNameOfClass();
        maskDimension = 3;
      }
      else if( !isMask )
      {
        if(this->m_SelectedImage == nullptr)
        {
          this->m_SelectedImage = static_cast<mitk::Image*>(this->m_SelectedDataNodes.at(i)->GetData());
          this->m_ImageObserverTag = this->m_SelectedImage->AddObserver(itk::ModifiedEvent(), changeListener);
        }
        featureImageName = this->m_SelectedDataNodes.at(i)->GetName();
      }
    }
    else if (planarFig.IsNotNull())
    {
      if(this->m_SelectedPlanarFigure == nullptr)
      {
        this->m_SelectedPlanarFigure = planarFig;
        this->m_PlanarFigureObserverTag  =
            this->m_SelectedPlanarFigure->AddObserver(mitk::EndInteractionPlanarFigureEvent(), changeListener);
        maskName = this->m_SelectedDataNodes.at(i)->GetName();
        maskType = this->m_SelectedPlanarFigure->GetNameOfClass();
        maskDimension = 2;
        planarFigureNode = m_SelectedDataNodes.at(i);
      }
    }
    else
    {
      m_Controls->m_ErrorMessageLabel->setText("<font color='red'>Invalid data node type!</font>");
      m_Controls->m_ErrorMessageLabel->show();
    }
  }

  if(maskName == "")
  {
    maskName = "None";
    maskType = "";
    maskDimension = 0;
  }

  if(featureImageName == "")
  {
    featureImageName = "None";
  }

  if (m_SelectedPlanarFigure != nullptr && m_SelectedImage == nullptr)
  {
    mitk::DataStorage::SetOfObjects::ConstPointer parentSet = this->GetDataStorage()->GetSources(planarFigureNode);
    for (unsigned int i=0; i<parentSet->Size(); i++)
    {
      mitk::DataNode::Pointer node = parentSet->ElementAt(i);
      if( imagePredicate->CheckNode(node) )
      {
        bool isMask = false;
        node->GetPropertyValue("binary", isMask);
        isMask |= isLabelSet->CheckNode(node);

        if( !isMask )
        {
          if(this->m_SelectedImage == nullptr)
          {
            this->m_SelectedImage = static_cast<mitk::Image*>(node->GetData());
            this->m_ImageObserverTag = this->m_SelectedImage->AddObserver(itk::ModifiedEvent(), changeListener);
          }
        }
      }
    }
  }

  unsigned int timeStep = renderPart->GetTimeNavigationController()->GetTime()->GetPos();

  if ( m_SelectedImage != nullptr && m_SelectedImage->IsInitialized())
  {
    // Check if a the selected image is a multi-channel image. If yes, statistics
    // cannot be calculated currently.
    if ( m_SelectedImage->GetPixelType().GetNumberOfComponents() > 1 )
    {
      m_Controls->m_ErrorMessageLabel->setText( "<font color='red'>Multi-component images not supported.</font>" );
      m_Controls->m_ErrorMessageLabel->show();

      this->InvalidateStatisticsTableView();
      m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
      m_CurrentStatisticsValid = false;
      this->m_StatisticsUpdatePending = false;
      this->DisableHistogramGUIElements();
      m_Controls->m_InfoLabel->setText("");
      return;
    }

    std::stringstream maskLabel;
    maskLabel << maskName;
    if ( maskDimension > 0 )
    {
      maskLabel << "  [" << maskDimension << "D " << maskType << "]";
    }
    m_Controls->m_SelectedMaskLabel->setText( maskLabel.str().c_str() );
    m_Controls->m_SelectedFeatureImageLabel->setText(featureImageName.c_str());

    // check time step validity
    if(m_SelectedImage->GetDimension() <= 3 && timeStep > m_SelectedImage->GetDimension(3)-1)
    {
      timeStep = m_SelectedImage->GetDimension(3)-1;
    }

    // Add the used mask time step to the mask label so the user knows which mask time step was used
    // if the image time step is bigger than the total number of mask time steps (see
    // ImageStatisticsCalculator::ExtractImageAndMask)
    if (m_SelectedImageMask != nullptr)
    {
      unsigned int maskTimeStep = timeStep;

      if (maskTimeStep >= m_SelectedImageMask->GetTimeSteps())
      {
        maskTimeStep = m_SelectedImageMask->GetTimeSteps() - 1;
      }

      m_Controls->m_SelectedMaskLabel->setText(m_Controls->m_SelectedMaskLabel->text() +
          QString(" (t=") +
          QString::number(maskTimeStep) +
          QString(")"));
    }

    // check if the segmentation mask is empty
    if (m_SelectedImageMask != nullptr)
    {
      auto maskStatistics = m_SelectedImageMask->GetStatistics();
      mitk::ScalarType maskMaxValue = maskStatistics->GetScalarValueMax(0);
      if (m_SelectedImageMask->GetDimension() == 4) {
        for (unsigned int curTimestep = 1; curTimestep < m_SelectedImageMask->GetTimeSteps(); curTimestep++) {
          maskMaxValue = std::max(maskStatistics->GetScalarValueMax(curTimestep), maskMaxValue);
        }
      }

      bool segmentationIsEmpty = maskMaxValue == 0;

      if (segmentationIsEmpty)
      {
        m_Controls->m_ErrorMessageLabel->setText( "<font color='red'>Empty segmentation mask selected...</font>" );
        m_Controls->m_ErrorMessageLabel->show();
        this->m_StatisticsUpdatePending = false;
        return;
      }
    }

    //// initialize thread and trigger it
    this->m_CalculationThread->SetIgnoreZeroValueVoxel( m_Controls->m_IgnoreZerosCheckbox->isChecked() );
    this->m_CalculationThread->Initialize( m_SelectedImage, m_SelectedImageMask, m_SelectedPlanarFigure );
    this->m_CalculationThread->SetTimeStep( timeStep );

    m_Controls->m_ErrorMessageLabel->setText("<font color='red'>Calculating statistics...</font>");
    m_Controls->m_ErrorMessageLabel->show();

    try
    {
      // Compute statistics
      this->m_CalculationThread->start();
    }
    catch ( const mitk::Exception& e)
    {
      m_Controls->m_ErrorMessageLabel->setText("<font color='red'>" + QString(e.GetDescription()) + "</font>");
      m_Controls->m_ErrorMessageLabel->show();
      this->m_StatisticsUpdatePending = false;
    }
    catch ( const std::runtime_error &e )
    {
      // In case of exception, print error message on GUI
      m_Controls->m_ErrorMessageLabel->setText("<font color='red'>" + QString(e.what()) + "</font>");
      m_Controls->m_ErrorMessageLabel->show();
      this->m_StatisticsUpdatePending = false;
    }
    catch ( const std::exception &e )
    {
      MITK_ERROR << "Caught exception: " << e.what();

      // In case of exception, print error message on GUI
      m_Controls->m_ErrorMessageLabel->setText("<font color='red'>" + QString(e.what()) + "</font>");
      m_Controls->m_ErrorMessageLabel->show();
      this->m_StatisticsUpdatePending = false;
    }
  }
  else
  {
    this->m_StatisticsUpdatePending = false;
  }
}

void QmitkImageStatisticsView::SelectedDataModified()
{
  if( !m_StatisticsUpdatePending )
  {
    emit StatisticsUpdate();
  }
}

void QmitkImageStatisticsView::NodeRemoved(const mitk::DataNode *node)
{
  while(this->m_CalculationThread->isRunning()) // wait until thread has finished
  {
    itksys::SystemTools::Delay(100);
  }

  if (node->GetData() == m_SelectedImage)
  {
    m_SelectedImage = nullptr;
  }
}

void QmitkImageStatisticsView::RequestStatisticsUpdate()
{
  if ( !m_StatisticsUpdatePending )
  {
    if(this->m_DataNodeSelectionChanged)
    {
      this->SelectionChanged(this->GetCurrentSelection());
    }
    else
    {
      this->m_StatisticsUpdatePending = true;
      this->UpdateStatistics();
    }
  }
  if (this->GetRenderWindowPart())
    this->GetRenderWindowPart()->RequestUpdate();
}

void QmitkImageStatisticsView::OnHistogramNBinsCheckBoxValueChanged()
{
    if (static_cast<unsigned int>(m_Controls->m_HistogramNBinsSpinbox->value()) != m_HistogramNBins)
    {
      m_HistogramNBins = m_Controls->m_HistogramNBinsSpinbox->value();
        this->m_CalculationThread->SetHistogramNBins(m_Controls->m_HistogramNBinsSpinbox->value());
        this->UpdateStatistics();
    }
}

void QmitkImageStatisticsView::WriteStatisticsToGUI()
{
  m_Controls->m_JSHistogram->Clear();
  m_IntensityProfileList.clear();

  //Disconnect OnLineRadioButtonSelected() to prevent reloading chart when radiobutton is checked programmatically
  disconnect((QObject*)(this->m_Controls->m_JSHistogram), SIGNAL(PageSuccessfullyLoaded()), 0, 0);
  connect((QObject*)(this->m_Controls->m_JSHistogram), SIGNAL(PageSuccessfullyLoaded()), (QObject*) this, SLOT(OnPageSuccessfullyLoaded()));
  m_Controls->m_InfoLabel->setText("");

  if (m_DataNodeSelectionChanged)
  {
    this->m_StatisticsUpdatePending = false;
    this->RequestStatisticsUpdate();
    return;    // stop visualization of results and calculate statistics of new selection
  }

  if (this->m_CalculationThread->GetStatisticsUpdateSuccessFlag())
  {
    if (this->m_CalculationThread->GetStatisticsChangedFlag())
    {
      // Do not show any error messages
      m_Controls->m_ErrorMessageLabel->hide();
      m_CurrentStatisticsValid = true;
    }

    if (m_SelectedImage != nullptr) {
      //all statistics are now computed also on planar figures (lines, paths...)!
    // If a (non-closed) PlanarFigure is selected, display a line profile widget
      if (m_SelectedPlanarFigure != nullptr && !m_SelectedPlanarFigure->IsClosed()) {

        // check whether PlanarFigure is initialized
        const mitk::PlaneGeometry *planarFigurePlaneGeometry = m_SelectedPlanarFigure->GetPlaneGeometry();

        if (planarFigurePlaneGeometry != nullptr)
        {
          unsigned int timeStep = this->GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();

          mitk::Image::Pointer image;

          if (this->m_CalculationThread->GetStatisticsImage()->GetDimension() == 4)
          {
            mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
            timeSelector->SetInput(this->m_CalculationThread->GetStatisticsImage());
            timeSelector->SetTimeNr(timeStep);
            timeSelector->Update();

            image = timeSelector->GetOutput();
          }
          else
          {
            image = this->m_CalculationThread->GetStatisticsImage();
          }

          mitk::IntensityProfile::ConstPointer intensityProfile = (mitk::IntensityProfile::ConstPointer)mitk::ComputeIntensityProfile(image, m_SelectedPlanarFigure);

          m_IntensityProfileList = ConvertIntensityProfileToVector(intensityProfile);
          auto lineDataLabel = "Intensity profile " + m_Controls->m_SelectedMaskLabel->text().toStdString();
          m_Controls->m_JSHistogram->SetChartType(lineDataLabel, QmitkChartWidget::ChartType::line);
          m_Controls->m_JSHistogram->AddData1D(m_IntensityProfileList, lineDataLabel);
          m_Controls->m_JSHistogram->SetXAxisLabel("Distance");
          m_Controls->m_JSHistogram->SetYAxisLabel("Intensity");
          m_Controls->m_JSHistogram->Show(m_Controls->m_ShowSubchartCheckBox->isChecked());

          m_Controls->m_lineRadioButton->setChecked(true);
          m_Controls->m_lineRadioButton->setEnabled(false);
          m_Controls->m_barRadioButton->setEnabled(false);
          m_Controls->m_HistogramNBinsSpinbox->setEnabled(false);
          m_Controls->m_BinSizeFrame->setEnabled(false);
          m_Controls->m_UseDefaultNBinsCheckBox->setEnabled(false);

          //Reconnect OnLineRadioButtonSelected()
          connect((QObject*)(this->m_Controls->m_JSHistogram), SIGNAL(PageSuccessfullyLoaded()), (QObject*) this, SLOT(OnLineRadioButtonSelected()));
          auto statisticsVector = this->m_CalculationThread->GetStatisticsData();
          //only one entry (current timestep)
          this->FillLinearProfileStatisticsTableView(statisticsVector.front().GetPointer(), this->m_CalculationThread->GetStatisticsImage());

          QString message("<font color='red'>Only linegraph available for an intensity profile!");
          if (this->m_CalculationThread->GetStatisticsImage()->GetDimension() == 4) {
            message += "Only current timestep displayed!";
          }
          message += "</font>";
          m_Controls->m_InfoLabel->setText(message);
          m_CurrentStatisticsValid = true;
        }
        else
        {
          // Clear statistics, histogram, and GUI
          this->InvalidateStatisticsTableView();
          m_Controls->m_StatisticsWidgetStack->setCurrentIndex(0);
          m_CurrentStatisticsValid = false;
          m_Controls->m_ErrorMessageLabel->hide();
          m_Controls->m_SelectedMaskLabel->setText("None");
          this->m_StatisticsUpdatePending = false;
          m_Controls->m_InfoLabel->setText("");
          return;
        }
      }
      else
      {
        m_Controls->m_StatisticsWidgetStack->setCurrentIndex(0);

        auto histogram = this->m_CalculationThread->GetTimeStepHistogram(this->m_CalculationThread->GetTimeStep()).GetPointer();

        auto imageLabelName = m_Controls->m_SelectedFeatureImageLabel->text().toStdString();
        m_Controls->m_JSHistogram->AddData2D(ConvertHistogramToMap(histogram), imageLabelName);
        m_Controls->m_JSHistogram->SetChartType(imageLabelName, QmitkChartWidget::ChartType::bar);
        this->m_Controls->m_JSHistogram->SetXAxisLabel("Gray value");
        this->m_Controls->m_JSHistogram->SetYAxisLabel("Frequency");
        m_Controls->m_UseDefaultNBinsCheckBox->setEnabled(true);
        m_Controls->m_JSHistogram->Show(this->m_Controls->m_ShowSubchartCheckBox->isChecked());
        this->FillStatisticsTableView(this->m_CalculationThread->GetStatisticsData(), this->m_CalculationThread->GetStatisticsImage());
      }
      m_CurrentStatisticsValid = true;
    }
  }
  else
  {
    m_Controls->m_SelectedMaskLabel->setText("None");
    m_Controls->m_ErrorMessageLabel->setText(m_CalculationThread->GetLastErrorMessage().c_str());
    m_Controls->m_ErrorMessageLabel->show();

    // Clear statistics and histogram
    this->InvalidateStatisticsTableView();
    m_Controls->m_StatisticsWidgetStack->setCurrentIndex(0);
    m_CurrentStatisticsValid = false;

  }
  berry::IPreferencesService* prefService = berry::WorkbenchPlugin::GetDefault()->GetPreferencesService();
  m_StylePref = prefService->GetSystemPreferences()->Node(berry::QtPreferences::QT_STYLES_NODE);

  this->m_StatisticsUpdatePending = false;
}

void QmitkImageStatisticsView::FillStatisticsTableView(
    const std::vector<mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer> &statistics,
    const mitk::Image *image )
{
  this->m_Controls->m_StatisticsTable->setColumnCount(image->GetTimeSteps());
  this->m_Controls->m_StatisticsTable->horizontalHeader()->setVisible(image->GetTimeSteps() > 1);

  // Set Checkbox for complete copy of statistic table
  if(image->GetTimeSteps()>1)
  {
    this->m_Controls->m_CheckBox4dCompleteTable->setEnabled(true);
  }
  else
  {
    this->m_Controls->m_CheckBox4dCompleteTable->setEnabled(false);
    this->m_Controls->m_CheckBox4dCompleteTable->setChecked(false);
  }


  for (unsigned int t = 0; t < image->GetTimeSteps(); t++)
  {
    this->m_Controls->m_StatisticsTable->setHorizontalHeaderItem(t,
        new QTableWidgetItem(QString::number(t)));

    if (statistics.at(t)->GetMaxIndex().size()==3)
    {
      mitk::Point3D index, max, min;
      index[0] = statistics.at(t)->GetMaxIndex()[0];
      index[1] = statistics.at(t)->GetMaxIndex()[1];
      index[2] = statistics.at(t)->GetMaxIndex()[2];
      m_SelectedImage->GetGeometry()->IndexToWorld(index, max);
      this->m_WorldMaxList.push_back(max);
      index[0] = statistics.at(t)->GetMinIndex()[0];
      index[1] = statistics.at(t)->GetMinIndex()[1];
      index[2] = statistics.at(t)->GetMinIndex()[2];
      m_SelectedImage->GetGeometry()->IndexToWorld(index, min);
      this->m_WorldMinList.push_back(min);
    }

    auto statisticsVector = AssembleStatisticsIntoVector(statistics.at(t).GetPointer(), image);

    unsigned int count = 0;
    for (const auto& entry : statisticsVector) {
      auto item = new QTableWidgetItem(entry);
      this->m_Controls->m_StatisticsTable->setItem(count, t, item);
      count++;
    }

    }


  this->m_Controls->m_StatisticsTable->resizeColumnsToContents();
  int height = STAT_TABLE_BASE_HEIGHT;

  if (this->m_Controls->m_StatisticsTable->horizontalHeader()->isVisible())
    height += this->m_Controls->m_StatisticsTable->horizontalHeader()->height();

  if (this->m_Controls->m_StatisticsTable->horizontalScrollBar()->isVisible())
    height += this->m_Controls->m_StatisticsTable->horizontalScrollBar()->height();

  this->m_Controls->m_StatisticsTable->setMinimumHeight(height);

  // make sure the current timestep's column is highlighted (and the correct histogram is displayed)
  unsigned int t = this->GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->
      GetPos();
  mitk::SliceNavigationController::GeometryTimeEvent timeEvent(this->m_SelectedImage->GetTimeGeometry(),
      t);
  this->OnTimeChanged(timeEvent);

  t = std::min(image->GetTimeSteps() - 1, t);

  // See bug 18340
  /*QString hotspotMean; hotspotMean.append(QString("%1").arg(s[t].GetHotspotStatistics().GetMean(), 0, 'f', decimals));
  hotspotMean += " (";
  for (int i=0; i<s[t].GetHotspotIndex().size(); i++)
  {
  hotspotMean += QString::number(s[t].GetHotspotIndex()[i]);
  if (i<s[t].GetHotspotIndex().size()-1)
  hotspotMean += ",";
  }
  hotspotMean += ")";

  this->m_Controls->m_StatisticsTable->setItem( 7, t, new QTableWidgetItem( hotspotMean ) );

  QString hotspotMax; hotspotMax.append(QString("%1").arg(s[t].GetHotspotStatistics().GetMax(), 0, 'f', decimals));
  hotspotMax += " (";
  for (int i=0; i<s[t].GetHotspotStatistics().GetMaxIndex().size(); i++)
  {
  hotspotMax += QString::number(s[t].GetHotspotStatistics().GetMaxIndex()[i]);
  if (i<s[t].GetHotspotStatistics().GetMaxIndex().size()-1)
  hotspotMax += ",";
  }
  hotspotMax += ")";

  this->m_Controls->m_StatisticsTable->setItem( 8, t, new QTableWidgetItem( hotspotMax ) );

  QString hotspotMin; hotspotMin.append(QString("%1").arg(s[t].GetHotspotStatistics().GetMin(), 0, 'f', decimals));
  hotspotMin += " (";
  for (int i=0; i<s[t].GetHotspotStatistics().GetMinIndex().size(); i++)
  {
  hotspotMin += QString::number(s[t].GetHotspotStatistics().GetMinIndex()[i]);
  if (i<s[t].GetHotspotStatistics().GetMinIndex().size()-1)
  hotspotMin += ",";
  }
  hotspotMin += ")";

  this->m_Controls->m_StatisticsTable->setItem( 9, t, new QTableWidgetItem( hotspotMin ) );*/
}

std::vector<QString> QmitkImageStatisticsView::AssembleStatisticsIntoVector(mitk::ImageStatisticsCalculator::StatisticsContainer::ConstPointer statistics, mitk::Image::ConstPointer image, bool noVolumeDefined) const
{
  std::vector<QString> result;

  unsigned int decimals = 2;

  //statistics of higher order should have 5 decimal places because they used to be very small
  unsigned int decimalsHigherOrderStatistics = 5;

  if (image->GetPixelType().GetComponentType() == itk::ImageIOBase::DOUBLE || image->GetPixelType().GetComponentType() == itk::ImageIOBase::FLOAT)
  {
    decimals = 5;
  }

  result.push_back(GetFormattedString(statistics->GetMean(), decimals));
  result.push_back(GetFormattedString(statistics->GetMedian(), decimals));
  result.push_back(GetFormattedString(statistics->GetStd(), decimals));
  result.push_back(GetFormattedString(statistics->GetRMS(), decimals));
  result.push_back(GetFormattedString(statistics->GetMax(), decimals) + " " + GetFormattedIndex(statistics->GetMaxIndex()));
  result.push_back(GetFormattedString(statistics->GetMin(), decimals) + " " + GetFormattedIndex(statistics->GetMinIndex()));
  //to prevent large negative values of empty image statistics
  if (statistics->GetN() != std::numeric_limits<long>::min()) {
    result.push_back(GetFormattedString(statistics->GetN(), 0));

    const mitk::BaseGeometry *geometry = image->GetGeometry();
    if (geometry != NULL && !noVolumeDefined)
  {
      const mitk::Vector3D &spacing = image->GetGeometry()->GetSpacing();
      double volume = spacing[0] * spacing[1] * spacing[2] * static_cast<double>(statistics->GetN());
      result.push_back(GetFormattedString(volume, decimals));
  }
    else {
      result.push_back("NA");
  }
  }
  else {
    result.push_back("NA");
    result.push_back("NA");
  }

  result.push_back(GetFormattedString(statistics->GetSkewness(), decimalsHigherOrderStatistics));
  result.push_back(GetFormattedString(statistics->GetKurtosis(), decimalsHigherOrderStatistics));
  result.push_back(GetFormattedString(statistics->GetUniformity(), decimalsHigherOrderStatistics));
  result.push_back(GetFormattedString(statistics->GetEntropy(), decimalsHigherOrderStatistics));
  result.push_back(GetFormattedString(statistics->GetMPP(), decimals));
  result.push_back(GetFormattedString(statistics->GetUPP(), decimalsHigherOrderStatistics));

  return result;
}

void QmitkImageStatisticsView::FillLinearProfileStatisticsTableView(mitk::ImageStatisticsCalculator::StatisticsContainer::ConstPointer statistics,
  const mitk::Image *image)
{
  this->m_Controls->m_StatisticsTable->setColumnCount(1);
  this->m_Controls->m_StatisticsTable->horizontalHeader()->setVisible(false);

  m_PlanarFigureStatistics = this->AssembleStatisticsIntoVector(statistics, image, true);

  for (unsigned int i = 0; i< m_PlanarFigureStatistics.size(); i++)
  {
    this->m_Controls->m_StatisticsTable->setItem( i, 0, new QTableWidgetItem(m_PlanarFigureStatistics[i] ));
  }

  this->m_Controls->m_StatisticsTable->resizeColumnsToContents();
  int height = STAT_TABLE_BASE_HEIGHT;

  if (this->m_Controls->m_StatisticsTable->horizontalHeader()->isVisible())
    height += this->m_Controls->m_StatisticsTable->horizontalHeader()->height();

  if (this->m_Controls->m_StatisticsTable->horizontalScrollBar()->isVisible())
    height += this->m_Controls->m_StatisticsTable->horizontalScrollBar()->height();

  this->m_Controls->m_StatisticsTable->setMinimumHeight(height);
}

void QmitkImageStatisticsView::InvalidateStatisticsTableView()
{
  this->m_Controls->m_StatisticsTable->horizontalHeader()->setVisible(false);
  this->m_Controls->m_StatisticsTable->setColumnCount(1);

  for ( int i = 0; i < this->m_Controls->m_StatisticsTable->rowCount(); ++i )
  {
    {
      this->m_Controls->m_StatisticsTable->setItem( i, 0, new QTableWidgetItem( "NA" ) );
    }
  }

  this->m_Controls->m_StatisticsTable->setMinimumHeight(STAT_TABLE_BASE_HEIGHT);
}

void QmitkImageStatisticsView::Activated()
{
}

void QmitkImageStatisticsView::Deactivated()
{
}

void QmitkImageStatisticsView::Visible()
{
  m_Visible = true;

  mitk::IRenderWindowPart* renderWindow = GetRenderWindowPart();

  if (renderWindow)
  {
    itk::ReceptorMemberCommand<QmitkImageStatisticsView>::Pointer cmdTimeEvent =
        itk::ReceptorMemberCommand<QmitkImageStatisticsView>::New();
    cmdTimeEvent->SetCallbackFunction(this, &QmitkImageStatisticsView::OnTimeChanged);

    // It is sufficient to add the observer to the axial render window since the GeometryTimeEvent
    // is always triggered by all views.
    m_TimeObserverTag = renderWindow->GetQmitkRenderWindow("axial")->
        GetSliceNavigationController()->
        AddObserver(mitk::SliceNavigationController::GeometryTimeEvent(nullptr, 0), cmdTimeEvent);
  }

  if (m_DataNodeSelectionChanged)
  {
    if (this->IsCurrentSelectionValid())
    {
      this->SelectionChanged(this->GetCurrentSelection());
    }
    else
    {
      this->SelectionChanged(this->GetDataManagerSelection());
    }
    m_DataNodeSelectionChanged = false;
  }
}

void QmitkImageStatisticsView::Hidden()
{
  m_Visible = false;

  // The slice navigation controller observer is removed here instead of in the destructor.
  // If it was called in the destructor, the application would freeze because the view's
  // destructor gets called after the render windows have been destructed.
  if ( m_TimeObserverTag != 0 )
  {
    mitk::IRenderWindowPart* renderWindow = GetRenderWindowPart();

    if (renderWindow)
    {
      renderWindow->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->
          RemoveObserver( m_TimeObserverTag );
    }
    m_TimeObserverTag = 0;
  }
}

void QmitkImageStatisticsView::SetFocus()
{
}

std::map<double, double> QmitkImageStatisticsView::ConvertHistogramToMap(itk::Statistics::Histogram<double>::ConstPointer histogram) const
{
  std::map<double, double> histogramMap;

  auto endIt = histogram->End();
  auto it = histogram->Begin();

  // generating Lists of measurement and frequencies
  for (; it != endIt; ++it)
  {
    double frequency = it.GetFrequency();
    double measurement = it.GetMeasurementVector()[0];
    histogramMap.emplace(measurement, frequency);
  }

  return histogramMap;
}

std::vector<double> QmitkImageStatisticsView::ConvertIntensityProfileToVector(mitk::IntensityProfile::ConstPointer intensityProfile) const
{
  std::vector<double> intensityProfileList;
  auto end = intensityProfile->End();

  for (auto it = intensityProfile->Begin(); it != end; ++it)
  {
    intensityProfileList.push_back(it.GetMeasurementVector()[0]);
  }
  return intensityProfileList;
}

QString QmitkImageStatisticsView::GetFormattedString(double value, unsigned int decimals) const
{
  typedef mitk::ImageStatisticsCalculator::StatisticsContainer::RealType RealType;
  RealType maxVal = std::numeric_limits<RealType>::max();

  if (value == maxVal)
  {
    return QString("NA");
  }
  else
  {
    return QString("%1").arg(value, 0, 'f', decimals);
  }
}

QString QmitkImageStatisticsView::GetFormattedIndex(const vnl_vector<int>& vector) const
{
  if (vector.empty()) {
    return QString();
  }
  QString formattedIndex("(");
  for (const auto& entry : vector)
  {
    formattedIndex += QString::number(entry);
    formattedIndex += ",";
  }
  formattedIndex.chop(1);
  formattedIndex += ")";
  return formattedIndex;
}
