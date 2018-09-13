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

#include "QmitkDiffusionQuantificationView.h"
#include "mitkDiffusionImagingConfigure.h"
#include "itkTimeProbe.h"
#include "itkImage.h"
#include "mitkNodePredicateDataType.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfImage.h"
#include <mitkTensorImage.h>
#include "mitkImageCast.h"
#include "mitkStatusBar.h"
#include "itkDiffusionOdfGeneralizedFaImageFilter.h"
#include "itkShiftScaleImageFilter.h"
#include "itkTensorFractionalAnisotropyImageFilter.h"
#include "itkTensorRelativeAnisotropyImageFilter.h"
#include "itkTensorDerivedMeasurementsFilter.h"
#include "QmitkDataStorageComboBox.h"
#include <QMessageBox>
#include "berryIWorkbenchWindow.h"
#include "berryISelectionService.h"
#include <mitkDiffusionPropertyHelper.h>
#include <itkAdcImageFilter.h>
#include <itkBallAndSticksImageFilter.h>
#include <itkMultiTensorImageFilter.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkITKImageImport.h>
#include <mitkLexicalCast.h>
#include <mitkNodePredicateIsDWI.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include <QmitkStyleManager.h>
#include <mitkLevelWindowProperty.h>
#include <mitkDiffusionFunctionCollection.h>

const std::string QmitkDiffusionQuantificationView::VIEW_ID = "org.mitk.views.diffusionquantification";


QmitkDiffusionQuantificationView::QmitkDiffusionQuantificationView()
  : QmitkAbstractView(),
    m_Controls(nullptr)
{

}

QmitkDiffusionQuantificationView::~QmitkDiffusionQuantificationView()
{

}

void QmitkDiffusionQuantificationView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkDiffusionQuantificationViewControls;
    m_Controls->setupUi(parent);

    m_Controls->m_BallStickButton->setVisible(false);
    m_Controls->m_MultiTensorButton->setVisible(false);

    connect( static_cast<QObject*>(m_Controls->m_GFAButton), SIGNAL(clicked()), this, SLOT(GFA()) );
    connect( static_cast<QObject*>(m_Controls->m_FAButton), SIGNAL(clicked()), this, SLOT(FA()) );
    connect( static_cast<QObject*>(m_Controls->m_RAButton), SIGNAL(clicked()), this, SLOT(RA()) );
    connect( static_cast<QObject*>(m_Controls->m_ADButton), SIGNAL(clicked()), this, SLOT(AD()) );
    connect( static_cast<QObject*>(m_Controls->m_RDButton), SIGNAL(clicked()), this, SLOT(RD()) );
    connect( static_cast<QObject*>(m_Controls->m_MDButton), SIGNAL(clicked()), this, SLOT(MD()) );
    connect( static_cast<QObject*>(m_Controls->m_MdDwiButton), SIGNAL(clicked()), this, SLOT(MD_DWI()) );
    connect( static_cast<QObject*>(m_Controls->m_AdcDwiButton), SIGNAL(clicked()), this, SLOT(ADC_DWI()) );
    connect( static_cast<QObject*>(m_Controls->m_ClusteringAnisotropy), SIGNAL(clicked()), this, SLOT(ClusterAnisotropy()) );
    connect( static_cast<QObject*>(m_Controls->m_BallStickButton), SIGNAL(clicked()), this, SLOT(DoBallStickCalculation()) );
    connect( static_cast<QObject*>(m_Controls->m_MultiTensorButton), SIGNAL(clicked()), this, SLOT(DoMultiTensorCalculation()) );

    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());
    mitk::TNodePredicateDataType<mitk::TensorImage>::Pointer isDti = mitk::TNodePredicateDataType<mitk::TensorImage>::New();
    mitk::TNodePredicateDataType<mitk::OdfImage>::Pointer isOdf = mitk::TNodePredicateDataType<mitk::OdfImage>::New();
    mitk::TNodePredicateDataType<mitk::ShImage>::Pointer isSh = mitk::TNodePredicateDataType<mitk::ShImage>::New();
    mitk::NodePredicateIsDWI::Pointer isDwi = mitk::NodePredicateIsDWI::New();
    m_Controls->m_ImageBox->SetPredicate( mitk::NodePredicateOr::New(isSh, mitk::NodePredicateOr::New(isDti, mitk::NodePredicateOr::New(isOdf, isDwi))) );

    connect( static_cast<QObject*>(m_Controls->m_ImageBox), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));
  }
}

