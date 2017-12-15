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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkFiberQuantificationView.h"

// Qt
#include <QMessageBox>

// MITK
#include <mitkNodePredicateProperty.h>
#include <mitkImageCast.h>
#include <mitkPeakImage.h>
#include <mitkLabelSetImage.h>
#include <mitkDICOMSegmentationConstants.h>
#include <mitkDICOMSegmentationPropertyHelper.cpp>

// ITK
#include <itkTractDensityImageFilter.h>
#include <itkTractsToRgbaImageFilter.h>
#include <itkTractsToVectorImageFilter.h>
#include <itkTractsToFiberEndingsImageFilter.h>

#include <math.h>
#include <boost/lexical_cast.hpp>

const std::string QmitkFiberQuantificationView::VIEW_ID = "org.mitk.views.fiberquantification";
using namespace mitk;

QmitkFiberQuantificationView::QmitkFiberQuantificationView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_UpsamplingFactor(5)
{

}

// Destructor
QmitkFiberQuantificationView::~QmitkFiberQuantificationView()
{

}

void QmitkFiberQuantificationView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkFiberQuantificationViewControls;
    m_Controls->setupUi( parent );

    connect( m_Controls->m_ProcessFiberBundleButton, SIGNAL(clicked()), this, SLOT(ProcessSelectedBundles()) );
    connect( m_Controls->m_ExtractFiberPeaks, SIGNAL(clicked()), this, SLOT(CalculateFiberDirections()) );
  }
}

void QmitkFiberQuantificationView::SetFocus()
{
  m_Controls->m_ProcessFiberBundleButton->setFocus();
}

void QmitkFiberQuantificationView::CalculateFiberDirections()
{
  typedef itk::Image<unsigned char, 3>                                            ItkUcharImgType;

  // load fiber bundle
  mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(m_SelectedFB.back()->GetData());

  itk::TractsToVectorImageFilter<float>::Pointer fOdfFilter = itk::TractsToVectorImageFilter<float>::New();
  if (m_SelectedImage.IsNotNull())
  {
    ItkUcharImgType::Pointer itkMaskImage = ItkUcharImgType::New();
    mitk::CastToItkImage<ItkUcharImgType>(m_SelectedImage, itkMaskImage);
    fOdfFilter->SetMaskImage(itkMaskImage);
  }

  // extract directions from fiber bundle
  fOdfFilter->SetFiberBundle(inputTractogram);
  fOdfFilter->SetAngularThreshold(cos(m_Controls->m_AngularThreshold->value()*M_PI/180));
  switch (m_Controls->m_FiberDirNormBox->currentIndex())
  {
  case 0:
    fOdfFilter->SetNormalizationMethod(itk::TractsToVectorImageFilter<float>::NormalizationMethods::GLOBAL_MAX);
    break;
  case 1:
    fOdfFilter->SetNormalizationMethod(itk::TractsToVectorImageFilter<float>::NormalizationMethods::SINGLE_VEC_NORM);
    break;
  case 2:
    fOdfFilter->SetNormalizationMethod(itk::TractsToVectorImageFilter<float>::NormalizationMethods::MAX_VEC_NORM);
    break;
  }
  fOdfFilter->SetUseWorkingCopy(true);
  fOdfFilter->SetSizeThreshold(m_Controls->m_PeakThreshold->value());
  fOdfFilter->SetMaxNumDirections(m_Controls->m_MaxNumDirections->value());
  fOdfFilter->Update();

  QString name = m_SelectedFB.back()->GetName().c_str();

  if (m_Controls->m_NumDirectionsBox->isChecked())
  {
    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( fOdfFilter->GetNumDirectionsImage().GetPointer() );
    mitkImage->SetVolume( fOdfFilter->GetNumDirectionsImage()->GetBufferPointer() );

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitkImage);
    node->SetName((name+"_NUM_DIRECTIONS").toStdString().c_str());
    GetDataStorage()->Add(node, m_SelectedFB.back());
  }

  Image::Pointer mitkImage = dynamic_cast<Image*>(PeakImage::New().GetPointer());
  mitk::CastToMitkImage(fOdfFilter->GetDirectionImage(), mitkImage);
  mitkImage->SetVolume(fOdfFilter->GetDirectionImage()->GetBufferPointer());

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(mitkImage);
  node->SetName( (name+"_DIRECTIONS").toStdString().c_str());
  GetDataStorage()->Add(node, m_SelectedFB.back());
}

