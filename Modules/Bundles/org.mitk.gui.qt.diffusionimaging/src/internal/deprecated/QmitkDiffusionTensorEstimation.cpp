/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-01 10:52:29 +0200 (Di, 01 Apr 2008) $
Version:   $Revision: 13934 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkTimeProbe.h"

#include "QmitkDiffusionTensorEstimation.h"
#include "QmitkDiffusionTensorEstimationControls.h"
#include <qaction.h>
#include "QmitkDataTreeComboBox.h"
#include "QmitkDataTreeListView.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkDiffusionTensorIcon.h"
#include <qfiledialog.h>
#include "QmitkPropertyViewFactory.h"
#include <qobjectlist.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qcheckbox.h>

// properties
#include "mitkStringProperty.h"
#include "mitkProperties.h"
#include "mitkMaterialProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkVtkRepresentationProperty.h"
#include "mitkVtkInterpolationProperty.h"
#include "mitkVtkScalarModeProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkLookupTable.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkGridRepresentationProperty.h"
#include "mitkGridVolumeMapperProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkVectorImageMapper2D.h"
#include "mitkOdfVtkMapper2D.h"

#include "itkOrientedImage.h"
#include "itkVectorImage.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkExceptionObject.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"
#include "itkDiffusionTensor3D.h"
#include "itkTensorFractionalAnisotropyImageFilter.h"
#include "itkTensorRelativeAnisotropyImageFilter.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageRegionConstIterator.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkDiffusionQballReconstructionImageFilter.h"
#include "itkAnalyticalDiffusionQballReconstructionImageFilter.h"
#include "itkPointShell.h"
#include "itkRGBPixel.h"
#include "itkOrientationDistributionFunction.h"
#include "itkDiffusionQballPrincipleDirectionsImageFilter.h"
#include "itkDiffusionQballGeneralizedFaImageFilter.h"
#include "itkShiftScaleImageFilter.h"
#include "itkDiffusionQballPrepareVisualizationImageFilter.h"
#include "itkDiffusionTensorPrincipleDirectionImageFilter.h"
#include "itkDiffusionQballSphericalDeconvolutionImageFilter.h"
#include "itkVectorImagesAngularErrorImageFilter.h"

#include "mitkDicomDiffusionVolumeHeaderReader.h"
#include "mitkGroupDiffusionHeadersFilter.h"
#include "mitkDicomDiffusionVolumesReader.h"
#include "mitkNrrdDiffusionVolumesWriter.h"
#include "mitkNrrdDiffusionVolumesReader.h"
#include "mitkDiffusionVolumes.h"
#include "mitkDataTreeFilterFunctions.h"
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkTeemDiffusionTensor3DReconstructionImageFilter.h"
#include "mitkSurface.h"
#include "mitkDataNodeFactory.h"

#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkDelaunay2D.h"
#include "vtkCleanPolyData.h"
#include "vtkAppendPolyData.h"

#include "mitkImageCast.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define DIFF_EST_PI       3.14159265358979323846

typedef float TTensorPixelType;
typedef itk::DiffusionTensor3D< TTensorPixelType >    TensorPixelType;
typedef itk::Image< TensorPixelType, 3 >              TensorImageType;

typedef itk::VectorImage< DiffusionPixelType, 3 >   DiffusionImageType;

//CAST_N_VEC(3)
//CAST_N_VEC(42)
//CAST_N_VEC(92)
//CAST_N_VEC(162)
//CAST_N_VEC(252)
//CAST_N_VEC(362)
//CAST_N_VEC(492)
//CAST_N_VEC(642)
//CAST_N_VEC(812)
//CAST_N_VEC(1002)

const int QmitkDiffusionTensorEstimation::odfsize = 252;
const int QmitkDiffusionTensorEstimation::nrconvkernels = 252;

// Compile-time Square Root Computation: ceil(sqrt(N))
template <int Size, int Low = 1, int High = Size> 
struct Root;
template <int Size, int Mid>                     
struct Root<Size,Mid,Mid> { 
  static const int root = Mid; 
};
template <int Size, int Low, int High>          
struct Root {
  static const int mean = (Low + High)/2;
  static const bool down = (mean * mean >= Size);
  static const int root = Root<Size, 
    (down ? Low : mean+1), 
    (down ? mean : High) >::root;     
};  

QmitkDiffusionTensorEstimation::QmitkDiffusionTensorEstimation(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it), m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL)
{
  SetAvailability(true);
  m_FilterInitialized = false;
}

QmitkDiffusionTensorEstimation::~QmitkDiffusionTensorEstimation()
{}

QWidget * QmitkDiffusionTensorEstimation::CreateMainWidget(QWidget *parent)
{
  if ( m_MultiWidget == NULL )
  {
    m_MultiWidget = new QmitkStdMultiWidget( parent );
  }
  return m_MultiWidget;
}

QWidget * QmitkDiffusionTensorEstimation::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkDiffusionTensorEstimationControls(parent);
  }

  m_Controls->m_TensorEstimationTeemErrorImage->setChecked(false);
  m_Controls->m_TensorEstimationTeemSigmaEdit->setText("NaN");
  m_Controls->m_TensorEstimationTeemEstimationMethodCombo->insertItem("LLS (Linear Least Squares)");
  m_Controls->m_TensorEstimationTeemEstimationMethodCombo->insertItem("MLE (Maximum Likelihood)");
  m_Controls->m_TensorEstimationTeemEstimationMethodCombo->insertItem("NLS (Nonlinear Least Squares)");
  m_Controls->m_TensorEstimationTeemEstimationMethodCombo->insertItem("WLS (Weighted Least Squares)");
  m_Controls->m_TensorEstimationTeemNumItsSpin->setValue(1);
  m_Controls->m_TensorEstimationTeemConfThresholdEdit->setText("NaN");
  m_Controls->m_TensorEstimationTeemFuzzyEdit->setText("0.0");
  m_Controls->m_TensorEstimationTeemMinValEdit->setText("1.0");

  m_Controls->m_QBallReconstructionThreasholdEdit->setText("0.0");
  m_Controls->m_QBallStandardAlgorithmsOrderSpinbox->setValue(0);
  m_Controls->m_QBallStandardAlgorithmsProbThreshEdit->setText(QString::number(1.0/double(odfsize)));
  m_Controls->m_QBallReconstructionNumberThreadsSpinbox->setValue(4);

  m_Controls->m_QBallReconstructionMaxLLevelComboBox->insertItem( QString("2") );
  m_Controls->m_QBallReconstructionMaxLLevelComboBox->insertItem( QString("4") );
  m_Controls->m_QBallReconstructionMaxLLevelComboBox->insertItem( QString("6") );
  m_Controls->m_QBallReconstructionMaxLLevelComboBox->insertItem( QString("8") );
  m_Controls->m_QBallReconstructionMaxLLevelComboBox->setCurrentItem( 3 );

  m_Controls->m_QBallReconstructionNumberThreadsAnalyticalSpinbox->setValue(4);
  m_Controls->m_QBallReconstructionThreasholdAnalyticalEdit->setText("0.0");
  m_Controls->m_QBallReconstructionLambdaLineEdit->setText("0.006");

  m_Controls->m_QBallStandardAlgorithmsNumberThreadsSpinbox->setValue(4);
  m_Controls->m_QBallStandardAlgorithmsDeconvNumberThreadsSpinbox->setValue(4);
  m_Controls->m_QBallStandardAlgorithmsDeconvolutionThreshEdit->setText("0.1");
  m_Controls->m_QBallStandardAlgorithmsDeconvolutionAngResThresholdEdit->setText("15");

  m_Controls->m_QBallStandardAlgorithmsGFAParam1->setText("2");
  m_Controls->m_QBallStandardAlgorithmsGFAParam2->setText("1");
  
  return m_Controls;
}

void QmitkDiffusionTensorEstimation::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_TensorEstimationButton), SIGNAL(clicked()),(QObject*) this, SLOT(TensorEstimationButton()));
    connect( (QObject*)(m_Controls->m_QBallReconstructionButton), SIGNAL(clicked()),(QObject*) this, SLOT(QBallReconstructionButton()));
    connect( (QObject*)(m_Controls->m_QBallReconstructionAnalyticalButton), SIGNAL(clicked()),(QObject*) this, SLOT(QBallReconstructionAnalyticalButton()));
    connect( (QObject*)(m_Controls->m_TensorEstimationTeemEstimateButton), SIGNAL(clicked()),(QObject*) this, SLOT(TensorEstimationTeemEstimateButton()));

    connect( (QObject*)(m_Controls->m_TensorVolumesSaveButton), SIGNAL(clicked()),(QObject*) this, SLOT(TensorVolumesSaveButton()));
    connect( (QObject*)(m_Controls->m_TensorVolumesLoadButton), SIGNAL(clicked()),(QObject*) this, SLOT(TensorVolumesLoadButton()));
    connect( (QObject*)(m_Controls->m_TensorVolumesRemoveButton), SIGNAL(clicked()),(QObject*) this, SLOT(TensorVolumesRemoveButton()));
    connect( (QObject*)(m_Controls->m_StandardAlgorithmsFAButton), SIGNAL(clicked()),(QObject*) this, SLOT(StandardAlgorithmsFAButton()));
    connect( (QObject*)(m_Controls->m_StandardAlgorithmsRAButton), SIGNAL(clicked()),(QObject*) this, SLOT(StandardAlgorithmsRAButton()));
    connect( (QObject*)(m_Controls->m_StandardAlgorithmsDirectionButton), SIGNAL(clicked()),(QObject*) this, SLOT(StandardAlgorithmsDirectionButton()));

    connect( (QObject*)(m_Controls->m_QBallVolumesSaveButton), SIGNAL(clicked()),(QObject*) this, SLOT(QBallVolumesSaveButton()));
    connect( (QObject*)(m_Controls->m_QBallVolumesLoadButton), SIGNAL(clicked()),(QObject*) this, SLOT(QBallVolumesLoadButton()));
    connect( (QObject*)(m_Controls->m_QBallVolumesRemoveButton), SIGNAL(clicked()),(QObject*) this, SLOT(QBallVolumesRemoveButton()));
    connect( (QObject*)(m_Controls->m_QBallStandardAlgorithmsDirectionButton), SIGNAL(clicked()),(QObject*) this, SLOT(QBallStandardAlgorithmsDirectionButton()));
    connect( (QObject*)(m_Controls->m_QBallStandardAlgorithmsDeconvolutionButton), SIGNAL(clicked()),(QObject*) this, SLOT(QBallStandardAlgorithmsDeconvolutionButton()));
    connect( (QObject*)(m_Controls->m_QBallStandardAlgorithmsGFAButton), SIGNAL(clicked()),(QObject*) this, SLOT(QBallStandardAlgorithmsGFAButton()));
    connect( (QObject*)(m_Controls->m_QBallVolumesVisualizeSelectedButton), SIGNAL(clicked()),(QObject*) this, SLOT(QBallVolumesVisualizeSelectedButton()));

    connect( (QObject*)(m_Controls->m_DirectionVolumesSaveButton), SIGNAL(clicked()),(QObject*) this, SLOT(DirectionVolumesSaveButton()));
    connect( (QObject*)(m_Controls->m_DirectionVolumesLoadButton), SIGNAL(clicked()),(QObject*) this, SLOT(DirectionVolumesLoadButton()));
    connect( (QObject*)(m_Controls->m_DirectionVolumesRemoveButton), SIGNAL(clicked()),(QObject*) this, SLOT(DirectionVolumesRemoveButton()));
    connect( (QObject*)(m_Controls->m_DirectionVolumesAngularErrorButton), SIGNAL(clicked()),(QObject*) this, SLOT(DirectionVolumesAngularErrorButton()));

    connect( (QObject*)(m_Controls->m_DiffusionVolumesLoadButton), SIGNAL(clicked()),(QObject*) this, SLOT(DiffusionVolumesLoadButton()));
    connect( (QObject*)(m_Controls->m_DiffusionVolumeSaveButton), SIGNAL(clicked()),(QObject*) this, SLOT(DiffusionVolumeSaveButton()));
    connect( (QObject*)(m_Controls->m_DiffusionVolumesRemoveButton), SIGNAL(clicked()),(QObject*) this, SLOT(DiffusionVolumesRemoveButton()));
    connect( (QObject*)(m_Controls->m_TensorEstimationDiffusionVolumesSelectAllButton), SIGNAL(clicked()),(QObject*) this, SLOT(DiffusionVolumesSelectAll()));
}
}