void QmitkDiffusionQuantificationView::SetFocus()
{
  m_Controls->m_ScaleImageValuesBox->setFocus();
}

void QmitkDiffusionQuantificationView::UpdateGui()
{
  bool foundOdfVolume = false;
  bool foundTensorVolume = false;
  bool foundDwVolume = false;
  mitk::DataNode::Pointer  selNode = m_Controls->m_ImageBox->GetSelectedNode();

  if( selNode.IsNotNull() && (dynamic_cast<mitk::OdfImage*>(selNode->GetData()) || dynamic_cast<mitk::ShImage*>(selNode->GetData())) )
    foundOdfVolume = true;
  else if( selNode.IsNotNull() && dynamic_cast<mitk::TensorImage*>(selNode->GetData()) )
    foundTensorVolume = true;
  else if( selNode.IsNotNull())
    foundDwVolume = true;

  m_Controls->m_GFAButton->setEnabled(foundOdfVolume);
  m_Controls->m_FAButton->setEnabled(foundTensorVolume);
  m_Controls->m_RAButton->setEnabled(foundTensorVolume);
  m_Controls->m_ADButton->setEnabled(foundTensorVolume);
  m_Controls->m_RDButton->setEnabled(foundTensorVolume);
  m_Controls->m_MDButton->setEnabled(foundTensorVolume);
  m_Controls->m_ClusteringAnisotropy->setEnabled(foundTensorVolume);
  m_Controls->m_AdcDwiButton->setEnabled(foundDwVolume);
  m_Controls->m_MdDwiButton->setEnabled(foundDwVolume);
  m_Controls->m_BallStickButton->setEnabled(foundDwVolume);
  m_Controls->m_MultiTensorButton->setEnabled(foundDwVolume);
}

void QmitkDiffusionQuantificationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& )
{
  UpdateGui();
}

void QmitkDiffusionQuantificationView::ADC_DWI()
{
  DoAdcCalculation(true);
}

void QmitkDiffusionQuantificationView::MD_DWI()
{
  DoAdcCalculation(false);
}

void QmitkDiffusionQuantificationView::DoBallStickCalculation()
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
  {
    mitk::DataNode* node = m_Controls->m_ImageBox->GetSelectedNode();
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    typedef itk::BallAndSticksImageFilter< short, double > FilterType;

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkVectorImagePointer );
    filter->SetGradientDirections(mitk::DiffusionPropertyHelper::GetGradientContainer(image));
    filter->SetB_value(static_cast<double>(mitk::DiffusionPropertyHelper::GetReferenceBValue(image)));
    filter->Update();

    mitk::Image::Pointer newImage = mitk::Image::New();
    newImage->InitializeByItk( filter->GetOutput() );
    newImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_f").toStdString().c_str());
    GetDataStorage()->Add(imageNode, node);

    {
      FilterType::PeakImageType::Pointer itkImg = filter->GetPeakImage();
      mitk::Image::Pointer newImage = mitk::Image::New();
      CastToMitkImage(itkImg, newImage);

      mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
      imageNode->SetData( newImage );
      QString name = node->GetName().c_str();

      imageNode->SetName((name+"_Sticks").toStdString().c_str());
      GetDataStorage()->Add(imageNode, node);
    }

    {
      mitk::Image::Pointer dOut = mitk::GrabItkImageMemory( filter->GetOutDwi().GetPointer() );

      mitk::DiffusionPropertyHelper::SetGradientContainer(dOut, mitk::DiffusionPropertyHelper::GetGradientContainer(image) );
      mitk::DiffusionPropertyHelper::SetReferenceBValue(dOut, mitk::DiffusionPropertyHelper::GetReferenceBValue(image) );
      mitk::DiffusionPropertyHelper::InitializeImage(dOut);

      mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
      imageNode->SetData(dOut);
      QString name = node->GetName().c_str();
      imageNode->SetName((name+"_Estimated-DWI").toStdString().c_str());
      GetDataStorage()->Add(imageNode, node);
    }
  }
}

