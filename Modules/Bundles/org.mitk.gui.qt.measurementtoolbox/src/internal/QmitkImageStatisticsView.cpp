/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-22 11:00:35 +0200 (Fr, 22 Mai 2009) $
Version:   $Revision: 10185 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkImageStatisticsView.h"

#include <limits>

#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qclipboard.h>
#include <qfiledialog.h>

#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryPlatform.h>


#include "QmitkStdMultiWidget.h"
#include "QmitkSliderNavigatorWidget.h"

#include "mitkNodePredicateDataType.h"
#include "mitkImageTimeSelector.h"
#include "mitkProperties.h"

#include "mitkProgressBar.h"

// Includes for image processing
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"

#include "mitkDataNodeObject.h"
#include "mitkNodePredicateData.h"
#include "mitkPlanarFigureInteractor.h"

#include <itkVectorImage.h>

const std::string QmitkImageStatisticsView::VIEW_ID =
"org.mitk.views.imagestatistics";

class QmitkRequestStatisticsUpdateEvent : public QEvent
{
public:
  enum Type
  {
    StatisticsUpdateRequest = QEvent::MaxUser - 1025
  };

  QmitkRequestStatisticsUpdateEvent()
    : QEvent( (QEvent::Type) StatisticsUpdateRequest ) {};
};



typedef itk::Image<short, 3>                 ImageType;
typedef itk::Image<float, 3>                 FloatImageType;
typedef itk::Image<itk::Vector<float,3>, 3>  VectorImageType;

inline bool my_isnan(float x)
 {
   volatile float d = x;

   if(d!=d)
     return true;

   if(d==d)
     return false;
   return d != d;

 }

QmitkImageStatisticsView::QmitkImageStatisticsView(QObject* /*parent*/, const char* /*name*/)
: QmitkFunctionality(),
  m_Controls( NULL ),
  m_TimeStepperAdapter( NULL ),
  m_SelectedImageNode( NULL ),
  m_SelectedImage( NULL ),
  m_SelectedMaskNode( NULL ),
  m_SelectedImageMask( NULL ),
  m_SelectedPlanarFigure( NULL ),
  m_ImageObserverTag( -1 ),
  m_ImageMaskObserverTag( -1 ),
  m_PlanarFigureObserverTag( -1 ),
  m_CurrentStatisticsValid( false ),
  m_StatisticsUpdatePending( false )
{
}


QmitkImageStatisticsView::~QmitkImageStatisticsView()
{
  if ( m_SelectedImage != NULL )
    m_SelectedImage->RemoveObserver( m_ImageObserverTag );
  if ( m_SelectedImageMask != NULL )
    m_SelectedImageMask->RemoveObserver( m_ImageMaskObserverTag );
  if ( m_SelectedPlanarFigure != NULL )
    m_SelectedPlanarFigure->RemoveObserver( m_PlanarFigureObserverTag );
}


void QmitkImageStatisticsView::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new Ui::QmitkImageStatisticsViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_ErrorMessageLabel->hide();

    m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
    m_Controls->m_LineProfileWidget->SetPathModeToPlanarFigure();
  }
}



void QmitkImageStatisticsView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ButtonCopyHistogramToClipboard), SIGNAL(clicked()),(QObject*) this, SLOT(ClipboardHistogramButtonClicked()));
    connect( (QObject*)(m_Controls->m_ButtonCopyStatisticsToClipboard), SIGNAL(clicked()),(QObject*) this, SLOT(ClipboardStatisticsButtonClicked()));
    connect( (QObject*)(m_Controls->m_IgnoreZerosCheckbox), SIGNAL(clicked()),(QObject*) this, SLOT(IgnoreZerosCheckboxClicked()));
  }
}

void QmitkImageStatisticsView::IgnoreZerosCheckboxClicked(  )
{
  UpdateStatistics();
}

void QmitkImageStatisticsView::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  QmitkFunctionality::StdMultiWidgetAvailable(stdMultiWidget);
}