QAction * QmitkDiffusionTensorEstimation::CreateAction(QActionGroup *parent)
{

  //action = new QAction( tr( "Brain Atrophy" ), pixmap, tr( "BrainAtrophy" ), 0, parent, "BrainAtrophy" );

  QImage icon = qembed_findImage("QmitkDiffusionTensorEstimation");
  QPixmap pixmap(icon);
  QAction* action;
  action = new QAction( tr( "Diffusion Tensor Estimation" ), pixmap, tr( "QmitkDiffusionTensorEstimation menu" ), 0, parent, "QmitkDiffusionTensorEstimation" );
  return action;
}

void QmitkDiffusionTensorEstimation::TreeChanged()
{
  m_Controls->m_TensorEstimationDiffusionVolumesSelector->Update();
  m_Controls->m_TensorVolumesSelector->Update();
  m_Controls->m_QBallVolumesSelector->Update();
  m_Controls->m_DirectionVolumesSelector->Update();

  if(m_DiffusionVolumesDataTreeFilter
    &&m_DiffusionVolumesDataTreeFilter->GetItems()->Size() > 0)
  {
    m_Controls->m_TensorEstimationButton->setEnabled(true);
    m_Controls->m_QBallReconstructionButton->setEnabled(true);
    m_Controls->m_QBallReconstructionAnalyticalButton->setEnabled(true);
  }
  else
  {
    m_Controls->m_QBallReconstructionButton->setEnabled(false);
    m_Controls->m_QBallReconstructionAnalyticalButton->setEnabled(false);
    m_Controls->m_TensorEstimationButton->setEnabled(false);
  }

  if(m_TensorVolumesDataTreeFilter
    && m_TensorVolumesDataTreeFilter->GetItems()->Size() > 0)
  {
    m_Controls->m_TensorVolumesSaveButton->setEnabled(true);
    m_Controls->m_TensorVolumesRemoveButton->setEnabled(true);
  }
  else
  {
    m_Controls->m_TensorVolumesSaveButton->setEnabled(false);
    m_Controls->m_TensorVolumesRemoveButton->setEnabled(false);
  }

  if(m_QballVolumesDataTreeFilter
    && m_QballVolumesDataTreeFilter->GetItems()->Size() > 0)
  {
    m_Controls->m_QBallVolumesSaveButton->setEnabled(true);
    m_Controls->m_QBallVolumesRemoveButton->setEnabled(true);
    m_Controls->m_QBallVolumesVisualizeSelectedButton->setEnabled(true);
  }
  else
  {
    m_Controls->m_QBallVolumesSaveButton->setEnabled(false);
    m_Controls->m_QBallVolumesRemoveButton->setEnabled(false);
    m_Controls->m_QBallVolumesVisualizeSelectedButton->setEnabled(false);
  }

  if(m_DirectionVolumesDataTreeFilter
    && m_DirectionVolumesDataTreeFilter->GetItems()->Size() > 0)
  {
    m_Controls->m_DirectionVolumesSaveButton->setEnabled(true);
    m_Controls->m_DirectionVolumesRemoveButton->setEnabled(true);
  }
  else
  {
    m_Controls->m_DirectionVolumesSaveButton->setEnabled(false);
    m_Controls->m_DirectionVolumesRemoveButton->setEnabled(false);
  }

  if(m_DirectionVolumesDataTreeFilter
    && m_DirectionVolumesDataTreeFilter->GetItems()->Size() > 1)
  {
    m_Controls->m_DirectionVolumesAngularErrorButton->setEnabled(true);
  }
  else
  {
    m_Controls->m_DirectionVolumesAngularErrorButton->setEnabled(false);
  }
}

void QmitkDiffusionTensorEstimation::Activated()
{
  QmitkFunctionality::Activated();

  if (m_FilterInitialized)
    return;

  // diffusion volumes filter
  m_DiffusionVolumesDataTreeFilter = mitk::DataTreeFilter::New( GetDataTreeIterator()->GetTree() );
  m_DiffusionVolumesDataTreeFilter->SetSelectionMode(mitk::DataTreeFilter::MULTI_SELECT);
  m_DiffusionVolumesDataTreeFilter->SetHierarchyHandling(mitk::DataTreeFilter::FLATTEN_HIERARCHY);
  m_DiffusionVolumesDataTreeFilter->SetFilter( mitk::IsBaseDataType<mitk::DiffusionVolumes<short> >() ); // show diffusion volumes 

  mitk::DataTreeFilter::PropertyList visible_props;
  visible_props.push_back("name");
  m_DiffusionVolumesDataTreeFilter->SetVisibleProperties(visible_props);

  mitk::DataTreeFilter::PropertyList property_labels;
  property_labels.push_back("Diffusion Volumes");
  m_DiffusionVolumesDataTreeFilter->SetPropertiesLabels(property_labels);

  m_Controls->m_TensorEstimationDiffusionVolumesSelector->SetDataTree( GetDataTreeIterator()->GetTree() );
  m_Controls->m_TensorEstimationDiffusionVolumesSelector->SetFilter( m_DiffusionVolumesDataTreeFilter );
  m_Controls->m_TensorEstimationDiffusionVolumesSelector->SetAutoUpdate( false );
  m_Controls->m_TensorEstimationDiffusionVolumesSelector->setStretchedColumn(1);

  // tensor volumes filter
  m_TensorVolumesDataTreeFilter = mitk::DataTreeFilter::New( GetDataTreeIterator()->GetTree() );
  m_TensorVolumesDataTreeFilter->SetSelectionMode(mitk::DataTreeFilter::SINGLE_SELECT);
  m_TensorVolumesDataTreeFilter->SetHierarchyHandling(mitk::DataTreeFilter::FLATTEN_HIERARCHY);
  m_TensorVolumesDataTreeFilter->SetFilter( mitk::IsBaseDataTypeWithBoolProperty<mitk::Image>("IsTensorVolume") ); // show tensor volumes 

  m_TensorVolumesDataTreeFilter->SetVisibleProperties(visible_props);

  mitk::DataTreeFilter::PropertyList tensor_property_labels;
  tensor_property_labels.push_back("Tensor Volumes");
  m_TensorVolumesDataTreeFilter->SetPropertiesLabels(tensor_property_labels);

  m_Controls->m_TensorVolumesSelector->SetDataTree( GetDataTreeIterator()->GetTree() );
  m_Controls->m_TensorVolumesSelector->SetFilter( m_TensorVolumesDataTreeFilter );
  m_Controls->m_TensorVolumesSelector->SetAutoUpdate( false );
  m_Controls->m_TensorVolumesSelector->setStretchedColumn(1);

  // qBall volumes filter
  m_QballVolumesDataTreeFilter = mitk::DataTreeFilter::New( GetDataTreeIterator()->GetTree() );
  m_QballVolumesDataTreeFilter->SetSelectionMode(mitk::DataTreeFilter::MULTI_SELECT);
  m_QballVolumesDataTreeFilter->SetHierarchyHandling(mitk::DataTreeFilter::FLATTEN_HIERARCHY);
  m_QballVolumesDataTreeFilter->SetFilter( mitk::IsBaseDataTypeWithBoolProperty<mitk::Image>("IsQBallVolume") );

  m_QballVolumesDataTreeFilter->SetVisibleProperties(visible_props);

  mitk::DataTreeFilter::PropertyList qball_property_labels;
  qball_property_labels.push_back("Q-Ball Volumes");
  m_QballVolumesDataTreeFilter->SetPropertiesLabels(qball_property_labels);

  m_Controls->m_QBallVolumesSelector->SetDataTree( GetDataTreeIterator()->GetTree() );
  m_Controls->m_QBallVolumesSelector->SetFilter( m_QballVolumesDataTreeFilter );
  m_Controls->m_QBallVolumesSelector->SetAutoUpdate( false );
  m_Controls->m_QBallVolumesSelector->setStretchedColumn(1);

  // direction volumes filter
  m_DirectionVolumesDataTreeFilter = mitk::DataTreeFilter::New( GetDataTreeIterator()->GetTree() );
  m_DirectionVolumesDataTreeFilter->SetSelectionMode(mitk::DataTreeFilter::MULTI_SELECT);
  m_DirectionVolumesDataTreeFilter->SetHierarchyHandling(mitk::DataTreeFilter::FLATTEN_HIERARCHY);
  m_DirectionVolumesDataTreeFilter->SetFilter( mitk::IsBaseDataTypeWithBoolProperty<mitk::Image>("IsDirectionVolume") );

  m_DirectionVolumesDataTreeFilter->SetVisibleProperties(visible_props);

  mitk::DataTreeFilter::PropertyList direction_property_labels;
  direction_property_labels.push_back("Direction Volumes");
  m_DirectionVolumesDataTreeFilter->SetPropertiesLabels(direction_property_labels);

  m_Controls->m_DirectionVolumesSelector->SetDataTree( GetDataTreeIterator()->GetTree() );
  m_Controls->m_DirectionVolumesSelector->SetFilter( m_DirectionVolumesDataTreeFilter );
  m_Controls->m_DirectionVolumesSelector->SetAutoUpdate( false );
  m_Controls->m_DirectionVolumesSelector->setStretchedColumn(1);

  m_FilterInitialized = true;

  TreeChanged();

}


void QmitkDiffusionTensorEstimation::TensorVolumesSaveButton() 
{
  // GET SELECTED ITEM
  const mitk::DataTreeFilter::Item* selectedItem
    = m_TensorVolumesDataTreeFilter->GetSelectedItem();
  if( !selectedItem )
    return;
  mitk::Image::Pointer tensorVol = 
    static_cast<mitk::Image*>(selectedItem->GetNode()->GetData());
  TensorImageType::Pointer itkTensorVol = 
    TensorImageType::New();
  mitk::CastToItkImage<TensorImageType>(tensorVol, itkTensorVol);

  // SELECT FILE DIALOG
  std::string sName = selectedItem->GetNode()->GetName();
  QString qName;
  qName.sprintf("%s.nhdr",sName.c_str());
  QString filename = QFileDialog::getSaveFileName(
    qName,
    "Nrrd Images (*.nrrd *.nhdr)",
    this->m_Controls,
    "save file dialog",
    "Select Nrrd Outputfile" );
  if ( !filename )
    return;

  // WRITING TENSORS TO FILE
  std::cout << "Writing tensors ";
  typedef itk::ImageFileWriter<TensorImageType> TensorWriterType;
  TensorWriterType::Pointer tensorWriter = TensorWriterType::New();
  tensorWriter->SetFileName(filename.ascii());
  tensorWriter->SetInput(itkTensorVol);
  tensorWriter->Update();
}

