/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkDataTreeNodeObject.h"
#include "mitkNodePredicateData.h"

#include <itkVectorImage.h>

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

QmitkImageStatistics::QmitkImageStatistics(QObject *parent, const char *name)
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


QmitkImageStatistics::~QmitkImageStatistics()
{
}


void QmitkImageStatistics::CreateQtPartControl(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new Ui::QmitkImageStatisticsViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_ImageSelector1->SetDataStorage(this->GetDefaultDataStorage());
    m_Controls->m_ImageSelector1->SetPredicate(mitk::NodePredicateDataType::New("Image"));
    m_Controls->m_ImageSelector2->SetDataStorage(this->GetDefaultDataStorage());
    m_Controls->m_ImageSelector2->SetPredicate(mitk::NodePredicateDataType::New("Image"));
    m_Controls->sliceNavigatorTime->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    m_Controls->m_IgnoreInfCheckbox->setChecked(false);

    m_Controls->sliceNavigatorTime->setEnabled(false);
    m_Controls->tlTime->setEnabled(false);

    m_Controls->gbOneImageOps->hide();
    m_Controls->gbTwoImageOps->hide();
    m_Controls->gbOptions->hide();

    m_Controls->m_AllTreeButton->hide();

    m_Controls->m_ErrorMessageLabel->hide();

    m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 0 );
    m_Controls->m_LineProfileWidget->SetPathModeToPlanarFigure();
  }
}



void QmitkImageStatistics::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ButtonCopyHistogramToClipboard), SIGNAL(clicked()),(QObject*) this, SLOT(ClipboardHistogramButtonClicked()));
    connect( (QObject*)(m_Controls->m_ButtonCopyStatisticsToClipboard), SIGNAL(clicked()),(QObject*) this, SLOT(ClipboardStatisticsButtonClicked()));
  }
}

void QmitkImageStatistics::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  QmitkFunctionality::StdMultiWidgetAvailable(stdMultiWidget);
  m_TimeStepperAdapter = new QmitkStepperAdapter((QObject*) m_Controls->sliceNavigatorTime, stdMultiWidget.GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromShapeBasedSegmentation");
  connect( m_TimeStepperAdapter, SIGNAL( Refetch() ), this, SLOT( UpdateTimestep() ) );
  this->UpdateTimestep();
}

void QmitkImageStatistics::UpdateTimestep()
{
  // Request update of statistics / histogram
  this->RequestStatisticsUpdate();
}

void QmitkImageStatistics::ClipboardHistogramButtonClicked()
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


void QmitkImageStatistics::ClipboardStatisticsButtonClicked()
{
  if ( m_CurrentStatisticsValid && (m_CurrentStatisticsCalculator.IsNotNull()) )
  {
    const mitk::ImageStatisticsCalculator::Statistics &statistics =
      m_CurrentStatisticsCalculator->GetStatistics();

    // Copy statistics to clipboard ("%Ln" will use the default locale for
    // number formatting)
    QString clipboard( "Mean \t StdDev \t RMS \t Max \t Min \t N\n" );
    clipboard = clipboard.append( "%L1 \t %L2 \t %L3 \t %L4 \t %L5 \t %L6" )
      .arg( statistics.Mean, 0, 'f', 10 )
      .arg( statistics.Sigma, 0, 'f', 10 )
      .arg( statistics.RMS, 0, 'f', 10 )
      .arg( statistics.Max, 0, 'f', 10 )
      .arg( statistics.Min, 0, 'f', 10 )
      .arg( statistics.N );

    QApplication::clipboard()->setText( 
      clipboard, QClipboard::Clipboard );
  }
  else
  {
    QApplication::clipboard()->clear();
  }
}


void QmitkImageStatistics::ComputeStatistics(bool appendSourceData, bool toClipboard)
{
  // TODO
  return;

  if(m_Controls->m_ImageSelector1->GetSelectedNode() && m_Controls->m_ImageSelector2->GetSelectedNode())
  {
    mitk::Image::Pointer newImage1 = dynamic_cast<mitk::Image*>(
      m_Controls->m_ImageSelector1->GetSelectedNode()->GetData());

    mitk::Image::Pointer newImage2 = dynamic_cast<mitk::Image*>(
      m_Controls->m_ImageSelector2->GetSelectedNode()->GetData());

    ComputeStatistics(newImage1, newImage2, appendSourceData, toClipboard);
  }
}

