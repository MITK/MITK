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

//#define MBILOG_ENABLE_DEBUG

#include "QmitkPreprocessingView.h"
#include "mitkDiffusionImagingConfigure.h"

// qt includes
#include <QMessageBox>

// itk includes
#include "itkTimeProbe.h"
#include "itkB0ImageExtractionImageFilter.h"
#include "itkB0ImageExtractionToSeparateImageFilter.h"
#include "itkBrainMaskExtractionImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkVectorContainer.h"
#include <itkElectrostaticRepulsionDiffusionGradientReductionFilter.h>
#include <itkMergeDiffusionImagesFilter.h>
#include <itkDwiGradientLengthCorrectionFilter.h>

// Multishell includes
#include <itkRadialMultishellToSingleshellImageFilter.h>

// Multishell Functors
#include <itkADCAverageFunctor.h>
#include <itkKurtosisFitFunctor.h>
#include <itkBiExpFitFunctor.h>
#include <itkADCFitFunctor.h>

// mitk includes
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkNodePredicateDataType.h"
#include "mitkProperties.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunction.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include <mitkPointSet.h>
#include <itkAdcImageFilter.h>

#include <QTableWidgetItem>
#include <QTableWidget>

const std::string QmitkPreprocessingView::VIEW_ID =
    "org.mitk.views.preprocessing";

#define DI_INFO MITK_INFO("DiffusionImaging")


typedef float TTensorPixelType;


QmitkPreprocessingView::QmitkPreprocessingView()
  : QmitkFunctionality(),
    m_Controls(NULL),
    m_MultiWidget(NULL),
    m_DiffusionImage(NULL)
{
}

QmitkPreprocessingView::QmitkPreprocessingView(const QmitkPreprocessingView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QmitkPreprocessingView::~QmitkPreprocessingView()
{

}

void QmitkPreprocessingView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkPreprocessingViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_MeasurementFrameTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_Controls->m_MeasurementFrameTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  }
}

void QmitkPreprocessingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkPreprocessingView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkPreprocessingView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ButtonAverageGradients), SIGNAL(clicked()), this, SLOT(AverageGradients()) );
    connect( (QObject*)(m_Controls->m_ButtonExtractB0), SIGNAL(clicked()), this, SLOT(ExtractB0()) );
    connect( (QObject*)(m_Controls->m_ModifyMeasurementFrame), SIGNAL(clicked()), this, SLOT(DoApplyMesurementFrame()) );
    connect( (QObject*)(m_Controls->m_ReduceGradientsButton), SIGNAL(clicked()), this, SLOT(DoReduceGradientDirections()) );
    connect( (QObject*)(m_Controls->m_ShowGradientsButton), SIGNAL(clicked()), this, SLOT(DoShowGradientDirections()) );
    connect( (QObject*)(m_Controls->m_MirrorGradientToHalfSphereButton), SIGNAL(clicked()), this, SLOT(DoHalfSphereGradientDirections()) );
    connect( (QObject*)(m_Controls->m_MergeDwisButton), SIGNAL(clicked()), this, SLOT(MergeDwis()) );
    connect( (QObject*)(m_Controls->m_AdcSignalAverage), SIGNAL(clicked()), this, SLOT(DoADCAverage()) );
    //connect( (QObject*)(m_Controls->m_AdcSignalFit), SIGNAL(clicked()), this, SLOT(DoADCFit()) );
    connect( (QObject*)(m_Controls->m_AkcSignalFit), SIGNAL(clicked()), this, SLOT(DoAKCFit()) );
    connect( (QObject*)(m_Controls->m_BiExpSignalFit), SIGNAL(clicked()), this, SLOT(DoBiExpFit()) );
    connect( (QObject*)(m_Controls->m_B_ValueMap_Rounder_SpinBox), SIGNAL(valueChanged(int)), this, SLOT(UpdateDwiBValueMapRounder(int)));
    connect( (QObject*)(m_Controls->m_CreateLengthCorrectedDwi), SIGNAL(clicked()), this, SLOT(DoLengthCorrection()) );
    connect( (QObject*)(m_Controls->m_CalcAdcButton), SIGNAL(clicked()), this, SLOT(DoAdcCalculation()) );
  }
}