void QmitkDiffusionTensorEstimation::TensorVolumesLoadButton() 
{
  // SELECT FOLDER DIALOG
  QFileDialog* w = new QFileDialog( this->m_Controls, "Select DWI data file", TRUE );
  w->setMode( QFileDialog::ExistingFiles );
  w->setFilter( "Nrrd Images (*.nrrd *.nhdr)" );

  // RETRIEVE SELECTION
  if ( w->exec() != QDialog::Accepted )
    return;

  QStringList filenames = w->selectedFiles();
  QStringList::Iterator it = filenames.begin();
  while( it != filenames.end() ) {
    std::string filename = ( *it ).ascii();
    ++it;

    // READING TENSOR VOLUME
    typedef itk::ImageFileReader<TensorImageType> ReaderType;
    ReaderType::Pointer tensorReader = ReaderType::New();
    tensorReader->SetFileName(filename);
    try
    {
      tensorReader->Update();
    }
    catch (itk::ExceptionObject e)
    {
      std::cout << e << std::endl;
    }

    // Tensorvolume
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( tensorReader->GetOutput() );
    image->SetVolume( tensorReader->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( image );
    mitk::DataStorage::GetInstance()->Add(node);
    SetDefaultNodeProperties(node, itksys::SystemTools::GetFilenameName(filename));
    node->SetProperty( "IsTensorVolume", mitk::BoolProperty::New( true ) );

    TreeChanged();
  }
}

void QmitkDiffusionTensorEstimation::TensorVolumesRemoveButton() 
{
  m_TensorVolumesDataTreeFilter->DeleteSelectedItems();
}

void QmitkDiffusionTensorEstimation::QBallVolumesSaveButton() 
{
  // GET SELECTED ITEM
  const mitk::DataTreeFilter::Item* selectedItem
    = m_QballVolumesDataTreeFilter->GetSelectedItem();
  if( !selectedItem ) return;

  mitk::Image::Pointer qBallVol = 
    static_cast<mitk::Image*>(selectedItem->GetNode()->GetData());
  if( !qBallVol)return;

  typedef itk::Image<itk::Vector<TTensorPixelType,odfsize>,3 > IType;
  IType::Pointer itkQBallVol = IType::New();
  mitk::CastToItkImage<IType>(qBallVol, itkQBallVol);

  typedef itk::VectorImage<TTensorPixelType, 3> VarVecImgType;
  VarVecImgType::Pointer vecImg = VarVecImgType::New();
  vecImg->SetSpacing( itkQBallVol->GetSpacing() );   // Set the image spacing
  vecImg->SetOrigin( itkQBallVol->GetOrigin() );     // Set the image origin
  vecImg->SetDirection( itkQBallVol->GetDirection() );  // Set the image direction
  vecImg->SetLargestPossibleRegion( itkQBallVol->GetLargestPossibleRegion());
  vecImg->SetBufferedRegion( itkQBallVol->GetLargestPossibleRegion() );
  vecImg->SetVectorLength(odfsize);
  vecImg->Allocate();
  itk::ImageRegionIterator<VarVecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
  ot = ot.Begin();

  itk::ImageRegionIterator<IType> it (itkQBallVol, itkQBallVol->GetLargestPossibleRegion() );
  it = it.Begin();
  std::cout << it.Get() << std::endl;
  for (it = it.Begin(); !it.IsAtEnd(); ++it)
  {
    itk::Vector<TTensorPixelType,odfsize> vec = it.Get();
    VarVecImgType::PixelType varvec(vec.GetDataPointer(), odfsize);
    ot.Set(varvec);
    ++ot;
  }

  // SELECT FILE DIALOG
  std::string sName = selectedItem->GetNode()->GetName();
  QString qName;
  qName.sprintf("%s.nhdr",sName.c_str());
  QString filename = QFileDialog::getSaveFileName(
    qName,
    "Nrrd Images (*.nrrd *.nhdr)",
    this->m_Controls,
    "save file dialog",
    "Select Nrrd Outputfile" );
  if ( !filename )
    return;

  // WRITING TENSORS TO FILE
  std::cout << "Writing data ";
  typedef itk::ImageFileWriter<VarVecImgType> QBallWriterType;
  QBallWriterType::Pointer qballWriter = QBallWriterType::New();
  qballWriter->SetFileName(filename.ascii());
  qballWriter->SetInput(vecImg);
  qballWriter->Update();
}

void QmitkDiffusionTensorEstimation::QBallVolumesLoadButton() 
{
  // SELECT FOLDER DIALOG
  QFileDialog* w = new QFileDialog( this->m_Controls, "Select DWI data file", TRUE );
  w->setMode( QFileDialog::ExistingFiles );
  w->setFilter( "Nrrd Images (*.nrrd *.nhdr)" );

  // RETRIEVE SELECTION
  if ( w->exec() != QDialog::Accepted )
    return;

  QStringList filenames = w->selectedFiles();
  QStringList::Iterator it = filenames.begin();
  while( it != filenames.end() ) {
    std::string filename = ( *it ).ascii();
    ++it;

    // READING TENSOR VOLUME
    typedef itk::Image<itk::Vector<TTensorPixelType,odfsize>,3 > IVType;
    typedef itk::ImageFileReader<IVType> ReaderType;
    ReaderType::Pointer qballReader = ReaderType::New();
    qballReader->SetFileName(filename);
    try
    {
      qballReader->Update();
    }
    catch (itk::ExceptionObject e)
    {
      std::cout << e << std::endl;
    }

    //itk::ImageRegionConstIterator<IVType> it (qballReader->GetOutput(), qballReader->GetOutput()->GetLargestPossibleRegion() );
    //it = it.Begin();
    //std::cout << it.Get() << std::endl;

    // Tensorvolume
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( qballReader->GetOutput() );
    image->SetVolume( qballReader->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( image );
    mitk::DataStorage::GetInstance()->Add(node);
    SetDefaultNodeProperties(node, itksys::SystemTools::GetFilenameName(filename));
    node->SetProperty( "IsQBallVolume", mitk::BoolProperty::New( true ) );
    node->SetProperty( "visible", mitk::BoolProperty::New( false ) );

    TreeChanged();
  }
}

void QmitkDiffusionTensorEstimation::QBallVolumesRemoveButton() 
{
  m_QballVolumesDataTreeFilter->DeleteSelectedItems();
}

void QmitkDiffusionTensorEstimation::QBallVolumesVisualizeSelectedButton() 
{
  itk::TimeProbe clock;
  QString status;
  const mitk::DataTreeFilter::Item* item
    = m_QballVolumesDataTreeFilter->GetSelectedItem();
  if(!item)return;

  typedef itk::Vector<TTensorPixelType,odfsize> OdfVectorType;
  typedef itk::Image<OdfVectorType,3> OdfVectorImgType;
  mitk::Image* vol = 
    static_cast<mitk::Image*>(item->GetNode()->GetData());
  OdfVectorImgType::Pointer itkvol = OdfVectorImgType::New();
  mitk::CastToItkImage<OdfVectorImgType>(vol, itkvol);
  std::string nodename = item->GetProperty("name");

  // PREPARE FOR VISUALIZATION
  clock.Start();
  std::cout << "Preparing for Visualization ";
  mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
    "Preparing for Visualization of %s", nodename.c_str()));
  typedef itk::DiffusionQballPrepareVisualizationImageFilter<TTensorPixelType,odfsize>
    FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(itkvol);
  filter->SetNumberOfThreads(4);

  switch(m_Controls->m_QBallVolumesVisualizeNormalizationMethod->currentItem())
  {
  case 0:
    {
      filter->SetNormalizationMethod(FilterType::PV_MIN_MAX);
      break;
    }
  case 1:
    {
      filter->SetNormalizationMethod(FilterType::PV_NONE);
      break;
    }
  case 2:
    {
      filter->SetNormalizationMethod(FilterType::PV_MAX);
      break;
    }
  case 3:
    {
      filter->SetNormalizationMethod(FilterType::PV_GLOBAL_MAX);
      break;
    }
  case 4:
    {
      filter->SetNormalizationMethod(FilterType::PV_MIN_MAX_INVERT);
      break;
    }
  default:
    {
      filter->SetNormalizationMethod(FilterType::PV_MIN_MAX);
      break;
    }
  }

  if(m_Controls->m_QBallVolumesVisualizeScaleGfaCheckbox->isChecked() )
  {
    typedef itk::DiffusionQballGeneralizedFaImageFilter<TTensorPixelType,TTensorPixelType,odfsize>
      GfaFilterType;
    filter->SetDoScaleGfa(true);
    float p1 = m_Controls->m_QBallStandardAlgorithmsGFAParam1->text().toFloat();
    float p2 = m_Controls->m_QBallStandardAlgorithmsGFAParam2->text().toFloat();
    switch(m_Controls->m_QBallStandardAlgorithmsGFAMethod->currentItem())
    {
    case 0:
      filter->SetScaleByGfaType(GfaFilterType::GFA_STANDARD);
      break;
    case 1:
      filter->SetScaleByGfaType(GfaFilterType::GFA_QUANTILES_HIGH_LOW);
      break;
    case 2:
      filter->SetScaleByGfaType(GfaFilterType::GFA_QUANTILE_HIGH);
      break;
    case 3:
      filter->SetScaleByGfaType(GfaFilterType::GFA_MAX_ODF_VALUE);
      break;
    case 4:
      filter->SetScaleByGfaType(GfaFilterType::GFA_DECONVOLUTION_COEFFS);
      break;
    case 5:
      filter->SetScaleByGfaType(GfaFilterType::GFA_MIN_MAX_NORMALIZED_STANDARD);
      break;
    case 6:
      filter->SetScaleByGfaType(GfaFilterType::GFA_NORMALIZED_ENTROPY);
      break;
    case 7:
      filter->SetScaleByGfaType(GfaFilterType::GFA_NEMATIC_ORDER_PARAMETER);
      break;
    case 8:
      filter->SetScaleByGfaType(GfaFilterType::GFA_QUANTILES_LOW_HIGH);
      break;
    case 9:
      filter->SetScaleByGfaType(GfaFilterType::GFA_QUANTILE_LOW);
      break;
    case 10:
      filter->SetScaleByGfaType(GfaFilterType::GFA_MIN_ODF_VALUE);
      break;
    case 11:
      filter->SetScaleByGfaType(GfaFilterType::GFA_STD_BY_MAX);
      break;
    case 12:
      filter->SetScaleByGfaType(GfaFilterType::GFA_PRINCIPLE_CURVATURE);
      filter->SetGfaParam1(p1);
      break;
    case 13:
      filter->SetScaleByGfaType(GfaFilterType::GFA_GENERALIZED_GFA);
      filter->SetGfaParam1(p1);
      filter->SetGfaParam2(p2);
      break;
    default:
      filter->SetScaleByGfaType(GfaFilterType::GFA_STANDARD);
    }
  }
  filter->Update();
  clock.Stop();
  std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;

  // VIZ TO DATATREE
  mitk::Image::Pointer image = mitk::Image::New();
  image->InitializeByItk( filter->GetOutput() );
  image->SetVolume( filter->GetOutput()->GetBufferPointer() );
  mitk::DataNode::Pointer node=mitk::DataNode::New();
  node->SetData( image );
  mitk::DataStorage::GetInstance()->Add(node);
  SetDefaultNodeProperties(node, nodename.append(" Viz"));
  node->SetProperty( "IsQBallVolume", mitk::BoolProperty::New( true ) );
  node->SetProperty( "ShowMaxNumber", mitk::IntProperty::New( 1500 ) );
  node->SetProperty( "DoRefresh", mitk::BoolProperty::New( true ) );
  node->SetProperty( "layer", mitk::IntProperty::New( 1 ) );
  node->SetProperty( "global_scaling", mitk::FloatProperty::New( 1.0 ) );
  mitk::OdfVtkMapper2D<TTensorPixelType,odfsize>::Pointer odfMapper = 
    mitk::OdfVtkMapper2D<TTensorPixelType,odfsize>::New();
  node->SetMapper(1,odfMapper);

  mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");

  m_DataTreeIterator->GetTree()->Modified();
  m_MultiWidget->RequestUpdate();
  TreeChanged();
  m_Controls->update();

}

void QmitkDiffusionTensorEstimation::DirectionVolumesSaveButton() 
{
  // GET SELECTED ITEM
  const mitk::DataTreeFilter::Item* selectedItem
    = m_DirectionVolumesDataTreeFilter->GetSelectedItem();
  if( !selectedItem ) return;

  mitk::Image::Pointer vol = 
    static_cast<mitk::Image*>(selectedItem->GetNode()->GetData());
  if( !vol)return;

  typedef itk::Image<itk::Vector<TTensorPixelType,3>,3 > IType;
  IType::Pointer itkVol = IType::New();
  mitk::CastToItkImage<IType>(vol, itkVol);

  typedef itk::VectorImage<TTensorPixelType, 3> VarVecImgType;
  VarVecImgType::Pointer vecImg = VarVecImgType::New();
  vecImg->SetSpacing( itkVol->GetSpacing() );   // Set the image spacing
  vecImg->SetOrigin( itkVol->GetOrigin() );     // Set the image origin
  vecImg->SetDirection( itkVol->GetDirection() );  // Set the image direction
  vecImg->SetLargestPossibleRegion( itkVol->GetLargestPossibleRegion());
  vecImg->SetBufferedRegion( itkVol->GetLargestPossibleRegion() );
  vecImg->SetVectorLength(3);
  vecImg->Allocate();
  itk::ImageRegionIterator<VarVecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
  ot = ot.Begin();

  itk::ImageRegionIterator<IType> it (itkVol, itkVol->GetLargestPossibleRegion() );
  it = it.Begin();
  for (it = it.Begin(); !it.IsAtEnd(); ++it)
  {
    itk::Vector<TTensorPixelType,3> vec = it.Get();
    VarVecImgType::PixelType varvec(vec.GetDataPointer(), 3);
    ot.Set(varvec);
    ++ot;
  }

  // SELECT FILE DIALOG
  std::string sName = selectedItem->GetNode()->GetName();
  QString qName;
  qName.sprintf("%s.nhdr",sName.c_str());
  QString filename = QFileDialog::getSaveFileName(
    qName,
    "Nrrd Images (*.nrrd *.nhdr)",
    this->m_Controls,
    "save file dialog",
    "Select Nrrd Outputfile" );
  if ( !filename )
    return;

  // WRITING TENSORS TO FILE
  std::cout << "Writing data ";
  typedef itk::ImageFileWriter<VarVecImgType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(filename.ascii());
  writer->SetInput(vecImg);
  writer->Update();
}

