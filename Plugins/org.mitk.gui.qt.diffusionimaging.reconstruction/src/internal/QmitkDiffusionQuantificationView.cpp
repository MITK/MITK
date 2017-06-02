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
#include "mitkQBallImage.h"
#include <mitkTensorImage.h>
#include "mitkImageCast.h"
#include "mitkStatusBar.h"
#include "itkDiffusionQballGeneralizedFaImageFilter.h"
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
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>


const std::string QmitkDiffusionQuantificationView::VIEW_ID = "org.mitk.views.diffusionquantification";



QmitkDiffusionQuantificationView::QmitkDiffusionQuantificationView()
    : QmitkAbstractView(),
      m_Controls(nullptr)
{
    m_QBallImages = mitk::DataStorage::SetOfObjects::New();
    m_TensorImages = mitk::DataStorage::SetOfObjects::New();
    m_DwImages = mitk::DataStorage::SetOfObjects::New();
}

QmitkDiffusionQuantificationView::QmitkDiffusionQuantificationView(const QmitkDiffusionQuantificationView& other)
{
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
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
        this->CreateConnections();
        GFACheckboxClicked();

#ifndef DIFFUSION_IMAGING_EXTENDED
        m_Controls->m_StandardGFACheckbox->setVisible(false);
        m_Controls->frame_3->setVisible(false);
        m_Controls->m_CurvatureButton->setVisible(false);
#endif
    }
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
        connect( (QObject*)(m_Controls->m_MdDwiButton), SIGNAL(clicked()), this, SLOT(MD_DWI()) );
        connect( (QObject*)(m_Controls->m_AdcDwiButton), SIGNAL(clicked()), this, SLOT(ADC_DWI()) );
        connect( (QObject*)(m_Controls->m_ClusteringAnisotropy), SIGNAL(clicked()), this, SLOT(ClusterAnisotropy()) );
    }
}

void QmitkDiffusionQuantificationView::SetFocus()
{
  m_Controls->m_ScaleImageValuesBox->setFocus();
}

void QmitkDiffusionQuantificationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
    m_QBallImages = mitk::DataStorage::SetOfObjects::New();
    m_TensorImages = mitk::DataStorage::SetOfObjects::New();
    m_DwImages = mitk::DataStorage::SetOfObjects::New();
    bool foundQBIVolume = false;
    bool foundTensorVolume = false;
    bool foundDwVolume = false;
    mitk::DataNode::Pointer  selNode = nullptr;

    int c=0;
    for (mitk::DataNode::Pointer node: nodes)
    {
        if( node.IsNotNull() && dynamic_cast<mitk::QBallImage*>(node->GetData()) )
        {
            foundQBIVolume = true;
            m_QBallImages->push_back(node);
            selNode = node;
            c++;
        }
        else if( node.IsNotNull() && dynamic_cast<mitk::TensorImage*>(node->GetData()) )
        {
            foundTensorVolume = true;
            m_TensorImages->push_back(node);
            selNode = node;
            c++;
        }
        else if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) && mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(dynamic_cast<mitk::Image*>(node->GetData())) )
        {
            foundDwVolume = true;
            m_DwImages->push_back(node);
            selNode = node;
            c++;
        }
    }

    m_Controls->m_GFAButton->setEnabled(foundQBIVolume);
    m_Controls->m_CurvatureButton->setEnabled(foundQBIVolume);

    if (c>0)
    {
        m_Controls->m_InputData->setTitle("Input Data");
        m_Controls->m_InputImageLabel->setText(selNode->GetName().c_str());
    }
    else
    {
        m_Controls->m_InputData->setTitle("Please Select Input Data");
        m_Controls->m_InputImageLabel->setText("<font color='red'>mandatory</font>");
    }

    m_Controls->m_FAButton->setEnabled(foundTensorVolume);
    m_Controls->m_RAButton->setEnabled(foundTensorVolume);
    m_Controls->m_ADButton->setEnabled(foundTensorVolume);
    m_Controls->m_RDButton->setEnabled(foundTensorVolume);
    m_Controls->m_MDButton->setEnabled(foundTensorVolume);
    m_Controls->m_ClusteringAnisotropy->setEnabled(foundTensorVolume);
    m_Controls->m_AdcDwiButton->setEnabled(foundDwVolume);
    m_Controls->m_MdDwiButton->setEnabled(foundDwVolume);
}

void QmitkDiffusionQuantificationView::ADC_DWI()
{
    DoAdcCalculation(true);
}

void QmitkDiffusionQuantificationView::MD_DWI()
{
    DoAdcCalculation(false);
}