void QmitkFiberQuantificationView::UpdateGui()
{
  m_Controls->m_ProcessFiberBundleButton->setEnabled(!m_SelectedFB.empty());
  m_Controls->m_ExtractFiberPeaks->setEnabled(!m_SelectedFB.empty());
}

void QmitkFiberQuantificationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  //reset existing Vectors containing FiberBundles and PlanarFigures from a previous selection
  m_SelectedFB.clear();
  m_SelectedImage = nullptr;

  for (mitk::DataNode::Pointer node: nodes)
  {
    if ( dynamic_cast<mitk::FiberBundle*>(node->GetData()) )
    {
      m_SelectedFB.push_back(node);
    }
    else if (dynamic_cast<mitk::Image*>(node->GetData()))
      m_SelectedImage = dynamic_cast<mitk::Image*>(node->GetData());
  }
  UpdateGui();
  GenerateStats();
}

void QmitkFiberQuantificationView::GenerateStats()
{
  if ( m_SelectedFB.empty() )
    return;

  QString stats("");

  for( unsigned int i=0; i<m_SelectedFB.size(); i++ )
  {
    mitk::DataNode::Pointer node = m_SelectedFB[i];
    if (node.IsNotNull() && dynamic_cast<mitk::FiberBundle*>(node->GetData()))
    {
      if (i>0)
        stats += "\n-----------------------------\n";
      stats += QString(node->GetName().c_str()) + "\n";
      mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
      stats += "Number of fibers: "+ QString::number(fib->GetNumFibers()) + "\n";
      stats += "Number of points: "+ QString::number(fib->GetNumberOfPoints()) + "\n";
      stats += "Min. length:         "+ QString::number(fib->GetMinFiberLength(),'f',1) + " mm\n";
      stats += "Max. length:         "+ QString::number(fib->GetMaxFiberLength(),'f',1) + " mm\n";
      stats += "Mean length:         "+ QString::number(fib->GetMeanFiberLength(),'f',1) + " mm\n";
      stats += "Median length:       "+ QString::number(fib->GetMedianFiberLength(),'f',1) + " mm\n";
      stats += "Standard deviation:  "+ QString::number(fib->GetLengthStDev(),'f',1) + " mm\n";

      vtkSmartPointer<vtkFloatArray> weights = fib->GetFiberWeights();

      if (weights!=nullptr)
      {
        std::vector< float > weights2;
        for (int i=0; i<weights->GetSize(); i++)
          weights2.push_back(weights->GetValue(i));

        std::sort(weights2.begin(), weights2.end());

        stats += "\nFiber weight statistics\n";
        stats += "Min: " + QString::number(weights2.front()) + "\n";
        stats += "1% quantile: " + QString::number(weights2.at(weights2.size()*0.01)) + "\n";
        stats += "5% quantile: " + QString::number(weights2.at(weights2.size()*0.05)) + "\n";
        stats += "25% quantile: " + QString::number(weights2.at(weights2.size()*0.25)) + "\n";
        stats += "Median: " + QString::number(weights2.at(weights2.size()*0.5)) + "\n";
        stats += "75% quantile: " + QString::number(weights2.at(weights2.size()*0.75)) + "\n";
        stats += "95% quantile: " + QString::number(weights2.at(weights2.size()*0.95)) + "\n";
        stats += "99% quantile: " + QString::number(weights2.at(weights2.size()*0.99)) + "\n";
        stats += "Max: " + QString::number(weights2.back()) + "\n";
      }
      else
        stats += "No fiber weight array found.\n";
    }
  }
  this->m_Controls->m_StatsTextEdit->setText(stats);
}