void QmitkDiffusionTensorEstimation::DirectionVolumesLoadButton() 
{
  // SELECT FOLDER DIALOG
  QFileDialog* w = new QFileDialog( this->m_Controls, "Select DWI data file", TRUE );
  w->setMode( QFileDialog::ExistingFiles );
  w->setFilter( "Nrrd Images (*.nrrd *.nhdr)" );

  // RETRIEVE SELECTION
  if ( w->exec() != QDialog::Accepted )
    return;

  QStringList filenames = w->selectedFiles();
  QStringList::Iterator it = filenames.begin();
  while( it != filenames.end() ) {
    std::string filename = ( *it ).ascii();
    ++it;

    // READING VOLUME
    typedef itk::Image<itk::Vector<TTensorPixelType,3>,3 > IType;
    typedef itk::ImageFileReader<IType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(filename);
    try
    {
      reader->Update();
    }
    catch (itk::ExceptionObject e)
    {
      std::cout << e << std::endl;
    }

    // Tensorvolume
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( reader->GetOutput() );
    image->SetVolume( reader->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( image );
    mitk::DataStorage::GetInstance()->Add(node);
    SetDefaultNodeProperties(node, itksys::SystemTools::GetFilenameName(filename));
    node->SetProperty( "IsDirectionVolume", mitk::BoolProperty::New( true ) );

    mitk::VectorImageMapper2D::Pointer mapper = 
      mitk::VectorImageMapper2D::New();
    node->SetMapper(1,mapper);

    TreeChanged();
  }
}

void QmitkDiffusionTensorEstimation::DirectionVolumesRemoveButton() 
{
  m_DirectionVolumesDataTreeFilter->DeleteSelectedItems();
}

void QmitkDiffusionTensorEstimation::TensorEstimationTeemEstimateButton() 
{
  try
  {
    itk::TimeProbe clock;

    const mitk::DataTreeFilter::ItemSet* selectedItems
      = m_DiffusionVolumesDataTreeFilter->GetSelectedItems();


    int nrFiles = selectedItems->size();
    if (!nrFiles) return;

    QString status;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(nrFiles);

    mitk::DataTreeFilter::ItemSet::const_iterator itemiter( selectedItems->begin() ); 
    mitk::DataTreeFilter::ItemSet::const_iterator itemiterend( selectedItems->end() ); 

    std::vector<mitk::DataNode::Pointer> nodes;
    while ( itemiter != itemiterend ) // for all items
    {

      mitk::DiffusionVolumes<DiffusionPixelType>* vols = 
        static_cast<mitk::DiffusionVolumes<DiffusionPixelType>*>(
        (*itemiter)->GetNode()->GetData());

      std::string nodename = (*itemiter)->GetProperty("name");
      itemiter++;

      // TENSOR RECONSTRUCTION
      clock.Start();
      std::cout << "Teem Tensor reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
        "Teem Tensor reconstruction for %s", nodename.c_str()));
      typedef mitk::TeemDiffusionTensor3DReconstructionImageFilter< 
        DiffusionPixelType, TTensorPixelType > TensorReconstructionImageFilterType;
      TensorReconstructionImageFilterType::Pointer tensorReconstructionFilter = 
        TensorReconstructionImageFilterType::New();
      tensorReconstructionFilter->SetInput( vols );
      tensorReconstructionFilter->SetEstimateErrorImage( m_Controls->m_TensorEstimationTeemErrorImage->isChecked() );
      if(!m_Controls->m_TensorEstimationTeemSigmaEdit->text().contains(QString("NaN")))
        tensorReconstructionFilter->SetSigma( m_Controls->m_TensorEstimationTeemSigmaEdit->text().toFloat() );
      switch(m_Controls->m_TensorEstimationTeemEstimationMethodCombo->currentItem())
      {
      case 0:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsLLS);
        break;
      case 1:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsNLS);
        break;
      case 2:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsWLS);
        break;
      case 3:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsMLE);
        break;
      default:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsLLS);
      }
      tensorReconstructionFilter->SetNumIterations( m_Controls->m_TensorEstimationTeemNumItsSpin->value() );
      if(!m_Controls->m_TensorEstimationTeemConfThresholdEdit->text().contains(QString("NaN")))
        tensorReconstructionFilter->SetConfidenceThreshold( m_Controls->m_TensorEstimationTeemConfThresholdEdit->text().toDouble() );
      tensorReconstructionFilter->SetConfidenceFuzzyness( m_Controls->m_TensorEstimationTeemFuzzyEdit->text().toFloat() );
      tensorReconstructionFilter->SetMinPlausibleValue( m_Controls->m_TensorEstimationTeemMinValEdit->text().toDouble() );
      tensorReconstructionFilter->Update();
      clock.Stop();
      std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;

      // TENSORS TO DATATREE
      //mitk::DataNode::Pointer node=mitk::DataNode::New();
      //node->SetData( tensorReconstructionFilter->GetOutput() );
      //mitk::DataStorage::GetInstance()->Add(node);
      //SetDefaultNodeProperties(node, nodename.append(" tensors"));
      //node->SetProperty( "IsConfidenceTensorVolume", mitk::BoolProperty::New( true ) );

      mitk::DataNode::Pointer node2=mitk::DataNode::New();
      node2->SetData( tensorReconstructionFilter->GetOutputItk() );
      SetDefaultNodeProperties(node2, nodename.append(" (itk)"));
      node2->SetProperty( "IsTensorVolume", mitk::BoolProperty::New( true ) );
      nodes.push_back(node2);

      mitk::ProgressBar::GetInstance()->Progress();

    }

    std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
    for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
      mitk::DataStorage::GetInstance()->Add(*nodeIt);

    mitk::ProgressBar::GetInstance()->Progress();
    TreeChanged();
    m_Controls->update();

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished Processing %d Files", nrFiles));

  }
  catch (itk::ExceptionObject &ex)
  {
    std::cout << ex << std::endl;
    return ;
  }
}

void QmitkDiffusionTensorEstimation::TensorEstimationButton() 
{
  try
  {
    itk::TimeProbe clock;

    const mitk::DataTreeFilter::ItemSet* selectedItems
      = m_DiffusionVolumesDataTreeFilter->GetSelectedItems();

    int nrFiles = selectedItems->size();
    if (!nrFiles) return;

    QString status;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(nrFiles);

    mitk::DataTreeFilter::ItemSet::const_iterator itemiter( selectedItems->begin() ); 
    mitk::DataTreeFilter::ItemSet::const_iterator itemiterend( selectedItems->end() ); 

    std::vector<mitk::DataNode::Pointer> nodes;
    while ( itemiter != itemiterend ) // for all items
    {

      mitk::DiffusionVolumes<DiffusionPixelType>* vols = 
        static_cast<mitk::DiffusionVolumes<DiffusionPixelType>*>(
        (*itemiter)->GetNode()->GetData());

      std::string nodename = (*itemiter)->GetProperty("name");
      itemiter++;

      // TENSOR RECONSTRUCTION
      clock.Start();
      std::cout << "Tensor reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
        "Tensor reconstruction for %s", nodename.c_str()));
      typedef itk::DiffusionTensor3DReconstructionImageFilter< 
        DiffusionPixelType, DiffusionPixelType, TTensorPixelType > TensorReconstructionImageFilterType;
      TensorReconstructionImageFilterType::Pointer tensorReconstructionFilter = 
        TensorReconstructionImageFilterType::New();
      tensorReconstructionFilter->SetGradientImage( vols->GetDirections(), vols->GetImage() );
      tensorReconstructionFilter->SetNumberOfThreads( m_Controls->m_TensorEstimationNumberThreadsSpinbox->value() );
      tensorReconstructionFilter->SetBValue(vols->GetB_Value());  
      tensorReconstructionFilter->SetThreshold( m_Controls->m_TensorEstimationThreasholdEdit->text().toFloat() );
      tensorReconstructionFilter->Update();
      clock.Stop();
      std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;

      // TENSORS TO DATATREE
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk( tensorReconstructionFilter->GetOutput() );
      image->SetVolume( tensorReconstructionFilter->GetOutput()->GetBufferPointer() );
      mitk::DataNode::Pointer node=mitk::DataNode::New();
      node->SetData( image );
      SetDefaultNodeProperties(node, nodename.append(" tensors"));
      node->SetProperty( "IsTensorVolume", mitk::BoolProperty::New( true ) );
      nodes.push_back(node);

      mitk::ProgressBar::GetInstance()->Progress();

    }

    std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
    for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
      mitk::DataStorage::GetInstance()->Add(*nodeIt);

    mitk::ProgressBar::GetInstance()->Progress();
    TreeChanged();
    m_Controls->update();

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished Processing %d Files", nrFiles));

  }
  catch (itk::ExceptionObject &ex)
  {
    std::cout << ex << std::endl;
    return ;
  }
}

void QmitkDiffusionTensorEstimation::QBallReconstructionButton() 
{
  try
  {
    itk::TimeProbe clock;

    const mitk::DataTreeFilter::ItemSet* selectedItems
      = m_DiffusionVolumesDataTreeFilter->GetSelectedItems();

    int nrFiles = selectedItems->size();
    if (!nrFiles) return;

    QString status;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(nrFiles);

    mitk::DataTreeFilter::ItemSet::const_iterator itemiter( selectedItems->begin() ); 
    mitk::DataTreeFilter::ItemSet::const_iterator itemiterend( selectedItems->end() ); 

    std::vector<mitk::DataNode::Pointer> nodes;
    while ( itemiter != itemiterend ) // for all items
    {

      mitk::DiffusionVolumes<DiffusionPixelType>* vols = 
        static_cast<mitk::DiffusionVolumes<DiffusionPixelType>*>(
        (*itemiter)->GetNode()->GetData());

      std::string nodename = (*itemiter)->GetProperty("name");
      ++itemiter;

      // QBALL RECONSTRUCTION
      clock.Start();
      std::cout << "QBall reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
        "QBall reconstruction for %s", nodename.c_str()));

      typedef itk::DiffusionQballReconstructionImageFilter< 
        DiffusionPixelType, DiffusionPixelType, TTensorPixelType, odfsize>
        //int NOdfDirections = 162,
        QballReconstructionImageFilterType;
      QballReconstructionImageFilterType::Pointer filter = 
        QballReconstructionImageFilterType::New();
      filter->SetGradientImage( vols->GetDirections(), vols->GetImage() );
      filter->SetNumberOfThreads( m_Controls->m_QBallReconstructionNumberThreadsSpinbox->value() );
      filter->SetBValue(vols->GetB_Value());  
      filter->SetThreshold( m_Controls->m_QBallReconstructionThreasholdEdit->text().toFloat() );

      int normalization = m_Controls->m_QBallReconstructionPostprocessingMethod->currentItem();
      switch(normalization)
      {
      case 0:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_STANDARD);
          break;
        }
      case 1:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_B_ZERO_B_VALUE);
          break;
        }
      case 2:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_B_ZERO);
          break;
        }
      case 3:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_NONE);
          break;
        }
      default:
        {
          filter->SetNormalizationMethod(QballReconstructionImageFilterType::QBR_STANDARD);
        }
      }

      filter->Update();
      clock.Stop();
      std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;

      // ODFs TO DATATREE
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk( filter->GetOutput() );
      //image->SetImportVolume( filter->GetOutput()->GetBufferPointer(), 0, 0, mitk::Image::ImportMemoryManagementType::ManageMemory );
      image->SetVolume( filter->GetOutput()->GetBufferPointer() );
      mitk::DataNode::Pointer node=mitk::DataNode::New();
      node->SetData( image );
      QString newname;
      newname = newname.append(nodename.c_str());
      newname = newname.append("_QN%1").arg(normalization);
      SetDefaultNodeProperties(node, newname.ascii());
      node->SetProperty( "IsQBallVolume", mitk::BoolProperty::New( true ) );
      nodes.push_back(node);

      // B-Zero TO DATATREE
      mitk::Image::Pointer image4 = mitk::Image::New();
      image4->InitializeByItk( filter->GetBZeroImage().GetPointer() );
      image4->SetVolume( filter->GetBZeroImage()->GetBufferPointer() );
      mitk::DataNode::Pointer node4=mitk::DataNode::New();
      node4->SetData( image4 );
      SetDefaultNodeProperties(node4, nodename.append("B0"));
      nodes.push_back(node4);

      mitk::ProgressBar::GetInstance()->Progress();

    }

    std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
    for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
      mitk::DataStorage::GetInstance()->Add(*nodeIt);

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished Processing %d Files", nrFiles));
    m_DataTreeIterator->GetTree()->Modified();
    m_MultiWidget->RequestUpdate();
    TreeChanged();
    m_Controls->update();

  }
  catch (itk::ExceptionObject &ex)
  {
    std::cout << ex << std::endl;
    return ;
  }
}

