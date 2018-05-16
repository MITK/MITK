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

#include "QmitkTensorReconstructionView.h"
#include "mitkDiffusionImagingConfigure.h"

// qt includes
#include <QMessageBox>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsLinearLayout>


// itk includes
#include "itkTimeProbe.h"
//#include "itkTensor.h"

// mitk includes
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"

#include "mitkNodePredicateDataType.h"
#include "QmitkDataStorageComboBox.h"

#include "mitkTeemDiffusionTensor3DReconstructionImageFilter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"
#include "itkTensorImageToDiffusionImageFilter.h"
#include "itkPointShell.h"
#include "itkVector.h"
#include "itkB0ImageExtractionImageFilter.h"
#include "itkTensorReconstructionWithEigenvalueCorrectionFilter.h"

#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include <itkBinaryThresholdImageFilter.h>
#include <mitkImageVtkMapper2D.h>

#include "mitkProperties.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkLookupTable.h"
#include "mitkImageStatisticsHolder.h"
#include <mitkITKImageImport.h>

#include <itkTensorImageToOdfImageFilter.h>
#include <itkResidualImageFilter.h>

#include <berryIWorkbenchWindow.h>
#include <berryISelectionService.h>
#include <mitkImageVtkMapper2D.h>
#include <mitkNodePredicateIsDWI.h>
#include <mitkNodePredicateDataType.h>

const std::string QmitkTensorReconstructionView::VIEW_ID = "org.mitk.views.tensorreconstruction";

typedef mitk::TensorImage::ScalarPixelType          TTensorPixelType;
typedef mitk::TensorImage::PixelType                TensorPixelType;
typedef mitk::TensorImage::ItkTensorImageType       TensorImageType;

using namespace berry;

QmitkTensorReconstructionView::QmitkTensorReconstructionView()
  : QmitkAbstractView(),
    m_Controls(nullptr)
{

}

QmitkTensorReconstructionView::~QmitkTensorReconstructionView()
{

}

void QmitkTensorReconstructionView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkTensorReconstructionViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    Advanced1CheckboxClicked();
  }
}

void QmitkTensorReconstructionView::SetFocus()
{
  m_Controls->m_Advanced1->setFocus();
}

void QmitkTensorReconstructionView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_StartReconstruction), SIGNAL(clicked()), this, SLOT(Reconstruct()) );
    connect( (QObject*)(m_Controls->m_Advanced1), SIGNAL(clicked()), this, SLOT(Advanced1CheckboxClicked()) );
    connect( (QObject*)(m_Controls->m_TensorsToDWIButton), SIGNAL(clicked()), this, SLOT(TensorsToDWI()) );
    connect( (QObject*)(m_Controls->m_TensorsToOdfButton), SIGNAL(clicked()), this, SLOT(TensorsToOdf()) );
    connect( (QObject*)(m_Controls->m_ResidualButton), SIGNAL(clicked()), this, SLOT(ResidualCalculation()) );
    connect( (QObject*)(m_Controls->m_PerSliceView), SIGNAL(pointSelected(int, int)), this, SLOT(ResidualClicked(int, int)) );
    connect( (QObject*)(m_Controls->m_TensorReconstructionThreshold), SIGNAL(valueChanged(int)), this, SLOT(PreviewThreshold(int)) );

    m_Controls->m_ResidualTab->setVisible(false);
    m_Controls->m_PercentagesOfOutliers->setVisible(false);

    m_Controls->m_DwiBox->SetDataStorage(this->GetDataStorage());
    mitk::NodePredicateIsDWI::Pointer isDwi = mitk::NodePredicateIsDWI::New();
    m_Controls->m_DwiBox->SetPredicate( isDwi );
    connect( (QObject*)(m_Controls->m_DwiBox), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));

    m_Controls->m_OdfBox->SetDataStorage(this->GetDataStorage());
    mitk::TNodePredicateDataType<mitk::TensorImage>::Pointer isDti = mitk::TNodePredicateDataType<mitk::TensorImage>::New();
    m_Controls->m_OdfBox->SetPredicate( isDti );
    connect( (QObject*)(m_Controls->m_OdfBox), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));

    m_Controls->m_DtiBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_DtiBox->SetPredicate( isDti );
    connect( (QObject*)(m_Controls->m_DtiBox), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));

    m_Controls->m_ResBox1->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ResBox1->SetPredicate( isDwi );
    connect( (QObject*)(m_Controls->m_ResBox1), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));

    m_Controls->m_ResBox2->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ResBox2->SetPredicate( isDti );
    connect( (QObject*)(m_Controls->m_ResBox2), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));
  }
}