void QmitkImageStatistics::ComputeStatistics(mitk::Image::Pointer im1, mitk::Image::Pointer im2, bool appendSourceData, bool toClipboard)
{
  // TODO
  return;

  im1->DisconnectPipeline();
  im2->DisconnectPipeline();

  FloatImageType::Pointer itkImage1 = FloatImageType::New();
  FloatImageType::Pointer itkImage2 = FloatImageType::New();

  if( im1->GetDimension() > 3)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(im1);
    timeSelector->SetTimeNr( ((QmitkSliderNavigatorWidget*)m_Controls->sliceNavigatorTime)->GetPos() );
    timeSelector->Update();
    im1 = timeSelector->GetOutput();
  }

  if(im2->GetDimension() > 3)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(im2);
    timeSelector->SetTimeNr( ((QmitkSliderNavigatorWidget*)m_Controls->sliceNavigatorTime)->GetPos() );
    timeSelector->Update();
    im2 = timeSelector->GetOutput();
  }

  CastToItkImage( im1, itkImage1 );
  CastToItkImage( im2, itkImage2 );

  itk::ImageRegionConstIterator<FloatImageType> it1 (itkImage1,
    itkImage1->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<FloatImageType> it2 (itkImage2,
    itkImage2->GetLargestPossibleRegion());

  it1.GoToBegin();
  it2.GoToBegin();

  int size = itkImage2->GetLargestPossibleRegion().GetSize(0)
    * itkImage2->GetLargestPossibleRegion().GetSize(1)
    * itkImage2->GetLargestPossibleRegion().GetSize(2);
  itk::VariableLengthVector<float> values;
  values.SetSize(size);

  bool ignoreUndef = m_Controls->m_IgnoreInfCheckbox->isChecked();
  std::vector<FloatImageType::IndexType> indices;
  int idx = 0;
  while (!(it1.IsAtEnd() || it2.IsAtEnd()))
  {
    bool valueNotMasked = !m_Controls->useMaskStatistics->isChecked() || it2.Get()!= 0;
    bool valueNotInf = std::numeric_limits<float>::infinity()!=it1.Get();
    bool valueNotMinusInf = - std::numeric_limits<float>::infinity()!=it1.Get();
    bool valueNotNan = !my_isnan(it1.Get());
    if( valueNotMasked && ((valueNotInf && valueNotMinusInf &&  valueNotNan) || !ignoreUndef))
    {
      values.SetElement(idx, it1.Get());
      indices.push_back(it1.GetIndex());
      ++idx;
    }
    ++it1;
    ++it2;
  }
  int _n = idx;

  int _minidx=-1, _maxidx=-1;
  float _val=0, _mean=0, _sum=0;
  float _max = itk::NumericTraits<float>::NonpositiveMin();
  float _min = itk::NumericTraits<float>::max();
  for(int i=0; i<_n; i++)
  {
    _val = values[i];
    _maxidx = _val>_max ? i : _maxidx;
    _minidx = _val<_min ? i : _minidx;
    _max = _val>_max ? _val : _max;
    _min = _val<_min ? _val : _min;
    _sum += _val;
  }
  _mean = _sum / _n;

  float _ssd=0, _ss=0, _stddev=0, _rms=0;
  for(int i=0; i<_n; i++)
  {
    _val = values[i];
    _ssd += (_val-_mean)*(_val-_mean);
    _ss  += _val*_val;
  }
  _stddev = sqrt(_ssd / (_n-1));
  _rms    = sqrt(_ss  / _n);

  if(indices.size())
  {
    m_Controls->m_StatisticsTable->setItem( 0, 0, new QTableWidgetItem(
      QString("%1").arg(_mean, 0, 'f', 2) ) );

    m_Controls->m_StatisticsTable->setItem( 0, 1, new QTableWidgetItem(
      QString("%1").arg(_stddev, 0, 'f', 2) ) );

    m_Controls->m_StatisticsTable->setItem( 0, 2, new QTableWidgetItem(
      QString("%1").arg(_rms, 0, 'f', 2) ) );

    m_Controls->m_StatisticsTable->setItem( 0, 3, new QTableWidgetItem(
      QString("%1").arg(_max, 0, 'f', 2) ) );
    m_Controls->m_StatisticsTable->setItem( 0, 4, new QTableWidgetItem(
      QString("%1").arg(_min, 0, 'f', 2) ) );
    m_Controls->m_StatisticsTable->setItem( 0, 5, new QTableWidgetItem(
      QString("%1 (%2 %3 %4)")
        .arg(_maxidx)
        .arg((indices[_maxidx])[0])
        .arg((indices[_maxidx])[1])
        .arg((indices[_maxidx])[2])) );
    m_Controls->m_StatisticsTable->setItem( 0, 6, new QTableWidgetItem(
      QString("%1 (%2 %3 %4)")
        .arg(_minidx)
        .arg((indices[_minidx])[0])
        .arg((indices[_minidx])[1])
        .arg((indices[_minidx])[2])) );
    m_Controls->m_StatisticsTable->setItem( 0, 7, new QTableWidgetItem(
      QString("%1").arg(_n) ) );

    m_Clipboard.clear();
    if(!appendSourceData || toClipboard)
    {
      m_Clipboard = m_Clipboard.append("%1 \t %2 \t %3 \t %4 \t %5 \t %6 \t [%7;%8;%9] \t %9 \t [%9;%9;%9] \t %9 \t");
      m_Clipboard = m_Clipboard
        .arg(_mean, 0, 'f', 10)
        .arg(_stddev, 0, 'f', 10)
        .arg(_rms, 0, 'f', 10)
        .arg(_max, 0, 'f', 10)
        .arg(_min, 0, 'f', 10)
        .arg(_maxidx)
        .arg((indices[_maxidx])[0])
        .arg((indices[_maxidx])[1])
        .arg((indices[_maxidx])[2])
        .arg(_minidx)
        .arg((indices[_minidx])[0])
        .arg((indices[_minidx])[1])
        .arg((indices[_minidx])[2])
        .arg(_n)
        .replace( QChar('.'), "," );
    }
    
    if(appendSourceData || toClipboard)
    {
      for(int i=0; i<_n; i++)
      {
        m_Clipboard.append(QString("%1 ")
          .arg(values[i], 0, 'f', 10)
          .replace( QChar('.'), "," ));
      }
    }      

    if(toClipboard)
    {
      QApplication::clipboard()->setText( 
        m_Clipboard, QClipboard::Clipboard);
    }
  }
}