template<int L>
void QmitkDiffusionTensorEstimation::ReconstructAnalytically(
  mitk::DiffusionVolumes<DiffusionPixelType>* vols, float lambda,
  std::string nodename, std::vector<mitk::DataNode::Pointer>* nodes) 
{
  typedef itk::AnalyticalDiffusionQballReconstructionImageFilter
    <DiffusionPixelType,DiffusionPixelType,TTensorPixelType,L,odfsize> FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetGradientImage( vols->GetDirections(), vols->GetImage() );
  filter->SetNumberOfThreads( m_Controls->m_QBallReconstructionNumberThreadsAnalyticalSpinbox->value() );
  filter->SetBValue(vols->GetB_Value());  
  filter->SetThreshold( m_Controls->m_QBallReconstructionThreasholdAnalyticalEdit->text().toFloat() );
  filter->SetLambda(lambda);
  filter->SetAdcProfileOnly(m_Controls->m_QBallReconstructionAdcOnlyCheckbox->isChecked());
  int normalization = m_Controls->m_QBallReconstructionPostprocessingMethodAnalytical->currentItem();
  switch(normalization)
  {
  case 0:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
      break;
    }
  case 1:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_B_ZERO_B_VALUE);
      break;
    }            
  case 2:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_B_ZERO);
      break;
    }
  case 3:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_NONE);
      break;
    }
  default:
    {
      filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
    }
  }

  filter->Update();

  // ODFs TO DATATREE
  mitk::Image::Pointer image = mitk::Image::New();
  image->InitializeByItk( filter->GetOutput() );
  image->SetVolume( filter->GetOutput()->GetBufferPointer() );
  mitk::DataNode::Pointer node=mitk::DataNode::New();
  node->SetData( image );
  nodes->push_back(node);
  QString newname;
  newname = newname.append(nodename.c_str());
  newname = newname.append("_QA%1").arg(normalization);
  SetDefaultNodeProperties(node, newname.ascii());
  node->SetProperty( "IsQBallVolume", mitk::BoolProperty::New( true ) );

  // B-Zero TO DATATREE
  mitk::Image::Pointer image4 = mitk::Image::New();
  image4->InitializeByItk( filter->GetBZeroImage().GetPointer() );
  image4->SetVolume( filter->GetBZeroImage()->GetBufferPointer() );
  mitk::DataNode::Pointer node4=mitk::DataNode::New();
  node4->SetData( image4 );
  nodes->push_back(node4);
  SetDefaultNodeProperties(node4, nodename.append("B0"));

}

void QmitkDiffusionTensorEstimation::QBallReconstructionAnalyticalButton() 
{
  try
  {
    itk::TimeProbe clock;

    const mitk::DataTreeFilter::ItemSet* selectedItems
      = m_DiffusionVolumesDataTreeFilter->GetSelectedItems();

    int nrFiles = selectedItems->size();
    if (!nrFiles) return;

    std::vector<float> lambdas;
    float minLambda  = m_Controls->m_QBallReconstructionLambdaLineEdit->text().toFloat();
    if(m_Controls->m_QBallReconstructionLambdaMultiCheckbox->isChecked())
    {
      float stepLambda = m_Controls->m_QBallReconstructionLambdaStepLineEdit->text().toFloat();
      float maxLambda  = m_Controls->m_QBallReconstructionLambdaMaxLineEdit->text().toFloat();
      for(float l=minLambda; l<maxLambda; l+=stepLambda)
      {
        lambdas.push_back(l);
      }
    }
    int nLambdas = lambdas.size();
    if(nLambdas == 0)
    {
      lambdas.push_back(minLambda);
      nLambdas = 1;
    }

    QString status;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(nrFiles*nLambdas);

    mitk::DataTreeFilter::ItemSet::const_iterator itemiter( selectedItems->begin() ); 
    mitk::DataTreeFilter::ItemSet::const_iterator itemiterend( selectedItems->end() ); 

    std::vector<mitk::DataNode::Pointer>* nodes
      = new std::vector<mitk::DataNode::Pointer>();
    while ( itemiter != itemiterend ) // for all items
    {

      mitk::DiffusionVolumes<DiffusionPixelType>* vols = 
        static_cast<mitk::DiffusionVolumes<DiffusionPixelType>*>(
        (*itemiter)->GetNode()->GetData());

      std::string nodename = (*itemiter)->GetProperty("name");
      itemiter++;

      // QBALL RECONSTRUCTION
      clock.Start();
      std::cout << "QBall reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
        "QBall reconstruction for %s", nodename.c_str()));

      for(int i=0; i<nLambdas; i++)
      {

        float currentLambda = lambdas[i];

        switch(m_Controls->m_QBallReconstructionMaxLLevelComboBox->currentItem())
        {
        case 0:
          {
            ReconstructAnalytically<2>(vols, currentLambda, nodename, nodes);
            break;
          }
        case 1:
          {
            ReconstructAnalytically<4>(vols, currentLambda, nodename, nodes);
            break;
          }
        case 2:
          {
            ReconstructAnalytically<6>(vols, currentLambda, nodename, nodes);
            break;
          }
        case 3:
          {
            ReconstructAnalytically<8>(vols, currentLambda, nodename, nodes);
            break;
          }
        }

        clock.Stop();
        std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;
        mitk::ProgressBar::GetInstance()->Progress();

      }
    }

    std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
    for(nodeIt = nodes->begin(); nodeIt != nodes->end(); ++nodeIt)
      mitk::DataStorage::GetInstance()->Add(*nodeIt);

    m_DataTreeIterator->GetTree()->Modified();
    m_MultiWidget->RequestUpdate();
    TreeChanged();
    m_Controls->update();

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished Processing %d Files", nrFiles));

  }
  catch (itk::ExceptionObject &ex)
  {
    std::cout << ex << std::endl;
    return ;
  }
}

void QmitkDiffusionTensorEstimation::StandardAlgorithmsFAButton() 
{
  itk::TimeProbe clock;
  QString status;
  const mitk::DataTreeFilter::Item* item
    = m_TensorVolumesDataTreeFilter->GetSelectedItem();
  if(!item)return;

  mitk::Image* vol = 
    static_cast<mitk::Image*>(item->GetNode()->GetData());
  itk::Image<TensorPixelType, 3>::Pointer itkvol = itk::Image<TensorPixelType, 3>::New();
  mitk::CastToItkImage<TensorImageType>(vol, itkvol);
  std::string nodename = item->GetProperty("name");

  // COMPUTE FA
  clock.Start();
  std::cout << "Computing FA ";
  mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
    "Computing FA for %s", nodename.c_str()));
  typedef TensorPixelType::RealValueType                 RealValueType;
  typedef itk::Image< RealValueType, 3 >                 FAImageType;
  typedef itk::TensorFractionalAnisotropyImageFilter< 
    TensorImageType, FAImageType >                       FAFilterType;
  FAFilterType::Pointer fractionalAnisotropyFilter = FAFilterType::New();
  fractionalAnisotropyFilter->SetInput( itkvol );

  typedef itk::ShiftScaleImageFilter<FAImageType, FAImageType> 
    ShiftScaleFilterType;
  ShiftScaleFilterType::Pointer multi =
    ShiftScaleFilterType::New();
  multi->SetShift(0);
  multi->SetScale(200);//itk::NumericTraits<RealValueType>::max()
  multi->SetInput(fractionalAnisotropyFilter->GetOutput());
  multi->Update();
  clock.Stop();
  std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;

  // FA TO DATATREE
  mitk::Image::Pointer image = mitk::Image::New();
  image->InitializeByItk( multi->GetOutput() );
  image->SetVolume( multi->GetOutput()->GetBufferPointer() );
  mitk::DataNode::Pointer node=mitk::DataNode::New();
  node->SetData( image );
  mitk::DataStorage::GetInstance()->Add(node);
  SetDefaultNodeProperties(node, nodename.append(" FA"));
  node->SetProperty( "IsFAVolume", mitk::BoolProperty::New( true ) );

  mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");

  m_DataTreeIterator->GetTree()->Modified();
  m_MultiWidget->RequestUpdate();
  TreeChanged();
  m_Controls->update();
}

void QmitkDiffusionTensorEstimation::StandardAlgorithmsRAButton() 
{
  itk::TimeProbe clock;
  QString status;
  const mitk::DataTreeFilter::Item* item
    = m_TensorVolumesDataTreeFilter->GetSelectedItem();
  if(!item)return;

  mitk::Image* vol = 
    static_cast<mitk::Image*>(item->GetNode()->GetData());
  itk::Image<TensorPixelType, 3>::Pointer itkvol = itk::Image<TensorPixelType, 3>::New();
  mitk::CastToItkImage<TensorImageType>(vol, itkvol);
  std::string nodename = item->GetProperty("name");

  // COMPUTE RA
  clock.Start();
  std::cout << "Computing RA ";
  mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
    "Computing RA for %s", nodename.c_str()));
  typedef TensorPixelType::RealValueType                 RealValueType;
  typedef itk::Image< RealValueType, 3 >                 RAImageType;
  typedef itk::TensorRelativeAnisotropyImageFilter< 
    TensorImageType, RAImageType >                       RAFilterType;
  RAFilterType::Pointer relativeAnisotropyFilter = RAFilterType::New();
  relativeAnisotropyFilter->SetInput( itkvol );
  relativeAnisotropyFilter->Update();
  clock.Stop();
  std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;

  // FA TO DATATREE
  mitk::Image::Pointer image = mitk::Image::New();
  image->InitializeByItk( relativeAnisotropyFilter->GetOutput() );
  image->SetVolume( relativeAnisotropyFilter->GetOutput()->GetBufferPointer() );
  mitk::DataNode::Pointer node=mitk::DataNode::New();
  node->SetData( image );
  mitk::DataStorage::GetInstance()->Add(node);
  SetDefaultNodeProperties(node, nodename.append(" RA"));
  node->SetProperty( "IsRAVolume", mitk::BoolProperty::New( true ) );

  mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");

  m_DataTreeIterator->GetTree()->Modified();
  m_MultiWidget->RequestUpdate();
  TreeChanged();
  m_Controls->update();

}

void QmitkDiffusionTensorEstimation::StandardAlgorithmsDirectionButton() 
{

    itk::TimeProbe clock;
  QString status;
  const mitk::DataTreeFilter::Item* item
    = m_TensorVolumesDataTreeFilter->GetSelectedItem();
  if(!item)return;

  typedef itk::DiffusionTensor3D<TTensorPixelType> TensorType;
  typedef itk::Image<TensorType,3> TensorImgType;
  mitk::Image* vol = 
    static_cast<mitk::Image*>(item->GetNode()->GetData());
  TensorImgType::Pointer itkvol = TensorImgType::New();
  mitk::CastToItkImage<TensorImgType>(vol, itkvol);
  std::string nodename = item->GetProperty("name");

  clock.Start();
  std::cout << "Computing Diffusion Direction ";
  mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
    "Computing Diffusion Direction for %s", nodename.c_str()));

  typedef itk::DiffusionTensorPrincipleDirectionImageFilter<TTensorPixelType,TTensorPixelType>
    FilterType;
  FilterType::Pointer filter
    = FilterType::New();
  filter->SetInput(itkvol);
  filter->SetNumberOfThreads(4);
  filter->Update();
  itk::ImageRegionIterator<FilterType::OutputImageType> 
    it (filter->GetOutput(), 
    filter->GetOutput()->GetLargestPossibleRegion() );
  it = it.Begin();

  // VECTORFIELD
  std::cout << "Converting to Vectorfield" << std::endl;

  typedef itk::Image<itk::Vector<TTensorPixelType,3>, 3> VecImgType2;
  VecImgType2::Pointer vecImg5 = VecImgType2::New();
  vecImg5->SetSpacing( itkvol->GetSpacing() );   // Set the image spacing
  vecImg5->SetOrigin( itkvol->GetOrigin() );     // Set the image origin
  vecImg5->SetDirection( itkvol->GetDirection() );  // Set the image direction
  vecImg5->SetLargestPossibleRegion( itkvol->GetLargestPossibleRegion());
  vecImg5->SetBufferedRegion( vecImg5->GetLargestPossibleRegion() );
  vecImg5->Allocate();
  itk::ImageRegionIterator<VecImgType2> ot5 (vecImg5, vecImg5->GetLargestPossibleRegion() );
  ot5 = ot5.Begin();

  typedef FilterType::OutputImageType::PixelType VecPixType;
  for (it = it.Begin(); !it.IsAtEnd(); ++it)
  {
    VecPixType vec = it.Get();
    itk::Vector<TTensorPixelType,3> pix;
    TTensorPixelType uvec[3] = {(TTensorPixelType)(vec[0]),(TTensorPixelType)(vec[1]),(TTensorPixelType)(vec[2])};
    //TTensorPixelType uvec[3] = {(TTensorPixelType)(vec[1]),(TTensorPixelType)(-vec[0]),(TTensorPixelType)(vec[2])};
    pix = uvec;
    ot5.Set(pix);
    ++ot5;
  }

  // Vectors TO DATATREE
  mitk::Image::Pointer image5 = mitk::Image::New();
  image5->InitializeByItk( vecImg5.GetPointer() );
  image5->SetVolume( vecImg5->GetBufferPointer() );
  mitk::DataNode::Pointer node5=mitk::DataNode::New();
  node5->SetData( image5 );
  node5->SetName( nodename.append(" vecs").c_str());
  mitk::DataStorage::GetInstance()->Add(node5);

  node5->SetProperty( "IsDirectionVolume", mitk::BoolProperty::New( true ) );
  node5->SetProperty( "NormalizeVecs", mitk::BoolProperty::New( true ) );
  node5->SetProperty( "Scale", mitk::FloatProperty::New( 1.0 ) );
  node5->SetProperty( "LineWidth", mitk::FloatProperty::New( 1 ) );

  mitk::VectorImageMapper2D::Pointer vecMapper5 = 
    mitk::VectorImageMapper2D::New();
  node5->SetMapper(1,vecMapper5);

  m_DataTreeIterator->GetTree()->Modified();
  m_MultiWidget->RequestUpdate();
  TreeChanged();
  m_Controls->update();

}

