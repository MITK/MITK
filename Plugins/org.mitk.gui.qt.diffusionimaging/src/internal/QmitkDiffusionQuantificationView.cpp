/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkDiffusionQuantificationView.h"
#include "mitkDiffusionImagingConfigure.h"

#include "itkTimeProbe.h"
#include "itkImage.h"

#include "mitkNodePredicateDataType.h"
#include "mitkDataNodeObject.h"
#include "mitkQBallImage.h"
#include "mitkImageCast.h"
#include "mitkStatusBar.h"
#include "itkDiffusionQballGeneralizedFaImageFilter.h"
#include "itkShiftScaleImageFilter.h"
#include "itkTensorFractionalAnisotropyImageFilter.h"
#include "itkTensorRelativeAnisotropyImageFilter.h"
#include "itkTensorDerivedMeasurementsFilter.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include <QMessageBox>

#include "berryIWorkbenchWindow.h"
#include "berryISelectionService.h"


const std::string QmitkDiffusionQuantificationView::VIEW_ID = "org.mitk.views.diffusionquantification";

using namespace berry;

const float QmitkDiffusionQuantificationView::m_ScaleDAIValues = 100;

struct DqSelListener : ISelectionListener
{

  berryObjectMacro(DqSelListener);

  DqSelListener(QmitkDiffusionQuantificationView* view)
  {
    m_View = view;
  }

  void DoSelectionChanged(ISelection::ConstPointer selection)
  {
    // save current selection in member variable
    m_View->m_CurrentSelection = selection.Cast<const IStructuredSelection>();

    // do something with the selected items
    if(m_View->m_CurrentSelection)
    {
      bool foundQBIVolume = false;
      bool foundTensorVolume = false;

      // iterate selection
      for (IStructuredSelection::iterator i = m_View->m_CurrentSelection->Begin();
        i != m_View->m_CurrentSelection->End(); ++i)
      {

        // extract datatree node
        if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
        {
          mitk::DataNode::Pointer node = nodeObj->GetDataNode();

          // only look at interesting types
          if(QString("QBallImage").compare(node->GetData()->GetNameOfClass())==0)
          {
            foundQBIVolume = true;
          }

          if(QString("TensorImage").compare(node->GetData()->GetNameOfClass())==0)
          {
            foundTensorVolume = true;
          }
        }
      }

      m_View->m_Controls->m_GFAButton->setEnabled(foundQBIVolume);
      m_View->m_Controls->m_CurvatureButton->setEnabled(foundQBIVolume);

      m_View->m_Controls->m_FAButton->setEnabled(foundTensorVolume);
      m_View->m_Controls->m_RAButton->setEnabled(foundTensorVolume);
      m_View->m_Controls->m_ADButton->setEnabled(foundTensorVolume);
      m_View->m_Controls->m_RDButton->setEnabled(foundTensorVolume);
      m_View->m_Controls->m_MDButton->setEnabled(foundTensorVolume);
      m_View->m_Controls->m_ClusteringAnisotropy->setEnabled(foundTensorVolume);
    }
  }

  void SelectionChanged(IWorkbenchPart::Pointer part, ISelection::ConstPointer selection)
  {
    // check, if selection comes from datamanager
    if (part)
    {
      QString partname(part->GetPartName().c_str());
      if(partname.compare("Datamanager")==0)
      {

        // apply selection
        DoSelectionChanged(selection);

      }
    }
  }

  QmitkDiffusionQuantificationView* m_View;
};

QmitkDiffusionQuantificationView::QmitkDiffusionQuantificationView()
: QmitkFunctionality(),
m_Controls(NULL),
m_MultiWidget(NULL)
{
}

QmitkDiffusionQuantificationView::QmitkDiffusionQuantificationView(const QmitkDiffusionQuantificationView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QmitkDiffusionQuantificationView::~QmitkDiffusionQuantificationView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
}

void QmitkDiffusionQuantificationView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkDiffusionQuantificationViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
    GFACheckboxClicked();