void QmitkDiffusionQuantificationView::DoAdcCalculation(bool fit)
{
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( m_DwImages->end() );
    mitk::DataStorage::SetOfObjects::const_iterator itemiter( m_DwImages->begin() );

    while ( itemiter != itemiterend ) // for all items
    {
        mitk::DataNode* node = *itemiter;
        mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

        typedef itk::AdcImageFilter< short, double > FilterType;

        ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
        mitk::CastToItkImage(image, itkVectorImagePointer);
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput( itkVectorImagePointer );

        filter->SetGradientDirections( static_cast<mitk::GradientDirectionsProperty*>
          ( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )
            ->GetGradientDirectionsContainer() );

        filter->SetB_value( static_cast<mitk::FloatProperty*>
          (image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )
            ->GetValue() );

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
        GetDataStorage()->Add(imageNode, node);

        ++itemiter;
    }
}

void QmitkDiffusionQuantificationView::GFACheckboxClicked()
{
    m_Controls->frame_2->setVisible(m_Controls->m_StandardGFACheckbox->isChecked());
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
    QBIQuantification(m_QBallImages, method);
}

void QmitkDiffusionQuantificationView::TensorQuantify(int method)
{
    TensorQuantification(m_TensorImages, method);
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
        mitk::CastToItkImage(vol, itkvol);

        std::string nodename;
        (*itemiter)->GetStringProperty("name", nodename);

        float p1 = m_Controls->m_ParamKEdit->text().toFloat();
        float p2 = m_Controls->m_ParamPEdit->text().toFloat();

        // COMPUTE RA
        clock.Start();
        MBI_INFO << "Computing GFA ";
        mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
                                                        "Computing GFA for %s", nodename.c_str()).toLatin1());
        typedef OdfVectorType::ValueType                 RealValueType;
        typedef itk::Image< RealValueType, 3 >                 RAImageType;
        typedef itk::DiffusionQballGeneralizedFaImageFilter<TOdfPixelType,TOdfPixelType,odfsize>
                GfaFilterType;
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
        case 12:
        {
            p1 = m_Controls->MinAngle->text().toFloat();
            p2 = m_Controls->MaxAngle->text().toFloat();
            gfaFilter->SetComputationMethod(GfaFilterType::GFA_PRINCIPLE_CURVATURE);
            QString paramString;
            paramString = paramString.append("PC%1-%2").arg(p1).arg(p2);
            newname.append(paramString.toLatin1());
            gfaFilter->SetParam1(p1);
            gfaFilter->SetParam2(p2);
            break;
        }
        case 13:
        {
            gfaFilter->SetComputationMethod(GfaFilterType::GFA_GENERALIZED_GFA);
            QString paramString;
            paramString = paramString.append("GFAK%1P%2").arg(p1).arg(p2);
            newname.append(paramString.toLatin1());
            gfaFilter->SetParam1(p1);
            gfaFilter->SetParam2(p2);
            break;
        }
        default:
        {
            newname.append("0");
            gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
        }
        }
        gfaFilter->Update();
        clock.Stop();

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
        mitk::DataNode::Pointer node=mitk::DataNode::New();
        node->SetData( image );
        node->SetProperty( "name", mitk::StringProperty::New(newname) );

        mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
        lut->SetType( mitk::LookupTable::JET_TRANSPARENT );
        mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
        lut_prop->SetLookupTable( lut );
        node->SetProperty("LookupTable", lut_prop );

        GetDataStorage()->Add(node, *itemiter);

        mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");

        ++itemiter;
    }

    this->GetRenderWindowPart()->RequestUpdate();

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
        mitk::CastToItkImage(vol, itkvol);

        std::string nodename;
        (*itemiter)->GetStringProperty("name", nodename);

        // COMPUTE FA
        clock.Start();
        MBI_INFO << "Computing FA ";
        mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
                                                        "Computing FA for %s", nodename.c_str()).toLatin1());
        typedef itk::Image< TTensorPixelType, 3 >              FAImageType;

        typedef itk::ShiftScaleImageFilter<FAImageType, FAImageType> ShiftScaleFilterType;
        ShiftScaleFilterType::Pointer multi = ShiftScaleFilterType::New();
        multi->SetShift(0.0);
        multi->SetScale(m_Controls->m_ScaleImageValuesBox->value());

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

        // FA TO DATATREE
        mitk::Image::Pointer image = mitk::Image::New();
        image->InitializeByItk( multi->GetOutput() );
        image->SetVolume( multi->GetOutput()->GetBufferPointer() );
        mitk::DataNode::Pointer node=mitk::DataNode::New();
        node->SetData( image );
        node->SetProperty( "name", mitk::StringProperty::New(nodename) );

        mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
        lut->SetType( mitk::LookupTable::JET_TRANSPARENT );
        mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
        lut_prop->SetLookupTable( lut );
        node->SetProperty("LookupTable", lut_prop );

        GetDataStorage()->Add(node, *itemiter);
        ++itemiter;

        mitk::StatusBar::GetInstance()->DisplayText("Computation complete.");
    }

    this->GetRenderWindowPart()->RequestUpdate();

}
