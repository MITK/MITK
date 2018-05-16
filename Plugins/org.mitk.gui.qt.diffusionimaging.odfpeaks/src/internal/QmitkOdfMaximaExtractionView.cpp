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

//misc
#include <QFileDialog>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkOdfMaximaExtractionView.h"

#include <mitkImageCast.h>
#include <mitkFiberBundle.h>
#include <mitkImage.h>
#include <mitkImageToItk.h>
#include <mitkTensorImage.h>
#include <itkVectorImage.h>
#include <itkOdfMaximaExtractionFilter.h>
#include <itkFiniteDiffOdfMaximaExtractionFilter.h>
#include <itkShCoefficientImageImporter.h>
#include <itkDiffusionTensorPrincipalDirectionImageFilter.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateIsDWI.h>
#include <mitkPeakImage.h>
#include <mitkShImage.h>

// Qt
#include <QMessageBox>

const std::string QmitkOdfMaximaExtractionView::VIEW_ID = "org.mitk.views.odfmaximaextractionview";
using namespace mitk;

QmitkOdfMaximaExtractionView::QmitkOdfMaximaExtractionView()
  : m_Controls(nullptr)
{

}

// Destructor
QmitkOdfMaximaExtractionView::~QmitkOdfMaximaExtractionView()
{

}


void QmitkOdfMaximaExtractionView::CreateQtPartControl(QWidget *parent)
{
  // build up qt view, unless already done
  if (!m_Controls)
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkOdfMaximaExtractionViewControls;
    m_Controls->setupUi(parent);

    connect((QObject*)m_Controls->m_StartPeakExtractionButton, SIGNAL(clicked()), (QObject*) this, SLOT(StartPeakExtraction()));
    connect((QObject*)m_Controls->m_ImportShCoeffs, SIGNAL(clicked()), (QObject*) this, SLOT(ConvertShCoeffs()));

    m_Controls->m_MaskBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());

    mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateNot::Pointer isDwi = mitk::NodePredicateNot::New(mitk::NodePredicateIsDWI::New());
    mitk::NodePredicateNot::Pointer isOdf = mitk::NodePredicateNot::New(mitk::NodePredicateDataType::New("OdfImage"));
    mitk::NodePredicateAnd::Pointer unwanted = mitk::NodePredicateAnd::New(isOdf, isDwi);
    mitk::NodePredicateDimension::Pointer dim3 = mitk::NodePredicateDimension::New(3);
    mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));

    m_Controls->m_MaskBox->SetPredicate(mitk::NodePredicateAnd::New(mitk::NodePredicateAnd::New(unwanted, dim3), isBinaryPredicate));
    m_Controls->m_ImageBox->SetPredicate(mitk::NodePredicateAnd::New(mitk::NodePredicateAnd::New(unwanted, isMitkImage), mitk::NodePredicateNot::New(isBinaryPredicate)));
    m_Controls->m_MaskBox->SetZeroEntryText("--");

    connect( (QObject*)(m_Controls->m_ImageBox), SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnImageSelectionChanged()) );

    m_Controls->m_StartPeakExtractionButton->setVisible(false);
    m_Controls->m_ImportShCoeffs->setVisible(false);
  }
}

void QmitkOdfMaximaExtractionView::SetFocus()
{
}

void QmitkOdfMaximaExtractionView::StartPeakExtraction()
{
  if (dynamic_cast<TensorImage*>(m_Controls->m_ImageBox->GetSelectedNode()->GetData()) != nullptr)
  {
    StartTensorPeakExtraction(dynamic_cast<TensorImage*>(m_Controls->m_ImageBox->GetSelectedNode()->GetData()));
  }
  else
  {
    StartMaximaExtraction(dynamic_cast<Image*>(m_Controls->m_ImageBox->GetSelectedNode()->GetData()));
  }
}

template<int shOrder>
void QmitkOdfMaximaExtractionView::TemplatedConvertShCoeffs(mitk::Image* mitkImg)
{
  typedef itk::ShCoefficientImageImporter< float, shOrder > FilterType;
  typedef mitk::ImageToItk< itk::Image< float, 4 > > CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput(mitkImg);
  caster->Update();

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInputImage(caster->GetOutput());
  filter->GenerateData();
  typename FilterType::CoefficientImageType::Pointer itkCi = filter->GetCoefficientImage();

  {
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::ShImage::New().GetPointer());
    img->InitializeByItk(itkCi.GetPointer());
    img->SetVolume(itkCi->GetBufferPointer());
    DataNode::Pointer node = DataNode::New();
    node->SetData(img);

    QString name(m_Controls->m_ImageBox->GetSelectedNode()->GetName().c_str());
    name += "_ShImage_Imported";
    node->SetName(name.toStdString().c_str());

    GetDataStorage()->Add(node, m_Controls->m_ImageBox->GetSelectedNode());
  }
}