void QmitkFiberQuantificationView::ProcessSelectedBundles()
{
  if ( m_SelectedFB.empty() ){
    QMessageBox::information( nullptr, "Warning", "No fibe bundle selected!");
    MITK_WARN("QmitkFiberQuantificationView") << "no fibe bundle selected";
    return;
  }

  int generationMethod = m_Controls->m_GenerationBox->currentIndex();

  for( unsigned int i=0; i<m_SelectedFB.size(); i++ )
  {
    mitk::DataNode::Pointer node = m_SelectedFB[i];
    if (node.IsNotNull() && dynamic_cast<mitk::FiberBundle*>(node->GetData()))
    {
      mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
      QString name(node->GetName().c_str());
      DataNode::Pointer newNode = nullptr;
      switch(generationMethod){
      case 0:
        newNode = GenerateTractDensityImage(fib, false, true);
        name += "_TDI";
        break;
      case 1:
        newNode = GenerateTractDensityImage(fib, false, false);
        name += "_TDI";
        break;
      case 2:
        newNode = GenerateTractDensityImage(fib, true, false);
        name += "_envelope";
        break;
      case 3:
        newNode = GenerateColorHeatmap(fib);
        break;
      case 4:
        newNode = GenerateFiberEndingsImage(fib);
        name += "_fiber_endings";
        break;
      case 5:
        newNode = GenerateFiberEndingsPointSet(fib);
        name += "_fiber_endings";
        break;
      }
      if (newNode.IsNotNull())
      {
        newNode->SetName(name.toStdString());
        GetDataStorage()->Add(newNode);
      }
    }
  }
}

// generate pointset displaying the fiber endings
mitk::DataNode::Pointer QmitkFiberQuantificationView::GenerateFiberEndingsPointSet(mitk::FiberBundle::Pointer fib)
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  vtkSmartPointer<vtkPolyData> fiberPolyData = fib->GetFiberPolyData();

  int count = 0;
  int numFibers = fib->GetNumFibers();
  for( int i=0; i<numFibers; i++ )
  {
    vtkCell* cell = fiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    if (numPoints>0)
    {
      double* point = points->GetPoint(0);
      itk::Point<float,3> itkPoint;
      itkPoint[0] = point[0];
      itkPoint[1] = point[1];
      itkPoint[2] = point[2];
      pointSet->InsertPoint(count, itkPoint);
      count++;
    }
    if (numPoints>2)
    {
      double* point = points->GetPoint(numPoints-1);
      itk::Point<float,3> itkPoint;
      itkPoint[0] = point[0];
      itkPoint[1] = point[1];
      itkPoint[2] = point[2];
      pointSet->InsertPoint(count, itkPoint);
      count++;
    }
  }

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData( pointSet );
  return node;
}

// generate image displaying the fiber endings
mitk::DataNode::Pointer QmitkFiberQuantificationView::GenerateFiberEndingsImage(mitk::FiberBundle::Pointer fib)
{
  typedef unsigned int OutPixType;
  typedef itk::Image<OutPixType,3> OutImageType;

  typedef itk::TractsToFiberEndingsImageFilter< OutImageType > ImageGeneratorType;
  ImageGeneratorType::Pointer generator = ImageGeneratorType::New();
  generator->SetFiberBundle(fib);
  generator->SetUpsamplingFactor(m_Controls->m_UpsamplingSpinBox->value());
  if (m_SelectedImage.IsNotNull())
  {
    OutImageType::Pointer itkImage = OutImageType::New();
    CastToItkImage(m_SelectedImage, itkImage);
    generator->SetInputImage(itkImage);
    generator->SetUseImageGeometry(true);
  }
  generator->Update();

  // get output image
  OutImageType::Pointer outImg = generator->GetOutput();
  mitk::Image::Pointer img = mitk::Image::New();
  img->InitializeByItk(outImg.GetPointer());
  img->SetVolume(outImg->GetBufferPointer());

  // init data node
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(img);
  return node;
}

