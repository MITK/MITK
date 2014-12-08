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
#include <qclipboard.h>
#include <qscrollbar.h>

// berry includes
#include <berryIWorkbenchPage.h>

// mitk includes
#include "mitkNodePredicateDataType.h"
#include "mitkPlanarFigureInteractor.h"

// itk includes
#include "itksys/SystemTools.hxx"
#include <mitkILinkedRenderWindowPart.h>
#include <QmitkRenderWindow.h>

const std::string QmitkImageStatisticsView::VIEW_ID = "org.mitk.views.imagestatistics";
const int QmitkImageStatisticsView::STAT_TABLE_BASE_HEIGHT = 180;

QmitkImageStatisticsView::QmitkImageStatisticsView(QObject* /*parent*/, const char* /*name*/)
  : m_Controls( NULL ),
  m_TimeStepperAdapter( NULL ),
  m_SelectedImage( NULL ),
  m_SelectedImageMask( NULL ),
  m_SelectedPlanarFigure( NULL ),
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
  if ( m_SelectedImage != NULL )
    m_SelectedImage->RemoveObserver( m_ImageObserverTag );
  if ( m_SelectedImageMask != NULL )
    m_SelectedImageMask->RemoveObserver( m_ImageMaskObserverTag );
  if ( m_SelectedPlanarFigure != NULL )
    m_SelectedPlanarFigure->RemoveObserver( m_PlanarFigureObserverTag );

  while(this->m_CalculationThread->isRunning()) // wait until thread has finished
  {
    itksys::SystemTools::Delay(100);
  }
  delete this->m_CalculationThread;
}

void QmitkImageStatisticsView::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new Ui::QmitkImageStatisticsViewControls;
    m_Controls->setupUi(parent);
    CreateConnections();

    m_Controls->m_ErrorMessageLabel->hide();
    m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
    m_Controls->m_BinSizeFrame->setVisible(false);
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
    connect( (QObject*) (this->m_Controls->m_barRadioButton), SIGNAL(clicked()), (QObject*) (this->m_Controls->m_JSHistogram), SLOT(OnBarRadioButtonSelected()));
    connect( (QObject*) (this->m_Controls->m_lineRadioButton), SIGNAL(clicked()), (QObject*) (this->m_Controls->m_JSHistogram), SLOT(OnLineRadioButtonSelected()));
    connect( (QObject*) (this->m_Controls->m_HistogramBinSizeSpinbox), SIGNAL(editingFinished()), this, SLOT(OnHistogramBinSizeBoxValueChanged()));
    connect( (QObject*)(this->m_Controls->m_UseDefaultBinSizeBox), SIGNAL(clicked()),(QObject*) this, SLOT(OnDefaultBinSizeBoxChanged()) );
  }
}

void QmitkImageStatisticsView::OnDefaultBinSizeBoxChanged()
{
    if (m_CalculationThread!=NULL)
        m_Controls->m_HistogramBinSizeSpinbox->setValue(m_CalculationThread->GetHistogramBinSize());
    if (m_Controls->m_UseDefaultBinSizeBox->isChecked())
        m_Controls->m_BinSizeFrame->setVisible(false);
    else
        m_Controls->m_BinSizeFrame->setVisible(true);
}

void QmitkImageStatisticsView::PartClosed( berry::IWorkbenchPartReference::Pointer )
{
}