void QmitkPreprocessingView::DoLengthCorrection()
{
  if (m_DiffusionImage.IsNull())
    return;

  typedef mitk::DiffusionImage<DiffusionPixelType>  DiffusionImageType;
  typedef itk::DwiGradientLengthCorrectionFilter  FilterType;

  FilterType::Pointer filter = FilterType::New();
  filter->SetRoundingValue( m_Controls->m_B_ValueMap_Rounder_SpinBox->value());
  filter->SetReferenceBValue(m_DiffusionImage->GetB_Value());
  filter->SetReferenceGradientDirectionContainer(m_DiffusionImage->GetDirections());
  filter->Update();

  DiffusionImageType::Pointer image = DiffusionImageType::New();
  image->SetVectorImage( m_DiffusionImage->GetVectorImage());
  image->SetB_Value( filter->GetNewBValue() );
  image->SetDirections( filter->GetOutputGradientDirectionContainer());
  image->InitializeFromVectorImage();

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( image );
  QString name = m_SelectedDiffusionNodes.front()->GetName().c_str();

  imageNode->SetName((name+"_rounded").toStdString().c_str());
  GetDefaultDataStorage()->Add(imageNode);
}

void QmitkPreprocessingView::UpdateDwiBValueMapRounder(int i)
{
  if (m_DiffusionImage.IsNull())
    return;
  m_DiffusionImage->UpdateBValueMap();
  UpdateBValueTableWidget(i);
}

void QmitkPreprocessingView::CallMultishellToSingleShellFilter(itk::DWIVoxelFunctor * functor, mitk::DiffusionImage<DiffusionPixelType>::Pointer ImPtr, QString imageName)
{
  typedef itk::RadialMultishellToSingleshellImageFilter<DiffusionPixelType, DiffusionPixelType> FilterType;

  // filter input parameter
  const mitk::DiffusionImage<DiffusionPixelType>::BValueMap
      &originalShellMap  = ImPtr->GetB_ValueMap();

  const mitk::DiffusionImage<DiffusionPixelType>::ImageType
      *vectorImage       = ImPtr->GetVectorImage();

  const mitk::DiffusionImage<DiffusionPixelType>::GradientDirectionContainerType::Pointer
      gradientContainer = ImPtr->GetDirections();

  const unsigned int
      &bValue            = ImPtr->GetB_Value();

  mitk::DataNode::Pointer imageNode = 0;

  // filter call
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(vectorImage);
  filter->SetOriginalGradientDirections(gradientContainer);
  filter->SetOriginalBValueMap(originalShellMap);
  filter->SetOriginalBValue(bValue);
  filter->SetFunctor(functor);
  filter->Update();

  // create new DWI image
  mitk::DiffusionImage<DiffusionPixelType>::Pointer outImage = mitk::DiffusionImage<DiffusionPixelType>::New();
  outImage->SetVectorImage( filter->GetOutput() );
  outImage->SetB_Value( m_Controls->m_targetBValueSpinBox->value() );
  outImage->SetDirections( filter->GetTargetGradientDirections() );
  outImage->InitializeFromVectorImage();

  imageNode = mitk::DataNode::New();
  imageNode->SetData( outImage );
  imageNode->SetName(imageName.toStdString().c_str());
  GetDefaultDataStorage()->Add(imageNode);

  if(m_Controls->m_OutputRMSErrorImage->isChecked()){
    // create new Error image
    FilterType::ErrorImageType::Pointer errImage = filter->GetErrorImage();
    mitk::Image::Pointer mitkErrImage = mitk::Image::New();
    mitkErrImage->InitializeByItk<FilterType::ErrorImageType>(errImage);
    mitkErrImage->SetVolume(errImage->GetBufferPointer());

    imageNode = mitk::DataNode::New();
    imageNode->SetData( mitkErrImage );
    imageNode->SetName((imageName+"_Error").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode);
  }
}