void QmitkDiffusionQuantificationView::DoMultiTensorCalculation()
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull()) // for all items
  {
    mitk::DataNode* node = m_Controls->m_ImageBox->GetSelectedNode();
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    typedef itk::MultiTensorImageFilter< short, double > FilterType;

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkVectorImagePointer );
    filter->SetGradientDirections(mitk::DiffusionPropertyHelper::GetGradientContainer(image));
    filter->SetB_value(static_cast<double>(mitk::DiffusionPropertyHelper::GetReferenceBValue(image)));
    filter->Update();

    typedef mitk::TensorImage::ItkTensorImageType TensorImageType;
    for (unsigned int i=0; i<NUM_TENSORS; i++)
    {
      TensorImageType::Pointer tensorImage = filter->GetTensorImages().at(i);
      mitk::TensorImage::Pointer image = mitk::TensorImage::New();
      image->InitializeByItk( tensorImage.GetPointer() );
      image->SetVolume( tensorImage->GetBufferPointer() );

      mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
      imageNode->SetData( image );
      QString name = node->GetName().c_str();
      name.append("_Tensor");
      name.append(boost::lexical_cast<std::string>(i).c_str());
      imageNode->SetName(name.toStdString().c_str());
      GetDataStorage()->Add(imageNode, node);
    }
  }
}

void QmitkDiffusionQuantificationView::DoAdcCalculation(bool fit)
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
  {
    mitk::DataNode* node = m_Controls->m_ImageBox->GetSelectedNode();
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    typedef itk::AdcImageFilter< short, double > FilterType;

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkVectorImagePointer );

    filter->SetGradientDirections( mitk::DiffusionPropertyHelper::GetGradientContainer(image) );
    filter->SetB_value( static_cast<double>(mitk::DiffusionPropertyHelper::GetReferenceBValue(image)) );

    filter->SetFitSignal(fit);
    filter->Update();

    typedef itk::ShiftScaleImageFilter<itk::AdcImageFilter< short, double >::OutputImageType, itk::AdcImageFilter< short, double >::OutputImageType> ShiftScaleFilterType;
    ShiftScaleFilterType::Pointer multi = ShiftScaleFilterType::New();
    multi->SetShift(0.0);
    multi->SetScale(m_Controls->m_ScaleImageValuesBox->value());
    multi->SetInput(filter->GetOutput());
    multi->Update();

    mitk::Image::Pointer newImage = mitk::Image::New();
    newImage->InitializeByItk( multi->GetOutput() );
    newImage->SetVolume( multi->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();

    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    lut->SetType( mitk::LookupTable::JET_TRANSPARENT );
    mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
    lut_prop->SetLookupTable( lut );

    imageNode->SetProperty("LookupTable", lut_prop );
    if (fit)
      imageNode->SetName((name+"_ADC").toStdString().c_str());
    else
      imageNode->SetName((name+"_MD").toStdString().c_str());

    mitk::LevelWindow lw;
    lw.SetLevelWindow(0.0015, 0.003);
    imageNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( lw ) );
    GetDataStorage()->Add(imageNode, node);
  }
}

void QmitkDiffusionQuantificationView::GFA()
{
  OdfQuantify(0);
}

void QmitkDiffusionQuantificationView::FA()
{
  TensorQuantify(0);
}