void QmitkImageStatistics::ClipboardToFile( int count, std::string imageName1, std::string imageName2, std::string filename, bool appendToFile, bool matlab )
{
  FILE * pFile;
  if(appendToFile)
  {
    pFile = fopen (filename.c_str(),"a");
  }
  else
  {
    pFile = fopen (filename.c_str(),"w");
  }
  if(matlab)
  {
    fprintf (pFile, "%% %04d -- %s -- %s \n", count, imageName1.c_str(), imageName2.c_str());
    fprintf (pFile, "a%05d=[%s];\n",count, m_Clipboard.toStdString().c_str());
  }
  else
  {
    fprintf (pFile, "%d \t %s \t %s \t %s\n", count, imageName1.c_str(), imageName2.c_str(), m_Clipboard.toStdString().c_str());
  }
  fclose (pFile);
}

void QmitkImageStatistics::ComputeStatisticsAllImages( bool appendSourceData )
{
  // TODO
  return;

  bool reverse = m_Controls->m_NameFilterReverse->isChecked();

  //std::string sName = selectedItem->GetNode()->GetName();
  //QString qName;
  //qName.sprintf("%s.nhdr",sName.c_str());

  QString filename = QFileDialog::getSaveFileName(
    m_Parent,
    "save file dialog",
    "result.txt",    
    "txt files (*.txt)");

  if ( filename.isEmpty() )
    return;

  bool append = false;
  int count = 0;


  ConstVectorPointer items1 = m_Controls->m_ImageSelector1->GetNodes();
  ConstVectorPointer items2 = m_Controls->m_ImageSelector2->GetNodes();

  ConstVectorIterator itemiterOuter( items2->Begin() ); 
  ConstVectorIterator itemiterOuterend( items2->End() ); 
  while ( itemiterOuter != itemiterOuterend )
  {

    ConstVectorIterator itemiterInner( items1->Begin() ); 
    ConstVectorIterator itemiterInnerend( items1->End() ); 
    while ( itemiterInner != itemiterInnerend )
    {

      std::string name1 = itemiterInner.Value()->GetName();
      std::string name2 = itemiterOuter.Value()->GetName();

      if(std::string::npos != m_Controls->m_NameFilterImages->text().toStdString().find("*")
        || std::string::npos != name1.find(m_Controls->m_NameFilterImages->text().toStdString()))
      {
        if(std::string::npos != m_Controls->m_NameFilterMasks->text().toStdString().find("*")
          || std::string::npos != name2.find(m_Controls->m_NameFilterMasks->text().toStdString()))
        {
          count++;

          mitk::Image::Pointer newImage1;
          mitk::Image::Pointer newImage2;

          if(!reverse)
          {
            newImage1 = dynamic_cast<mitk::Image*>(
              itemiterInner.Value()->GetData());
            newImage2 = dynamic_cast<mitk::Image*>(
              itemiterOuter.Value()->GetData());
          }
          else
          {
            newImage2 = dynamic_cast<mitk::Image*>(
              itemiterInner.Value()->GetData());
            newImage1 = dynamic_cast<mitk::Image*>(
              itemiterOuter.Value()->GetData());
          }

          ComputeStatistics(newImage1, newImage2, false, false);
          ClipboardToFile( count, name1, name2, filename.toStdString(), append, false );

          QString filename2;
          filename2.append(filename);
          filename2.replace(".txt",".m");
          ComputeStatistics(newImage1, newImage2, true, false);
          ClipboardToFile( count, name1, name2, filename2.toStdString(), append, true );

          append = true;

        }
      }

      ++itemiterInner; 
    } 

    ++itemiterOuter; 
  }
}