void QmitkDiffusionTensorEstimation::QBallStandardAlgorithmsGFAButton() 
{
  itk::TimeProbe clock;
  QString status;

  const mitk::DataTreeFilter::ItemSet* selectedItems
    = m_QballVolumesDataTreeFilter->GetSelectedItems();

  int nrFiles = selectedItems->size();
  if (!nrFiles) return;

  mitk::DataTreeFilter::ItemSet::const_iterator itemiter( selectedItems->begin() ); 
  mitk::DataTreeFilter::ItemSet::const_iterator itemiterend( selectedItems->end() ); 

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    typedef itk::Vector<TTensorPixelType,odfsize> OdfVectorType;
    typedef itk::Image<OdfVectorType,3> OdfVectorImgType;
    mitk::Image* vol = 
      static_cast<mitk::Image*>((*itemiter)->GetNode()->GetData());
    OdfVectorImgType::Pointer itkvol = OdfVectorImgType::New();
    mitk::CastToItkImage<OdfVectorImgType>(vol, itkvol);

    std::string nodename = (*itemiter)->GetProperty("name");
    ++itemiter;

    float p1 = m_Controls->m_QBallStandardAlgorithmsGFAParam1->text().toFloat();
    float p2 = m_Controls->m_QBallStandardAlgorithmsGFAParam2->text().toFloat();

    // COMPUTE RA
    clock.Start();
    std::cout << "Computing GFA ";
    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
      "Computing GFA for %s", nodename.c_str()));
    typedef OdfVectorType::ValueType                 RealValueType;
    typedef itk::Image< RealValueType, 3 >                 RAImageType;
    typedef itk::DiffusionQballGeneralizedFaImageFilter<TTensorPixelType,TTensorPixelType,odfsize>
      GfaFilterType;
    GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
    gfaFilter->SetInput(itkvol);
    gfaFilter->SetNumberOfThreads(4);

    double scale = 1;
    std::string newname;
    newname.append(nodename);
    newname.append(" GFA");
    switch(m_Controls->m_QBallStandardAlgorithmsGFAMethod->currentItem())
    {
    case 0:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
        newname.append("00");
        scale = 200.0;
        break;
      }
    case 1:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILES_HIGH_LOW);
        newname.append("01");
        scale = 200.0;
        break;
      }
    case 2:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILE_HIGH);
        newname.append("02");
        scale = 200.0;
        break;
      }
    case 3:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_MAX_ODF_VALUE);
        newname.append("03");
        scale = 200.0;
        break;
      }
    case 4:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_DECONVOLUTION_COEFFS);
        newname.append("04");
        scale = 200.0;
        break;
      }
    case 5:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_MIN_MAX_NORMALIZED_STANDARD);
        newname.append("05");
        scale = 200.0;
        break;
      }    
    case 6:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_NORMALIZED_ENTROPY);
        newname.append("06");
        break;
      }
    case 7:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_NEMATIC_ORDER_PARAMETER);
        newname.append("07");
        scale = 200.0;
        break;
      }
    case 8:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILES_LOW_HIGH);
        newname.append("08");
        scale = 200.0;
        break;
      }
    case 9:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILE_LOW);
        newname.append("09");
        scale = 200.0;
        break;
      }
    case 10:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_MIN_ODF_VALUE);
        newname.append("10");
        scale = 200.0;
        break;
      }
    case 11:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_STD_BY_MAX);
        newname.append("11");
        scale = 200.0;
        break;
      }
    case 12:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_PRINCIPLE_CURVATURE);
        newname.append("12");
        gfaFilter->SetParam1(p1);
        scale = 200.0;
        break;
      }
    case 13:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_GENERALIZED_GFA);
        QString paramString;
        paramString = paramString.append(" K%1P%2").arg(p1).arg(p2);
        newname.append("13").append(paramString.ascii());
        gfaFilter->SetParam1(p1);
        gfaFilter->SetParam2(p2);
        scale = 200.0;
        break;
      }
    default:
      {
        newname.append("0");
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
        scale = 200.0;
      }
    }
    gfaFilter->Update();
    clock.Stop();
    std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;

    typedef itk::Image<TTensorPixelType, 3> ImgType;
    ImgType::Pointer img = ImgType::New();
    img->SetSpacing( gfaFilter->GetOutput()->GetSpacing() );   // Set the image spacing
    img->SetOrigin( gfaFilter->GetOutput()->GetOrigin() );     // Set the image origin
    img->SetDirection( gfaFilter->GetOutput()->GetDirection() );  // Set the image direction
    img->SetLargestPossibleRegion( gfaFilter->GetOutput()->GetLargestPossibleRegion());
    img->SetBufferedRegion( gfaFilter->GetOutput()->GetLargestPossibleRegion() );
    img->Allocate();
    itk::ImageRegionIterator<ImgType> ot (img, img->GetLargestPossibleRegion() );
    ot = ot.Begin();
    itk::ImageRegionConstIterator<GfaFilterType::OutputImageType> it 
      (gfaFilter->GetOutput(), gfaFilter->GetOutput()->GetLargestPossibleRegion() );
    it = it.Begin();

    for (it = it.Begin(); !it.IsAtEnd(); ++it)
    {
      GfaFilterType::OutputImageType::PixelType val = it.Get();
      ot.Set(val * scale);
      ++ot;
    }


    // GFA TO DATATREE
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( img.GetPointer() );
    image->SetVolume( img->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( image );
    nodes.push_back(node);
    SetDefaultNodeProperties(node, newname.c_str());
    node->SetProperty( "IsGFAVolume", mitk::BoolProperty::New( true ) );

    mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");

  }

  std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
  for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
    mitk::DataStorage::GetInstance()->Add(*nodeIt);

  m_DataTreeIterator->GetTree()->Modified();
  m_MultiWidget->RequestUpdate();
  TreeChanged();
  m_Controls->update();

}

void QmitkDiffusionTensorEstimation::QBallStandardAlgorithmsDirectionButton() 
{

  itk::TimeProbe clock;
  QString status;
  const mitk::DataTreeFilter::Item* item
    = m_QballVolumesDataTreeFilter->GetSelectedItem();
  if(!item)return;

  typedef itk::Vector<TTensorPixelType,odfsize> OdfVectorType;
  typedef itk::Image<OdfVectorType,3> OdfVectorImgType;
  mitk::Image* vol = 
    static_cast<mitk::Image*>(item->GetNode()->GetData());
  OdfVectorImgType::Pointer itkvol = OdfVectorImgType::New();
  mitk::CastToItkImage<OdfVectorImgType>(vol, itkvol);
  std::string nodename = item->GetProperty("name");

  clock.Start();
  std::cout << "Computing Diffusion Direction ";
  mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
    "Computing Diffusion Direction for %s", nodename.c_str()));

  typedef itk::DiffusionQballGeneralizedFaImageFilter<TTensorPixelType,TTensorPixelType,odfsize>
    GfaFilterType;
  GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
  gfaFilter->SetInput(itkvol);
  gfaFilter->SetNumberOfThreads(4);
  gfaFilter->Update();
  itk::ImageRegionIterator<GfaFilterType::OutputImageType> 
    itGfa (gfaFilter->GetOutput(), 
    gfaFilter->GetOutput()->GetLargestPossibleRegion() );
  itGfa = itGfa.Begin();

  int numdir = m_Controls->m_QBallStandardAlgorithmsOrderSpinbox->value();

  typedef itk::DiffusionQballPrincipleDirectionsImageFilter<TTensorPixelType,TTensorPixelType,odfsize>
    PrincipleDirectionsFilterType;
  PrincipleDirectionsFilterType::Pointer principleDirectionsFilter
    = PrincipleDirectionsFilterType::New();
  principleDirectionsFilter->SetThreshold(m_Controls->m_QBallStandardAlgorithmsProbThreshEdit->text().toFloat());
  principleDirectionsFilter->SetNrDirectionToExtract(numdir);
  principleDirectionsFilter->SetInput(itkvol);
  principleDirectionsFilter->SetNumberOfThreads(m_Controls->m_QBallStandardAlgorithmsNumberThreadsSpinbox->value());
  principleDirectionsFilter->SetMultiplyGfa(false);
  principleDirectionsFilter->Update();

  itk::ImageRegionIterator<PrincipleDirectionsFilterType::OutputImageType> 
    it (principleDirectionsFilter->GetOutput(), 
    principleDirectionsFilter->GetOutput()->GetLargestPossibleRegion() );

  if(numdir == 0)
  {

    std::cout << "Converting to RGB" << std::endl;

    typedef itk::Image<itk::RGBPixel<uchar>, 3> VecImgType;
    VecImgType::Pointer vecImg = VecImgType::New();
    vecImg->SetSpacing( itkvol->GetSpacing() );   // Set the image spacing
    vecImg->SetOrigin( itkvol->GetOrigin() );     // Set the image origin
    vecImg->SetDirection( itkvol->GetDirection() );  // Set the image direction
    vecImg->SetLargestPossibleRegion( itkvol->GetLargestPossibleRegion());
    vecImg->SetBufferedRegion( vecImg->GetLargestPossibleRegion() );
    vecImg->Allocate();
    itk::ImageRegionIterator<VecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
    ot = ot.Begin();

    typedef PrincipleDirectionsFilterType::OutputImageType::PixelType VecPixType;
    for (it = it.Begin(); !it.IsAtEnd(); ++it)
    {
      VecPixType vec = it.Get();

      itk::RGBPixel<uchar> pix;
      vec*=200*itGfa.Get();
      vec[0] = abs(vec[0]);
      vec[1] = abs(vec[1]);
      vec[2] = abs(vec[2]);
      if(vec[0] > 255 || vec[1] > 255 || vec[2] > 255)
      {
        // should never get in here
        double max = vec[0];
        max = max<vec[1] ? vec[1] : max;
        max = max<vec[2] ? vec[2] : max;
        vec /= max/255;
      }
      uchar uvec[3] = {(uchar)(vec[0]),(uchar)(vec[1]),(uchar)(vec[2])};
      pix = uvec;
      ot.Set(pix);
      ++ot;
      ++itGfa;
    }

    // RGB TO DATATREE
    mitk::Image::Pointer image2 = mitk::Image::New();
    image2->InitializeByItk( vecImg.GetPointer() );
    image2->SetVolume( vecImg->GetBufferPointer() );
    mitk::DataNode::Pointer node2=mitk::DataNode::New();
    node2->SetData( image2 );
    mitk::DataStorage::GetInstance()->Add(node2);
    switch(numdir)
    {
    case 0:
      {
        SetDefaultNodeProperties(node2, nodename.append(" PD0"));
        break;
      }
    case 1:
      {
        SetDefaultNodeProperties(node2, nodename.append(" PD1"));
      }
    case 2:
      {
        SetDefaultNodeProperties(node2, nodename.append(" PD2"));
      }
    default:
      {
        SetDefaultNodeProperties(node2, nodename.append(" PDn"));
      }
    }

    node2->SetProperty( "IsRGBVolume", mitk::BoolProperty::New( true ) );

  }

  // VECTORFIELD
  std::cout << "Converting to Vectorfield" << std::endl;

  typedef itk::Image<itk::Vector<TTensorPixelType,3>, 3> VecImgType2;
  VecImgType2::Pointer vecImg5 = VecImgType2::New();
  vecImg5->SetSpacing( itkvol->GetSpacing() );   // Set the image spacing
  vecImg5->SetOrigin( itkvol->GetOrigin() );     // Set the image origin
  vecImg5->SetDirection( itkvol->GetDirection() );  // Set the image direction
  vecImg5->SetLargestPossibleRegion( itkvol->GetLargestPossibleRegion());
  vecImg5->SetBufferedRegion( vecImg5->GetLargestPossibleRegion() );
  vecImg5->Allocate();
  itk::ImageRegionIterator<VecImgType2> ot5 (vecImg5, vecImg5->GetLargestPossibleRegion() );
  ot5 = ot5.Begin();

  typedef PrincipleDirectionsFilterType::OutputImageType::PixelType VecPixType;
  for (it = it.Begin(); !it.IsAtEnd(); ++it)
  {
    VecPixType vec = it.Get();
    itk::Vector<TTensorPixelType,3> pix;
    TTensorPixelType uvec[3] = {(TTensorPixelType)(vec[0]),(TTensorPixelType)(vec[1]),(TTensorPixelType)(vec[2])};
    pix = uvec;
    ot5.Set(pix);
    ++ot5;
  }

  // Vectors TO DATATREE
  mitk::Image::Pointer image5 = mitk::Image::New();
  image5->InitializeByItk( vecImg5.GetPointer() );
  image5->SetVolume( vecImg5->GetBufferPointer() );
  mitk::DataNode::Pointer node5=mitk::DataNode::New();
  node5->SetData( image5 );
  mitk::DataStorage::GetInstance()->Add(node5);
  switch(numdir)
  {
  case 0:
    {
      SetDefaultNodeProperties(node5, nodename.append(" Vec0"));
      break;
    }
  case 1:
    {
      SetDefaultNodeProperties(node5, nodename.append(" Vec1"));
    }
  case 2:
    {
      SetDefaultNodeProperties(node5, nodename.append(" Vec2"));
    }
  default:
    {
      SetDefaultNodeProperties(node5, nodename.append(" Vecn"));
    }
  }
  node5->SetProperty( "IsDirectionVolume", mitk::BoolProperty::New( true ) );
  node5->SetProperty( "NormalizeVecs", mitk::BoolProperty::New( true ) );
  node5->SetProperty( "Scale", mitk::FloatProperty::New( 0.8 ) );
  node5->SetProperty( "LineWidth", mitk::FloatProperty::New( 3 ) );

  mitk::VectorImageMapper2D::Pointer vecMapper5 = 
    mitk::VectorImageMapper2D::New();
  node5->SetMapper(1,vecMapper5);

  m_DataTreeIterator->GetTree()->Modified();
  m_MultiWidget->RequestUpdate();
  TreeChanged();
  m_Controls->update();

}