void QmitkImageStatisticsView::ClipboardHistogramButtonClicked()
{
  if ( m_CurrentStatisticsValid && (m_CurrentStatisticsCalculator.IsNotNull()) )
  {
    typedef mitk::ImageStatisticsCalculator::HistogramType HistogramType;
    const HistogramType *histogram = m_CurrentStatisticsCalculator->GetHistogram();

    QString clipboard( "Measurement \t Frequency\n" );
    for ( HistogramType::ConstIterator it = histogram->Begin();
          it != histogram->End();
          ++it )
    {
      clipboard = clipboard.append( "%L1 \t %L2\n" )
        .arg( it.GetMeasurementVector()[0], 0, 'f', 2 )
        .arg( it.GetFrequency() );
    }

    QApplication::clipboard()->setText(
      clipboard, QClipboard::Clipboard );
  }
  else
  {
    QApplication::clipboard()->clear();
  }
}


void QmitkImageStatisticsView::ClipboardStatisticsButtonClicked()
{
  if ( m_CurrentStatisticsValid && (m_CurrentStatisticsCalculator.IsNotNull()) )
  {
    const mitk::ImageStatisticsCalculator::Statistics &statistics =
      m_CurrentStatisticsCalculator->GetStatistics();

    // Copy statistics to clipboard ("%Ln" will use the default locale for
    // number formatting)
    QString clipboard( "Mean \t StdDev \t RMS \t Max \t Min \t N \t V (mm³)\n" );
    clipboard = clipboard.append( "%L1 \t %L2 \t %L3 \t %L4 \t %L5 \t %L6 \t %L7" )
      .arg( statistics.Mean, 0, 'f', 10 )
      .arg( statistics.Sigma, 0, 'f', 10 )
      .arg( statistics.RMS, 0, 'f', 10 )
      .arg( statistics.Max, 0, 'f', 10 )
      .arg( statistics.Min, 0, 'f', 10 )
      .arg( statistics.N )
      .arg( m_Controls->m_StatisticsTable->item( 0, 6 )->text() );

    QApplication::clipboard()->setText(
      clipboard, QClipboard::Clipboard );
  }
  else
  {
    QApplication::clipboard()->clear();
  }
}


void QmitkImageStatisticsView::FillStatisticsTableView(
  const mitk::ImageStatisticsCalculator::Statistics &s,
  const mitk::Image *image )
{
  m_Controls->m_StatisticsTable->setItem( 0, 0, new QTableWidgetItem(
    QString("%1").arg(s.Mean, 0, 'f', 2) ) );

  m_Controls->m_StatisticsTable->setItem( 0, 1, new QTableWidgetItem(
    QString("%1").arg(s.Sigma, 0, 'f', 2) ) );

  m_Controls->m_StatisticsTable->setItem( 0, 2, new QTableWidgetItem(
    QString("%1").arg(s.RMS, 0, 'f', 2) ) );

  m_Controls->m_StatisticsTable->setItem( 0, 3, new QTableWidgetItem(
    QString("%1").arg(s.Max, 0, 'f', 2) ) );

  m_Controls->m_StatisticsTable->setItem( 0, 4, new QTableWidgetItem(
    QString("%1").arg(s.Min, 0, 'f', 2) ) );

  m_Controls->m_StatisticsTable->setItem( 0, 5, new QTableWidgetItem(
    QString("%1").arg(s.N) ) );

  const mitk::Geometry3D *geometry = image->GetGeometry();
  if ( geometry != NULL )
  {
    const mitk::Vector3D &spacing = image->GetGeometry()->GetSpacing();
    double volume = spacing[0] * spacing[1] * spacing[2] * (double) s.N;
    m_Controls->m_StatisticsTable->setItem( 0, 6, new QTableWidgetItem(
      QString("%1").arg(volume, 0, 'f', 2) ) );
  }
  else
  {
    m_Controls->m_StatisticsTable->setItem( 0, 6, new QTableWidgetItem(
      "NA" ) );
  }
}


void QmitkImageStatisticsView::InvalidateStatisticsTableView()
{
  for ( unsigned int i = 0; i < 7; ++i )
  {
    m_Controls->m_StatisticsTable->setItem( 0, i, new QTableWidgetItem( "NA" ) );
  }
}