void QmitkImageStatistics::FillStatisticsTableView( 
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


void QmitkImageStatistics::InvalidateStatisticsTableView()
{
  for ( unsigned int i = 0; i < 7; ++i )
  {
    m_Controls->m_StatisticsTable->setItem( 0, i, new QTableWidgetItem( "NA" ) );
  }
}


void QmitkImageStatistics::OnSelectionChanged( std::vector<mitk::DataTreeNode*> nodes )
{
  // Clear any unreferenced images
  this->RemoveOrphanImages();

  if ( !this->IsVisible() )
  {
    return;
  }

  if ( nodes.empty() || nodes.size() > 1 )
  {
    // Nothing to do: invalidate image, clear statistics, histogram, and GUI
    m_SelectedImage = NULL;
    this->InvalidateStatisticsTableView();
    m_Controls->m_HistogramWidget->ClearItemModel();
    m_Controls->m_LineProfileWidget->ClearItemModel();

    m_CurrentStatisticsValid = false;
    m_Controls->m_ErrorMessageLabel->hide();
    m_Controls->m_SelectedMaskLabel->setText( "None" );
    return;
  }

  // Get selected element
  mitk::DataTreeNode *selectedNode = nodes.front();
  mitk::Image *selectedImage = dynamic_cast< mitk::Image * >( selectedNode->GetData() );

  // Find the next parent/grand-parent node containing an image, if any
  const mitk::DataStorage::SetOfObjects *parentObjects;
  mitk::DataTreeNode *parentNode = NULL;
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

  do
  {
    parentObjects = this->GetDefaultDataStorage()->GetSources( selectedNode );
    if ( parentObjects->Size() > 0 )
    {
      // Use first parent object (if multiple parents are present)
      parentNode = parentObjects->ElementAt( 0 );
      parentImage = dynamic_cast< mitk::Image * >( parentNode->GetData() );
    }
  } while ( (parentImage == NULL) && (parentObjects->Size() != 0) );

  if ( parentImage != NULL )
  {
    m_SelectedImageNode = parentNode;
    m_SelectedImage = parentImage;

    // Check if a valid mask has been selected (Image or PlanarFigure)
    m_SelectedImageMask = dynamic_cast< mitk::Image * >( selectedNode->GetData() );
    m_SelectedPlanarFigure = dynamic_cast< mitk::PlanarFigure * >( selectedNode->GetData() );

    if ( (m_SelectedImageMask != NULL) || (m_SelectedPlanarFigure != NULL) )
    {
      m_SelectedMaskNode = selectedNode;
    }

  }
  else if ( selectedImage != NULL )
  {
    m_SelectedImageNode = selectedNode;
    m_SelectedImage = selectedImage;
  }


  typedef itk::SimpleMemberCommand< QmitkImageStatistics > ITKCommandType;
  ITKCommandType::Pointer changeListener;
  changeListener = ITKCommandType::New();
  changeListener->SetCallbackFunction( this, &QmitkImageStatistics::RequestStatisticsUpdate );

  // Add change listeners to selected objects
  if ( m_SelectedImage != NULL )
  {
    m_ImageObserverTag = m_SelectedImage->AddObserver( 
      itk::ModifiedEvent(), changeListener );
  }

  if ( m_SelectedImageMask != NULL )
  {
    m_ImageObserverTag = m_SelectedImageMask->AddObserver( 
      itk::ModifiedEvent(), changeListener );
  }

  if ( m_SelectedPlanarFigure != NULL )
  {
    m_ImageObserverTag = m_SelectedPlanarFigure->AddObserver( 
      itk::EndEvent(), changeListener );
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


void QmitkImageStatistics::UpdateStatistics()
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
    typedef itk::SimpleMemberCommand< QmitkImageStatistics > ITKCommandType;
    ITKCommandType::Pointer progressListener;
    progressListener = ITKCommandType::New();
    progressListener->SetCallbackFunction( this, &QmitkImageStatistics::UpdateProgressBar );
    unsigned long progressObserverTag = m_CurrentStatisticsCalculator
      ->AddObserver( itk::ProgressEvent(), progressListener );

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
      message << "<font color='red'>Error in calculating histogram</font>";
      m_Controls->m_ErrorMessageLabel->setText( message.str().c_str() );
      m_Controls->m_ErrorMessageLabel->show();
    }

    m_CurrentStatisticsCalculator->RemoveObserver( progressObserverTag );

    // Make sure that progress bar closes
    mitk::ProgressBar::GetInstance()->Progress( 100 );

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
        // TODO: enable line profile widget
        //m_Controls->m_StatisticsWidgetStack->setCurrentIndex( 1 );       
        //m_Controls->m_LineProfileWidget->SetImage( m_SelectedImage );
        //m_Controls->m_LineProfileWidget->SetPlanarFigure( m_SelectedPlanarFigure );
        //m_Controls->m_LineProfileWidget->UpdateItemModelFromPath();
      }
    }
  }
}

void QmitkImageStatistics::UpdateProgressBar()
{
  mitk::ProgressBar::GetInstance()->Progress();
}


void QmitkImageStatistics::RequestStatisticsUpdate()
{
  if ( !m_StatisticsUpdatePending )
  {
    QApplication::postEvent( this, new QmitkRequestStatisticsUpdateEvent );
    m_StatisticsUpdatePending = true;
  }
}


void QmitkImageStatistics::RemoveOrphanImages()
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


bool QmitkImageStatistics::event( QEvent *event ) 
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

void QmitkImageStatistics::ComputeIntensityProfile( mitk::PlanarLine* line )
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

bool QmitkImageStatistics::IsExclusiveFunctionality() const
{
  return false;
}