void QmitkTensorReconstructionView::ResidualClicked(int slice, int volume)
{
  mitk::Image* diffImage;
  mitk::DataNode::Pointer correctNode;
  mitk::BaseGeometry* geometry;

  if (m_Controls->m_ResBox1->GetSelectedNode().IsNotNull())
  {
    diffImage = static_cast<mitk::Image*>(m_Controls->m_ResBox1->GetSelectedNode()->GetData());

    geometry = m_Controls->m_ResBox1->GetSelectedNode()->GetData()->GetGeometry();

    // Remember the node whose display index must be updated
    correctNode = mitk::DataNode::New();
    correctNode = m_Controls->m_ResBox1->GetSelectedNode();

    GradientDirectionContainerType::Pointer dirs = static_cast<mitk::GradientDirectionsProperty*>( diffImage->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

    for(itk::SizeValueType i=0; i<dirs->Size() && i<=(itk::SizeValueType)volume; i++)
    {
      GradientDirectionType grad = dirs->ElementAt(i);

      // check if image is b0 weighted
      if(fabs(grad[0]) < 0.001 && fabs(grad[1]) < 0.001 && fabs(grad[2]) < 0.001)
      {
        volume++;
      }
    }

    QString pos = "Volume: ";
    pos.append(QString::number(volume));
    pos.append(", Slice: ");
    pos.append(QString::number(slice));
    m_Controls->m_PositionLabel->setText(pos);

    if(correctNode)
    {
      int oldDisplayVal;
      correctNode->GetIntProperty("DisplayChannel", oldDisplayVal);
      QString oldVal = QString::number(oldDisplayVal);
      QString newVal = QString::number(volume);
      correctNode->SetIntProperty("DisplayChannel",volume);
      correctNode->SetSelected(true);
      this->FirePropertyChanged("DisplayChannel", oldVal, newVal);
      correctNode->UpdateOutputInformation();


      mitk::Point3D p3 = this->GetRenderWindowPart()->GetSelectedPosition();
      itk::Index<3> ix;
      geometry->WorldToIndex(p3, ix);
      // ix[2] = slice;

      mitk::Vector3D vec;
      vec[0] = ix[0];
      vec[1] = ix[1];
      vec[2] = slice;

      mitk::Vector3D v3New;
      geometry->IndexToWorld(vec, v3New);
      mitk::Point3D origin = geometry->GetOrigin();
      mitk::Point3D p3New;
      p3New[0] = v3New[0] + origin[0];
      p3New[1] = v3New[1] + origin[1];
      p3New[2] = v3New[2] + origin[2];

      this->GetRenderWindowPart()->SetSelectedPosition(p3New);
      this->GetRenderWindowPart()->RequestUpdate();
    }
  }
}

void QmitkTensorReconstructionView::Advanced1CheckboxClicked()
{
  bool check = m_Controls->
               m_Advanced1->isChecked();

  m_Controls->frame->setVisible(check);
}

void QmitkTensorReconstructionView::Activated()
{
}

void QmitkTensorReconstructionView::Deactivated()
{

  // Get all current nodes

  mitk::DataStorage::SetOfObjects::ConstPointer objects =  this->GetDataStorage()->GetAll();
  mitk::DataStorage::SetOfObjects::const_iterator itemiter( objects->begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( objects->end() );
  while ( itemiter != itemiterend ) // for all items
  {
    mitk::DataNode::Pointer node = *itemiter;
    if (node.IsNull())
      continue;

    // only look at interesting types
    if( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(dynamic_cast<mitk::Image*>(node->GetData())))
    {
      if (this->GetDataStorage()->GetNamedDerivedNode("ThresholdOverlay", *itemiter))
      {
        node = this->GetDataStorage()->GetNamedDerivedNode("ThresholdOverlay", *itemiter);
        this->GetDataStorage()->Remove(node);
      }
    }
    itemiter++;
  }


  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkTensorReconstructionView::Visible()
{
}

void QmitkTensorReconstructionView::Hidden()
{
}

void QmitkTensorReconstructionView::ResidualCalculation()
{
  // Extract dwi and dti from current selection
  // In case of multiple selections, take the first one, since taking all combinations is not meaningful

  mitk::DataStorage::SetOfObjects::Pointer set =
      mitk::DataStorage::SetOfObjects::New();

  mitk::Image::Pointer diffImage
      = mitk::Image::New();

  TensorImageType::Pointer tensorImage;

  std::string nodename;

  if(m_Controls->m_ResBox1->GetSelectedNode())
  {
    diffImage = static_cast<mitk::Image*>(m_Controls->m_ResBox1->GetSelectedNode()->GetData());
  }
  else
    return;
  if(m_Controls->m_ResBox2->GetSelectedNode().IsNotNull())
  {
    mitk::TensorImage* mitkVol;
    mitkVol = static_cast<mitk::TensorImage*>(m_Controls->m_ResBox2->GetSelectedNode()->GetData());
    mitk::CastToItkImage(mitkVol, tensorImage);
    m_Controls->m_ResBox2->GetSelectedNode()->GetStringProperty("name", nodename);
  }
  else
    return;

  typedef itk::TensorImageToDiffusionImageFilter<
      TTensorPixelType, DiffusionPixelType > FilterType;

  GradientDirectionContainerType* gradients
      =  static_cast<mitk::GradientDirectionsProperty*>( diffImage->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

  // Find the min and the max values from a baseline image
  mitk::ImageStatisticsHolder *stats = diffImage->GetStatistics();

  //Initialize filter that calculates the modeled diffusion weighted signals
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( tensorImage );
  filter->SetBValue( static_cast<mitk::FloatProperty*>(diffImage->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue());
  filter->SetGradientList(gradients);
  filter->SetMin(stats->GetScalarValueMin());
  filter->SetMax(stats->GetScalarValueMax());
  filter->Update();


  // TENSORS TO DATATREE
  mitk::Image::Pointer image = mitk::GrabItkImageMemory( filter->GetOutput() );
  mitk::DiffusionPropertyHelper::CopyProperties(diffImage, image, true);
  image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( gradients ) );
  mitk::DiffusionPropertyHelper propertyHelper( image );
  propertyHelper.InitializeImage();

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData( image );
  mitk::ImageVtkMapper2D::SetDefaultProperties(node);

  QString newname;
  newname = newname.append(nodename.c_str());
  newname = newname.append("_Estimated DWI");
  node->SetName(newname.toLatin1());

  GetDataStorage()->Add(node, m_Controls->m_ResBox2->GetSelectedNode());

  BValueMapType map = static_cast<mitk::BValueMapProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();;
  std::vector< unsigned int > b0Indices = map[0];


  typedef itk::ResidualImageFilter<DiffusionPixelType, float> ResidualImageFilterType;

  ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
  mitk::CastToItkImage(diffImage, itkVectorImagePointer);
  ITKDiffusionImageType::Pointer itkSecondVectorImagePointer = ITKDiffusionImageType::New();
  mitk::CastToItkImage(image, itkSecondVectorImagePointer);

  ResidualImageFilterType::Pointer residualFilter = ResidualImageFilterType::New();
  residualFilter->SetInput( itkVectorImagePointer );
  residualFilter->SetSecondDiffusionImage( itkSecondVectorImagePointer );
  residualFilter->SetGradients(gradients);
  residualFilter->SetB0Index(b0Indices[0]);
  residualFilter->SetB0Threshold(30);
  residualFilter->Update();

  itk::Image<float, 3>::Pointer residualImage = itk::Image<float, 3>::New();
  residualImage = residualFilter->GetOutput();

  mitk::Image::Pointer mitkResImg = mitk::Image::New();

  mitk::CastToMitkImage(residualImage, mitkResImg);

  stats = mitkResImg->GetStatistics();
  float min = stats->GetScalarValueMin();
  float max = stats->GetScalarValueMax();

  mitk::LookupTableProperty::Pointer lutProp = mitk::LookupTableProperty::New();
  mitk::LookupTable::Pointer lut = mitk::LookupTable::New();


  vtkSmartPointer<vtkLookupTable> lookupTable =
      vtkSmartPointer<vtkLookupTable>::New();

  lookupTable->SetTableRange(min, max);


  // If you don't want to use the whole color range, you can use
  // SetValueRange, SetHueRange, and SetSaturationRange
  lookupTable->Build();

  vtkSmartPointer<vtkLookupTable> reversedlookupTable =
      vtkSmartPointer<vtkLookupTable>::New();
  reversedlookupTable->SetTableRange(min+1, max);
  reversedlookupTable->Build();

  for(int i=0; i<256; i++)
  {
    double* rgba = reversedlookupTable->GetTableValue(255-i);

    lookupTable->SetTableValue(i, rgba[0], rgba[1], rgba[2], rgba[3]);
  }

  lut->SetVtkLookupTable(lookupTable);
  lutProp->SetLookupTable(lut);

  // Create lookuptable

  mitk::DataNode::Pointer resNode=mitk::DataNode::New();
  resNode->SetData( mitkResImg );
  resNode->SetName("Residuals");

  resNode->SetProperty("LookupTable", lutProp);

  bool b;
  resNode->GetBoolProperty("use color", b);
  resNode->SetBoolProperty("use color", false);

  GetDataStorage()->Add(resNode, m_Controls->m_ResBox2->GetSelectedNode());

  this->GetRenderWindowPart()->RequestUpdate();



  // Draw Graph
  std::vector<double> means = residualFilter->GetMeans();
  std::vector<double> q1s = residualFilter->GetQ1();
  std::vector<double> q3s = residualFilter->GetQ3();
  std::vector<double> percentagesOfOUtliers = residualFilter->GetPercentagesOfOutliers();

  m_Controls->m_ResidualAnalysis->SetMeans(means);
  m_Controls->m_ResidualAnalysis->SetQ1(q1s);
  m_Controls->m_ResidualAnalysis->SetQ3(q3s);
  m_Controls->m_ResidualAnalysis->SetPercentagesOfOutliers(percentagesOfOUtliers);

  if(m_Controls->m_PercentagesOfOutliers->isChecked())
  {
    m_Controls->m_ResidualAnalysis->DrawPercentagesOfOutliers();
  }
  else
  {
    m_Controls->m_ResidualAnalysis->DrawMeans();
  }



  // Draw Graph for volumes per slice in the QGraphicsView
  std::vector< std::vector<double> > outliersPerSlice = residualFilter->GetOutliersPerSlice();
  int xSize = outliersPerSlice.size();
  if(xSize == 0)
  {
    return;
  }
  int ySize = outliersPerSlice[0].size();


  // Find maximum in outliersPerSlice
  double maxOutlier= 0.0;
  for(int i=0; i<xSize; i++)
  {
    for(int j=0; j<ySize; j++)
    {
      if(outliersPerSlice[i][j]>maxOutlier)
      {
        maxOutlier = outliersPerSlice[i][j];
      }
    }
  }


  // Create some QImage
  QImage qImage(xSize, ySize, QImage::Format_RGB32);
  QImage legend(1, 256, QImage::Format_RGB32);
  QRgb value;

  vtkSmartPointer<vtkLookupTable> lookup =
      vtkSmartPointer<vtkLookupTable>::New();

  lookup->SetTableRange(0.0, maxOutlier);
  lookup->Build();

  reversedlookupTable->SetTableRange(0, maxOutlier);
  reversedlookupTable->Build();

  for(int i=0; i<256; i++)
  {
    double* rgba = reversedlookupTable->GetTableValue(255-i);
    lookup->SetTableValue(i, rgba[0], rgba[1], rgba[2], rgba[3]);
  }


  // Fill qImage
  for(int i=0; i<xSize; i++)
  {
    for(int j=0; j<ySize; j++)
    {
      double out = outliersPerSlice[i][j];

      unsigned char *_rgba = lookup->MapValue(out);
      int r, g, b;
      r = _rgba[0];
      g = _rgba[1];
      b = _rgba[2];

      value = qRgb(r, g, b);

      qImage.setPixel(i,j,value);

    }
  }

  for(int i=0; i<256; i++)
  {
    double* rgba = lookup->GetTableValue(i);
    int r, g, b;
    r = rgba[0]*255;
    g = rgba[1]*255;
    b = rgba[2]*255;
    value = qRgb(r, g, b);
    legend.setPixel(0,255-i,value);
  }

  QString upper = QString::number(maxOutlier, 'g', 3);
  upper.append(" %");
  QString lower = QString::number(0.0);
  lower.append(" %");
  m_Controls->m_UpperLabel->setText(upper);
  m_Controls->m_LowerLabel->setText(lower);

  QPixmap pixmap(QPixmap::fromImage(qImage));
  QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
  item->setTransform(QTransform::fromScale(10.0, 3.0), true);

  QPixmap pixmap2(QPixmap::fromImage(legend));
  QGraphicsPixmapItem *item2 = new QGraphicsPixmapItem(pixmap2);
  item2->setTransform(QTransform::fromScale(20.0, 1.0), true);

  m_Controls->m_PerSliceView->SetResidualPixmapItem(item);

  QGraphicsScene* scene = new QGraphicsScene;
  QGraphicsScene* scene2 = new QGraphicsScene;

  scene->addItem(item);
  scene2->addItem(item2);

  m_Controls->m_PerSliceView->setScene(scene);
  m_Controls->m_LegendView->setScene(scene2);
  m_Controls->m_PerSliceView->show();
  m_Controls->m_PerSliceView->repaint();

  m_Controls->m_LegendView->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
  m_Controls->m_LegendView->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
  m_Controls->m_LegendView->show();
  m_Controls->m_LegendView->repaint();
}

void QmitkTensorReconstructionView::Reconstruct()
{
  int method = m_Controls->m_ReconctructionMethodBox->currentIndex();

  switch (method)
  {
  case 0:
    ItkTensorReconstruction();
    break;
  case 1:
    TensorReconstructionWithCorr();
    break;
  default:
    ItkTensorReconstruction();
  }
}

void QmitkTensorReconstructionView::TensorReconstructionWithCorr()
{
  try
  {
    if ( m_Controls->m_DwiBox->GetSelectedNode().IsNotNull() ) // for all items
    {
      mitk::Image* vols = static_cast<mitk::Image*>(m_Controls->m_DwiBox->GetSelectedNode()->GetData());
      std::string nodename = m_Controls->m_DwiBox->GetSelectedNode()->GetName();

      typedef itk::TensorReconstructionWithEigenvalueCorrectionFilter< DiffusionPixelType, TTensorPixelType > ReconstructionFilter;

      float b0Threshold = m_Controls->m_TensorReconstructionThreshold->value();
      mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradientContainerCopy = mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::New();
      for(auto it = mitk::DiffusionPropertyHelper::GetGradientContainer(vols)->Begin(); it != mitk::DiffusionPropertyHelper::GetGradientContainer(vols)->End(); it++)
        gradientContainerCopy->push_back(it.Value());

      ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
      mitk::CastToItkImage(vols, itkVectorImagePointer);

      ReconstructionFilter::Pointer reconFilter = ReconstructionFilter::New();
      reconFilter->SetBValue( static_cast<mitk::FloatProperty*>(vols->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() );
      reconFilter->SetGradientImage( gradientContainerCopy, itkVectorImagePointer);
      reconFilter->SetB0Threshold(b0Threshold);
      reconFilter->Update();

      typedef mitk::TensorImage::ItkTensorImageType TensorImageType;
      TensorImageType::Pointer outputTensorImg = reconFilter->GetOutput();

      typedef itk::ImageRegionIterator<TensorImageType> TensorImageIteratorType;
      TensorImageIteratorType tensorIt(outputTensorImg, outputTensorImg->GetRequestedRegion());
      tensorIt.GoToBegin();

      int negatives = 0;
      while(!tensorIt.IsAtEnd())
      {
        typedef mitk::TensorImage::PixelType TensorType;
        TensorType tensor = tensorIt.Get();

        TensorType::EigenValuesArrayType ev;
        tensor.ComputeEigenValues(ev);

        for(unsigned int i=0; i<ev.Size(); i++)
        {
          if(ev[i] < 0.0)
          {
            tensor.Fill(0.0);
            tensorIt.Set(tensor);
            negatives++;
            break;
          }
        }
        ++tensorIt;
      }
      MITK_INFO << negatives << " tensors with negative eigenvalues" << std::endl;

      mitk::TensorImage::Pointer image = mitk::TensorImage::New();
      image->InitializeByItk( outputTensorImg.GetPointer() );
      image->SetVolume( outputTensorImg->GetBufferPointer() );
      mitk::DataNode::Pointer node=mitk::DataNode::New();
      node->SetData( image );
      node->SetName(nodename+"_EigenvalueCorrected_DT");
      GetDataStorage()->Add(node, m_Controls->m_DwiBox->GetSelectedNode());
    }

    this->GetRenderWindowPart()->RequestUpdate();
  }
  catch (itk::ExceptionObject &ex)
  {
    MITK_INFO << ex ;
    QMessageBox::information(0, "Reconstruction not possible:", ex.GetDescription());
  }
}

void QmitkTensorReconstructionView::ItkTensorReconstruction()
{
  try
  {
    if ( m_Controls->m_DwiBox->GetSelectedNode().IsNotNull() ) // for all items
    {
      mitk::Image* vols = static_cast<mitk::Image*>(m_Controls->m_DwiBox->GetSelectedNode()->GetData());

      std::string nodename = m_Controls->m_DwiBox->GetSelectedNode()->GetName();


      typedef itk::DiffusionTensor3DReconstructionImageFilter<DiffusionPixelType, DiffusionPixelType, TTensorPixelType > TensorReconstructionImageFilterType;
      TensorReconstructionImageFilterType::Pointer tensorReconstructionFilter = TensorReconstructionImageFilterType::New();

      GradientDirectionContainerType::Pointer gradientContainerCopy = GradientDirectionContainerType::New();
      for(GradientDirectionContainerType::ConstIterator it = static_cast<mitk::GradientDirectionsProperty*>( vols->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer()->Begin();
          it != static_cast<mitk::GradientDirectionsProperty*>( vols->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer()->End(); it++)
      {
        gradientContainerCopy->push_back(it.Value());
      }

      ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
      mitk::CastToItkImage(vols, itkVectorImagePointer);

      tensorReconstructionFilter->SetBValue(  static_cast<mitk::FloatProperty*>(vols->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() );
      tensorReconstructionFilter->SetGradientImage( gradientContainerCopy, itkVectorImagePointer );
      tensorReconstructionFilter->SetThreshold( m_Controls->m_TensorReconstructionThreshold->value() );
      tensorReconstructionFilter->Update();

      // TENSORS TO DATATREE
      mitk::TensorImage::Pointer image = mitk::TensorImage::New();

      typedef mitk::TensorImage::ItkTensorImageType TensorImageType;
      TensorImageType::Pointer tensorImage;
      tensorImage = tensorReconstructionFilter->GetOutput();

      // Check the tensor for negative eigenvalues
      if(m_Controls->m_CheckNegativeEigenvalues->isChecked())
      {
        typedef itk::ImageRegionIterator<TensorImageType> TensorImageIteratorType;
        TensorImageIteratorType tensorIt(tensorImage, tensorImage->GetRequestedRegion());
        tensorIt.GoToBegin();

        while(!tensorIt.IsAtEnd())
        {

          typedef mitk::TensorImage::PixelType TensorType;
          //typedef itk::Tensor<TTensorPixelType, 3> TensorType2;

          TensorType tensor = tensorIt.Get();

          TensorType::EigenValuesArrayType ev;
          tensor.ComputeEigenValues(ev);
          for(unsigned int i=0; i<ev.Size(); i++)
          {
            if(ev[i] < 0.0)
            {
              tensor.Fill(0.0);
              tensorIt.Set(tensor);
              break;
            }
          }
          ++tensorIt;
        }
      }

      tensorImage->SetDirection( itkVectorImagePointer->GetDirection() );
      image->InitializeByItk( tensorImage.GetPointer() );
      image->SetVolume( tensorReconstructionFilter->GetOutput()->GetBufferPointer() );
      mitk::DataNode::Pointer node=mitk::DataNode::New();
      node->SetData( image );
      node->SetName(nodename+"_LinearLeastSquares_DT");
      GetDataStorage()->Add(node, m_Controls->m_DwiBox->GetSelectedNode());
    }

    this->GetRenderWindowPart()->RequestUpdate();
  }
  catch (itk::ExceptionObject &ex)
  {
    MITK_INFO << ex ;
    QMessageBox::information(0, "Reconstruction not possible:", ex.GetDescription());
    return;
  }
}

void QmitkTensorReconstructionView::TensorsToDWI()
{
  DoTensorsToDWI();
}

void QmitkTensorReconstructionView::TensorsToOdf()
{
  if (m_Controls->m_OdfBox->GetSelectedNode().IsNotNull())
  {
    mitk::DataNode::Pointer tensorImageNode = m_Controls->m_OdfBox->GetSelectedNode();

    typedef mitk::TensorImage::ScalarPixelType    TTensorPixelType;
    typedef mitk::TensorImage::ItkTensorImageType TensorImageType;

    TensorImageType::Pointer itkvol = TensorImageType::New();
    mitk::CastToItkImage(dynamic_cast<mitk::TensorImage*>(tensorImageNode->GetData()), itkvol);

    typedef itk::TensorImageToOdfImageFilter< TTensorPixelType, TTensorPixelType > FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkvol );
    filter->Update();

    typedef itk::Vector<TTensorPixelType,ODF_SAMPLING_SIZE>  OutputPixelType;
    typedef itk::Image<OutputPixelType,3>                OutputImageType;

    mitk::OdfImage::Pointer image = mitk::OdfImage::New();
    OutputImageType::Pointer outimg = filter->GetOutput();
    image->InitializeByItk( outimg.GetPointer() );
    image->SetVolume( outimg->GetBufferPointer() );
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( image );
    node->SetName(tensorImageNode->GetName()+"_Odf");
    GetDataStorage()->Add(node, tensorImageNode);
  }
}

void QmitkTensorReconstructionView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& )
{
  UpdateGui();
}

void QmitkTensorReconstructionView::UpdateGui()
{
  m_Controls->m_StartReconstruction->setEnabled(m_Controls->m_DwiBox->GetSelectedNode().IsNotNull());

  m_Controls->m_TensorsToDWIButton->setEnabled(m_Controls->m_DtiBox->GetSelectedNode().IsNotNull());
  m_Controls->m_TensorsToOdfButton->setEnabled(m_Controls->m_OdfBox->GetSelectedNode().IsNotNull());

  m_Controls->m_ResidualButton->setEnabled(m_Controls->m_ResBox1->GetSelectedNode().IsNotNull() && m_Controls->m_ResBox2->GetSelectedNode().IsNotNull());
  m_Controls->m_PercentagesOfOutliers->setEnabled(m_Controls->m_ResBox1->GetSelectedNode().IsNotNull() && m_Controls->m_ResBox2->GetSelectedNode().IsNotNull());
  m_Controls->m_PerSliceView->setEnabled(m_Controls->m_ResBox1->GetSelectedNode().IsNotNull() && m_Controls->m_ResBox2->GetSelectedNode().IsNotNull());
}

template<int ndirs>
itk::VectorContainer<unsigned int, vnl_vector_fixed<double, 3> >::Pointer
QmitkTensorReconstructionView::MakeGradientList()
{
  itk::VectorContainer<unsigned int, vnl_vector_fixed<double,3> >::Pointer retval =
      itk::VectorContainer<unsigned int, vnl_vector_fixed<double,3> >::New();
  vnl_matrix_fixed<double, 3, ndirs>* U =
      itk::PointShell<ndirs, vnl_matrix_fixed<double, 3, ndirs> >::DistributePointShell();

  for(int i=0; i<ndirs;i++)
  {
    vnl_vector_fixed<double,3> v;
    v[0] = U->get(0,i); v[1] = U->get(1,i); v[2] = U->get(2,i);
    retval->push_back(v);
  }
  // Add 0 vector for B0
  vnl_vector_fixed<double,3> v;
  v.fill(0.0);
  retval->push_back(v);

  return retval;
}

void QmitkTensorReconstructionView::DoTensorsToDWI()
{
  try
  {
    if (m_Controls->m_DtiBox->GetSelectedNode().IsNotNull())
    {
      std::string nodename = m_Controls->m_DtiBox->GetSelectedNode()->GetName();

      mitk::TensorImage* vol = static_cast<mitk::TensorImage*>(m_Controls->m_DtiBox->GetSelectedNode()->GetData());

      typedef mitk::TensorImage::ScalarPixelType      TTensorPixelType;
      typedef mitk::TensorImage::ItkTensorImageType   TensorImageType;


      TensorImageType::Pointer itkvol = TensorImageType::New();
      mitk::CastToItkImage(vol, itkvol);

      typedef itk::TensorImageToDiffusionImageFilter<
          TTensorPixelType, DiffusionPixelType > FilterType;

      FilterType::GradientListPointerType gradientList = FilterType::GradientListType::New();

      switch(m_Controls->m_TensorsToDWINumDirsSelect->currentIndex())
      {
      case 0:
        gradientList = MakeGradientList<12>();
        break;
      case 1:
        gradientList = MakeGradientList<42>();
        break;
      case 2:
        gradientList = MakeGradientList<92>();
        break;
      case 3:
        gradientList = MakeGradientList<162>();
        break;
      case 4:
        gradientList = MakeGradientList<252>();
        break;
      case 5:
        gradientList = MakeGradientList<362>();
        break;
      case 6:
        gradientList = MakeGradientList<492>();
        break;
      case 7:
        gradientList = MakeGradientList<642>();
        break;
      case 8:
        gradientList = MakeGradientList<812>();
        break;
      case 9:
        gradientList = MakeGradientList<1002>();
        break;
      default:
        gradientList = MakeGradientList<92>();

      }
      double bVal = m_Controls->m_TensorsToDWIBValueEdit->text().toDouble();

      FilterType::Pointer filter = FilterType::New();
      filter->SetInput( itkvol );
      filter->SetBValue(bVal);
      filter->SetGradientList(gradientList);
      filter->Update();

      mitk::Image::Pointer image = mitk::GrabItkImageMemory( filter->GetOutput() );

      image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( gradientList ) );
      image->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( bVal ) );
      image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New() );
      mitk::DiffusionPropertyHelper propertyHelper( image );
      propertyHelper.InitializeImage();

      mitk::DataNode::Pointer node=mitk::DataNode::New();
      node->SetData( image );
      mitk::ImageVtkMapper2D::SetDefaultProperties(node);
      node->SetName(nodename+"_DWI");
      GetDataStorage()->Add(node, m_Controls->m_DtiBox->GetSelectedNode());
    }
    this->GetRenderWindowPart()->RequestUpdate();
  }
  catch (itk::ExceptionObject &ex)
  {
    MITK_INFO << ex ;
    QMessageBox::information(0, "DWI estimation failed:", ex.GetDescription());
    return ;
  }
}


void QmitkTensorReconstructionView::PreviewThreshold(int threshold)
{
  if (m_Controls->m_DwiBox->GetSelectedNode().IsNotNull())
  {
    mitk::Image* vols = static_cast<mitk::Image*>(m_Controls->m_DwiBox->GetSelectedNode()->GetData());

    ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
    mitk::CastToItkImage(vols, itkVectorImagePointer);

    // Extract b0 image
    typedef itk::B0ImageExtractionImageFilter<short, short> FilterType;
    FilterType::Pointer filterB0 = FilterType::New();
    filterB0->SetInput(itkVectorImagePointer);
    filterB0->SetDirections(mitk::DiffusionPropertyHelper::GetGradientContainer(vols));
    filterB0->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();

    typedef itk::Image<short, 3> ImageType;
    typedef itk::Image<short, 3> SegmentationType;
    typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;
    // apply threshold
    ThresholdFilterType::Pointer filterThreshold = ThresholdFilterType::New();
    filterThreshold->SetInput(filterB0->GetOutput());
    filterThreshold->SetLowerThreshold(threshold);
    filterThreshold->SetInsideValue(0);
    filterThreshold->SetOutsideValue(1); // mark cut off values red
    filterThreshold->Update();

    mitkImage->InitializeByItk( filterThreshold->GetOutput() );
    mitkImage->SetVolume( filterThreshold->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node;
    if (this->GetDataStorage()->GetNamedDerivedNode("ThresholdOverlay", m_Controls->m_DwiBox->GetSelectedNode()))
    {
      node = this->GetDataStorage()->GetNamedDerivedNode("ThresholdOverlay", m_Controls->m_DwiBox->GetSelectedNode());
    }
    else
    {
      // create a new node, to show thresholded values
      node = mitk::DataNode::New();
      GetDataStorage()->Add( node, m_Controls->m_DwiBox->GetSelectedNode() );
      node->SetProperty( "name", mitk::StringProperty::New("ThresholdOverlay"));
      node->SetBoolProperty("helper object", true);
    }
    node->SetData( mitkImage );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