void QmitkImageStatisticsView::OnTimeChanged(const itk::EventObject& e)
{
  if (this->m_SelectedDataNodes.isEmpty() || this->m_SelectedImage == NULL)
    return;

  const mitk::SliceNavigationController::GeometryTimeEvent* timeEvent =
    dynamic_cast<const mitk::SliceNavigationController::GeometryTimeEvent*>(&e);
  assert(timeEvent != NULL);
  unsigned int timestep = timeEvent->GetPos();

  if (this->m_SelectedImage->GetTimeSteps() > 1)
  {
    for (unsigned int x = 0; x < this->m_Controls->m_StatisticsTable->columnCount(); x++)
    {
      for (unsigned int y = 0; y < this->m_Controls->m_StatisticsTable->rowCount(); y++)
      {
        QTableWidgetItem* item = this->m_Controls->m_StatisticsTable->item(y, x);
        if (item == NULL)
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
    // display histogram for selected timestep
    this->m_Controls->m_JSHistogram->ClearHistogram();
    QmitkImageStatisticsCalculationThread::HistogramType::Pointer histogram =
      this->m_CalculationThread->GetTimeStepHistogram(timestep);

    if (histogram.IsNotNull())
    {
      this->m_Controls->m_JSHistogram->ComputeHistogram(histogram.GetPointer());
//      this->m_Controls->m_JSHistogram->SignalGraphChanged();

      // hacky way to make sure the protected SignalGraphChanged() is called
      if (this->m_Controls->m_JSHistogram->GetUseLineGraph())
      {
        this->m_Controls->m_JSHistogram->OnBarRadioButtonSelected();
        this->m_Controls->m_JSHistogram->OnLineRadioButtonSelected();
      }
      else
      {
        this->m_Controls->m_JSHistogram->OnLineRadioButtonSelected();
        this->m_Controls->m_JSHistogram->OnBarRadioButtonSelected();
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
  if (row==4 && !m_WorldMinList.empty())
    world = m_WorldMinList[col];
  else if (row==3 && !m_WorldMaxList.empty())
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
  if ( m_CurrentStatisticsValid )
  {
    const unsigned int t = this->GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();

    typedef mitk::ImageStatisticsCalculator::HistogramType HistogramType;
    const HistogramType *histogram = this->m_CalculationThread->GetTimeStepHistogram(t).GetPointer();

    QString clipboard( "Measurement \t Frequency\n" );
    for ( HistogramType::ConstIterator it = histogram->Begin();
      it != histogram->End();
      ++it )
    {
      if( m_Controls->m_HistogramBinSizeSpinbox->value() == 1)
      {
        clipboard = clipboard.append( "%L1 \t %L2\n" )
          .arg( it.GetMeasurementVector()[0], 0, 'f', 0 )
          .arg( it.GetFrequency() );
      }
      else
      {
        clipboard = clipboard.append( "%L1 \t %L2\n" )
          .arg( it.GetMeasurementVector()[0], 0, 'f', 2 )
          .arg( it.GetFrequency() );
      }
    }

    QApplication::clipboard()->setText(
      clipboard, QClipboard::Clipboard );
  }
  else
  {
    QApplication::clipboard()->clear();
  }
}

void QmitkImageStatisticsView::OnClipboardStatisticsButtonClicked()
{
  QLocale tempLocal;
  QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
  if ( this->m_CurrentStatisticsValid )
  {
    const std::vector<mitk::ImageStatisticsCalculator::Statistics> &statistics =
        this->m_CalculationThread->GetStatisticsData();
    const unsigned int t = this->GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->
        GetPos();

    // Copy statistics to clipboard ("%Ln" will use the default locale for
    // number formatting)
    QString clipboard( "Mean \t StdDev \t RMS \t Max \t Min \t N \t V (mm³)\n" );
    clipboard = clipboard.append( "%L1 \t %L2 \t %L3 \t %L4 \t %L5 \t %L6 \t %L7" )
      .arg( statistics[t].GetMean(), 0, 'f', 10 )
      .arg( statistics[t].GetSigma(), 0, 'f', 10 )
      .arg( statistics[t].GetRMS(), 0, 'f', 10 )
      .arg( statistics[t].GetMax(), 0, 'f', 10 )
      .arg( statistics[t].GetMin(), 0, 'f', 10 )
      .arg( statistics[t].GetN() )
      .arg( m_Controls->m_StatisticsTable->item( 0, 6 )->text().toDouble(), 0, 'f', 10 );

    QApplication::clipboard()->setText(
      clipboard, QClipboard::Clipboard );
  }
  else
  {
    QApplication::clipboard()->clear();
  }
  QLocale::setDefault(tempLocal);
}

void QmitkImageStatisticsView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*part*/,
                                                  const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  if (this->m_Visible)
  {
    this->SelectionChanged( selectedNodes );
  }
  else
  {
    this->m_DataNodeSelectionChanged = true;
  }
}

void QmitkImageStatisticsView::SelectionChanged(const QList<mitk::DataNode::Pointer> &selectedNodes)
{
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
    m_Controls->m_JSHistogram->ClearHistogram();
    m_Controls->m_lineRadioButton->setEnabled(true);
    m_Controls->m_barRadioButton->setEnabled(true);
    m_Controls->m_HistogramBinSizeSpinbox->setEnabled(true);
    m_Controls->m_HistogramBinSizeCaptionLabel->setEnabled(true);
//    m_Controls->m_HistogramBinSizeLabel->setEnabled(true);
    m_Controls->m_InfoLabel->setText(QString(""));

    //   m_Controls->horizontalLayout_3->setEnabled(false);
    m_Controls->groupBox->setEnabled(false);
    m_Controls->groupBox_3->setEnabled(false);
  }
  else
  {
    //  m_Controls->horizontalLayout_3->setEnabled(true);
    m_Controls->groupBox->setEnabled(true);
    m_Controls->groupBox_3->setEnabled(true);
  }
  if(selectedNodes.size() == 1 || selectedNodes.size() == 2)
  {
    bool isBinary = false;
    selectedNodes.value(0)->GetBoolProperty("binary",isBinary);
    if(isBinary)
    {
      m_Controls->m_JSHistogram->ClearHistogram();
      m_Controls->m_lineRadioButton->setEnabled(true);
      m_Controls->m_barRadioButton->setEnabled(true);
      m_Controls->m_HistogramBinSizeSpinbox->setEnabled(true);
      m_Controls->m_HistogramBinSizeCaptionLabel->setEnabled(true);
//      m_Controls->m_HistogramBinSizeLabel->setEnabled(true);
      m_Controls->m_InfoLabel->setText(QString(""));
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

void QmitkImageStatisticsView::ReinitData()
{
  while( this->m_CalculationThread->isRunning()) // wait until thread has finished
  {
    itksys::SystemTools::Delay(100);
  }

  if(this->m_SelectedImage != NULL)
  {
    this->m_SelectedImage->RemoveObserver( this->m_ImageObserverTag);
    this->m_SelectedImage = NULL;
  }
  if(this->m_SelectedImageMask != NULL)
  {
    this->m_SelectedImageMask->RemoveObserver( this->m_ImageMaskObserverTag);
    this->m_SelectedImageMask = NULL;
  }
  if(this->m_SelectedPlanarFigure != NULL)
  {
    this->m_SelectedPlanarFigure->RemoveObserver( this->m_PlanarFigureObserverTag);
    this->m_SelectedPlanarFigure = NULL;
  }
  this->m_SelectedDataNodes.clear();
  this->m_StatisticsUpdatePending = false;

  m_Controls->m_ErrorMessageLabel->setText( "" );
  m_Controls->m_ErrorMessageLabel->hide();
  this->InvalidateStatisticsTableView();
  m_Controls->m_JSHistogram->ClearHistogram();
  m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
}

void QmitkImageStatisticsView::OnThreadedStatisticsCalculationEnds()
{
  std::stringstream message;
  message << "";
  m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
  m_Controls->m_ErrorMessageLabel->hide();
  this->WriteStatisticsToGUI();
}

void QmitkImageStatisticsView::UpdateStatistics()
{
  mitk::IRenderWindowPart* renderPart = this->GetRenderWindowPart();
  if ( renderPart == NULL )
  {
    this->m_StatisticsUpdatePending =  false;
    return;
  }
  m_WorldMinList.clear();
  m_WorldMaxList.clear();

  // classify selected nodes
  mitk::NodePredicateDataType::Pointer imagePredicate = mitk::NodePredicateDataType::New("Image");

  std::string maskName = std::string();
  std::string maskType = std::string();
  std::string featureImageName = std::string();
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

      if( this->m_SelectedImageMask == NULL && isMask)
      {
        this->m_SelectedImageMask = dynamic_cast<mitk::Image*>(this->m_SelectedDataNodes.at(i)->GetData());
        this->m_ImageMaskObserverTag = this->m_SelectedImageMask->AddObserver(itk::ModifiedEvent(), changeListener);

        maskName = this->m_SelectedDataNodes.at(i)->GetName();
        maskType = m_SelectedImageMask->GetNameOfClass();
        maskDimension = 3;
      }
      else if( !isMask )
      {
        if(this->m_SelectedImage == NULL)
        {
          this->m_SelectedImage = static_cast<mitk::Image*>(this->m_SelectedDataNodes.at(i)->GetData());
          this->m_ImageObserverTag = this->m_SelectedImage->AddObserver(itk::ModifiedEvent(), changeListener);
        }
        featureImageName = this->m_SelectedDataNodes.at(i)->GetName();
      }
    }
    else if (planarFig.IsNotNull())
    {
      if(this->m_SelectedPlanarFigure == NULL)
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
      std::stringstream message;
      message << "<font color='red'>" << "Invalid data node type!" << "</font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
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

  if (m_SelectedPlanarFigure != NULL && m_SelectedImage == NULL)
  {
    mitk::DataStorage::SetOfObjects::ConstPointer parentSet = this->GetDataStorage()->GetSources(planarFigureNode);
    for (int i=0; i<parentSet->Size(); i++)
    {
      mitk::DataNode::Pointer node = parentSet->ElementAt(i);
      if( imagePredicate->CheckNode(node) )
      {
        bool isMask = false;
        node->GetPropertyValue("binary", isMask);

        if( !isMask )
        {
          if(this->m_SelectedImage == NULL)
          {
            this->m_SelectedImage = static_cast<mitk::Image*>(node->GetData());
            this->m_ImageObserverTag = this->m_SelectedImage->AddObserver(itk::ModifiedEvent(), changeListener);
          }
        }
      }
    }
  }

  unsigned int timeStep = renderPart->GetTimeNavigationController()->GetTime()->GetPos();

  if ( m_SelectedImage != NULL && m_SelectedImage->IsInitialized())
  {
    // Check if a the selected image is a multi-channel image. If yes, statistics
    // cannot be calculated currently.
    if ( m_SelectedImage->GetPixelType().GetNumberOfComponents() > 1 )
    {
      std::stringstream message;
      message << "<font color='red'>Multi-component images not supported.</font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();

      this->InvalidateStatisticsTableView();
      m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
      m_Controls->m_JSHistogram->ClearHistogram();
      m_CurrentStatisticsValid = false;
      this->m_StatisticsUpdatePending = false;
      m_Controls->m_lineRadioButton->setEnabled(true);
      m_Controls->m_barRadioButton->setEnabled(true);
      m_Controls->m_HistogramBinSizeSpinbox->setEnabled(true);
      m_Controls->m_HistogramBinSizeCaptionLabel->setEnabled(true);
//      m_Controls->m_HistogramBinSizeLabel->setEnabled(true);
      m_Controls->m_InfoLabel->setText(QString(""));
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
    if (m_SelectedImageMask != NULL)
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

    //// initialize thread and trigger it
    this->m_CalculationThread->SetIgnoreZeroValueVoxel( m_Controls->m_IgnoreZerosCheckbox->isChecked() );
    this->m_CalculationThread->Initialize( m_SelectedImage, m_SelectedImageMask, m_SelectedPlanarFigure );
    this->m_CalculationThread->SetTimeStep( timeStep );
    this->m_CalculationThread->SetHistogramBinSize(m_Controls->m_HistogramBinSizeSpinbox->value());
    std::stringstream message;
    message << "<font color='red'>Calculating statistics...</font>";
    m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
    m_Controls->m_ErrorMessageLabel->show();

    try
    {
      // Compute statistics
      this->m_CalculationThread->SetUseDefaultBinSize(m_Controls->m_UseDefaultBinSizeBox->isChecked());
      this->m_CalculationThread->start();
    }
    catch ( const mitk::Exception& e)
    {
      std::stringstream message;
      message << "<font color='red'>" << e.GetDescription() << "</font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
      this->m_StatisticsUpdatePending = false;
    }
    catch ( const std::runtime_error &e )
    {
      // In case of exception, print error message on GUI
      std::stringstream message;
      message << "<font color='red'>" << e.what() << "</font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
      this->m_StatisticsUpdatePending = false;
    }
    catch ( const std::exception &e )
    {
      MITK_ERROR << "Caught exception: " << e.what();

      // In case of exception, print error message on GUI
      std::stringstream message;
      message << "<font color='red'>Error! Unequal Dimensions of Image and Segmentation. No recompute possible </font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
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
    m_SelectedImage = NULL;
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

void QmitkImageStatisticsView::OnHistogramBinSizeBoxValueChanged()
{
  this->UpdateStatistics();
}
void QmitkImageStatisticsView::WriteStatisticsToGUI()
{
  m_Controls->m_lineRadioButton->setEnabled(true);
  m_Controls->m_barRadioButton->setEnabled(true);
  m_Controls->m_HistogramBinSizeSpinbox->setEnabled(true);
  m_Controls->m_HistogramBinSizeCaptionLabel->setEnabled(true);
//  m_Controls->m_HistogramBinSizeLabel->setEnabled(true);
  m_Controls->m_InfoLabel->setText(QString(""));

  if(m_DataNodeSelectionChanged)
  {
    this->m_StatisticsUpdatePending = false;
    this->RequestStatisticsUpdate();
    return;    // stop visualization of results and calculate statistics of new selection
  }

  if ( this->m_CalculationThread->GetStatisticsUpdateSuccessFlag())
  {
    if ( this->m_CalculationThread->GetStatisticsChangedFlag() )
    {
      // Do not show any error messages
      m_Controls->m_ErrorMessageLabel->hide();
      m_CurrentStatisticsValid = true;
    }

    if (m_Controls->m_barRadioButton->isChecked())
    {
      m_Controls->m_JSHistogram->OnBarRadioButtonSelected();
    }
    m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
    m_Controls->m_HistogramBinSizeSpinbox->setValue( this->m_CalculationThread->GetHistogramBinSize() );
    //m_Controls->m_JSHistogram->ComputeHistogram( this->m_CalculationThread->GetTimeStepHistogram(this->m_CalculationThread->GetTimeStep()).GetPointer() );
    this->FillStatisticsTableView( this->m_CalculationThread->GetStatisticsData(), this->m_CalculationThread->GetStatisticsImage());
  }
  else
  {
    m_Controls->m_SelectedMaskLabel->setText( "None" );
    m_Controls->m_ErrorMessageLabel->setText( m_CalculationThread->GetLastErrorMessage().c_str() );
    m_Controls->m_ErrorMessageLabel->show();
    // Clear statistics and histogram
    this->InvalidateStatisticsTableView();
    m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
    //m_Controls->m_JSHistogram->clearHistogram();
    m_CurrentStatisticsValid = false;

    // If a (non-closed) PlanarFigure is selected, display a line profile widget
    if ( m_SelectedPlanarFigure != NULL )
    {
      // Check if the (closed) planar figure is out of bounds and so no image mask could be calculated--> Intensity Profile can not be calculated
      bool outOfBounds = false;
      if ( m_SelectedPlanarFigure->IsClosed() && m_SelectedImageMask == NULL)
      {
        outOfBounds = true;
        std::stringstream message;
        message << "<font color='red'>Planar figure is on a rotated image plane or outside the image bounds.</font>";
        m_Controls->m_InfoLabel->setText(message.str().c_str());
      }

      // check whether PlanarFigure is initialized
      const mitk::PlaneGeometry *planarFigurePlaneGeometry = m_SelectedPlanarFigure->GetPlaneGeometry();
      if ( planarFigurePlaneGeometry == NULL || outOfBounds)
      {
        // Clear statistics, histogram, and GUI
        this->InvalidateStatisticsTableView();
        m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
        m_Controls->m_JSHistogram->ClearHistogram();
        m_CurrentStatisticsValid = false;
        m_Controls->m_ErrorMessageLabel->hide();
        m_Controls->m_SelectedMaskLabel->setText( "None" );
        this->m_StatisticsUpdatePending = false;
        m_Controls->m_lineRadioButton->setEnabled(true);
        m_Controls->m_barRadioButton->setEnabled(true);
        m_Controls->m_HistogramBinSizeSpinbox->setEnabled(true);
        m_Controls->m_HistogramBinSizeCaptionLabel->setEnabled(true);
//        m_Controls->m_HistogramBinSizeLabel->setEnabled(true);
        if (!outOfBounds)
          m_Controls->m_InfoLabel->setText(QString(""));
        return;
      }
      unsigned int timeStep = this->GetRenderWindowPart()->GetTimeNavigationController()->GetTime()->GetPos();
      m_Controls->m_JSHistogram->SetImage(this->m_CalculationThread->GetStatisticsImage());
      m_Controls->m_JSHistogram->SetPlanarFigure(m_SelectedPlanarFigure);
      m_Controls->m_JSHistogram->ComputeIntensityProfile(timeStep);
      m_Controls->m_lineRadioButton->setEnabled(false);
      m_Controls->m_barRadioButton->setEnabled(false);
      m_Controls->m_HistogramBinSizeSpinbox->setEnabled(false);
      m_Controls->m_HistogramBinSizeCaptionLabel->setEnabled(false);
//      m_Controls->m_HistogramBinSizeLabel->setEnabled(false);
      std::stringstream message;
      message << "<font color='red'>Only linegraph available for an intesityprofile!</font>";
      m_Controls->m_InfoLabel->setText(message.str().c_str());
    }
  }
  this->m_StatisticsUpdatePending = false;
}

void QmitkImageStatisticsView::FillStatisticsTableView(
  const std::vector<mitk::ImageStatisticsCalculator::Statistics> &s,
  const mitk::Image *image )
{
  this->m_Controls->m_StatisticsTable->setColumnCount(image->GetTimeSteps());
  this->m_Controls->m_StatisticsTable->horizontalHeader()->setVisible(image->GetTimeSteps() > 1);

  int decimals = 2;

  mitk::PixelType doublePix = mitk::MakeScalarPixelType< double >();
  mitk::PixelType floatPix = mitk::MakeScalarPixelType< float >();
  if (image->GetPixelType()==doublePix || image->GetPixelType()==floatPix)
  {
    decimals = 5;
  }

  for (unsigned int t = 0; t < image->GetTimeSteps(); t++)
  {
    this->m_Controls->m_StatisticsTable->setHorizontalHeaderItem(t,
      new QTableWidgetItem(QString::number(t)));

    if (s[t].GetMaxIndex().size()==3)
    {
      mitk::Point3D index, max, min;
      index[0] = s[t].GetMaxIndex()[0];
      index[1] = s[t].GetMaxIndex()[1];
      index[2] = s[t].GetMaxIndex()[2];
      m_SelectedImage->GetGeometry()->IndexToWorld(index, max);
      this->m_WorldMaxList.push_back(max);
      index[0] = s[t].GetMinIndex()[0];
      index[1] = s[t].GetMinIndex()[1];
      index[2] = s[t].GetMinIndex()[2];
      m_SelectedImage->GetGeometry()->IndexToWorld(index, min);
      this->m_WorldMinList.push_back(min);
    }


    this->m_Controls->m_StatisticsTable->setItem( 0, t, new QTableWidgetItem(
      QString("%1").arg(s[t].GetMean(), 0, 'f', decimals) ) );
    this->m_Controls->m_StatisticsTable->setItem( 1, t, new QTableWidgetItem(
      QString("%1").arg(s[t].GetSigma(), 0, 'f', decimals) ) );

    this->m_Controls->m_StatisticsTable->setItem( 2, t, new QTableWidgetItem(
      QString("%1").arg(s[t].GetRMS(), 0, 'f', decimals) ) );

    QString max; max.append(QString("%1").arg(s[t].GetMax(), 0, 'f', decimals));
    max += " (";
    for (int i=0; i<s[t].GetMaxIndex().size(); i++)
    {
      max += QString::number(s[t].GetMaxIndex()[i]);
      if (i<s[t].GetMaxIndex().size()-1)
        max += ",";
    }
    max += ")";
    this->m_Controls->m_StatisticsTable->setItem( 3, t, new QTableWidgetItem( max ) );

    QString min; min.append(QString("%1").arg(s[t].GetMin(), 0, 'f', decimals));
    min += " (";
    for (int i=0; i<s[t].GetMinIndex().size(); i++)
    {
      min += QString::number(s[t].GetMinIndex()[i]);
      if (i<s[t].GetMinIndex().size()-1)
        min += ",";
    }
    min += ")";
    this->m_Controls->m_StatisticsTable->setItem( 4, t, new QTableWidgetItem( min ) );

    this->m_Controls->m_StatisticsTable->setItem( 5, t, new QTableWidgetItem(
      QString("%1").arg(s[t].GetN()) ) );

    const mitk::BaseGeometry *geometry = image->GetGeometry();
    if ( geometry != NULL )
    {
      const mitk::Vector3D &spacing = image->GetGeometry()->GetSpacing();
      double volume = spacing[0] * spacing[1] * spacing[2] * (double) s[t].GetN();
      this->m_Controls->m_StatisticsTable->setItem( 6, t, new QTableWidgetItem(
        QString("%1").arg(volume, 0, 'f', decimals) ) );
    }
    else
    {
      this->m_Controls->m_StatisticsTable->setItem( 6, t, new QTableWidgetItem(
        "NA" ) );
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

  QString hotspotMean; hotspotMean.append(QString("%1").arg(s[t].GetHotspotStatistics().GetMean(), 0, 'f', decimals));
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

  this->m_Controls->m_StatisticsTable->setItem( 9, t, new QTableWidgetItem( hotspotMin ) );
}

void QmitkImageStatisticsView::InvalidateStatisticsTableView()
{
  this->m_Controls->m_StatisticsTable->horizontalHeader()->setVisible(false);
  this->m_Controls->m_StatisticsTable->setColumnCount(1);

  for ( unsigned int i = 0; i < this->m_Controls->m_StatisticsTable->rowCount(); ++i )
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
      AddObserver(mitk::SliceNavigationController::GeometryTimeEvent(NULL, 0), cmdTimeEvent);
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
  if ( m_TimeObserverTag != NULL )
  {
    mitk::IRenderWindowPart* renderWindow = GetRenderWindowPart();

    if (renderWindow)
    {
      renderWindow->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->
        RemoveObserver( m_TimeObserverTag );
    }
    m_TimeObserverTag = NULL;
  }

}

void QmitkImageStatisticsView::SetFocus()
{
}