void QmitkPreprocessingView::DoBiExpFit()
{
  itk::BiExpFitFunctor::Pointer functor = itk::BiExpFitFunctor::New();

  for (int i=0; i<m_SelectedDiffusionNodes.size(); i++)
  {
    mitk::DiffusionImage<DiffusionPixelType>::Pointer inImage =
        dynamic_cast< mitk::DiffusionImage<DiffusionPixelType>* >(m_SelectedDiffusionNodes.at(i)->GetData());

    QString name(m_SelectedDiffusionNodes.at(i)->GetName().c_str());

    const mitk::DiffusionImage<DiffusionPixelType>::BValueMap & originalShellMap = inImage->GetB_ValueMap();
    mitk::DiffusionImage<DiffusionPixelType>::BValueMap::const_iterator it = originalShellMap.begin();
    ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
    vnl_vector<double> bValueList(originalShellMap.size()-1);
    while(it != originalShellMap.end())
      bValueList.put(s++,(it++)->first);

    const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
    functor->setListOfBValues(bValueList);
    functor->setTargetBValue(targetBValue);
    CallMultishellToSingleShellFilter(functor,inImage,name + "_BiExp");
  }
}

void QmitkPreprocessingView::DoAKCFit()
{
  itk::KurtosisFitFunctor::Pointer functor = itk::KurtosisFitFunctor::New();

  for (int i=0; i<m_SelectedDiffusionNodes.size(); i++)
  {
    mitk::DiffusionImage<DiffusionPixelType>::Pointer inImage =
        dynamic_cast< mitk::DiffusionImage<DiffusionPixelType>* >(m_SelectedDiffusionNodes.at(i)->GetData());

    QString name(m_SelectedDiffusionNodes.at(i)->GetName().c_str());

    const mitk::DiffusionImage<DiffusionPixelType>::BValueMap & originalShellMap = inImage->GetB_ValueMap();
    mitk::DiffusionImage<DiffusionPixelType>::BValueMap::const_iterator it = originalShellMap.begin();
    ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
    vnl_vector<double> bValueList(originalShellMap.size()-1);
    while(it != originalShellMap.end())
      bValueList.put(s++,(it++)->first);

    const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
    functor->setListOfBValues(bValueList);
    functor->setTargetBValue(targetBValue);
    CallMultishellToSingleShellFilter(functor,inImage,name + "_AKC");
  }
}

void QmitkPreprocessingView::DoADCFit()
{
  // later
}

void QmitkPreprocessingView::DoADCAverage()
{
  itk::ADCAverageFunctor::Pointer functor = itk::ADCAverageFunctor::New();

  for (int i=0; i<m_SelectedDiffusionNodes.size(); i++)
  {
    mitk::DiffusionImage<DiffusionPixelType>::Pointer inImage =
        dynamic_cast< mitk::DiffusionImage<DiffusionPixelType>* >(m_SelectedDiffusionNodes.at(i)->GetData());

    QString name(m_SelectedDiffusionNodes.at(i)->GetName().c_str());

    const mitk::DiffusionImage<DiffusionPixelType>::BValueMap & originalShellMap = inImage->GetB_ValueMap();
    mitk::DiffusionImage<DiffusionPixelType>::BValueMap::const_iterator it = originalShellMap.begin();
    ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
    vnl_vector<double> bValueList(originalShellMap.size()-1);
    while(it != originalShellMap.end())
      bValueList.put(s++,(it++)->first);

    const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
    functor->setListOfBValues(bValueList);
    functor->setTargetBValue(targetBValue);
    CallMultishellToSingleShellFilter(functor,inImage,name + "_ADC");
  }
}

void QmitkPreprocessingView::DoAdcCalculation()
{
  if (m_DiffusionImage.IsNull())
    return;

  typedef mitk::DiffusionImage< DiffusionPixelType >            DiffusionImageType;
  typedef itk::AdcImageFilter< DiffusionPixelType, double >     FilterType;


  for (int i=0; i<m_SelectedDiffusionNodes.size(); i++)
  {
    DiffusionImageType::Pointer inImage = dynamic_cast< DiffusionImageType* >(m_SelectedDiffusionNodes.at(i)->GetData());
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(inImage->GetVectorImage());
    filter->SetGradientDirections(inImage->GetDirections());
    filter->SetB_value(inImage->GetB_Value());
    filter->Update();

    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( filter->GetOutput() );
    image->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = m_SelectedDiffusionNodes.at(i)->GetName().c_str();

    imageNode->SetName((name+"_ADC").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode);
  }
}