void QmitkDiffusionQuantificationView::RA()
{
  TensorQuantify(1);
}

void QmitkDiffusionQuantificationView::AD()
{
  TensorQuantify(2);
}

void QmitkDiffusionQuantificationView::RD()
{
  TensorQuantify(3);
}

void QmitkDiffusionQuantificationView::ClusterAnisotropy()
{
  TensorQuantify(4);
}

void QmitkDiffusionQuantificationView::MD()
{
  TensorQuantify(5);
}

void QmitkDiffusionQuantificationView::OdfQuantify(int method)
{
  OdfQuantification(method);
}

void QmitkDiffusionQuantificationView::TensorQuantify(int method)
{
  TensorQuantification(method);
}

void QmitkDiffusionQuantificationView::OdfQuantification(int method)
{
  QString status;

  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
  {
    mitk::DataNode* node = m_Controls->m_ImageBox->GetSelectedNode();

    typedef float TOdfPixelType;
    typedef itk::Vector<TOdfPixelType,ODF_SAMPLING_SIZE> OdfVectorType;
    typedef itk::Image<OdfVectorType,3> OdfVectorImgType;
    mitk::Image::Pointer vol = dynamic_cast<mitk::Image*>(node->GetData());

    OdfVectorImgType::Pointer itkvol;
    if (dynamic_cast<mitk::ShImage*>(vol.GetPointer()))
      itkvol = mitk::convert::GetItkOdfFromShImage(vol);
    else
      itkvol = mitk::convert::GetItkOdfFromOdfImage(vol);
    std::string nodename = node->GetName();

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Computing GFA for %s", nodename.c_str()).toLatin1());
    typedef itk::DiffusionOdfGeneralizedFaImageFilter<TOdfPixelType,TOdfPixelType,ODF_SAMPLING_SIZE> GfaFilterType;
    GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
    gfaFilter->SetInput(itkvol);

    std::string newname;
    newname.append(nodename);
    switch(method)
    {
    case 0:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
      newname.append("GFA");
      break;
    }
    case 1:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILES_HIGH_LOW);
      newname.append("01");
      break;
    }
    case 2:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILE_HIGH);
      newname.append("02");
      break;
    }
    case 3:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_MAX_ODF_VALUE);
      newname.append("03");
      break;
    }
    case 4:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_DECONVOLUTION_COEFFS);
      newname.append("04");
      break;
    }
    case 5:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_MIN_MAX_NORMALIZED_STANDARD);
      newname.append("05");
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
      break;
    }
    case 8:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILES_LOW_HIGH);
      newname.append("08");
      break;
    }
    case 9:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILE_LOW);
      newname.append("09");
      break;
    }
    case 10:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_MIN_ODF_VALUE);
      newname.append("10");
      break;
    }
    case 11:
    {
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_STD_BY_MAX);
      newname.append("11");
      break;
    }
    default:
    {
      newname.append("0");
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
    }
    }
    gfaFilter->Update();

    typedef itk::Image<TOdfPixelType, 3> ImgType;
    ImgType::Pointer img = ImgType::New();
    img->SetSpacing( gfaFilter->GetOutput()->GetSpacing() );   // Set the image spacing
    img->SetOrigin( gfaFilter->GetOutput()->GetOrigin() );     // Set the image origin
    img->SetDirection( gfaFilter->GetOutput()->GetDirection() );  // Set the image direction
    img->SetLargestPossibleRegion( gfaFilter->GetOutput()->GetLargestPossibleRegion());
    img->SetBufferedRegion( gfaFilter->GetOutput()->GetLargestPossibleRegion() );
    img->Allocate();
    itk::ImageRegionIterator<ImgType> ot (img, img->GetLargestPossibleRegion() );
    ot.GoToBegin();
    itk::ImageRegionConstIterator<GfaFilterType::OutputImageType> it
        (gfaFilter->GetOutput(), gfaFilter->GetOutput()->GetLargestPossibleRegion() );

    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      GfaFilterType::OutputImageType::PixelType val = it.Get();
      ot.Set(val * m_Controls->m_ScaleImageValuesBox->value());
      ++ot;
    }


    // GFA TO DATATREE
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( img.GetPointer() );
    image->SetVolume( img->GetBufferPointer() );
    mitk::DataNode::Pointer new_node=mitk::DataNode::New();
    new_node->SetData( image );
    new_node->SetProperty( "name", mitk::StringProperty::New(newname) );

    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    lut->SetType( mitk::LookupTable::JET_TRANSPARENT );
    mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
    lut_prop->SetLookupTable( lut );
    new_node->SetProperty("LookupTable", lut_prop );

    mitk::LevelWindow lw;
    lw.SetLevelWindow(0.5, 1.0);
    new_node->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( lw ) );

    GetDataStorage()->Add(new_node, node);

    mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");
  }

  this->GetRenderWindowPart()->RequestUpdate();

}