// generate rgba heatmap from fiber bundle
mitk::DataNode::Pointer QmitkFiberQuantificationView::GenerateColorHeatmap(mitk::FiberBundle::Pointer fib)
{
  typedef itk::RGBAPixel<unsigned char> OutPixType;
  typedef itk::Image<OutPixType, 3> OutImageType;
  typedef itk::TractsToRgbaImageFilter< OutImageType > ImageGeneratorType;
  ImageGeneratorType::Pointer generator = ImageGeneratorType::New();
  generator->SetFiberBundle(fib);
  generator->SetUpsamplingFactor(m_Controls->m_UpsamplingSpinBox->value());
  if (m_SelectedImage.IsNotNull())
  {
    itk::Image<unsigned char, 3>::Pointer itkImage = itk::Image<unsigned char, 3>::New();
    CastToItkImage(m_SelectedImage, itkImage);
    generator->SetInputImage(itkImage);
    generator->SetUseImageGeometry(true);
  }
  generator->Update();

  // get output image
  typedef itk::Image<OutPixType,3> OutType;
  OutType::Pointer outImg = generator->GetOutput();
  mitk::Image::Pointer img = mitk::Image::New();
  img->InitializeByItk(outImg.GetPointer());
  img->SetVolume(outImg->GetBufferPointer());

  // init data node
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData(img);
  return node;
}

// generate tract density image from fiber bundle
mitk::DataNode::Pointer QmitkFiberQuantificationView::GenerateTractDensityImage(mitk::FiberBundle::Pointer fib, bool binary, bool absolute)
{
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  if (binary)
  {
    typedef unsigned char OutPixType;
    typedef itk::Image<OutPixType, 3> OutImageType;

    itk::TractDensityImageFilter< OutImageType >::Pointer generator = itk::TractDensityImageFilter< OutImageType >::New();
    generator->SetFiberBundle(fib);
    generator->SetBinaryOutput(binary);
    generator->SetOutputAbsoluteValues(absolute);
    generator->SetUpsamplingFactor(m_Controls->m_UpsamplingSpinBox->value());
    if (m_SelectedImage.IsNotNull())
    {
      OutImageType::Pointer itkImage = OutImageType::New();
      CastToItkImage(m_SelectedImage, itkImage);
      generator->SetInputImage(itkImage);
      generator->SetUseImageGeometry(true);

    }
    generator->Update();

    // get output image
    typedef itk::Image<OutPixType,3> OutType;
    OutType::Pointer outImg = generator->GetOutput();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());


    if (m_SelectedImage.IsNotNull())
    {
      mitk::LabelSetImage::Pointer multilabelImage = mitk::LabelSetImage::New();
      multilabelImage->InitializeByLabeledImage(img);



      mitk::Label::Pointer label = multilabelImage->GetActiveLabel();
      label->SetName("Tractogram");
//      label->SetColor(color);
      label->SetValue(1);
//      multilabelImage->GetActiveLabelSet()->AddLabel(label);
      multilabelImage->GetActiveLabelSet()->SetActiveLabel(1);

      PropertyList::Pointer dicomSegPropertyList = mitk::DICOMSegmentationPropertyHandler::GetDICOMSegmentationProperties(m_SelectedImage->GetPropertyList());

      multilabelImage->GetPropertyList()->ConcatenatePropertyList(dicomSegPropertyList);
      mitk::DICOMSegmentationPropertyHandler::GetDICOMSegmentProperties(multilabelImage->GetActiveLabel(multilabelImage->GetActiveLayer()));
      // init data node
      node->SetData(multilabelImage);
    }
    else
    {
      // init data node
      node->SetData(img);
    }

  }
  else
  {
    typedef float OutPixType;
    typedef itk::Image<OutPixType, 3> OutImageType;

    itk::TractDensityImageFilter< OutImageType >::Pointer generator = itk::TractDensityImageFilter< OutImageType >::New();
    generator->SetFiberBundle(fib);
    generator->SetBinaryOutput(binary);
    generator->SetOutputAbsoluteValues(absolute);
    generator->SetUpsamplingFactor(m_Controls->m_UpsamplingSpinBox->value());
    if (m_SelectedImage.IsNotNull())
    {
      OutImageType::Pointer itkImage = OutImageType::New();
      CastToItkImage(m_SelectedImage, itkImage);
      generator->SetInputImage(itkImage);
      generator->SetUseImageGeometry(true);

    }
    //generator->SetDoFiberResampling(false);
    generator->Update();

    // get output image
    typedef itk::Image<OutPixType,3> OutType;
    OutType::Pointer outImg = generator->GetOutput();
    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    // init data node
    node->SetData(img);
  }
  return node;
}