void QmitkPreprocessingView::UpdateBValueTableWidget(int i)
{

  foreach(QCheckBox * box, m_ReduceGradientCheckboxes)
  {
    m_Controls->m_ReductionFrame->layout()->removeWidget(box);
    delete box;
  }
  foreach(QSpinBox * box, m_ReduceGradientSpinboxes)
  {
    m_Controls->m_ReductionFrame->layout()->removeWidget(box);
    delete box;
  }
  m_ReduceGradientCheckboxes.clear();
  m_ReduceGradientSpinboxes.clear();

  if (m_DiffusionImage.IsNull())
  {
    m_Controls->m_B_ValueMap_TableWidget->clear();
    m_Controls->m_B_ValueMap_TableWidget->setRowCount(1);
    QStringList headerList;
    headerList << "b-Value" << "Number of gradients";
    m_Controls->m_B_ValueMap_TableWidget->setHorizontalHeaderLabels(headerList);
    m_Controls->m_B_ValueMap_TableWidget->setItem(0,0,new QTableWidgetItem("-"));
    m_Controls->m_B_ValueMap_TableWidget->setItem(0,1,new QTableWidgetItem("-"));
  }else{

    typedef mitk::DiffusionImage<short*>::BValueMap BValueMap;
    typedef mitk::DiffusionImage<short*>::BValueMap::iterator BValueMapIterator;

    BValueMapIterator it;

    BValueMap roundedBValueMap;
    GradientDirectionContainerType::ConstIterator gdcit;
    for( gdcit = m_DiffusionImage->GetDirections()->Begin(); gdcit != m_DiffusionImage->GetDirections()->End(); ++gdcit)
    {
      float currentBvalue = std::floor(m_DiffusionImage->GetB_Value(gdcit.Index()));
      unsigned int rounded = int((currentBvalue + 0.5 * i)/i)*i;
      roundedBValueMap[rounded].push_back(gdcit.Index());
    }

    m_Controls->m_B_ValueMap_TableWidget->clear();
    m_Controls->m_B_ValueMap_TableWidget->setRowCount(roundedBValueMap.size() );
    QStringList headerList;
    headerList << "b-Value" << "Number of gradients";
    m_Controls->m_B_ValueMap_TableWidget->setHorizontalHeaderLabels(headerList);


    QCheckBox* checkBox;
    QSpinBox* spinBox;
    int i = 0 ;
    for(it = roundedBValueMap.begin() ;it != roundedBValueMap.end(); it++)
    {
      m_Controls->m_B_ValueMap_TableWidget->setItem(i,0,new QTableWidgetItem(QString::number(it->first)));
      m_Controls->m_B_ValueMap_TableWidget->setItem(i,1,new QTableWidgetItem(QString::number(it->second.size())));

      // Reduce Gradients GUI adaption
      if(it->first != 0 && roundedBValueMap.size() > 1){
        checkBox = new QCheckBox(QString::number(it->first) + " with " + QString::number(it->second.size()) + " directions");
        checkBox->setEnabled(true);
        checkBox->setChecked(true);
        checkBox->setCheckable(true);
        m_ReduceGradientCheckboxes.push_back(checkBox);
        m_Controls->m_ReductionFrame->layout()->addWidget(checkBox);

        spinBox = new QSpinBox();
        spinBox->setMaximum(it->second.size());
        spinBox->setValue(std::ceil((float)it->second.size()));
        spinBox->setMinimum(0);
        m_ReduceGradientSpinboxes.push_back(spinBox);
        m_Controls->m_ReductionFrame->layout()->addWidget(spinBox);
      }
      i++;
    }
  }

}

void QmitkPreprocessingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  bool foundDwiVolume = false;
  m_DiffusionImage = NULL;
  m_SelectedDiffusionNodes.clear();

  // iterate selection
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData()) )
    {
      foundDwiVolume = true;
      m_DiffusionImage = dynamic_cast<mitk::DiffusionImage<DiffusionPixelType>*>(node->GetData());
      m_Controls->m_DiffusionImageLabel->setText(node->GetName().c_str());
      m_SelectedDiffusionNodes.push_back(node);
    }
  }

  m_Controls->m_ButtonAverageGradients->setEnabled(foundDwiVolume);
  m_Controls->m_ButtonExtractB0->setEnabled(foundDwiVolume);
  m_Controls->m_CheckExtractAll->setEnabled(foundDwiVolume);
  m_Controls->m_ModifyMeasurementFrame->setEnabled(foundDwiVolume);
  m_Controls->m_MeasurementFrameTable->setEnabled(foundDwiVolume);
  m_Controls->m_ReduceGradientsButton->setEnabled(foundDwiVolume);
  m_Controls->m_ShowGradientsButton->setEnabled(foundDwiVolume);
  m_Controls->m_MirrorGradientToHalfSphereButton->setEnabled(foundDwiVolume);
  m_Controls->m_MergeDwisButton->setEnabled(foundDwiVolume);
  m_Controls->m_B_ValueMap_Rounder_SpinBox->setEnabled(foundDwiVolume);
  //m_Controls->m_AdcSignalFit->setEnabled(foundDwiVolume);
  m_Controls->m_AdcSignalAverage->setEnabled(foundDwiVolume);
  m_Controls->m_AkcSignalFit->setEnabled(foundDwiVolume);
  m_Controls->m_BiExpSignalFit->setEnabled(foundDwiVolume);
  m_Controls->m_CreateLengthCorrectedDwi->setEnabled(foundDwiVolume);
  m_Controls->m_CalcAdcButton->setEnabled(foundDwiVolume);
  m_Controls->m_targetBValueSpinBox->setEnabled(foundDwiVolume);
  m_Controls->m_OutputRMSErrorImage->setEnabled(foundDwiVolume);

  // reset sampling frame to 1 and update all ealted components
  m_Controls->m_B_ValueMap_Rounder_SpinBox->setValue(1);

  UpdateBValueTableWidget(m_Controls->m_B_ValueMap_Rounder_SpinBox->value());

  if (foundDwiVolume)
  {
    m_Controls->m_InputData->setTitle("Input Data");
    vnl_matrix_fixed< double, 3, 3 > mf = m_DiffusionImage->GetMeasurementFrame();
    for (int r=0; r<3; r++)
      for (int c=0; c<3; c++)
      {
        QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
        delete item;
        item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        item->setText(QString::number(mf.get(r,c)));
        m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
      }
    //calculate target bValue for MultishellToSingleShellfilter
    const mitk::DiffusionImage<DiffusionPixelType>::BValueMap & bValMap = m_DiffusionImage->GetB_ValueMap();
    mitk::DiffusionImage<DiffusionPixelType>::BValueMap::const_iterator it = bValMap.begin();
    unsigned int targetBVal = 0;
    while(it != bValMap.end())
      targetBVal += (it++)->first;
    targetBVal /= (float)bValMap.size()-1;
    m_Controls->m_targetBValueSpinBox->setValue(targetBVal);

  }
  else
  {
    for (int r=0; r<3; r++)
      for (int c=0; c<3; c++)
      {
        QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
        delete item;
        item = new QTableWidgetItem();
        m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
      }

    m_Controls->m_DiffusionImageLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_InputData->setTitle("Please Select Input Data");
  }
}

void QmitkPreprocessingView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkPreprocessingView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkPreprocessingView::DoHalfSphereGradientDirections()
{
  GradientDirectionContainerType::Pointer gradientContainer = m_DiffusionImage->GetDirections();

  for (int j=0; j<gradientContainer->Size(); j++)
    if (gradientContainer->at(j)[0]<0)
      gradientContainer->at(j) = -gradientContainer->at(j);
  m_DiffusionImage->SetDirections(gradientContainer);
}