void QmitkDiffusionTensorEstimation::QBallStandardAlgorithmsDeconvolutionButton() 
{

  itk::TimeProbe clock;
  QString status;
  const mitk::DataTreeFilter::Item* item
    = m_QballVolumesDataTreeFilter->GetSelectedItem();
  if(!item)return;

  typedef itk::Vector<TTensorPixelType,odfsize> OdfVectorType;
  typedef itk::Image<OdfVectorType,3> OdfVectorImgType;
  mitk::Image* vol = 
    static_cast<mitk::Image*>(item->GetNode()->GetData());
  OdfVectorImgType::Pointer itkvol = OdfVectorImgType::New();
  mitk::CastToItkImage<OdfVectorImgType>(vol, itkvol);
  std::string nodename = item->GetProperty("name");

  clock.Start();
  std::cout << "Computing Diffusion Direction ";
  mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
    "Computing Diffusion Direction for %s", nodename.c_str()));

  typedef itk::DiffusionQballGeneralizedFaImageFilter<TTensorPixelType,TTensorPixelType,odfsize>
    GfaFilterType;
  GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
  gfaFilter->SetInput(itkvol);
  gfaFilter->SetNumberOfThreads(4);
  gfaFilter->Update();
  itk::ImageRegionIterator<GfaFilterType::OutputImageType> 
    itGfa (gfaFilter->GetOutput(), 
    gfaFilter->GetOutput()->GetLargestPossibleRegion() );
  itGfa = itGfa.Begin();

  int numdirs = m_Controls->m_QBallStandardAlgorithmsDeconvolutionSpinbox->value();

  //vnl_matrix_fixed<double, nrconvkernels, odfsize>* kernels
  //  = new vnl_matrix_fixed<double, nrconvkernels, odfsize>();
  //itk::ImageRegionIterator<OdfVectorImgType> inIt(itkvol, itkvol->GetLargestPossibleRegion());
  //inIt.GoToBegin();
  //for(int i=0; i<nrconvkernels; i++)
  //{
  //  OdfVectorImgType::PixelType vec = inIt.Get();
  //  for(int j=0; j<odfsize; j++)
  //  {
  //    (*kernels)(i,j) = vec[j];
  //  }
  //  ++inIt;
  //}

  typedef itk::DiffusionQballSphericalDeconvolutionImageFilter<TTensorPixelType,TTensorPixelType,odfsize,nrconvkernels>
    DeconvolutionFilterType;
  DeconvolutionFilterType::Pointer devonvolutionFilter
    = DeconvolutionFilterType::New();
  devonvolutionFilter->SetFractionalThreshold(m_Controls->m_QBallStandardAlgorithmsDeconvolutionThreshEdit->text().toFloat());
  if(!m_Controls->m_QBallStandardAlgorithmsDeconvolutionAngResThresholdEdit->text().contains(QString("NaN")))
  {
    float angRes = m_Controls->m_QBallStandardAlgorithmsDeconvolutionAngResThresholdEdit->text().toFloat();
    angRes /= 360/DIFF_EST_PI;
    devonvolutionFilter->SetAngularResolutionThreshold(angRes);
  }
  devonvolutionFilter->SetSamplingQuantileStart(m_Controls->m_QBallStandardAlgorithmsDeconvQuantStart->text().toFloat());
  devonvolutionFilter->SetSamplingQuantileStep(m_Controls->m_QBallStandardAlgorithmsDeconvQuantStep->text().toFloat());
  devonvolutionFilter->SetMinimumNumberOfSamples(m_Controls->m_QBallStandardAlgorithmsDeconvQuantMinNr->text().toInt());
  devonvolutionFilter->SetIterateQuantiles(m_Controls->m_QBallStandardAlgorithmsDeconvQuantMulti->isChecked());
  devonvolutionFilter->SetNrDirectionsToExtract(numdirs);
  devonvolutionFilter->SetInput(itkvol);
  devonvolutionFilter->SetNumberOfThreads(m_Controls->m_QBallStandardAlgorithmsDeconvNumberThreadsSpinbox->value());
  devonvolutionFilter->SetGfaImage(gfaFilter->GetOutput());
  //devonvolutionFilter->SetPresetConvolutionKernels(kernels);
  devonvolutionFilter->Update();

  for(int i=0; i<numdirs; i++)
  {
    itk::ImageRegionIterator<DeconvolutionFilterType::OutputImageType> 
      it (devonvolutionFilter->GetOutput(i), 
      devonvolutionFilter->GetOutput()->GetLargestPossibleRegion() );
    it = it.Begin();

    if(i==0)
    {
      std::cout << "Converting to RGB" << std::endl;

      typedef itk::Image<itk::RGBPixel<uchar>, 3> VecImgType;
      VecImgType::Pointer vecImg = VecImgType::New();
      vecImg->SetSpacing( itkvol->GetSpacing() );   // Set the image spacing
      vecImg->SetOrigin( itkvol->GetOrigin() );     // Set the image origin
      vecImg->SetDirection( itkvol->GetDirection() );  // Set the image direction
      vecImg->SetLargestPossibleRegion( itkvol->GetLargestPossibleRegion());
      vecImg->SetBufferedRegion( vecImg->GetLargestPossibleRegion() );
      vecImg->Allocate();
      itk::ImageRegionIterator<VecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
      ot = ot.Begin();

      typedef DeconvolutionFilterType::OutputImageType::PixelType VecPixType;
      for (it = it.Begin(); !it.IsAtEnd(); ++it)
      {
        VecPixType vec = it.Get();
        vnl_vector_fixed<TTensorPixelType, 3> vnlvec = vec.GetVnlVector();
        TTensorPixelType len = vnlvec.two_norm();
        vnlvec = vnlvec.normalize();

        itk::RGBPixel<uchar> pix;
        vnlvec*=200*itGfa.Get();
        vnlvec[0] = abs(vnlvec[0]);
        vnlvec[1] = abs(vnlvec[1]);
        vnlvec[2] = abs(vnlvec[2]);
        if(vnlvec[0] > 255 || vnlvec[1] > 255 || vnlvec[2] > 255)
        {
          //should never get in here
          double max = vnlvec[0];
          max = max<vnlvec[1] ? vnlvec[1] : max;
          max = max<vnlvec[2] ? vnlvec[2] : max;
          vnlvec /= max/255;
        }
        uchar uvec[3] = {(uchar)(vnlvec[0]),(uchar)(vnlvec[1]),(uchar)(vnlvec[2])};
        pix = uvec;
        ot.Set(pix);
        ++ot;
        ++itGfa;
      }

      // RGB TO DATATREE
      mitk::Image::Pointer image2 = mitk::Image::New();
      image2->InitializeByItk( vecImg.GetPointer() );
      image2->SetVolume( vecImg->GetBufferPointer() );
      mitk::DataNode::Pointer node2=mitk::DataNode::New();
      node2->SetData( image2 );
      mitk::DataStorage::GetInstance()->Add(node2);
      switch(i)
      {
      case 0:
        {
          SetDefaultNodeProperties(node2, nodename.append(" PD0"));
          break;
        }
      case 1:
        {
          SetDefaultNodeProperties(node2, nodename.append(" PD1"));
          break;
        }
      case 2:
        {
          SetDefaultNodeProperties(node2, nodename.append(" PD2"));
          break;
        }
      default:
        {
          SetDefaultNodeProperties(node2, nodename.append(" PDn"));
          break;
        }
      }

      node2->SetProperty( "IsRGBVolume", mitk::BoolProperty::New( true ) );
    }

    // VECTORFIELD
    std::cout << "Converting to Vectorfield" << std::endl;

    typedef itk::Image<itk::Vector<TTensorPixelType,3>, 3> VecImgType2;
    VecImgType2::Pointer vecImg5 = VecImgType2::New();
    vecImg5->SetSpacing( itkvol->GetSpacing() );   // Set the image spacing
    vecImg5->SetOrigin( itkvol->GetOrigin() );     // Set the image origin
    vecImg5->SetDirection( itkvol->GetDirection() );  // Set the image direction
    vecImg5->SetLargestPossibleRegion( itkvol->GetLargestPossibleRegion());
    vecImg5->SetBufferedRegion( vecImg5->GetLargestPossibleRegion() );
    vecImg5->Allocate();
    itk::ImageRegionIterator<VecImgType2> ot5 (vecImg5, vecImg5->GetLargestPossibleRegion() );
    ot5 = ot5.Begin();

    typedef DeconvolutionFilterType::OutputImageType::PixelType VecPixType;
    for (it = it.Begin(); !it.IsAtEnd(); ++it)
    {
      VecPixType vec = it.Get();
      vnl_vector_fixed<TTensorPixelType, 3> vnlvec = vec.GetVnlVector();
      vnlvec = vnlvec.normalize();

      itk::Vector<TTensorPixelType,3> pix;
      TTensorPixelType uvec[3] = {(TTensorPixelType)(vnlvec[0]),(TTensorPixelType)(vnlvec[1]),(TTensorPixelType)(vnlvec[2])};
      pix = uvec;
      ot5.Set(pix);
      ++ot5;
    }

    // Vectors TO DATATREE
    mitk::Image::Pointer image5 = mitk::Image::New();
    image5->InitializeByItk( vecImg5.GetPointer() );
    image5->SetVolume( vecImg5->GetBufferPointer() );
    mitk::DataNode::Pointer node5=mitk::DataNode::New();
    node5->SetData( image5 );
    mitk::DataStorage::GetInstance()->Add(node5);
    switch(i)
    {
    case 0:
      {
        SetDefaultNodeProperties(node5, nodename.append(" Vec0"));
        break;
      }
    case 1:
      {
        SetDefaultNodeProperties(node5, nodename.append(" Vec1"));
        break;
      }
    case 2:
      {
        SetDefaultNodeProperties(node5, nodename.append(" Vec2"));
        break;
      }
    default:
      {
        SetDefaultNodeProperties(node5, nodename.append(" Vecn"));
        break;
      }
    }
    node5->SetProperty( "IsDirectionVolume", mitk::BoolProperty::New( true ) );
    node5->SetProperty( "NormalizeVecs", mitk::BoolProperty::New( true ) );
    node5->SetProperty( "Scale", mitk::FloatProperty::New( 0.8 ) );
    node5->SetProperty( "LineWidth", mitk::FloatProperty::New( 3 ) );

    mitk::VectorImageMapper2D::Pointer vecMapper5 = 
      mitk::VectorImageMapper2D::New();
    node5->SetMapper(1,vecMapper5);
  }
  m_DataTreeIterator->GetTree()->Modified();
  m_MultiWidget->RequestUpdate();
  TreeChanged();
  m_Controls->update();

}