void QmitkImageStatisticsView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  // Clear any unreferenced images
  this->RemoveOrphanImages();

  if ( !this->IsVisible() )
  {
    return;
  }

  // Check if selection makeup consists only of valid nodes:
  // One image, segmentation or planarFigure
  // One image and one of the other two
  bool tooManyNodes( true );
  bool invalidNodes( true );

  if ( nodes.size() < 3 )
  {
    tooManyNodes = false;
  }

  if( !tooManyNodes )
  {
    unsigned int numberImages = 0;
    unsigned int numberSegmentations = 0;
    unsigned int numberPlanarFigures = 0;

    for ( unsigned int index = 0; index < nodes.size(); index++ )
    {
      m_SelectedImageMask = dynamic_cast< mitk::Image * >( nodes[ index ]->GetData() );
      m_SelectedPlanarFigure = dynamic_cast< mitk::PlanarFigure * >( nodes[ index ]->GetData() );

      if ( m_SelectedImageMask != NULL )
      {
        bool isMask( false );
        nodes[ index ]->GetPropertyValue("binary", isMask);
        if ( !isMask )
        {
          numberImages++;
        }
        else
        {
          numberSegmentations++;
          if ( numberImages != 0 ) // image should be last element
          {
            std::swap( nodes[ index ], nodes[ index - 1 ] );
          }
        }
      }
      else if ( m_SelectedPlanarFigure != NULL )
      {
        numberPlanarFigures++;
        if ( numberImages != 0 ) // image should be last element
        {
          std::swap( nodes[ index ], nodes[ index - 1 ] );
        }
      }
    }

    if ( ( numberPlanarFigures + numberSegmentations + numberImages ) == nodes.size() && //No invalid nodes
      ( numberPlanarFigures + numberSegmentations ) < 2 && numberImages < 2
      // maximum of one image and/or one of either planar figure or segmentation
      )
    {
      invalidNodes = false;
    }
  }

  if ( nodes.empty() || tooManyNodes || invalidNodes )
  {
    // Nothing to do: invalidate image, clear statistics, histogram, and GUI
    m_SelectedImage = NULL;
    this->InvalidateStatisticsTableView() ;
    m_Controls->m_HistogramWidget->ClearItemModel();
    m_Controls->m_LineProfileWidget->ClearItemModel();

    m_CurrentStatisticsValid = false;
    m_Controls->m_ErrorMessageLabel->hide();
    m_Controls->m_SelectedMaskLabel->setText( "None" );
    return;
  }

  // Get selected element

  mitk::DataNode *selectedNode = nodes.front();
  mitk::Image *selectedImage = dynamic_cast< mitk::Image * >( selectedNode->GetData() );

  // Find the next parent/grand-parent node containing an image, if any
  mitk::DataStorage::SetOfObjects::ConstPointer parentObjects;
  mitk::DataNode *parentNode = NULL;
  mitk::Image *parentImage = NULL;

  // Possibly previous change listeners
  if ( (m_SelectedPlanarFigure != NULL) && (m_PlanarFigureObserverTag >= 0) )
  {
    m_SelectedPlanarFigure->RemoveObserver( m_PlanarFigureObserverTag );
    m_PlanarFigureObserverTag = -1;
  }
  if ( (m_SelectedImage != NULL) && (m_ImageObserverTag >= 0) )
  {
    m_SelectedImage->RemoveObserver( m_ImageObserverTag );
    m_ImageObserverTag = -1;
  }
  if ( (m_SelectedImageMask != NULL) && (m_ImageMaskObserverTag >= 0) )
  {
    m_SelectedImageMask->RemoveObserver( m_ImageMaskObserverTag );
    m_ImageMaskObserverTag = -1;
  }

  // Deselect all images and masks by default
  m_SelectedImageNode = NULL;
  m_SelectedImage = NULL;
  m_SelectedMaskNode = NULL;
  m_SelectedImageMask = NULL;
  m_SelectedPlanarFigure = NULL;

  {
    unsigned int parentObjectIndex = 0;
    parentObjects = this->GetDefaultDataStorage()->GetSources( selectedNode );
    while( parentObjectIndex < parentObjects->Size() )
    {
      // Use first parent object (if multiple parents are present)
      parentNode = parentObjects->ElementAt( parentObjectIndex );
      parentImage = dynamic_cast< mitk::Image * >( parentNode->GetData() );
      if( parentImage != NULL )
      {
        break;
      }
      parentObjectIndex++;
    }
  }

  if ( nodes.size() == 2 )
  {
    parentNode = nodes.back();
    parentImage = dynamic_cast< mitk::Image * >( parentNode->GetData() );
  }

  if ( parentImage != NULL )
  {
    m_SelectedImageNode = parentNode;
    m_SelectedImage = parentImage;

    // Check if a valid mask has been selected (Image or PlanarFigure)
    m_SelectedImageMask = dynamic_cast< mitk::Image * >( selectedNode->GetData() );
    m_SelectedPlanarFigure = dynamic_cast< mitk::PlanarFigure * >( selectedNode->GetData() );

    // Check whether ImageMask is a binary segmentation

    if ( (m_SelectedImageMask != NULL) )
    {
      bool isMask( false );
      selectedNode->GetPropertyValue("binary", isMask);
      if ( !isMask )
      {
        m_SelectedImageNode = selectedNode;
        m_SelectedImage = selectedImage;
        m_SelectedImageMask = NULL;
      }
      else
      {
        m_SelectedMaskNode = selectedNode;
      }
    }
    else if ( (m_SelectedPlanarFigure != NULL) )
    {
      m_SelectedMaskNode = selectedNode;
    }
  }
  else if ( selectedImage != NULL )
  {
    m_SelectedImageNode = selectedNode;
    m_SelectedImage = selectedImage;
  }


  typedef itk::SimpleMemberCommand< QmitkImageStatisticsView > ITKCommandType;
  ITKCommandType::Pointer changeListener;
  changeListener = ITKCommandType::New();
  changeListener->SetCallbackFunction( this, &QmitkImageStatisticsView::RequestStatisticsUpdate );

  // Add change listeners to selected objects
  if ( m_SelectedImage != NULL )
  {
    m_ImageObserverTag = m_SelectedImage->AddObserver(
      itk::ModifiedEvent(), changeListener );
  }

  if ( m_SelectedImageMask != NULL )
  {
    m_ImageMaskObserverTag = m_SelectedImageMask->AddObserver(
      itk::ModifiedEvent(), changeListener );
  }

  if ( m_SelectedPlanarFigure != NULL )
  {
    m_PlanarFigureObserverTag = m_SelectedPlanarFigure->AddObserver(
      mitk::EndInteractionPlanarFigureEvent(), changeListener );
  }


  // Clear statistics / histogram GUI if nothing is selected
  if ( m_SelectedImage == NULL )
  {
    // Clear statistics, histogram, and GUI
    this->InvalidateStatisticsTableView();
    m_Controls->m_HistogramWidget->ClearItemModel();
    m_Controls->m_LineProfileWidget->ClearItemModel();
    m_CurrentStatisticsValid = false;
    m_Controls->m_ErrorMessageLabel->hide();
    m_Controls->m_SelectedMaskLabel->setText( "None" );
  }
  else
  {
    // Else, request statistics and GUI update
    this->RequestStatisticsUpdate();
  }
}


