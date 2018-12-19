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
#include "QmitkDipyReconstructionsView.h"

#include <mitkNodePredicateIsDWI.h>
#include <mitkIPythonService.h>
#include <boost/lexical_cast.hpp>
#include <QFile>
#include <QMessageBox>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkOdfImage.h>
#include <mitkShImage.h>
#include <mitkImageCast.h>
#include <itkVectorImageToFourDImageFilter.h>
#include <mitkPeakImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDimension.h>
#include <itkShCoefficientImageImporter.h>

const std::string QmitkDipyReconstructionsView::VIEW_ID = "org.mitk.views.dipyreconstruction";

QmitkDipyReconstructionsView::QmitkDipyReconstructionsView()
  : QmitkAbstractView()
  , m_Controls( 0 )
{

}

// Destructor
QmitkDipyReconstructionsView::~QmitkDipyReconstructionsView()
{
}

void QmitkDipyReconstructionsView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkDipyReconstructionsViewControls;
    m_Controls->setupUi( parent );
    connect( m_Controls->m_ImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGUI()) );
    connect( m_Controls->m_StartButton, SIGNAL(clicked()), this, SLOT(StartFit()) );
    connect( m_Controls->m_ModelBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGUI()) );
    this->m_Parent = parent;

    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());
    mitk::NodePredicateIsDWI::Pointer isDwi = mitk::NodePredicateIsDWI::New();
    m_Controls->m_ImageBox->SetPredicate( isDwi );

    mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImagePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateAnd::Pointer isBinary3dImage = mitk::NodePredicateAnd::New( mitk::NodePredicateAnd::New( isImagePredicate, isBinaryPredicate ), mitk::NodePredicateDimension::New(3));

    m_Controls->m_MaskBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MaskBox->SetPredicate( isBinary3dImage );
    m_Controls->m_MaskBox->SetZeroEntryText("--");

    UpdateGUI();
  }
}

void QmitkDipyReconstructionsView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& )
{
}

void QmitkDipyReconstructionsView::UpdateGUI()
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
    m_Controls->m_StartButton->setEnabled(true);
  else
    m_Controls->m_StartButton->setEnabled(false);

  m_Controls->m_ShoreBox->setVisible(false);
  m_Controls->m_SfmBox->setVisible(false);
  m_Controls->m_CsdBox->setVisible(false);
  m_Controls->m_CsaBox->setVisible(false);
  m_Controls->m_OpdtBox->setVisible(false);
  switch(m_Controls->m_ModelBox->currentIndex())
  {
  case 0:
  {
    m_Controls->m_ShoreBox->setVisible(true);
    break;
  }
  case 1:
  {
    m_Controls->m_SfmBox->setVisible(true);
    break;
  }
  case 2:
  {
    m_Controls->m_CsdBox->setVisible(true);
    break;
  }
  case 3:
  {
    m_Controls->m_CsaBox->setVisible(true);
    break;
  }
  case 4:
  {
    m_Controls->m_OpdtBox->setVisible(true);
    break;
  }
  }
}

void QmitkDipyReconstructionsView::SetFocus()
{
  UpdateGUI();
  m_Controls->m_StartButton->setFocus();
}