void QmitkPreprocessingView::DoApplyMesurementFrame()
{
  if (m_DiffusionImage.IsNull())
    return;

  vnl_matrix_fixed< double, 3, 3 > mf;
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
    {
      QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
      if (!item)
        return;
      mf[r][c] = item->text().toDouble();
    }
  m_DiffusionImage->SetMeasurementFrame(mf);
}

void QmitkPreprocessingView::DoShowGradientDirections()
{
  if (m_DiffusionImage.IsNull())
    return;

  int maxIndex = 0;
  int maxSize = m_DiffusionImage->GetDimension(0);
  if (maxSize<m_DiffusionImage->GetDimension(1))
  {
    maxSize = m_DiffusionImage->GetDimension(1);
    maxIndex = 1;
  }
  if (maxSize<m_DiffusionImage->GetDimension(2))
  {
    maxSize = m_DiffusionImage->GetDimension(2);
    maxIndex = 2;
  }

  mitk::Point3D origin = m_DiffusionImage->GetGeometry()->GetOrigin();
  mitk::PointSet::Pointer originSet = mitk::PointSet::New();

  typedef mitk::DiffusionImage<short*>::BValueMap BValueMap;
  typedef mitk::DiffusionImage<short*>::BValueMap::iterator BValueMapIterator;
  BValueMap bValMap =  m_DiffusionImage->GetB_ValueMap();

  GradientDirectionContainerType::Pointer gradientContainer = m_DiffusionImage->GetDirections();
  mitk::Geometry3D::Pointer geometry = m_DiffusionImage->GetGeometry();
  int shellCount = 1;
  for(BValueMapIterator it = bValMap.begin(); it!=bValMap.end(); ++it)
  {
    mitk::PointSet::Pointer pointset = mitk::PointSet::New();
    for (int j=0; j<it->second.size(); j++)
    {
      mitk::Point3D ip;
      vnl_vector_fixed< double, 3 > v = gradientContainer->at(it->second[j]);
      if (v.magnitude()>mitk::eps)
      {
        ip[0] = v[0]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[0]-0.5*geometry->GetSpacing()[0] + geometry->GetSpacing()[0]*m_DiffusionImage->GetDimension(0)/2;
        ip[1] = v[1]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[1]-0.5*geometry->GetSpacing()[1] + geometry->GetSpacing()[1]*m_DiffusionImage->GetDimension(1)/2;
        ip[2] = v[2]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[2]-0.5*geometry->GetSpacing()[2] + geometry->GetSpacing()[2]*m_DiffusionImage->GetDimension(2)/2;

        pointset->InsertPoint(j, ip);
      }
      else if (originSet->IsEmpty())
      {
        ip[0] = v[0]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[0]-0.5*geometry->GetSpacing()[0] + geometry->GetSpacing()[0]*m_DiffusionImage->GetDimension(0)/2;
        ip[1] = v[1]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[1]-0.5*geometry->GetSpacing()[1] + geometry->GetSpacing()[1]*m_DiffusionImage->GetDimension(1)/2;
        ip[2] = v[2]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[2]-0.5*geometry->GetSpacing()[2] + geometry->GetSpacing()[2]*m_DiffusionImage->GetDimension(2)/2;

        originSet->InsertPoint(j, ip);
      }
    }
    if (it->first<mitk::eps)
      continue;

    // add shell to datastorage
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(pointset);
    QString name = m_SelectedDiffusionNodes.front()->GetName().c_str();
    name += "_Shell_";
    name += QString::number(it->first);
    node->SetName(name.toStdString().c_str());
    node->SetProperty("pointsize", mitk::FloatProperty::New((float)maxSize/50));
    int b0 = shellCount%2;
    int b1 = 0;
    int b2 = 0;
    if (shellCount>4)
      b2 = 1;
    if (shellCount%4 >= 2)
      b1 = 1;

    node->SetProperty("color", mitk::ColorProperty::New(b2, b1, b0));
    GetDefaultDataStorage()->Add(node, m_SelectedDiffusionNodes.front());
    shellCount++;
  }

  // add origin to datastorage
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(originSet);
  QString name = m_SelectedDiffusionNodes.front()->GetName().c_str();
  name += "_Origin";
  node->SetName(name.toStdString().c_str());
  node->SetProperty("pointsize", mitk::FloatProperty::New((float)maxSize/50));
  node->SetProperty("color", mitk::ColorProperty::New(1,1,1));
  GetDefaultDataStorage()->Add(node, m_SelectedDiffusionNodes.front());
}