void QmitkImageStatisticsView::UpdateStatistics()
{

  // Remove any cached images that are no longer referenced elsewhere
  this->RemoveOrphanImages();

  QmitkStdMultiWidget *multiWidget = this->GetActiveStdMultiWidget();
  if ( multiWidget == NULL )
  {
    return;
  }

  unsigned int timeStep = multiWidget->GetTimeNavigationController()->GetTime()->GetPos();

  if ( m_SelectedImage != NULL )
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
      m_Controls->m_HistogramWidget->ClearItemModel();
      m_CurrentStatisticsValid = false;
      return;
    }

    // Retrieve ImageStatisticsCalculator from has map (or create a new one
    // for this image if non-existant)
    ImageStatisticsMapType::iterator it =
      m_ImageStatisticsMap.find( m_SelectedImage );

    if ( it != m_ImageStatisticsMap.end() )
    {
      m_CurrentStatisticsCalculator = it->second;
      MITK_INFO << "Retrieving StatisticsCalculator";
    }
    else
    {
      m_CurrentStatisticsCalculator = mitk::ImageStatisticsCalculator::New();
      m_CurrentStatisticsCalculator->SetImage( m_SelectedImage );
      m_ImageStatisticsMap[m_SelectedImage] = m_CurrentStatisticsCalculator;
      MITK_INFO << "Creating StatisticsCalculator";
    }

    std::string maskName;
    std::string maskType;
    unsigned int maskDimension;

    if ( m_SelectedImageMask != NULL )
    {
      m_CurrentStatisticsCalculator->SetImageMask( m_SelectedImageMask );
      m_CurrentStatisticsCalculator->SetMaskingModeToImage();

      maskName = m_SelectedMaskNode->GetName();
      maskType = m_SelectedImageMask->GetNameOfClass();
      maskDimension = 3;
    }
    else if ( m_SelectedPlanarFigure != NULL )
    {
      m_CurrentStatisticsCalculator->SetPlanarFigure( m_SelectedPlanarFigure );
      m_CurrentStatisticsCalculator->SetMaskingModeToPlanarFigure();

      maskName = m_SelectedMaskNode->GetName();
      maskType = m_SelectedPlanarFigure->GetNameOfClass();
      maskDimension = 2;
    }
    else
    {
      m_CurrentStatisticsCalculator->SetMaskingModeToNone();

      maskName = "None";
      maskType = "";
      maskDimension = 0;
    }

    if(m_Controls->m_IgnoreZerosCheckbox->isChecked())
    {
      m_CurrentStatisticsCalculator->SetIgnorePixelValue(0);
      m_CurrentStatisticsCalculator->SetDoIgnorePixelValue(true);
    }
    else
    {
      m_CurrentStatisticsCalculator->SetDoIgnorePixelValue(false);
    }

    std::stringstream maskLabel;
    maskLabel << maskName;
    if ( maskDimension > 0 )
    {
      maskLabel << "  [" << maskDimension << "D " << maskType << "]";
    }

    m_Controls->m_SelectedMaskLabel->setText( maskLabel.str().c_str() );

    bool statisticsChanged = false;
    bool statisticsCalculationSuccessful = false;

    // Initialize progress bar
    mitk::ProgressBar::GetInstance()->AddStepsToDo( 100 );

    // Install listener for progress events and initialize progress bar
    typedef itk::SimpleMemberCommand< QmitkImageStatisticsView > ITKCommandType;
    ITKCommandType::Pointer progressListener;
    progressListener = ITKCommandType::New();
    progressListener->SetCallbackFunction( this, &QmitkImageStatisticsView::UpdateProgressBar );
    unsigned long progressObserverTag = m_CurrentStatisticsCalculator
      ->AddObserver( itk::ProgressEvent(), progressListener );

    // show wait cursor
    this->WaitCursorOn();

    try
    {
      // Compute statistics
      statisticsChanged =
        m_CurrentStatisticsCalculator->ComputeStatistics( timeStep );

      statisticsCalculationSuccessful = true;
    }
    catch ( const std::runtime_error &e )
    {
      // In case of exception, print error message on GUI
      std::stringstream message;
      message << "<font color='red'>" << e.what() << "</font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
    }
    catch ( const std::exception &e )
    {
      MITK_ERROR << "Caught exception: " << e.what();

      // In case of exception, print error message on GUI
      std::stringstream message;
      message << "<font color='red'>Error! Unequal Dimensions of Image and Segmentation. No recompute possible </font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
    }

    m_CurrentStatisticsCalculator->RemoveObserver( progressObserverTag );

    // Make sure that progress bar closes
    mitk::ProgressBar::GetInstance()->Progress( 100 );

    // remove wait cursor
    this->WaitCursorOff();

    if ( statisticsCalculationSuccessful )
    {
      if ( statisticsChanged )
      {
        // Do not show any error messages
        m_Controls->m_ErrorMessageLabel->hide();

        m_CurrentStatisticsValid = true;
      }

      m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
      m_Controls->m_HistogramWidget->SetHistogramModeToDirectHistogram();
      m_Controls->m_HistogramWidget->SetHistogram(
        m_CurrentStatisticsCalculator->GetHistogram( timeStep ) );
      m_Controls->m_HistogramWidget->UpdateItemModelFromHistogram();

      MITK_INFO << "UpdateItemModelFromHistogram()";

      this->FillStatisticsTableView(
        m_CurrentStatisticsCalculator->GetStatistics( timeStep ),
        m_SelectedImage );
    }
    else
    {
      m_Controls->m_SelectedMaskLabel->setText( "None" );

      // Clear statistics and histogram
      this->InvalidateStatisticsTableView();
      m_Controls->m_HistogramWidget->ClearItemModel();
      m_CurrentStatisticsValid = false;


      // If a (non-closed) PlanarFigure is selected, display a line profile widget
      if ( m_SelectedPlanarFigure != NULL )
      {
        // check whether PlanarFigure is initialized
        const mitk::Geometry2D *planarFigureGeometry2D = m_SelectedPlanarFigure->GetGeometry2D();
        if ( planarFigureGeometry2D == NULL )
        {
          // Clear statistics, histogram, and GUI
          this->InvalidateStatisticsTableView();
          m_Controls->m_HistogramWidget->ClearItemModel();
          m_Controls->m_LineProfileWidget->ClearItemModel();
          m_CurrentStatisticsValid = false;
          m_Controls->m_ErrorMessageLabel->hide();
          m_Controls->m_SelectedMaskLabel->setText( "None" );
          return;
        }
        // TODO: enable line profile widget
        m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 1 );
        m_Controls->m_LineProfileWidget->SetImage( m_SelectedImage );
        m_Controls->m_LineProfileWidget->SetPlanarFigure( m_SelectedPlanarFigure );
        m_Controls->m_LineProfileWidget->UpdateItemModelFromPath();
      }
    }
  }
}