void QmitkDipyReconstructionsView::StartFit()
{
  mitk::DataNode::Pointer node = m_Controls->m_ImageBox->GetSelectedNode();
  mitk::Image::Pointer input_image = dynamic_cast<mitk::Image*>(node->GetData());

  // get python script as string
  QString data;
  QString fileName(":/QmitkDiffusionImaging/dipy_reconstructions.py");
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    qDebug()<<"filenot opened"<<endl;
  }
  else
  {
    qDebug()<<"file opened"<<endl;
    data = file.readAll();
  }
  file.close();

  // get bvals and bvecs
  std::locale::global(std::locale::classic());
  std::string bvals = "[";
  std::string bvecs = "[";
  auto gcont = mitk::DiffusionPropertyHelper::GetGradientContainer(input_image);
  auto bvaluevector = mitk::DiffusionPropertyHelper::GetBValueVector(input_image);
  int b0_count = 0;
  for (unsigned int i=0; i<gcont->Size(); ++i)
  {
    bvals += boost::lexical_cast<std::string>(bvaluevector.at(i));
    if (bvaluevector.at(i)==0)
      b0_count++;
    auto g = gcont->GetElement(i);

    if (g.two_norm()>0.000001)
      g /= g.two_norm();
    bvecs += "[" + boost::lexical_cast<std::string>(g[0]) + "," + boost::lexical_cast<std::string>(g[1]) + "," + boost::lexical_cast<std::string>(g[2]) + "]";

    if (i<gcont->Size()-1)
    {
      bvals += ", ";
      bvecs += ", ";
    }
  }
  bvals += "]";
  bvecs += "]";
  if (b0_count==0)
  {
    QMessageBox::warning(nullptr, "Error", "No b=0 volume found. Do your b-values need rounding? Use the Preprocessing View for rounding b-values,", QMessageBox::Ok);
    return;
  }


  us::ModuleContext* context = us::GetModuleContext();
  us::ServiceReference<mitk::IPythonService> m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
  mitk::IPythonService* m_PythonService = dynamic_cast<mitk::IPythonService*> ( context->GetService<mitk::IPythonService>(m_PythonServiceRef) );
  mitk::IPythonService::ForceLoadModule();

  m_PythonService->CopyToPythonAsSimpleItkImage( input_image, "in_image");
  m_PythonService->Execute("mask=None");
  if (m_Controls->m_MaskBox->GetSelectedNode().IsNotNull())
  {
    auto mitk_mask = dynamic_cast<mitk::Image*>(m_Controls->m_MaskBox->GetSelectedNode()->GetData());
    if (mitk_mask->GetLargestPossibleRegion().GetSize()==input_image->GetLargestPossibleRegion().GetSize())
      m_PythonService->CopyToPythonAsSimpleItkImage( mitk_mask, "mask");
    else
      MITK_INFO << "Mask image not used. Does not match data size: " << mitk_mask->GetLargestPossibleRegion().GetSize() << " vs. "
                << input_image->GetLargestPossibleRegion().GetSize();
  }

  m_PythonService->Execute("normalize_peaks=False");
  if (m_Controls->m_NormalizePeaks->isChecked())
    m_PythonService->Execute("normalize_peaks=True");

  std::string model = "3D-SHORE";
  int sh_order = 0;
  switch(m_Controls->m_ModelBox->currentIndex())
  {
  case 0:
  {
    model = "3D-SHORE";
    m_PythonService->Execute("radial_order=" + boost::lexical_cast<std::string>(m_Controls->m_RadialOrder->value()));
    m_PythonService->Execute("zeta=" + boost::lexical_cast<std::string>(m_Controls->m_Zeta->value()));
    m_PythonService->Execute("lambdaN=" + m_Controls->m_LambdaN->text().toStdString());
    m_PythonService->Execute("lambdaL=" + m_Controls->m_LambdaL->text().toStdString());
    break;
  }
  case 1:
  {
    model = "SFM";
    m_PythonService->Execute("fa_thr=" + boost::lexical_cast<std::string>(m_Controls->m_FaThresholdSfm->value()));
    break;
  }
  case 2:
  {
    model = "CSD";
    sh_order = m_Controls->m_ShOrderCsd->value();
    m_PythonService->Execute("sh_order=" + boost::lexical_cast<std::string>(sh_order));
    m_PythonService->Execute("fa_thr=" + boost::lexical_cast<std::string>(m_Controls->m_FaThresholdCsd->value()));
    break;
  }
  case 3:
  {
    model = "CSA-QBALL";
    sh_order = m_Controls->m_ShOrderCsa->value();
    m_PythonService->Execute("sh_order=" + boost::lexical_cast<std::string>(sh_order));
    m_PythonService->Execute("smooth=" + boost::lexical_cast<std::string>(m_Controls->m_LambdaCsa->value()));
    break;
  }
  case 4:
  {
    model = "Opdt";
    sh_order = m_Controls->m_ShOrderOpdt->value();
    m_PythonService->Execute("sh_order=" + boost::lexical_cast<std::string>(sh_order));
    m_PythonService->Execute("smooth=" + boost::lexical_cast<std::string>(m_Controls->m_LambdaOpdt->value()));
    break;
  }
  }
  m_PythonService->Execute("model_type='"+model+"'");

  m_PythonService->Execute("num_peaks=0");
  if (m_Controls->m_DoCalculatePeaks->isChecked())
  {
    m_PythonService->Execute("num_peaks=" + boost::lexical_cast<std::string>(m_Controls->m_NumPeaks->value()));
    m_PythonService->Execute("min_separation_angle=" + boost::lexical_cast<std::string>(m_Controls->m_SepAngle->value()));
    m_PythonService->Execute("relative_peak_threshold=" + boost::lexical_cast<std::string>(m_Controls->m_RelativeThreshold->value()));
  }

  m_PythonService->Execute("data=False");
  m_PythonService->Execute("bvals=" + bvals);
  m_PythonService->Execute("bvecs=" + bvecs);
  m_PythonService->Execute(data.toStdString(), mitk::IPythonService::MULTI_LINE_COMMAND);

  // clean up after running script (better way than deleting individual variables?)
  if(m_PythonService->DoesVariableExist("in_image"))
    m_PythonService->Execute("del in_image");

  // check for errors
  if(!m_PythonService->GetVariable("error_string").empty())
  {
    QMessageBox::warning(nullptr, "Error", QString(m_PythonService->GetVariable("error_string").c_str()), QMessageBox::Ok);
    return;
  }

  if (m_PythonService->DoesVariableExist("odf_image"))
  {
    mitk::OdfImage::ItkOdfImageType::Pointer itkImg = mitk::OdfImage::ItkOdfImageType::New();

    mitk::Image::Pointer out_image = m_PythonService->CopySimpleItkImageFromPython("odf_image");
    mitk::CastToItkImage(out_image, itkImg);

    mitk::OdfImage::Pointer image = mitk::OdfImage::New();
    image->InitializeByItk( itkImg.GetPointer() );
    image->SetVolume( itkImg->GetBufferPointer() );

    mitk::DataNode::Pointer odfs = mitk::DataNode::New();
    odfs->SetData( image );
    QString name(node->GetName().c_str());
    odfs->SetName(name.toStdString() + "_" + model);
    GetDataStorage()->Add(odfs, node);
    m_PythonService->Execute("del odf_image");
  }
  else if (m_PythonService->DoesVariableExist("sh_image"))
  {
    mitk::Image::Pointer out_image = m_PythonService->CopySimpleItkImageFromPython("sh_image");
    itk::VectorImage<float>::Pointer vectorImage = itk::VectorImage<float>::New();
    mitk::CastToItkImage(out_image, vectorImage);

    itk::VectorImageToFourDImageFilter< float >::Pointer converter = itk::VectorImageToFourDImageFilter< float >::New();
    converter->SetInputImage(vectorImage);
    converter->GenerateData();
    mitk::ShImage::ShOnDiskType::Pointer itkImg = converter->GetOutputImage();

    mitk::ShImage::Pointer shImage = mitk::ShImage::New();
    mitk::Image::Pointer mitkImage = dynamic_cast<mitk::Image*>(shImage.GetPointer());

    switch(sh_order)
    {
    case 2:
    {
      typedef itk::ShCoefficientImageImporter< float, 2 > ImporterType;
      typename ImporterType::Pointer importer = ImporterType::New();
      importer->SetInputImage(itkImg);
      importer->GenerateData();
      mitk::CastToMitkImage(importer->GetCoefficientImage(), mitkImage);
      mitkImage->SetVolume(importer->GetCoefficientImage()->GetBufferPointer());
      break;
    }
    case 4:
    {
      typedef itk::ShCoefficientImageImporter< float, 4 > ImporterType;
      typename ImporterType::Pointer importer = ImporterType::New();
      importer->SetInputImage(itkImg);
      importer->GenerateData();
      mitk::CastToMitkImage(importer->GetCoefficientImage(), mitkImage);
      mitkImage->SetVolume(importer->GetCoefficientImage()->GetBufferPointer());
      break;
    }
    case 6:
    {
      typedef itk::ShCoefficientImageImporter< float, 6 > ImporterType;
      typename ImporterType::Pointer importer = ImporterType::New();
      importer->SetInputImage(itkImg);
      importer->GenerateData();
      mitk::CastToMitkImage(importer->GetCoefficientImage(), mitkImage);
      mitkImage->SetVolume(importer->GetCoefficientImage()->GetBufferPointer());
      break;
    }
    case 8:
    {
      typedef itk::ShCoefficientImageImporter< float, 8 > ImporterType;
      typename ImporterType::Pointer importer = ImporterType::New();
      importer->SetInputImage(itkImg);
      importer->GenerateData();
      mitk::CastToMitkImage(importer->GetCoefficientImage(), mitkImage);
      mitkImage->SetVolume(importer->GetCoefficientImage()->GetBufferPointer());
      break;
    }
    case 10:
    {
      typedef itk::ShCoefficientImageImporter< float, 10 > ImporterType;
      typename ImporterType::Pointer importer = ImporterType::New();
      importer->SetInputImage(itkImg);
      importer->GenerateData();
      mitk::CastToMitkImage(importer->GetCoefficientImage(), mitkImage);
      mitkImage->SetVolume(importer->GetCoefficientImage()->GetBufferPointer());
      break;
    }
    case 12:
    {
      typedef itk::ShCoefficientImageImporter< float, 12 > ImporterType;
      typename ImporterType::Pointer importer = ImporterType::New();
      importer->SetInputImage(itkImg);
      importer->GenerateData();
      mitk::CastToMitkImage(importer->GetCoefficientImage(), mitkImage);
      mitkImage->SetVolume(importer->GetCoefficientImage()->GetBufferPointer());
      break;
    }
    default:
      mitkThrow() << "SH order not supported";
    }

    mitk::DataNode::Pointer shNode = mitk::DataNode::New();
    shNode->SetData( mitkImage );
    QString name(node->GetName().c_str());
    shNode->SetName(name.toStdString() + "_" + model);
    GetDataStorage()->Add(shNode, node);
    m_PythonService->Execute("del sh_image");
  }

  if (m_Controls->m_DoCalculatePeaks->isChecked() && m_PythonService->DoesVariableExist("peak_image"))
  {
    mitk::Image::Pointer out_image = m_PythonService->CopySimpleItkImageFromPython("peak_image");
    itk::VectorImage<float>::Pointer vectorImage = itk::VectorImage<float>::New();
    mitk::CastToItkImage(out_image, vectorImage);

    itk::VectorImageToFourDImageFilter< float >::Pointer converter = itk::VectorImageToFourDImageFilter< float >::New();
    converter->SetInputImage(vectorImage);
    converter->GenerateData();
    mitk::PeakImage::ItkPeakImageType::Pointer itk_peaks = converter->GetOutputImage();

    mitk::Image::Pointer mitk_peaks = dynamic_cast<mitk::Image*>(mitk::PeakImage::New().GetPointer());
    mitk::CastToMitkImage(itk_peaks, mitk_peaks);
    mitk_peaks->SetVolume(itk_peaks->GetBufferPointer());

    mitk::DataNode::Pointer seg = mitk::DataNode::New();
    seg->SetData( mitk_peaks );
    seg->SetName("Peaks");
    GetDataStorage()->Add(seg, node);
    m_PythonService->Execute("del peak_image");
  }
}