void QmitkDiffusionTensorEstimation::SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name)
{
  node->SetProperty( "volumerendering", mitk::BoolProperty::New( false ) );
  node->SetProperty( "use color", mitk::BoolProperty::New( true ) );
  node->SetProperty( "texture interpolation", mitk::BoolProperty::New( true ) );
  node->SetProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() );
  node->SetProperty( "layer", mitk::IntProperty::New(0));
  node->SetProperty( "in plane resample extent by geometry", mitk::BoolProperty::New( false ) );
  node->SetOpacity(1.0f);
  node->SetColor(1.0,1.0,1.0);  
  node->SetVisibility(true);

  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  mitk::LevelWindow levelwindow;
  //  levelwindow.SetAuto( image );
  levWinProp->SetLevelWindow( levelwindow );
  node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );

  // add a default rainbow lookup table for color mapping
  if(!node->GetProperty("LookupTable"))
  {
    mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
    vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
    vtkLut->SetHueRange(0.6667, 0.0);
    vtkLut->SetTableRange(0.0, 20.0);
    vtkLut->Build();
    mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
    mitkLutProp->SetLookupTable(mitkLut);
    node->SetProperty( "LookupTable", mitkLutProp );
  }
  if(!node->GetProperty("binary"))
    node->SetProperty( "binary", mitk::BoolProperty::New( false ) );

  // add a default transfer function
  mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
  node->SetProperty ( "TransferFunction", mitk::TransferFunctionProperty::New ( tf.GetPointer() ) );

  // set foldername as string property
  mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New( name );
  node->SetProperty( "name", nameProp );
}

void QmitkDiffusionTensorEstimation::DirectionVolumesAngularErrorButton()
{
  try
  {

    const mitk::DataTreeFilter::ItemSet* selectedItems
      = m_DirectionVolumesDataTreeFilter->GetSelectedItems();
    int nrFiles = selectedItems->size();
    if (nrFiles != 2) return;

    mitk::DataTreeFilter::ItemSet::const_iterator itemiter( selectedItems->begin() ); 

    mitk::Image::Pointer vol1 = 
      static_cast<mitk::Image*>((*itemiter)->GetNode()->GetData());
    if( !vol1)return;
    std::string nodename1 = (*itemiter)->GetProperty("name");

    itemiter++;

    mitk::Image::Pointer vol2 = 
      static_cast<mitk::Image*>((*itemiter)->GetNode()->GetData());
    if( !vol2)return;
    std::string nodename2 = (*itemiter)->GetProperty("name");

    typedef itk::Image<itk::Vector<TTensorPixelType,3>,3 > IType;
    IType::Pointer itkVol1 = IType::New();
    mitk::CastToItkImage<IType>(vol1, itkVol1);

    IType::Pointer itkVol2 = IType::New();
    mitk::CastToItkImage<IType>(vol2, itkVol2);

    typedef itk::VectorImagesAngularErrorImageFilter<TTensorPixelType,3> FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(itkVol1);
    filter->SetImage2(itkVol2.GetPointer());
    filter->SetNumberOfThreads(4);
    filter->Update();

    // Angluar Error TO DATATREE
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( filter->GetOutput() );
    image->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( image );
    mitk::DataStorage::GetInstance()->Add(node);
    SetDefaultNodeProperties(node, nodename1.append(" ").append(nodename2).append(" ERR"));
    node->SetProperty( "IsErrorVolume", mitk::BoolProperty::New( true ) );

    TreeChanged();
    m_Controls->update();

    QString status;
    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished computing Angular Error"));

  }
  catch (itk::ExceptionObject &ex)
  {
    std::cout << ex << std::endl;
    return ;
  }

}

//void QmitkDiffusionTensorEstimation::DwiStandardAlgorithmsGFAButton()
//{
//
//  itk::TimeProbe clock;
//  QString status;
//  const mitk::DataTreeFilter::Item* item
//    = m_DiffusionVolumesDataTreeFilter->GetSelectedItem();
//  if(!item)return;
//
//  typedef itk::Vector<TTensorPixelType,odfsize> OdfVectorType;
//  typedef itk::Image<OdfVectorType,3> OdfVectorImgType;
//  mitk::Image* vol = 
//    static_cast<mitk::Image*>(item->GetNode()->GetData());
//  OdfVectorImgType::Pointer itkvol = OdfVectorImgType::New();
//  mitk::CastToItkImage<OdfVectorImgType>(vol, itkvol);
//  std::string nodename = item->GetProperty("name");
//
//  // COMPUTE RA
//  clock.Start();
//  std::cout << "Computing GFA ";
//  mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
//    "Computing GFA for %s", nodename.c_str()));
//  typedef OdfVectorType::ValueType                 RealValueType;
//  typedef itk::Image< RealValueType, 3 >                 RAImageType;
//  typedef itk::DiffusionQballGeneralizedFaImageFilter<TTensorPixelType,TTensorPixelType,odfsize>
//    GfaFilterType;
//  GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
//  gfaFilter->SetInput(itkvol);
//  gfaFilter->SetNumberOfThreads(4);
//  switch(m_Controls->m_QBallStandardAlgorithmsGFAMethodSpinbox->value())
//  {
//  case 1:
//    {
//      gfaFilter->SetComputationMethod(GfaFilterType::STANDARD);
//      break;
//    }
//  case 2:
//    {
//      gfaFilter->SetComputationMethod(GfaFilterType::QUANTILES_HIGH_LOW);
//      break;
//    }
//  case 3:
//    {
//      gfaFilter->SetComputationMethod(GfaFilterType::QUANTILES_MIDDLE);
//      break;
//    }
//  case 4:
//    {
//      gfaFilter->SetComputationMethod(GfaFilterType::MAX_ODF_VALUE);
//      break;
//    }
//  case 5:
//    {
//      gfaFilter->SetComputationMethod(GfaFilterType::DECONVOLUTION_COEFFS);
//      break;
//    }
//  default:
//    {
//      gfaFilter->SetComputationMethod(GfaFilterType::STANDARD);
//    }
//  }
//  gfaFilter->Update();
//  clock.Stop();
//  std::cout << "took " << clock.GetMeanTime() << "s." << std::endl;
//
//  typedef itk::Image<TTensorPixelType, 3> ImgType;
//  ImgType::Pointer img = ImgType::New();
//  img->SetSpacing( gfaFilter->GetOutput()->GetSpacing() );   // Set the image spacing
//  img->SetOrigin( gfaFilter->GetOutput()->GetOrigin() );     // Set the image origin
//  img->SetDirection( gfaFilter->GetOutput()->GetDirection() );  // Set the image direction
//  img->SetLargestPossibleRegion( gfaFilter->GetOutput()->GetLargestPossibleRegion());
//  img->SetBufferedRegion( gfaFilter->GetOutput()->GetLargestPossibleRegion() );
//  img->Allocate();
//  itk::ImageRegionIterator<ImgType> ot (img, img->GetLargestPossibleRegion() );
//  ot = ot.Begin();
//  itk::ImageRegionConstIterator<GfaFilterType::OutputImageType> it 
//    (gfaFilter->GetOutput(), gfaFilter->GetOutput()->GetLargestPossibleRegion() );
//  it = it.Begin();
//
//  for (it = it.Begin(); !it.IsAtEnd(); ++it)
//  {
//    GfaFilterType::OutputImageType::PixelType val = it.Get();
//    ot.Set(val * 200);
//    ++ot;
//  }
//
//
//  // GFA TO DATATREE
//  mitk::Image::Pointer image = mitk::Image::New();
//  image->InitializeByItk( img.GetPointer() );
//  image->SetVolume( img->GetBufferPointer() );
//  mitk::DataNode::Pointer node=mitk::DataNode::New();
//  node->SetData( image );
//  mitk::DataStorage::GetInstance()->Add(node);
//  SetDefaultNodeProperties(node, nodename.append(" GFA"));
//  node->SetProperty( "IsGFAVolume", mitk::BoolProperty::New( true ) );
//
//  mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");
//
//  m_DataTreeIterator->GetTree()->Modified();
//  m_MultiWidget->RequestUpdate();
//  TreeChanged();
//  m_Controls->update();
//
//}

void QmitkDiffusionTensorEstimation::DiffusionVolumeSaveButton() 
{
  // GET SELECTED ITEM
  const mitk::DataTreeFilter::Item* selectedItem
    = m_DiffusionVolumesDataTreeFilter->GetSelectedItem();
  if( !selectedItem )
    return;
  mitk::DiffusionVolumes<DiffusionPixelType>::Pointer diffVolumes = 
    static_cast<mitk::DiffusionVolumes<DiffusionPixelType>*>(selectedItem->GetNode()->GetData());

  std::string sName = selectedItem->GetNode()->GetName();
  QString qName;
  qName.sprintf("%s.nhdr",sName.c_str());

  // SELECT FILE DIALOG
  //QFileDialog::getSaveFileName()
  //QFileDialog* w = new QFileDialog( this->m_Controls, "Select Nrrd Outputfile", TRUE );
  //w->setMode( QFileDialog::AnyFile );
  //w->setFilter( "Nrrd Images (*.nrrd *.nhdr)" );
  //w->setName(qName);
  //if ( w->exec() != QDialog::Accepted )
  //  return;
  //QString filename = w->selectedFile();

  QString filename = QFileDialog::getSaveFileName(
    qName,
    "Nrrd Images (*.nrrd *.nhdr)",
    this->m_Controls,
    "save file dialog",
    "Select Nrrd Outputfile" );

  if ( !filename )
    return;

  // WRITING DWIs TO NRRD VOLUME
  typedef mitk::NrrdDiffusionVolumesWriter<DiffusionImageType> WriterType;
  WriterType::Pointer nrrdWriter = WriterType::New();
  nrrdWriter->SetInput( diffVolumes->GetImage() );
  nrrdWriter->SetDirections(diffVolumes->GetDirections());
  nrrdWriter->SetB_Value(diffVolumes->GetB_Value());
  nrrdWriter->SetFileName(filename.ascii());
  try
  {
    nrrdWriter->Update();
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e << std::endl;
  }
}

void QmitkDiffusionTensorEstimation::DiffusionVolumesLoadButton() 
{
  // SELECT FOLDER DIALOG
  QFileDialog* w = new QFileDialog( this->m_Controls, "Select DWI data file", TRUE );
  w->setMode( QFileDialog::ExistingFiles );
  w->setFilter( "Nrrd Images (*.nrrd *.nhdr)" );

  // RETRIEVE SELECTION
  if ( w->exec() != QDialog::Accepted )
    return;

  QStringList filenames = w->selectedFiles();
  QStringList::Iterator it = filenames.begin();
  while( it != filenames.end() ) {
    std::string filename = ( *it ).ascii();
    ++it;

    // READING NRRD DWI VOLUMES
    typedef mitk::NrrdDiffusionVolumesReader<DiffusionImageType> ReaderType;
    ReaderType::Pointer nrrdReader = ReaderType::New();
    nrrdReader->SetFileName(filename);
    try
    {
      nrrdReader->Update();

      // DWI TO DATATREE
      typedef mitk::DiffusionVolumes<DiffusionPixelType> DiffVolumesType;
      DiffVolumesType::Pointer diffVolumes = DiffVolumesType::New();
      diffVolumes->SetDirections(nrrdReader->GetDiffusionVectors());
      diffVolumes->SetB_Value(nrrdReader->GetB_Value());
      diffVolumes->SetImage(nrrdReader->GetOutput());
      mitk::DataNode::Pointer node=mitk::DataNode::New();
      node->SetData( diffVolumes );
      mitk::DataStorage::GetInstance()->Add(node);
      SetDefaultNodeProperties(node, itksys::SystemTools::GetFilenameName(filename));

      TreeChanged();
    }
    catch (itk::ExceptionObject e)
    {
      std::cout << e << std::endl;
    }

  }
}

void QmitkDiffusionTensorEstimation::DiffusionVolumesRemoveButton()
{
  m_DiffusionVolumesDataTreeFilter->DeleteSelectedItems();
}

void QmitkDiffusionTensorEstimation::DiffusionVolumesSelectAll()
{
  const mitk::DataTreeFilter::ItemList* items = m_DiffusionVolumesDataTreeFilter->GetItems();

  mitk::DataTreeFilter::ConstItemIterator itemiter( items->Begin() ); 
  mitk::DataTreeFilter::ConstItemIterator itemiterend( items->End() ); 

  while ( itemiter != itemiterend )
  {
    m_DiffusionVolumesDataTreeFilter->SelectItem(*itemiter);
    ++itemiter; 
  }

}