void QmitkPreprocessingView::DoReduceGradientDirections()
{
  if (m_DiffusionImage.IsNull())
    return;

  typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
  typedef itk::ElectrostaticRepulsionDiffusionGradientReductionFilter<DiffusionPixelType, DiffusionPixelType> FilterType;
  typedef DiffusionImageType::BValueMap BValueMap;

  // GetShellSelection from GUI
  BValueMap shellSlectionMap;
  BValueMap originalShellMap = m_DiffusionImage->GetB_ValueMap();
  std::vector<int> newNumGradientDirections;
  int shellCounter = 0;

  foreach(QCheckBox * box , m_ReduceGradientCheckboxes)
  {
    if(box->isChecked())
    {
      double BValue = (box->text().split(' ')).at(0).toDouble();
      shellSlectionMap[BValue] = originalShellMap[BValue];
      newNumGradientDirections.push_back(m_ReduceGradientSpinboxes.at(shellCounter)->value());
    }
    shellCounter++;
  }

  if (newNumGradientDirections.empty())
    return;

  GradientDirectionContainerType::Pointer gradientContainer = m_DiffusionImage->GetDirections();
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(m_DiffusionImage->GetVectorImage());
  filter->SetOriginalGradientDirections(gradientContainer);
  filter->SetNumGradientDirections(newNumGradientDirections);
  filter->SetOriginalBValueMap(originalShellMap);
  filter->SetShellSelectionBValueMap(shellSlectionMap);
  filter->Update();

  DiffusionImageType::Pointer image = DiffusionImageType::New();
  image->SetVectorImage( filter->GetOutput() );
  image->SetB_Value(m_DiffusionImage->GetB_Value());
  image->SetDirections(filter->GetGradientDirections());
  image->SetMeasurementFrame(m_DiffusionImage->GetMeasurementFrame());
  image->InitializeFromVectorImage();

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( image );
  QString name = m_SelectedDiffusionNodes.front()->GetName().c_str();


  QList<QSpinBox*>::iterator itSpinBox = m_ReduceGradientSpinboxes.begin();
  QList<QCheckBox*>::iterator itCheckBox = m_ReduceGradientCheckboxes.begin();
  while(itSpinBox != m_ReduceGradientSpinboxes.end() && itCheckBox != m_ReduceGradientCheckboxes.end())
  {

    name += "_";
    if((*itCheckBox)->isChecked()){
      name += QString::number((*itSpinBox)->value());
    }else
    {
      name += QString::number(0);
    }
    ++itSpinBox;
    ++itCheckBox;
  }
  imageNode->SetName(name.toStdString().c_str());
  GetDefaultDataStorage()->Add(imageNode);
}