void QmitkOdfMaximaExtractionView::ConvertShCoeffs()
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNull())
    return;

  Image::Pointer mitkImg = dynamic_cast<Image*>(m_Controls->m_ImageBox->GetSelectedNode()->GetData());
  if (mitkImg->GetDimension() != 4 && mitkImg->GetLargestPossibleRegion().GetSize()[3]<6)
  {
    MITK_INFO << "wrong image type (need 4 dimensions)";
    return;
  }

  int nrCoeffs = mitkImg->GetLargestPossibleRegion().GetSize()[3];
  switch (nrCoeffs)
  {
  case 6:
    TemplatedConvertShCoeffs<2>(mitkImg);
    break;
  case 15:
    TemplatedConvertShCoeffs<4>(mitkImg);
    break;
  case 28:
    TemplatedConvertShCoeffs<6>(mitkImg);
    break;
  case 45:
    TemplatedConvertShCoeffs<8>(mitkImg);
    break;
  case 66:
    TemplatedConvertShCoeffs<10>(mitkImg);
    break;
  case 91:
    TemplatedConvertShCoeffs<12>(mitkImg);
    break;
  default :
    QMessageBox::warning(nullptr, "Error", "Only spherical harmonics orders 2-12 are supported.", QMessageBox::Ok);
  }
}

void QmitkOdfMaximaExtractionView::StartTensorPeakExtraction(mitk::TensorImage* img)
{
  typedef itk::DiffusionTensorPrincipalDirectionImageFilter< float > MaximaExtractionFilterType;
  MaximaExtractionFilterType::Pointer filter = MaximaExtractionFilterType::New();
  filter->SetUsePolarCoordinates(false);

  mitk::BaseGeometry::Pointer geometry;
  try{
    ItkTensorImage::Pointer itkImage = ItkTensorImage::New();
    CastToItkImage(img, itkImage);
    filter->SetInput(itkImage);
    geometry = img->GetGeometry();
  }
  catch (itk::ExceptionObject &e)
  {
    MITK_INFO << "wrong image type: " << e.what();
    QMessageBox::warning(nullptr, "Wrong pixel type", "Could not perform Tensor Principal Direction Extraction due to Image has wrong pixel type.", QMessageBox::Ok);
    return;
  }

  if (m_Controls->m_MaskBox->GetSelectedNode().IsNotNull())
  {
    ItkUcharImgType::Pointer itkMaskImage = ItkUcharImgType::New();
    Image::Pointer mitkMaskImg = dynamic_cast<Image*>(m_Controls->m_MaskBox->GetSelectedNode()->GetData());
    CastToItkImage(mitkMaskImg, itkMaskImage);
    filter->SetMaskImage(itkMaskImage);
  }

  if (m_Controls->m_NormalizationBox->currentIndex() == 0)
    filter->SetNormalizeVectors(false);
  filter->SetFaThreshold(m_Controls->m_AbsoluteThresholdBox->value());

  filter->Update();

  MaximaExtractionFilterType::PeakImageType::Pointer itkImg = filter->GetPeakImage();
  mitk::Image::Pointer mitkPeakImage = dynamic_cast<Image*>(PeakImage::New().GetPointer());
  CastToMitkImage(itkImg, mitkPeakImage);

  DataNode::Pointer node = DataNode::New();
  node->SetData(mitkPeakImage);
  QString name(m_Controls->m_ImageBox->GetSelectedNode()->GetName().c_str());
  name += "_PrincipalDirection";
  node->SetName(name.toStdString().c_str());
  GetDataStorage()->Add(node, m_Controls->m_ImageBox->GetSelectedNode());

  if (m_Controls->m_OutputNumDirectionsBox->isChecked())
  {
    ItkUcharImgType::Pointer numDirImage = filter->GetOutput();
    mitk::Image::Pointer image2 = mitk::Image::New();
    image2->InitializeByItk(numDirImage.GetPointer());
    image2->SetVolume(numDirImage->GetBufferPointer());
    DataNode::Pointer node2 = DataNode::New();
    node2->SetData(image2);
    QString name(m_Controls->m_ImageBox->GetSelectedNode()->GetName().c_str());
    name += "_NumDirections";
    node2->SetName(name.toStdString().c_str());
    GetDataStorage()->Add(node2, m_Controls->m_ImageBox->GetSelectedNode());
  }
}