#ifndef DIFFUSION_IMAGING_EXTENDED
    m_Controls->m_StandardGFACheckbox->setVisible(false);
    m_Controls->frame_3->setVisible(false);
    m_Controls->m_CurvatureButton->setVisible(false);
#endif
  }

  m_SelListener = berry::ISelectionListener::Pointer(new DqSelListener(this));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<DqSelListener>()->DoSelectionChanged(sel);
}

void QmitkDiffusionQuantificationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkDiffusionQuantificationView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkDiffusionQuantificationView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_StandardGFACheckbox), SIGNAL(clicked()), this, SLOT(GFACheckboxClicked()) );
    connect( (QObject*)(m_Controls->m_GFAButton), SIGNAL(clicked()), this, SLOT(GFA()) );
    connect( (QObject*)(m_Controls->m_CurvatureButton), SIGNAL(clicked()), this, SLOT(Curvature()) );
    connect( (QObject*)(m_Controls->m_FAButton), SIGNAL(clicked()), this, SLOT(FA()) );
    connect( (QObject*)(m_Controls->m_RAButton), SIGNAL(clicked()), this, SLOT(RA()) );
    connect( (QObject*)(m_Controls->m_ADButton), SIGNAL(clicked()), this, SLOT(AD()) );
    connect( (QObject*)(m_Controls->m_RDButton), SIGNAL(clicked()), this, SLOT(RD()) );
    connect( (QObject*)(m_Controls->m_MDButton), SIGNAL(clicked()), this, SLOT(MD()) );
    connect( (QObject*)(m_Controls->m_ClusteringAnisotropy), SIGNAL(clicked()), this, SLOT(ClusterAnisotropy()) );
  }
}

void QmitkDiffusionQuantificationView::Activated()
{
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<DqSelListener>()->DoSelectionChanged(sel);
  QmitkFunctionality::Activated();
}

void QmitkDiffusionQuantificationView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkDiffusionQuantificationView::GFACheckboxClicked()
{
  m_Controls->frame_2->setVisible(m_Controls->
    m_StandardGFACheckbox->isChecked());
}

void QmitkDiffusionQuantificationView::GFA()
{
  if(m_Controls->m_StandardGFACheckbox->isChecked())
  {
    QBIQuantify(13);
  }
  else
  {
    QBIQuantify(0);
  }
}