void QmitkImageStatisticsView::UpdateProgressBar()
{
  mitk::ProgressBar::GetInstance()->Progress();
}


void QmitkImageStatisticsView::RequestStatisticsUpdate()
{
  if ( !m_StatisticsUpdatePending )
  {
    QApplication::postEvent( this, new QmitkRequestStatisticsUpdateEvent );
    m_StatisticsUpdatePending = true;
  }
}


void QmitkImageStatisticsView::RemoveOrphanImages()
{
  ImageStatisticsMapType::iterator it = m_ImageStatisticsMap.begin();

  while ( it != m_ImageStatisticsMap.end() )
  {
    mitk::Image *image = it->first;
    mitk::ImageStatisticsCalculator *calculator = it->second;
    ++it;

    mitk::NodePredicateData::Pointer hasImage = mitk::NodePredicateData::New( image );
    if ( this->GetDefaultDataStorage()->GetNode( hasImage ) == NULL )
    {
      if ( m_SelectedImage == image )
      {
        m_SelectedImage = NULL;
        m_SelectedImageNode = NULL;
      }
      if ( m_CurrentStatisticsCalculator == calculator )
      {
        m_CurrentStatisticsCalculator = NULL;
      }
      m_ImageStatisticsMap.erase( image );
      it = m_ImageStatisticsMap.begin();
    }
  }
}