template<int shOrder>
void QmitkOdfMaximaExtractionView::StartMaximaExtraction(Image *image)
{
  typedef itk::FiniteDiffOdfMaximaExtractionFilter< float, shOrder, 10000 > MaximaExtractionFilterType;
  typename MaximaExtractionFilterType::Pointer filter = MaximaExtractionFilterType::New();

  switch (m_Controls->m_ToolkitBox->currentIndex())
  {
  case 0:
    filter->SetToolkit(MaximaExtractionFilterType::MRTRIX);
    break;
  case 1:
    filter->SetToolkit(MaximaExtractionFilterType::FSL);
    break;
  default:
    filter->SetToolkit(MaximaExtractionFilterType::MRTRIX);
  }

  mitk::BaseGeometry::Pointer geometry;
  try{
    typedef ImageToItk< typename MaximaExtractionFilterType::CoefficientImageType > CasterType;
    typename CasterType::Pointer caster = CasterType::New();
    caster->SetInput(image);
    caster->Update();
    filter->SetInput(caster->GetOutput());
    geometry = image->GetGeometry();
  }
  catch (itk::ExceptionObject &e)
  {
    MITK_INFO << "wrong image type: " << e.what();
    QMessageBox::warning(nullptr, "Wrong pixel type", "Could not perform Finite Differences Extraction due to Image has wrong pixel type.", QMessageBox::Ok);
    return;
  }

  filter->SetAngularThreshold(cos((float)m_Controls->m_AngularThreshold->value()*itk::Math::pi / 180));
  filter->SetClusteringThreshold(cos((float)m_Controls->m_ClusteringAngleBox->value()*itk::Math::pi / 180));
  filter->SetMaxNumPeaks(m_Controls->m_MaxNumPeaksBox->value());
  filter->SetPeakThreshold(m_Controls->m_PeakThresholdBox->value());
  filter->SetAbsolutePeakThreshold(m_Controls->m_AbsoluteThresholdBox->value());

  if (m_Controls->m_MaskBox->GetSelectedNode().IsNotNull())
  {
    ItkUcharImgType::Pointer itkMaskImage = ItkUcharImgType::New();
    Image::Pointer mitkMaskImg = dynamic_cast<Image*>(m_Controls->m_MaskBox->GetSelectedNode()->GetData());
    CastToItkImage(mitkMaskImg, itkMaskImage);
    filter->SetMaskImage(itkMaskImage);
  }

  switch (m_Controls->m_NormalizationBox->currentIndex())
  {
  case 0:
    filter->SetNormalizationMethod(MaximaExtractionFilterType::NO_NORM);
    break;
  case 1:
    filter->SetNormalizationMethod(MaximaExtractionFilterType::MAX_VEC_NORM);
    break;
  case 2:
    filter->SetNormalizationMethod(MaximaExtractionFilterType::SINGLE_VEC_NORM);
    break;
  }

  filter->Update();

  typename MaximaExtractionFilterType::PeakImageType::Pointer itkImg = filter->GetPeakImage();
  mitk::Image::Pointer img = dynamic_cast<Image*>(PeakImage::New().GetPointer());
  CastToMitkImage(itkImg, img);

  DataNode::Pointer node = DataNode::New();
  node->SetData(img);
  QString name(m_Controls->m_ImageBox->GetSelectedNode()->GetName().c_str());
  name += "_PEAKS";
  node->SetName(name.toStdString().c_str());
  GetDataStorage()->Add(node, m_Controls->m_ImageBox->GetSelectedNode());

  if (m_Controls->m_OutputNumDirectionsBox->isChecked())
  {
    ItkUcharImgType::Pointer numDirImage = filter->GetNumDirectionsImage();
    mitk::Image::Pointer image2 = mitk::Image::New();
    CastToMitkImage(numDirImage, image2);

    DataNode::Pointer node2 = DataNode::New();
    node2->SetData(image2);
    QString name(m_Controls->m_ImageBox->GetSelectedNode()->GetName().c_str());
    name += "_NUM_DIRECTIONS";
    node2->SetName(name.toStdString().c_str());
    GetDataStorage()->Add(node2, m_Controls->m_ImageBox->GetSelectedNode());
  }
}

void QmitkOdfMaximaExtractionView::StartMaximaExtraction(Image* img)
{
  mitk::PixelType pixT = img->GetPixelType();

  switch (pixT.GetNumberOfComponents())
  {
  case 6:
    StartMaximaExtraction<2>(img);
    break;
  case 15:
    StartMaximaExtraction<4>(img);
    break;
  case 28:
    StartMaximaExtraction<6>(img);
    break;
  case 45:
    StartMaximaExtraction<8>(img);
    break;
  case 66:
    StartMaximaExtraction<10>(img);
    break;
  case 91:
    StartMaximaExtraction<12>(img);
    break;
  default :
    QMessageBox::warning(nullptr, "Error", "Only spherical harmonics orders 2-12 are supported.", QMessageBox::Ok);
  }
}

void QmitkOdfMaximaExtractionView::OnSelectionChanged(berry::IWorkbenchPart::Pointer , const QList<mitk::DataNode::Pointer>& nodes)
{
  (void) nodes;
  this->OnImageSelectionChanged();
}

void QmitkOdfMaximaExtractionView::OnImageSelectionChanged()
{
  m_Controls->m_StartPeakExtractionButton->setVisible(false);
  m_Controls->m_ImportShCoeffs->setVisible(false);

  mitk::DataNode::Pointer node = m_Controls->m_ImageBox->GetSelectedNode();
  if (node.IsNull())
    return;

  Image::Pointer img = dynamic_cast<Image*>(node->GetData());
  if (img->GetDimension()==4)
    m_Controls->m_ImportShCoeffs->setVisible(true);
  else
    m_Controls->m_StartPeakExtractionButton->setVisible(true);
}