void QmitkDiffusionQuantificationView::Curvature()
{
  QBIQuantify(12);
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

void QmitkDiffusionQuantificationView::QBIQuantify(int method)
{
  if (m_CurrentSelection)
  {
    mitk::DataStorage::SetOfObjects::Pointer set =
      mitk::DataStorage::SetOfObjects::New();

    int at = 0;
    for (IStructuredSelection::iterator i = m_CurrentSelection->Begin();
      i != m_CurrentSelection->End();
      ++i)
    {

      if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();
        if(QString("QBallImage").compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    QBIQuantification(set, method);

  }
}

void QmitkDiffusionQuantificationView::TensorQuantify(int method)
{
  if (m_CurrentSelection)
  {
    mitk::DataStorage::SetOfObjects::Pointer set =
      mitk::DataStorage::SetOfObjects::New();

    int at = 0;
    for (IStructuredSelection::iterator i = m_CurrentSelection->Begin();
      i != m_CurrentSelection->End();
      ++i)
    {

      if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();
        if(QString("TensorImage").compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    TensorQuantification(set, method);

  }
}

void QmitkDiffusionQuantificationView::QBIQuantification(
  mitk::DataStorage::SetOfObjects::Pointer inImages, int method)
{
  itk::TimeProbe clock;
  QString status;

  int nrFiles = inImages->size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() );

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    typedef float TOdfPixelType;
    const int odfsize = QBALL_ODFSIZE;
    typedef itk::Vector<TOdfPixelType,odfsize> OdfVectorType;
    typedef itk::Image<OdfVectorType,3> OdfVectorImgType;
    mitk::Image* vol =
      static_cast<mitk::Image*>((*itemiter)->GetData());
    OdfVectorImgType::Pointer itkvol = OdfVectorImgType::New();
    mitk::CastToItkImage<OdfVectorImgType>(vol, itkvol);

    std::string nodename;
    (*itemiter)->GetStringProperty("name", nodename);
    ++itemiter;

    float p1 = m_Controls->m_ParamKEdit->text().toFloat();
    float p2 = m_Controls->m_ParamPEdit->text().toFloat();

    // COMPUTE RA
    clock.Start();
    MBI_INFO << "Computing GFA ";
    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
      "Computing GFA for %s", nodename.c_str()).toAscii());
    typedef OdfVectorType::ValueType                 RealValueType;
    typedef itk::Image< RealValueType, 3 >                 RAImageType;
    typedef itk::DiffusionQballGeneralizedFaImageFilter<TOdfPixelType,TOdfPixelType,odfsize>
      GfaFilterType;
    GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
    gfaFilter->SetInput(itkvol);
    gfaFilter->SetNumberOfThreads(8);

    double scale = 1;
    std::string newname;
    newname.append(nodename);
    switch(method)
    {
    case 0:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
        newname.append("GFA");
        scale = m_ScaleDAIValues;
        break;
      }
    case 1:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILES_HIGH_LOW);
        newname.append("01");
        scale = m_ScaleDAIValues;
        break;
      }
    case 2:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILE_HIGH);
        newname.append("02");
        scale = m_ScaleDAIValues;
        break;
      }
    case 3:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_MAX_ODF_VALUE);
        newname.append("03");
        scale = m_ScaleDAIValues;
        break;
      }
    case 4:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_DECONVOLUTION_COEFFS);
        newname.append("04");
        scale = m_ScaleDAIValues;
        break;
      }
    case 5:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_MIN_MAX_NORMALIZED_STANDARD);
        newname.append("05");
        scale = m_ScaleDAIValues;
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
        scale = m_ScaleDAIValues;
        break;
      }
    case 8:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILES_LOW_HIGH);
        newname.append("08");
        scale = m_ScaleDAIValues;
        break;
      }
    case 9:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_QUANTILE_LOW);
        newname.append("09");
        scale = m_ScaleDAIValues;
        break;
      }
    case 10:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_MIN_ODF_VALUE);
        newname.append("10");
        scale = m_ScaleDAIValues;
        break;
      }
    case 11:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_STD_BY_MAX);
        newname.append("11");
        scale = m_ScaleDAIValues;
        break;
      }
    case 12:
      {
        p1 = m_Controls->MinAngle->text().toFloat();
        p2 = m_Controls->MaxAngle->text().toFloat();
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_PRINCIPLE_CURVATURE);
        QString paramString;
        paramString = paramString.append("PC%1-%2").arg(p1).arg(p2);
        newname.append(paramString.toAscii());
        gfaFilter->SetParam1(p1);
        gfaFilter->SetParam2(p2);
        scale = m_ScaleDAIValues;
        break;
      }
    case 13:
      {
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_GENERALIZED_GFA);
        QString paramString;
        paramString = paramString.append("GFAK%1P%2").arg(p1).arg(p2);
        newname.append(paramString.toAscii());
        gfaFilter->SetParam1(p1);
        gfaFilter->SetParam2(p2);
        scale = m_ScaleDAIValues;
        break;
      }
    default:
      {
        newname.append("0");
        gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
        scale = m_ScaleDAIValues;
      }
    }
    gfaFilter->Update();
    clock.Stop();
    MBI_DEBUG << "took " << clock.GetMeanTime() << "s.";

    typedef itk::Image<TOdfPixelType, 3> ImgType;
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
    node->SetProperty( "name", mitk::StringProperty::New(newname) );
    nodes.push_back(node);

    mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");

  }

  std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
  for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
    GetDefaultDataStorage()->Add(*nodeIt);

  m_MultiWidget->RequestUpdate();

}