bool QmitkImageStatisticsView::event( QEvent *event )
{
  if ( event->type() == (QEvent::Type) QmitkRequestStatisticsUpdateEvent::StatisticsUpdateRequest )
  {
    // Update statistics

    m_StatisticsUpdatePending = false;

    this->UpdateStatistics();
    return true;
  }

  return false;
}

void QmitkImageStatisticsView::ComputeIntensityProfile( mitk::PlanarLine* line )
{
  double sampling = 300;
  QmitkVtkHistogramWidget::HistogramType::Pointer histogram = QmitkVtkHistogramWidget::HistogramType::New();
  itk::Size<1> siz;
  siz[0] = sampling;
  itk::FixedArray<double,1> lower, higher;
  lower.Fill(0);
    mitk::Point3D begin = line->GetWorldControlPoint(0);
  mitk::Point3D end = line->GetWorldControlPoint(1);
  itk::Vector<double,3> direction = (end - begin);
  higher.Fill(direction.GetNorm());
  histogram->Initialize(siz, lower, higher);
  for(int i = 0; i < sampling; i++)
  {
    mitk::Point3D location = begin + double(i)/sampling * direction;
    double d = m_SelectedImage->GetPixelValueByWorldCoordinate(begin + double(i)/sampling * direction);
    histogram->SetFrequency(i,d);
  }
  m_Controls->m_HistogramWidget->SetHistogramModeToDirectHistogram();
  m_Controls->m_HistogramWidget->SetHistogram( histogram );
  m_Controls->m_HistogramWidget->UpdateItemModelFromHistogram();

}

bool QmitkImageStatisticsView::IsExclusiveFunctionality() const
{
  return false;
}

void QmitkImageStatisticsView::Visible()
{
  this->OnSelectionChanged( this->GetDataManagerSelection() );
}