void QmitkPreprocessingView::MergeDwis()
{
  typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
  typedef DiffusionImageType::GradientDirectionContainerType    GradientContainerType;

  if (m_SelectedDiffusionNodes.size()<2)
    return;

  typedef itk::VectorImage<DiffusionPixelType,3>                  DwiImageType;
  typedef DwiImageType::PixelType                        DwiPixelType;
  typedef DwiImageType::RegionType                       DwiRegionType;
  typedef std::vector< DwiImageType::Pointer >  DwiImageContainerType;

  typedef std::vector< GradientContainerType::Pointer >  GradientListContainerType;

  DwiImageContainerType       imageContainer;
  GradientListContainerType   gradientListContainer;
  std::vector< double >       bValueContainer;

  QString name = m_SelectedDiffusionNodes.front()->GetName().c_str();
  for (int i=0; i<m_SelectedDiffusionNodes.size(); i++)
  {
    DiffusionImageType::Pointer dwi = dynamic_cast< mitk::DiffusionImage<DiffusionPixelType>* >( m_SelectedDiffusionNodes.at(i)->GetData() );
    if ( dwi.IsNotNull() )
    {
      imageContainer.push_back(dwi->GetVectorImage());
      gradientListContainer.push_back(dwi->GetDirections());
      bValueContainer.push_back(dwi->GetB_Value());
      if (i>0)
      {
        name += "+";
        name += m_SelectedDiffusionNodes.at(i)->GetName().c_str();
      }
    }
  }

  typedef itk::MergeDiffusionImagesFilter<short> FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetImageVolumes(imageContainer);
  filter->SetGradientLists(gradientListContainer);
  filter->SetBValues(bValueContainer);
  filter->Update();

  vnl_matrix_fixed< double, 3, 3 > mf; mf.set_identity();
  DiffusionImageType::Pointer image = DiffusionImageType::New();
  image->SetVectorImage( filter->GetOutput() );
  image->SetB_Value(filter->GetB_Value());
  image->SetDirections(filter->GetOutputGradients());
  image->SetMeasurementFrame(mf);
  image->InitializeFromVectorImage();

  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData( image );
  imageNode->SetName(name.toStdString().c_str());
  GetDefaultDataStorage()->Add(imageNode);
}

void QmitkPreprocessingView::ExtractB0()
{
  typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
  typedef DiffusionImageType::GradientDirectionContainerType    GradientContainerType;

  int nrFiles = m_SelectedDiffusionNodes.size();
  if (!nrFiles) return;

  // call the extraction withou averaging if the check-box is checked
  if( this->m_Controls->m_CheckExtractAll->isChecked() )
  {
    DoExtractBOWithoutAveraging();
    return;
  }

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( m_SelectedDiffusionNodes.begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( m_SelectedDiffusionNodes.end() );

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    DiffusionImageType* vols =
        static_cast<DiffusionImageType*>(
          (*itemiter)->GetData());

    std::string nodename;
    (*itemiter)->GetStringProperty("name", nodename);

    // Extract image using found index
    typedef itk::B0ImageExtractionImageFilter<short, short> FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(vols->GetVectorImage());
    filter->SetDirections(vols->GetDirections());
    filter->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( filter->GetOutput() );
    mitkImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( mitkImage );
    node->SetProperty( "name", mitk::StringProperty::New(nodename + "_B0"));

    GetDefaultDataStorage()->Add(node);

    ++itemiter;
  }
}

void QmitkPreprocessingView::DoExtractBOWithoutAveraging()
{
  // typedefs
  typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
  typedef DiffusionImageType::GradientDirectionContainerType    GradientContainerType;
  typedef itk::B0ImageExtractionToSeparateImageFilter< short, short> FilterType;

  // check number of selected objects, return if empty
  int nrFiles = m_SelectedDiffusionNodes.size();
  if (!nrFiles)
    return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( m_SelectedDiffusionNodes.begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( m_SelectedDiffusionNodes.end() );

  while ( itemiter != itemiterend ) // for all items
  {
    DiffusionImageType* vols =
        static_cast<DiffusionImageType*>(
          (*itemiter)->GetData());

    std::string nodename;
    (*itemiter)->GetStringProperty("name", nodename);

    // Extract image using found index
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(vols->GetVectorImage());
    filter->SetDirections(vols->GetDirections());
    filter->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( filter->GetOutput() );
    mitkImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( mitkImage );
    node->SetProperty( "name", mitk::StringProperty::New(nodename + "_B0_ALL"));

    GetDefaultDataStorage()->Add(node);

    ++itemiter;
  }

}

void QmitkPreprocessingView::AverageGradients()
{
  int nrFiles = m_SelectedDiffusionNodes.size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( m_SelectedDiffusionNodes.begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( m_SelectedDiffusionNodes.end() );

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    mitk::DiffusionImage<DiffusionPixelType>* vols =
        static_cast<mitk::DiffusionImage<DiffusionPixelType>*>(
          (*itemiter)->GetData());

    vols->AverageRedundantGradients(m_Controls->m_Blur->value());

    ++itemiter;
  }
}