void QmitkDiffusionQuantificationView::TensorQuantification(
  mitk::DataStorage::SetOfObjects::Pointer inImages, int method)
{
  itk::TimeProbe clock;
  QString status;

  int nrFiles = inImages->size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() );

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    typedef float                                       TTensorPixelType;
    typedef itk::DiffusionTensor3D< TTensorPixelType >  TensorPixelType;
    typedef itk::Image< TensorPixelType, 3 >            TensorImageType;

    mitk::Image* vol =
      static_cast<mitk::Image*>((*itemiter)->GetData());
    TensorImageType::Pointer itkvol = TensorImageType::New();
    mitk::CastToItkImage<TensorImageType>(vol, itkvol);

    std::string nodename;
    (*itemiter)->GetStringProperty("name", nodename);
    ++itemiter;

    // COMPUTE FA
    clock.Start();
    MBI_INFO << "Computing FA ";
    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
      "Computing FA for %s", nodename.c_str()).toAscii());
    typedef itk::Image< TTensorPixelType, 3 >              FAImageType;

    typedef itk::ShiftScaleImageFilter<FAImageType, FAImageType>
      ShiftScaleFilterType;
    ShiftScaleFilterType::Pointer multi =
      ShiftScaleFilterType::New();
    multi->SetShift(0.0);
    multi->SetScale(m_ScaleDAIValues);//itk::NumericTraits<RealValueType>::max()

    typedef itk::TensorDerivedMeasurementsFilter<TTensorPixelType> MeasurementsType;

    if(method == 0) //FA
    {
     /* typedef itk::TensorFractionalAnisotropyImageFilter<
        TensorImageType, FAImageType >                       FilterType;
      FilterType::Pointer anisotropyFilter = FilterType::New();
      anisotropyFilter->SetInput( itkvol.GetPointer() );
      anisotropyFilter->Update();
      multi->SetInput(anisotropyFilter->GetOutput());
      nodename = QString(nodename.c_str()).append("_FA").toStdString();*/


      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::FA);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_FA").toStdString();

    }
    else if(method == 1) //RA
    {
      /*typedef itk::TensorRelativeAnisotropyImageFilter<
        TensorImageType, FAImageType >                       FilterType;
      FilterType::Pointer anisotropyFilter = FilterType::New();
      anisotropyFilter->SetInput( itkvol.GetPointer() );
      anisotropyFilter->Update();
      multi->SetInput(anisotropyFilter->GetOutput());
      nodename = QString(nodename.c_str()).append("_RA").toStdString();*/

      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::RA);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_RA").toStdString();

    }
    else if(method == 2) // AD (Axial diffusivity)
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::AD);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_AD").toStdString();
    }
    else if(method == 3) // RD (Radial diffusivity, (Lambda2+Lambda3)/2
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::RD);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_RD").toStdString();
    }
    else if(method == 4) // 1-(Lambda2+Lambda3)/(2*Lambda1)
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::CA);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_CA").toStdString();
    }
    else if(method == 5) // MD (Mean Diffusivity, (Lambda1+Lambda2+Lambda3)/3 )
    {
      MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
      measurementsCalculator->SetInput(itkvol.GetPointer() );
      measurementsCalculator->SetMeasure(MeasurementsType::MD);
      measurementsCalculator->Update();
      multi->SetInput(measurementsCalculator->GetOutput());
      nodename = QString(nodename.c_str()).append("_MD").toStdString();
    }

    multi->Update();
    clock.Stop();
    MBI_DEBUG << "took " << clock.GetMeanTime() << "s.";

    // FA TO DATATREE
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( multi->GetOutput() );
    image->SetVolume( multi->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( image );
    node->SetProperty( "name", mitk::StringProperty::New(nodename) );
    nodes.push_back(node);

    mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");

  }

  std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
  for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
    GetDefaultDataStorage()->Add(*nodeIt);

  m_MultiWidget->RequestUpdate();

}