void QmitkDiffusionQuantificationView::TensorQuantification(int method)
{
  QString status;

  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
  {
    mitk::DataNode* node = m_Controls->m_ImageBox->GetSelectedNode();

    typedef mitk::TensorImage::ScalarPixelType    TTensorPixelType;
    typedef mitk::TensorImage::ItkTensorImageType TensorImageType;

    mitk::Image* vol = static_cast<mitk::Image*>(node->GetData());
    TensorImageType::Pointer itkvol = TensorImageType::New();
    mitk::CastToItkImage(vol, itkvol);

    std::string nodename = node->GetName();

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Computing FA for %s", nodename.c_str()).toLatin1());
    typedef itk::Image< TTensorPixelType, 3 > FAImageType;

    typedef itk::ShiftScaleImageFilter<FAImageType, FAImageType> ShiftScaleFilterType;
    ShiftScaleFilterType::Pointer multi = ShiftScaleFilterType::New();
    multi->SetShift(0.0);
    multi->SetScale(m_Controls->m_ScaleImageValuesBox->value());

    typedef itk::TensorDerivedMeasurementsFilter<TTensorPixelType> MeasurementsType;

    mitk::LevelWindow lw;
    if(method == 0) //FA
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::FA);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_FA").toStdString();
      lw.SetLevelWindow(0.5, 1.0);
    }
    else if(method == 1) //RA
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::RA);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_RA").toStdString();
      lw.SetLevelWindow(0.015, 0.03);
    }
    else if(method == 2) // AD (Axial diffusivity)
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::AD);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_AD").toStdString();
      lw.SetLevelWindow(0.0015, 0.003);
    }
    else if(method == 3) // RD (Radial diffusivity, (Lambda2+Lambda3)/2
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::RD);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_RD").toStdString();
      lw.SetLevelWindow(0.0015, 0.003);
    }
    else if(method == 4) // 1-(Lambda2+Lambda3)/(2*Lambda1)
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::CA);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_CA").toStdString();
      lw.SetLevelWindow(0.5, 1.0);
    }
    else if(method == 5) // MD (Mean Diffusivity, (Lambda1+Lambda2+Lambda3)/3 )
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::MD);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_MD").toStdString();
      lw.SetLevelWindow(0.0015, 0.003);
    }

    multi->Update();

    // FA TO DATATREE
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( multi->GetOutput() );
    image->SetVolume( multi->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer new_node=mitk::DataNode::New();
    new_node->SetData( image );
    new_node->SetProperty( "name", mitk::StringProperty::New(nodename) );

    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    lut->SetType( mitk::LookupTable::JET_TRANSPARENT );
    mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
    lut_prop->SetLookupTable( lut );
    new_node->SetProperty("LookupTable", lut_prop );
    new_node->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( lw ) );

    GetDataStorage()->Add(new_node, node);

    mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");
  }

  this->GetRenderWindowPart()->RequestUpdate();

}
