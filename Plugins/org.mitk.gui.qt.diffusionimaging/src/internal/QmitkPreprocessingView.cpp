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
#include <itkDwiNormilzationFilter.h>

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
#include <itkBrainMaskExtractionImageFilter.h>
#include <mitkImageCast.h>
#include <mitkRotationOperation.h>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <mitkInteractionConst.h>
#include <mitkImageAccessByItk.h>
#include <itkResampleDwiImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkMaskImageFilter.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <itkCropImageFilter.h>
#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkRemoveDwiChannelFilter.h>
#include <itkExtractDwiChannelFilter.h>

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

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        m_Controls->m_MeasurementFrameTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        m_Controls->m_MeasurementFrameTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
        m_Controls->m_DirectionMatrixTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        m_Controls->m_DirectionMatrixTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
#else
        m_Controls->m_MeasurementFrameTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_Controls->m_MeasurementFrameTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_Controls->m_DirectionMatrixTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_Controls->m_DirectionMatrixTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#endif
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
        m_Controls->m_NormalizationMaskBox->SetDataStorage(this->GetDataStorage());
        mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
        mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
        mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
        mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");
        mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isDwi, isDti);
        isDiffusionImage = mitk::NodePredicateOr::New(isDiffusionImage, isQbi);
        mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
        mitk::NodePredicateAnd::Pointer finalPredicate = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
        mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
        finalPredicate = mitk::NodePredicateAnd::New(finalPredicate, isBinaryPredicate);
        m_Controls->m_NormalizationMaskBox->SetPredicate(finalPredicate);

        m_Controls->m_ExtractBrainMask->setVisible(false);
        m_Controls->m_BrainMaskIterationsBox->setVisible(false);
        m_Controls->m_ResampleIntFrame->setVisible(false);
        connect( (QObject*)(m_Controls->m_ButtonAverageGradients), SIGNAL(clicked()), this, SLOT(AverageGradients()) );
        connect( (QObject*)(m_Controls->m_ButtonExtractB0), SIGNAL(clicked()), this, SLOT(ExtractB0()) );
        connect( (QObject*)(m_Controls->m_ModifyMeasurementFrame), SIGNAL(clicked()), this, SLOT(DoApplyMesurementFrame()) );
        connect( (QObject*)(m_Controls->m_ReduceGradientsButton), SIGNAL(clicked()), this, SLOT(DoReduceGradientDirections()) );
        connect( (QObject*)(m_Controls->m_ShowGradientsButton), SIGNAL(clicked()), this, SLOT(DoShowGradientDirections()) );
        connect( (QObject*)(m_Controls->m_MirrorGradientToHalfSphereButton), SIGNAL(clicked()), this, SLOT(DoHalfSphereGradientDirections()) );
        connect( (QObject*)(m_Controls->m_MergeDwisButton), SIGNAL(clicked()), this, SLOT(MergeDwis()) );
        connect( (QObject*)(m_Controls->m_ProjectSignalButton), SIGNAL(clicked()), this, SLOT(DoProjectSignal()) );
        connect( (QObject*)(m_Controls->m_B_ValueMap_Rounder_SpinBox), SIGNAL(valueChanged(int)), this, SLOT(UpdateDwiBValueMapRounder(int)));
        connect( (QObject*)(m_Controls->m_CreateLengthCorrectedDwi), SIGNAL(clicked()), this, SLOT(DoLengthCorrection()) );
        connect( (QObject*)(m_Controls->m_CalcAdcButton), SIGNAL(clicked()), this, SLOT(DoAdcCalculation()) );
        connect( (QObject*)(m_Controls->m_NormalizeImageValuesButton), SIGNAL(clicked()), this, SLOT(DoDwiNormalization()) );
        connect( (QObject*)(m_Controls->m_ModifyDirection), SIGNAL(clicked()), this, SLOT(DoApplyDirectionMatrix()) );
        connect( (QObject*)(m_Controls->m_ModifySpacingButton), SIGNAL(clicked()), this, SLOT(DoApplySpacing()) );
        connect( (QObject*)(m_Controls->m_ModifyOriginButton), SIGNAL(clicked()), this, SLOT(DoApplyOrigin()) );
        connect( (QObject*)(m_Controls->m_ResampleImageButton), SIGNAL(clicked()), this, SLOT(DoResampleImage()) );
        connect( (QObject*)(m_Controls->m_ResampleTypeBox), SIGNAL(currentIndexChanged(int)), this, SLOT(DoUpdateInterpolationGui(int)) );
        connect( (QObject*)(m_Controls->m_CropImageButton), SIGNAL(clicked()), this, SLOT(DoCropImage()) );
        connect( (QObject*)(m_Controls->m_RemoveGradientButton), SIGNAL(clicked()), this, SLOT(DoRemoveGradient()) );
        connect( (QObject*)(m_Controls->m_ExtractGradientButton), SIGNAL(clicked()), this, SLOT(DoExtractGradient()) );


        //        connect( (QObject*)(m_Controls->m_ExtractBrainMask), SIGNAL(clicked()), this, SLOT(DoExtractBrainMask()) );
    }
}

void QmitkPreprocessingView::DoRemoveGradient()
{
    if (m_DiffusionImage.IsNull())
        return;

    std::vector< unsigned int > channelsToRemove; channelsToRemove.push_back(m_Controls->m_RemoveGradientBox->value());
    itk::RemoveDwiChannelFilter< short >::Pointer filter = itk::RemoveDwiChannelFilter< short >::New();
    filter->SetInput(m_DiffusionImage->GetVectorImage());
    filter->SetChannelIndices(channelsToRemove);
    filter->SetDirections(m_DiffusionImage->GetDirections());
    filter->Update();

    MitkDwiType::Pointer image = MitkDwiType::New();
    image->SetVectorImage( filter->GetOutput() );
    image->SetReferenceBValue( m_DiffusionImage->GetReferenceBValue() );
    image->SetDirections( filter->GetNewDirections() );
    image->InitializeFromVectorImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();
    imageNode->SetName((name+"_removedgradients").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoExtractGradient()
{
    if (m_DiffusionImage.IsNull())
        return;

    unsigned int channel = m_Controls->m_ExtractGradientBox->value();
    itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
    filter->SetInput(m_DiffusionImage->GetVectorImage());
    filter->SetChannelIndex(channel);
    filter->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( filter->GetOutput() );
    mitkImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( mitkImage );
    QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();
    imageNode->SetName((name+"_direction-"+QString::number(channel)).toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());

    mitk::RenderingManager::GetInstance()->InitializeViews(imageNode->GetData()->GetTimeGeometry(),mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
}

void QmitkPreprocessingView::DoCropImage()
{
    if (m_DiffusionImage.IsNotNull())
    {
        ItkDwiType::SizeType lower;
        ItkDwiType::SizeType upper;
        lower[0] = m_Controls->m_XstartBox->value();
        lower[1] = m_Controls->m_YstartBox->value();
        lower[2] = m_Controls->m_ZstartBox->value();
        upper[0] = m_Controls->m_XendBox->value();
        upper[1] = m_Controls->m_YendBox->value();
        upper[2] = m_Controls->m_ZendBox->value();

        itk::CropImageFilter< ItkDwiType, ItkDwiType >::Pointer cropper = itk::CropImageFilter< ItkDwiType, ItkDwiType >::New();
        cropper->SetLowerBoundaryCropSize(lower);
        cropper->SetUpperBoundaryCropSize(upper);
        cropper->SetInput(m_DiffusionImage->GetVectorImage());
        cropper->Update();

        MitkDwiType::Pointer image = MitkDwiType::New();
        image->SetVectorImage( cropper->GetOutput() );
        image->SetReferenceBValue( m_DiffusionImage->GetReferenceBValue() );
        image->SetDirections( m_DiffusionImage->GetDirections() );
        image->InitializeFromVectorImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( image );
        QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();
        imageNode->SetName((name+"_cropped").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();

        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if(m_SelectedImage.IsNotNull())
    {
        AccessFixedDimensionByItk(m_SelectedImage, TemplatedCropImage,3);
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedCropImage( itk::Image<TPixel, VImageDimension>* itkImage)
{
    ItkDwiType::SizeType lower;
    ItkDwiType::SizeType upper;
    lower[0] = m_Controls->m_XstartBox->value();
    lower[1] = m_Controls->m_YstartBox->value();
    lower[2] = m_Controls->m_ZstartBox->value();
    upper[0] = m_Controls->m_XendBox->value();
    upper[1] = m_Controls->m_YendBox->value();
    upper[2] = m_Controls->m_ZendBox->value();

    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typename itk::CropImageFilter< ImageType, ImageType >::Pointer cropper = itk::CropImageFilter< ImageType, ImageType >::New();
    cropper->SetLowerBoundaryCropSize(lower);
    cropper->SetUpperBoundaryCropSize(upper);
    cropper->SetInput(itkImage);
    cropper->Update();

    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( cropper->GetOutput() );
    image->SetVolume( cropper->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = m_SelectedImageNode->GetName().c_str();

    imageNode->SetName((name+"_cropped").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedImageNode);

    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoApplySpacing()
{
    if (m_DiffusionImage.IsNotNull())
    {
        mitk::Vector3D spacing;
        spacing[0] = m_Controls->m_HeaderSpacingX->value();
        spacing[1] = m_Controls->m_HeaderSpacingY->value();
        spacing[2] = m_Controls->m_HeaderSpacingZ->value();

        MitkDwiType::Pointer image = m_DiffusionImage->Clone();
        image->GetVectorImage()->SetSpacing(spacing);
        image->InitializeFromVectorImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( image );
        QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();
        imageNode->SetName((name+"_newspacing").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if(m_SelectedImage.IsNotNull())
    {
        AccessFixedDimensionByItk(m_SelectedImage, TemplatedSetImageSpacing,3);
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedSetImageSpacing( itk::Image<TPixel, VImageDimension>* itkImage)
{
    mitk::Vector3D spacing;
    spacing[0] = m_Controls->m_HeaderSpacingX->value();
    spacing[1] = m_Controls->m_HeaderSpacingY->value();
    spacing[2] = m_Controls->m_HeaderSpacingZ->value();

    typedef itk::ImageDuplicator< itk::Image<TPixel, VImageDimension> > DuplicateFilterType;
    typename DuplicateFilterType::Pointer duplicator = DuplicateFilterType::New();
    duplicator->SetInputImage( itkImage );
    duplicator->Update();
    typename itk::Image<TPixel, VImageDimension>::Pointer newImage = duplicator->GetOutput();
    newImage->SetSpacing(spacing);

    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( newImage.GetPointer() );
    image->SetVolume( newImage->GetBufferPointer() );
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = m_SelectedImageNode->GetName().c_str();

    imageNode->SetName((name+"_newspacing").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedImageNode);
    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoApplyOrigin()
{
    if (m_DiffusionImage.IsNotNull())
    {
        mitk::Vector3D origin;
        origin[0] = m_Controls->m_HeaderOriginX->value();
        origin[1] = m_Controls->m_HeaderOriginY->value();
        origin[2] = m_Controls->m_HeaderOriginZ->value();

        MitkDwiType::Pointer image = m_DiffusionImage->Clone();
        image->GetVectorImage()->SetOrigin(origin);
        image->InitializeFromVectorImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( image );
        QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();
        imageNode->SetName((name+"_neworigin").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if(m_SelectedImage.IsNotNull())
    {
        AccessFixedDimensionByItk(m_SelectedImage, TemplatedSetImageOrigin,3);
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedSetImageOrigin( itk::Image<TPixel, VImageDimension>* itkImage)
{
    mitk::Vector3D origin;
    origin[0] = m_Controls->m_HeaderOriginX->value();
    origin[1] = m_Controls->m_HeaderOriginY->value();
    origin[2] = m_Controls->m_HeaderOriginZ->value();

    typedef itk::ImageDuplicator< itk::Image<TPixel, VImageDimension> > DuplicateFilterType;
    typename DuplicateFilterType::Pointer duplicator = DuplicateFilterType::New();
    duplicator->SetInputImage( itkImage );
    duplicator->Update();
    typename itk::Image<TPixel, VImageDimension>::Pointer newImage = duplicator->GetOutput();
    newImage->SetOrigin(origin);

    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( newImage.GetPointer() );
    image->SetVolume( newImage->GetBufferPointer() );
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = m_SelectedImageNode->GetName().c_str();

    imageNode->SetName((name+"_neworigin").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedImageNode);
    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoUpdateInterpolationGui(int i)
{
    switch (i)
    {
    case 0:
    {
        m_Controls->m_ResampleIntFrame->setVisible(false);
        m_Controls->m_ResampleDoubleFrame->setVisible(true);
        break;
    }
    case 1:
    {
        m_Controls->m_ResampleIntFrame->setVisible(false);
        m_Controls->m_ResampleDoubleFrame->setVisible(true);

        if (m_DiffusionImage.IsNotNull())
        {
            ItkDwiType::Pointer itkDwi = m_DiffusionImage->GetVectorImage();
            m_Controls->m_ResampleDoubleX->setValue(itkDwi->GetSpacing()[0]);
            m_Controls->m_ResampleDoubleY->setValue(itkDwi->GetSpacing()[1]);
            m_Controls->m_ResampleDoubleZ->setValue(itkDwi->GetSpacing()[2]);
        }
        else if (m_SelectedImage.IsNotNull())
        {
            mitk::BaseGeometry* geom = m_SelectedImage->GetGeometry();
            m_Controls->m_ResampleDoubleX->setValue(geom->GetSpacing()[0]);
            m_Controls->m_ResampleDoubleY->setValue(geom->GetSpacing()[1]);
            m_Controls->m_ResampleDoubleZ->setValue(geom->GetSpacing()[2]);
        }
        break;
    }
    case 2:
    {
        m_Controls->m_ResampleIntFrame->setVisible(true);
        m_Controls->m_ResampleDoubleFrame->setVisible(false);

        if (m_DiffusionImage.IsNotNull())
        {
            ItkDwiType::Pointer itkDwi = m_DiffusionImage->GetVectorImage();
            m_Controls->m_ResampleIntX->setValue(itkDwi->GetLargestPossibleRegion().GetSize(0));
            m_Controls->m_ResampleIntY->setValue(itkDwi->GetLargestPossibleRegion().GetSize(1));
            m_Controls->m_ResampleIntZ->setValue(itkDwi->GetLargestPossibleRegion().GetSize(2));
        }
        else if (m_SelectedImage.IsNotNull())
        {
            mitk::BaseGeometry* geom = m_SelectedImage->GetGeometry();
            m_Controls->m_ResampleIntX->setValue(geom->GetExtent(0));
            m_Controls->m_ResampleIntY->setValue(geom->GetExtent(1));
            m_Controls->m_ResampleIntZ->setValue(geom->GetExtent(2));
        }
        break;
    }
    default:
    {
        m_Controls->m_ResampleIntFrame->setVisible(false);
        m_Controls->m_ResampleDoubleFrame->setVisible(true);
    }
    }
}

void QmitkPreprocessingView::DoExtractBrainMask()
{
    //    if (m_SelectedImage.IsNull())
    //        return;

    //    typedef itk::Image<unsigned short, 3> ShortImageType;
    //    ShortImageType::Pointer itkImage = ShortImageType::New();
    //    mitk::CastToItkImage(m_SelectedImage, itkImage);

    //    typedef itk::BrainMaskExtractionImageFilter< unsigned char > FilterType;
    //    FilterType::Pointer filter = FilterType::New();
    //    filter->SetInput(itkImage);
    //    filter->SetMaxNumIterations(m_Controls->m_BrainMaskIterationsBox->value());
    //    filter->Update();

    //    mitk::Image::Pointer image = mitk::Image::New();
    //    image->InitializeByItk( filter->GetOutput() );
    //    image->SetVolume( filter->GetOutput()->GetBufferPointer() );
    //    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    //    imageNode->SetData( image );
    //    imageNode->SetName("BRAINMASK");
    //    GetDefaultDataStorage()->Add(imageNode);
}

void QmitkPreprocessingView::DoResampleImage()
{
    if (m_DiffusionImage.IsNotNull())
    {
        typedef itk::ResampleDwiImageFilter< short > ResampleFilter;
        ResampleFilter::Pointer resampler = ResampleFilter::New();
        resampler->SetInput(m_DiffusionImage->GetVectorImage());

        switch (m_Controls->m_ResampleTypeBox->currentIndex())
        {
        case 0:
        {
            itk::Vector< double, 3 > samplingFactor;
            samplingFactor[0] = m_Controls->m_ResampleDoubleX->value();
            samplingFactor[1] = m_Controls->m_ResampleDoubleY->value();
            samplingFactor[2] = m_Controls->m_ResampleDoubleZ->value();
            resampler->SetSamplingFactor(samplingFactor);
            break;
        }
        case 1:
        {
            itk::Vector< double, 3 > newSpacing;
            newSpacing[0] = m_Controls->m_ResampleDoubleX->value();
            newSpacing[1] = m_Controls->m_ResampleDoubleY->value();
            newSpacing[2] = m_Controls->m_ResampleDoubleZ->value();
            resampler->SetNewSpacing(newSpacing);
            break;
        }
        case 2:
        {
            itk::ImageRegion<3> newRegion;
            newRegion.SetSize(0, m_Controls->m_ResampleIntX->value());
            newRegion.SetSize(1, m_Controls->m_ResampleIntY->value());
            newRegion.SetSize(2, m_Controls->m_ResampleIntZ->value());
            resampler->SetNewImageSize(newRegion);
            break;
        }
        default:
        {
            MITK_WARN << "Unknown resampling parameters!";
            return;
        }
        }

        QString outAdd;
        switch (m_Controls->m_InterpolatorBox->currentIndex())
        {
        case 0:
        {
            resampler->SetInterpolation(ResampleFilter::Interpolate_NearestNeighbour);
            outAdd = "NearestNeighbour";
            break;
        }
        case 1:
        {
            resampler->SetInterpolation(ResampleFilter::Interpolate_Linear);
            outAdd = "Linear";
            break;
        }
        case 2:
        {
            resampler->SetInterpolation(ResampleFilter::Interpolate_BSpline);
            outAdd = "BSpline";
            break;
        }
        case 3:
        {
            resampler->SetInterpolation(ResampleFilter::Interpolate_WindowedSinc);
            outAdd = "WindowedSinc";
            break;
        }
        default:
        {
            resampler->SetInterpolation(ResampleFilter::Interpolate_NearestNeighbour);
            outAdd = "NearestNeighbour";
        }
        }

        resampler->Update();

        typedef mitk::DiffusionImage<DiffusionPixelType>  DiffusionImageType;
        DiffusionImageType::Pointer image = DiffusionImageType::New();
        image->SetVectorImage( resampler->GetOutput() );
        image->SetReferenceBValue( m_DiffusionImage->GetReferenceBValue() );
        image->SetDirections( m_DiffusionImage->GetDirections() );
        image->InitializeFromVectorImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( image );
        QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();

        imageNode->SetName((name+"_resampled_"+outAdd).toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
    }
    else if (m_SelectedImage.IsNotNull())
    {
        AccessFixedDimensionByItk(m_SelectedImage, TemplatedResampleImage,3);
    }
}


template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedResampleImage( itk::Image<TPixel, VImageDimension>* itkImage)
{
    itk::Vector< double, 3 > newSpacing;
    itk::ImageRegion<3>   newRegion;

    switch (m_Controls->m_ResampleTypeBox->currentIndex())
    {
    case 0:
    {
        itk::Vector< double, 3 > sampling;
        sampling[0] = m_Controls->m_ResampleDoubleX->value();
        sampling[1] = m_Controls->m_ResampleDoubleY->value();
        sampling[2] = m_Controls->m_ResampleDoubleZ->value();

        newSpacing = itkImage->GetSpacing();
        newSpacing[0] /= sampling[0];
        newSpacing[1] /= sampling[1];
        newSpacing[2] /= sampling[2];
        newRegion = itkImage->GetLargestPossibleRegion();
        newRegion.SetSize(0, newRegion.GetSize(0)*sampling[0]);
        newRegion.SetSize(1, newRegion.GetSize(1)*sampling[1]);
        newRegion.SetSize(2, newRegion.GetSize(2)*sampling[2]);

        break;
    }
    case 1:
    {
        newSpacing[0] = m_Controls->m_ResampleDoubleX->value();
        newSpacing[1] = m_Controls->m_ResampleDoubleY->value();
        newSpacing[2] = m_Controls->m_ResampleDoubleZ->value();

        itk::Vector< double, 3 > oldSpacing = itkImage->GetSpacing();
        itk::Vector< double, 3 > sampling;
        sampling[0] = oldSpacing[0]/newSpacing[0];
        sampling[1] = oldSpacing[1]/newSpacing[1];
        sampling[2] = oldSpacing[2]/newSpacing[2];
        newRegion = itkImage->GetLargestPossibleRegion();
        newRegion.SetSize(0, newRegion.GetSize(0)*sampling[0]);
        newRegion.SetSize(1, newRegion.GetSize(1)*sampling[1]);
        newRegion.SetSize(2, newRegion.GetSize(2)*sampling[2]);
        break;
    }
    case 2:
    {
        newRegion.SetSize(0, m_Controls->m_ResampleIntX->value());
        newRegion.SetSize(1, m_Controls->m_ResampleIntY->value());
        newRegion.SetSize(2, m_Controls->m_ResampleIntZ->value());

        itk::ImageRegion<3> oldRegion = itkImage->GetLargestPossibleRegion();
        itk::Vector< double, 3 > sampling;
        sampling[0] = (double)newRegion.GetSize(0)/oldRegion.GetSize(0);
        sampling[1] = (double)newRegion.GetSize(1)/oldRegion.GetSize(1);
        sampling[2] = (double)newRegion.GetSize(2)/oldRegion.GetSize(2);

        newSpacing = itkImage->GetSpacing();
        newSpacing[0] /= sampling[0];
        newSpacing[1] /= sampling[1];
        newSpacing[2] /= sampling[2];
        break;
    }
    default:
    {
        MITK_WARN << "Unknown resampling parameters!";
        return;
    }
    }

    itk::Point<double,3> origin = itkImage->GetOrigin();
    origin[0] -= itkImage->GetSpacing()[0]/2;
    origin[1] -= itkImage->GetSpacing()[1]/2;
    origin[2] -= itkImage->GetSpacing()[2]/2;
    origin[0] += newSpacing[0]/2;
    origin[1] += newSpacing[1]/2;
    origin[2] += newSpacing[2]/2;

    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typename ImageType::Pointer outImage = ImageType::New();
    outImage->SetSpacing( newSpacing );
    outImage->SetOrigin( origin );
    outImage->SetDirection( itkImage->GetDirection() );
    outImage->SetLargestPossibleRegion( newRegion );
    outImage->SetBufferedRegion( newRegion );
    outImage->SetRequestedRegion( newRegion );
    outImage->Allocate();

    typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleFilter;
    typename ResampleFilter::Pointer resampler = ResampleFilter::New();
    resampler->SetInput(itkImage);
    resampler->SetOutputParametersFromImage(outImage);

    QString outAdd;
    switch (m_Controls->m_InterpolatorBox->currentIndex())
    {
    case 0:
    {
        typename itk::NearestNeighborInterpolateImageFunction<ImageType>::Pointer interp = itk::NearestNeighborInterpolateImageFunction<ImageType>::New();
        resampler->SetInterpolator(interp);
        outAdd = "NearestNeighbour";
        break;
    }
    case 1:
    {
        typename itk::LinearInterpolateImageFunction<ImageType>::Pointer interp = itk::LinearInterpolateImageFunction<ImageType>::New();
        resampler->SetInterpolator(interp);
        outAdd = "Linear";
        break;
    }
    case 2:
    {
        typename itk::BSplineInterpolateImageFunction<ImageType>::Pointer interp = itk::BSplineInterpolateImageFunction<ImageType>::New();
        resampler->SetInterpolator(interp);
        outAdd = "BSpline";
        break;
    }
    case 3:
    {
        typename itk::WindowedSincInterpolateImageFunction<ImageType, 3>::Pointer interp = itk::WindowedSincInterpolateImageFunction<ImageType, 3>::New();
        resampler->SetInterpolator(interp);
        outAdd = "WindowedSinc";
        break;
    }
    default:
    {
        typename itk::NearestNeighborInterpolateImageFunction<ImageType>::Pointer interp = itk::NearestNeighborInterpolateImageFunction<ImageType>::New();
        resampler->SetInterpolator(interp);
        outAdd = "NearestNeighbour";
    }
    }

    resampler->Update();

    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( resampler->GetOutput() );
    image->SetVolume( resampler->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = m_SelectedImageNode->GetName().c_str();

    imageNode->SetName((name+"_resampled_"+outAdd).toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedImageNode);
}

void QmitkPreprocessingView::DoApplyDirectionMatrix()
{
    if (m_DiffusionImage.IsNotNull())
    {
        MitkDwiType::Pointer newDwi = m_DiffusionImage->Clone();
        ItkDwiType::DirectionType newDirection;
        for (int r=0; r<3; r++)
            for (int c=0; c<3; c++)
            {
                QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item(r,c);
                if (!item)
                    return;
                newDirection[r][c] = item->text().toDouble();
            }

        ItkDwiType::Pointer itkDwi = newDwi->GetVectorImage();

        typedef mitk::DiffusionImage<DiffusionPixelType>  MitkDwiType;
        vnl_matrix_fixed< double,3,3 > oldInverseDirection = itkDwi->GetDirection().GetInverse();
        MitkDwiType::GradientDirectionContainerType::Pointer oldGradients = m_DiffusionImage->GetDirectionsWithoutMeasurementFrame();
        MitkDwiType::GradientDirectionContainerType::Pointer newGradients = MitkDwiType::GradientDirectionContainerType::New();

        for (unsigned int i=0; i<oldGradients->Size(); i++)
        {
            MitkDwiType::GradientDirectionType g = oldGradients->GetElement(i);
            double mag = g.magnitude();
            MitkDwiType::GradientDirectionType newG = oldInverseDirection*g;
            newG = newDirection.GetVnlMatrix()*newG;
            newG.normalize();
            newGradients->InsertElement(i, newG*mag);
        }

        newDwi->SetDirections(newGradients);
        itkDwi->SetDirection(newDirection);
        newDwi->InitializeFromVectorImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( newDwi );
        QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();
        imageNode->SetName((name+"_newdirection").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());

        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if (m_SelectedImage.IsNotNull())
    {
        AccessFixedDimensionByItk(m_SelectedImage, TemplatedApplyRotation,3);
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedApplyRotation( itk::Image<TPixel, VImageDimension>* itkImage)
{
    ItkDwiType::DirectionType newDirection;
    for (int r=0; r<3; r++)
        for (int c=0; c<3; c++)
        {
            QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item(r,c);
            if (!item)
                return;
            newDirection[r][c] = item->text().toDouble();
        }
    typedef itk::Image<TPixel, VImageDimension> ImageType;

    typename ImageType::Pointer newImage = ImageType::New();
    newImage->SetSpacing( itkImage->GetSpacing() );
    newImage->SetOrigin( itkImage->GetOrigin() );
    newImage->SetDirection( newDirection );
    newImage->SetLargestPossibleRegion( itkImage->GetLargestPossibleRegion() );
    newImage->SetBufferedRegion( itkImage->GetLargestPossibleRegion() );
    newImage->SetRequestedRegion( itkImage->GetLargestPossibleRegion() );
    newImage->Allocate();
    newImage->FillBuffer(0);

    itk::ImageRegionIterator< itk::Image<TPixel, VImageDimension> > it(itkImage, itkImage->GetLargestPossibleRegion());
    while(!it.IsAtEnd())
    {
        newImage->SetPixel(it.GetIndex(), it.Get());
        ++it;
    }

    mitk::Image::Pointer newMitkImage = mitk::Image::New();
    newMitkImage->InitializeByItk(newImage.GetPointer());
    newMitkImage->SetVolume(newImage->GetBufferPointer());

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newMitkImage );
    QString name = m_SelectedImageNode->GetName().c_str();
    imageNode->SetName((name+"_newdirection").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedImageNode);

    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoProjectSignal()
{
    switch(m_Controls->m_ProjectionMethodBox->currentIndex())
    {
    case 0:
        DoADCAverage();
        break;
    case 1:
        DoAKCFit();
        break;
    case 2:
        DoBiExpFit();
        break;
    default:
        DoADCAverage();
    }
}

void QmitkPreprocessingView::DoDwiNormalization()
{
    if (m_DiffusionImage.IsNull())
        return;

    int b0Index = -1;
    for (unsigned int i=0; i<m_DiffusionImage->GetNumberOfChannels(); i++)
    {
        GradientDirectionType g = m_DiffusionImage->GetDirections()->GetElement(i);
        if (g.magnitude()<0.001)
        {
            b0Index = i;
            break;
        }
    }
    if (b0Index==-1)
        return;

    typedef mitk::DiffusionImage<DiffusionPixelType>  DiffusionImageType;
    typedef itk::DwiNormilzationFilter<short>  FilterType;

    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(m_DiffusionImage->GetVectorImage());
    filter->SetGradientDirections(m_DiffusionImage->GetDirections());

    UcharImageType::Pointer itkImage = NULL;
    if (m_Controls->m_NormalizationMaskBox->GetSelectedNode().IsNotNull())
    {
        itkImage = UcharImageType::New();
        mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_Controls->m_NormalizationMaskBox->GetSelectedNode()->GetData()), itkImage);
        filter->SetMaskImage(itkImage);
    }

    // determin normalization reference
    switch(m_Controls->m_NormalizationReferenceBox->currentIndex())
    {
    case 0: // normalize relative to mean white matter signal intensity
    {
        typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, double > TensorReconstructionImageFilterType;
        TensorReconstructionImageFilterType::Pointer dtFilter = TensorReconstructionImageFilterType::New();
        dtFilter->SetGradientImage( m_DiffusionImage->GetDirections(), m_DiffusionImage->GetVectorImage() );
        dtFilter->SetBValue(m_DiffusionImage->GetReferenceBValue());
        dtFilter->Update();
        itk::Image< itk::DiffusionTensor3D< double >, 3 >::Pointer tensorImage = dtFilter->GetOutput();
        itk::ImageRegionIterator< itk::Image< itk::DiffusionTensor3D< double >, 3 > > inIt(tensorImage, tensorImage->GetLargestPossibleRegion());
        double ref = 0;
        unsigned int count = 0;
        while ( !inIt.IsAtEnd() )
        {
            if (itkImage.IsNotNull() && itkImage->GetPixel(inIt.GetIndex())<=0)
            {
                ++inIt;
                continue;
            }

            double FA = inIt.Get().GetFractionalAnisotropy();
            if (FA>0.4 && FA<0.99)
            {
                ref += m_DiffusionImage->GetVectorImage()->GetPixel(inIt.GetIndex())[b0Index];
                count++;
            }
            ++inIt;
        }
        if (count>0)
        {
            ref /= count;
            filter->SetUseGlobalReference(true);
            filter->SetReference(ref);
        }
        break;
    }
    case 1: // normalize relative to mean CSF signal intensity
    {
        itk::AdcImageFilter< short, double >::Pointer adcFilter = itk::AdcImageFilter< short, double >::New();
        adcFilter->SetInput(m_DiffusionImage->GetVectorImage());
        adcFilter->SetGradientDirections(m_DiffusionImage->GetDirections());
        adcFilter->SetB_value(m_DiffusionImage->GetReferenceBValue());
        adcFilter->Update();
        ItkDoubleImageType::Pointer adcImage = adcFilter->GetOutput();
        itk::ImageRegionIterator<ItkDoubleImageType> inIt(adcImage, adcImage->GetLargestPossibleRegion());
        double max = 0.0030;
        double ref = 0;
        unsigned int count = 0;
        while ( !inIt.IsAtEnd() )
        {
            if (itkImage.IsNotNull() && itkImage->GetPixel(inIt.GetIndex())<=0)
            {
                ++inIt;
                continue;
            }
            if (inIt.Get()>max && inIt.Get()<0.004)
            {
                ref += m_DiffusionImage->GetVectorImage()->GetPixel(inIt.GetIndex())[b0Index];
                count++;
            }
            ++inIt;
        }
        if (count>0)
        {
            ref /= count;
            filter->SetUseGlobalReference(true);
            filter->SetReference(ref);
        }
        break;
    }
    case 2:
    {
        filter->SetUseGlobalReference(false);
    }
    }
    filter->Update();

    DiffusionImageType::Pointer image = DiffusionImageType::New();
    image->SetVectorImage( filter->GetOutput() );
    image->SetReferenceBValue( m_DiffusionImage->GetReferenceBValue() );
    image->SetDirections( m_DiffusionImage->GetDirections() );
    image->InitializeFromVectorImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();

    imageNode->SetName((name+"_normalized").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
}

void QmitkPreprocessingView::DoLengthCorrection()
{
    if (m_DiffusionImage.IsNull())
        return;

    typedef mitk::DiffusionImage<DiffusionPixelType>  DiffusionImageType;
    typedef itk::DwiGradientLengthCorrectionFilter  FilterType;

    FilterType::Pointer filter = FilterType::New();
    filter->SetRoundingValue( m_Controls->m_B_ValueMap_Rounder_SpinBox->value());
    filter->SetReferenceBValue(m_DiffusionImage->GetReferenceBValue());
    filter->SetReferenceGradientDirectionContainer(m_DiffusionImage->GetDirections());
    filter->Update();

    DiffusionImageType::Pointer image = DiffusionImageType::New();
    image->SetVectorImage( m_DiffusionImage->GetVectorImage());
    image->SetReferenceBValue( filter->GetNewBValue() );
    image->SetDirections( filter->GetOutputGradientDirectionContainer());
    image->InitializeFromVectorImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();

    imageNode->SetName((name+"_rounded").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
}

void QmitkPreprocessingView::UpdateDwiBValueMapRounder(int i)
{
    if (m_DiffusionImage.IsNull())
        return;
    //m_DiffusionImage->UpdateBValueMap();
    UpdateBValueTableWidget(i);
}

void QmitkPreprocessingView::CallMultishellToSingleShellFilter(itk::DWIVoxelFunctor * functor, mitk::DiffusionImage<DiffusionPixelType>::Pointer ImPtr, QString imageName, mitk::DataNode* parent)
{
    typedef itk::RadialMultishellToSingleshellImageFilter<DiffusionPixelType, DiffusionPixelType> FilterType;

    // filter input parameter
    const mitk::DiffusionImage<DiffusionPixelType>::BValueMap
            &originalShellMap  = ImPtr->GetBValueMap();

    const mitk::DiffusionImage<DiffusionPixelType>::ImageType
            *vectorImage       = ImPtr->GetVectorImage();

    const mitk::DiffusionImage<DiffusionPixelType>::GradientDirectionContainerType::Pointer
            gradientContainer = ImPtr->GetDirections();

    const unsigned int
            &bValue            = ImPtr->GetReferenceBValue();

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
    outImage->SetReferenceBValue( m_Controls->m_targetBValueSpinBox->value() );
    outImage->SetDirections( filter->GetTargetGradientDirections() );
    outImage->InitializeFromVectorImage();

    imageNode = mitk::DataNode::New();
    imageNode->SetData( outImage );
    imageNode->SetName(imageName.toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, parent);

    //  if(m_Controls->m_OutputRMSErrorImage->isChecked()){
    //    // create new Error image
    //    FilterType::ErrorImageType::Pointer errImage = filter->GetErrorImage();
    //    mitk::Image::Pointer mitkErrImage = mitk::Image::New();
    //    mitkErrImage->InitializeByItk<FilterType::ErrorImageType>(errImage);
    //    mitkErrImage->SetVolume(errImage->GetBufferPointer());

    //    imageNode = mitk::DataNode::New();
    //    imageNode->SetData( mitkErrImage );
    //    imageNode->SetName((imageName+"_Error").toStdString().c_str());
    //    GetDefaultDataStorage()->Add(imageNode);
    //  }
}

void QmitkPreprocessingView::DoBiExpFit()
{
    itk::BiExpFitFunctor::Pointer functor = itk::BiExpFitFunctor::New();

    for (unsigned int i=0; i<m_SelectedDiffusionNodes.size(); i++)
    {
        mitk::DiffusionImage<DiffusionPixelType>::Pointer inImage =
                dynamic_cast< mitk::DiffusionImage<DiffusionPixelType>* >(m_SelectedDiffusionNodes.at(i)->GetData());

        QString name(m_SelectedDiffusionNodes.at(i)->GetName().c_str());

        const mitk::DiffusionImage<DiffusionPixelType>::BValueMap & originalShellMap = inImage->GetBValueMap();
        mitk::DiffusionImage<DiffusionPixelType>::BValueMap::const_iterator it = originalShellMap.begin();
        ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
        vnl_vector<double> bValueList(originalShellMap.size()-1);
        while(it != originalShellMap.end())
            bValueList.put(s++,(it++)->first);

        const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
        functor->setListOfBValues(bValueList);
        functor->setTargetBValue(targetBValue);
        CallMultishellToSingleShellFilter(functor,inImage,name + "_BiExp", m_SelectedDiffusionNodes.at(i));
    }
}

void QmitkPreprocessingView::DoAKCFit()
{
    itk::KurtosisFitFunctor::Pointer functor = itk::KurtosisFitFunctor::New();

    for (unsigned int i=0; i<m_SelectedDiffusionNodes.size(); i++)
    {
        mitk::DiffusionImage<DiffusionPixelType>::Pointer inImage =
                dynamic_cast< mitk::DiffusionImage<DiffusionPixelType>* >(m_SelectedDiffusionNodes.at(i)->GetData());

        QString name(m_SelectedDiffusionNodes.at(i)->GetName().c_str());

        const mitk::DiffusionImage<DiffusionPixelType>::BValueMap & originalShellMap = inImage->GetBValueMap();
        mitk::DiffusionImage<DiffusionPixelType>::BValueMap::const_iterator it = originalShellMap.begin();
        ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
        vnl_vector<double> bValueList(originalShellMap.size()-1);
        while(it != originalShellMap.end())
            bValueList.put(s++,(it++)->first);

        const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
        functor->setListOfBValues(bValueList);
        functor->setTargetBValue(targetBValue);
        CallMultishellToSingleShellFilter(functor,inImage,name + "_AKC", m_SelectedDiffusionNodes.at(i));
    }
}

void QmitkPreprocessingView::DoADCFit()
{
    // later
}

void QmitkPreprocessingView::DoADCAverage()
{
    itk::ADCAverageFunctor::Pointer functor = itk::ADCAverageFunctor::New();

    for (unsigned int i=0; i<m_SelectedDiffusionNodes.size(); i++)
    {
        mitk::DiffusionImage<DiffusionPixelType>::Pointer inImage =
                dynamic_cast< mitk::DiffusionImage<DiffusionPixelType>* >(m_SelectedDiffusionNodes.at(i)->GetData());

        QString name(m_SelectedDiffusionNodes.at(i)->GetName().c_str());

        const mitk::DiffusionImage<DiffusionPixelType>::BValueMap & originalShellMap = inImage->GetBValueMap();
        mitk::DiffusionImage<DiffusionPixelType>::BValueMap::const_iterator it = originalShellMap.begin();
        ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
        vnl_vector<double> bValueList(originalShellMap.size()-1);
        while(it != originalShellMap.end())
            bValueList.put(s++,(it++)->first);

        const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
        functor->setListOfBValues(bValueList);
        functor->setTargetBValue(targetBValue);
        CallMultishellToSingleShellFilter(functor,inImage,name + "_ADC", m_SelectedDiffusionNodes.at(i));
    }
}

void QmitkPreprocessingView::DoAdcCalculation()
{
    if (m_DiffusionImage.IsNull())
        return;

    typedef mitk::DiffusionImage< DiffusionPixelType >            DiffusionImageType;
    typedef itk::AdcImageFilter< DiffusionPixelType, double >     FilterType;


    for (unsigned int i=0; i<m_SelectedDiffusionNodes.size(); i++)
    {
        DiffusionImageType::Pointer inImage = dynamic_cast< DiffusionImageType* >(m_SelectedDiffusionNodes.at(i)->GetData());
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(inImage->GetVectorImage());
        filter->SetGradientDirections(inImage->GetDirections());
        filter->SetB_value(inImage->GetReferenceBValue());
        filter->Update();

        mitk::Image::Pointer image = mitk::Image::New();
        image->InitializeByItk( filter->GetOutput() );
        image->SetVolume( filter->GetOutput()->GetBufferPointer() );
        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( image );
        QString name = m_SelectedDiffusionNodes.at(i)->GetName().c_str();

        imageNode->SetName((name+"_ADC").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.at(i));
    }
}


void QmitkPreprocessingView::UpdateBValueTableWidget(int i)
{
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

        typedef mitk::DiffusionImage<short>::BValueMap BValueMap;
        typedef mitk::DiffusionImage<short>::BValueMap::iterator BValueMapIterator;

        BValueMapIterator it;

        BValueMap roundedBValueMap = m_DiffusionImage->GetBValueMap();

        m_Controls->m_B_ValueMap_TableWidget->clear();
        m_Controls->m_B_ValueMap_TableWidget->setRowCount(roundedBValueMap.size() );
        QStringList headerList;
        headerList << "b-Value" << "Number of gradients";
        m_Controls->m_B_ValueMap_TableWidget->setHorizontalHeaderLabels(headerList);

        int i = 0 ;
        for(it = roundedBValueMap.begin() ;it != roundedBValueMap.end(); it++)
        {
            m_Controls->m_B_ValueMap_TableWidget->setItem(i,0,new QTableWidgetItem(QString::number(it->first)));
            QTableWidgetItem* item = m_Controls->m_B_ValueMap_TableWidget->item(i,0);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            m_Controls->m_B_ValueMap_TableWidget->setItem(i,1,new QTableWidgetItem(QString::number(it->second.size())));
            i++;
        }
    }

}


template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedUpdateGui( itk::Image<TPixel, VImageDimension>* itkImage)
{
    for (int r=0; r<3; r++)
        for (int c=0; c<3; c++)
        {
            QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item(r,c);
            delete item;
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            item->setText(QString::number(itkImage->GetDirection()[r][c]));
            m_Controls->m_DirectionMatrixTable->setItem(r,c,item);
        }
}

void QmitkPreprocessingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    bool foundDwiVolume = false;
    bool foundImageVolume = false;
    m_DiffusionImage = NULL;
    m_SelectedImage = NULL;
    m_SelectedImageNode = NULL;
    m_SelectedDiffusionNodes.clear();

    // iterate selection
    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;

        if( node.IsNotNull() && dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData()) )
        {
            foundDwiVolume = true;
            foundImageVolume = true;
            m_DiffusionImage = dynamic_cast<mitk::DiffusionImage<DiffusionPixelType>*>(node->GetData());
            m_Controls->m_DiffusionImageLabel->setText(node->GetName().c_str());
            m_SelectedDiffusionNodes.push_back(node);
        }
        else if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            foundImageVolume = true;
            m_SelectedImage = dynamic_cast<mitk::Image*>(node->GetData());
            m_Controls->m_DiffusionImageLabel->setText(node->GetName().c_str());
            m_SelectedImageNode = node;
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
    m_Controls->m_ProjectSignalButton->setEnabled(foundDwiVolume);
    m_Controls->m_CreateLengthCorrectedDwi->setEnabled(foundDwiVolume);
    m_Controls->m_CalcAdcButton->setEnabled(foundDwiVolume);
    m_Controls->m_targetBValueSpinBox->setEnabled(foundDwiVolume);
    m_Controls->m_NormalizeImageValuesButton->setEnabled(foundDwiVolume);
    m_Controls->m_DirectionMatrixTable->setEnabled(foundImageVolume);
    m_Controls->m_ModifyDirection->setEnabled(foundImageVolume);
    m_Controls->m_ExtractBrainMask->setEnabled(foundImageVolume);
    m_Controls->m_ResampleImageButton->setEnabled(foundImageVolume);
    m_Controls->m_ModifySpacingButton->setEnabled(foundImageVolume);
    m_Controls->m_ModifyOriginButton->setEnabled(foundImageVolume);
    m_Controls->m_CropImageButton->setEnabled(foundImageVolume);
    m_Controls->m_RemoveGradientButton->setEnabled(foundDwiVolume);
    m_Controls->m_ExtractGradientButton->setEnabled(foundDwiVolume);

    // reset sampling frame to 1 and update all ealted components
    m_Controls->m_B_ValueMap_Rounder_SpinBox->setValue(1);

    UpdateBValueTableWidget(m_Controls->m_B_ValueMap_Rounder_SpinBox->value());
    DoUpdateInterpolationGui(m_Controls->m_ResampleTypeBox->currentIndex());

    if (foundDwiVolume)
    {
        m_Controls->m_InputData->setTitle("Input Data");
        vnl_matrix_fixed< double, 3, 3 > mf = m_DiffusionImage->GetMeasurementFrame();
        for (int r=0; r<3; r++)
            for (int c=0; c<3; c++)
            {
                // Measurement frame
                {
                    QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
                    delete item;
                    item = new QTableWidgetItem();
                    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                    item->setText(QString::number(mf.get(r,c)));
                    m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
                }

                // Direction matrix
                {
                    QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item(r,c);
                    delete item;
                    item = new QTableWidgetItem();
                    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                    item->setText(QString::number(m_DiffusionImage->GetVectorImage()->GetDirection()[r][c]));
                    m_Controls->m_DirectionMatrixTable->setItem(r,c,item);
                }
            }

        //calculate target bValue for MultishellToSingleShellfilter
        const mitk::DiffusionImage<DiffusionPixelType>::BValueMap & bValMap = m_DiffusionImage->GetBValueMap();
        mitk::DiffusionImage<DiffusionPixelType>::BValueMap::const_iterator it = bValMap.begin();
        unsigned int targetBVal = 0;
        while(it != bValMap.end())
            targetBVal += (it++)->first;
        targetBVal /= (float)bValMap.size()-1;
        m_Controls->m_targetBValueSpinBox->setValue(targetBVal);

        m_Controls->m_HeaderSpacingX->setValue(m_DiffusionImage->GetGeometry()->GetSpacing()[0]);
        m_Controls->m_HeaderSpacingY->setValue(m_DiffusionImage->GetGeometry()->GetSpacing()[1]);
        m_Controls->m_HeaderSpacingZ->setValue(m_DiffusionImage->GetGeometry()->GetSpacing()[2]);
        m_Controls->m_HeaderOriginX->setValue(m_DiffusionImage->GetGeometry()->GetOrigin()[0]);
        m_Controls->m_HeaderOriginY->setValue(m_DiffusionImage->GetGeometry()->GetOrigin()[1]);
        m_Controls->m_HeaderOriginZ->setValue(m_DiffusionImage->GetGeometry()->GetOrigin()[2]);
        m_Controls->m_XstartBox->setMaximum(m_DiffusionImage->GetGeometry()->GetExtent(0)-1);
        m_Controls->m_YstartBox->setMaximum(m_DiffusionImage->GetGeometry()->GetExtent(1)-1);
        m_Controls->m_ZstartBox->setMaximum(m_DiffusionImage->GetGeometry()->GetExtent(2)-1);
        m_Controls->m_XendBox->setMaximum(m_DiffusionImage->GetGeometry()->GetExtent(0)-1);
        m_Controls->m_YendBox->setMaximum(m_DiffusionImage->GetGeometry()->GetExtent(1)-1);
        m_Controls->m_ZendBox->setMaximum(m_DiffusionImage->GetGeometry()->GetExtent(2)-1);
        m_Controls->m_RemoveGradientBox->setMaximum(m_DiffusionImage->GetDirections()->Size()-1);
        m_Controls->m_ExtractGradientBox->setMaximum(m_DiffusionImage->GetDirections()->Size()-1);
    }
    else if (foundImageVolume)
    {
        for (int r=0; r<3; r++)
            for (int c=0; c<3; c++)
            {
                QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
                delete item;
                item = new QTableWidgetItem();
                m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
            }

        m_Controls->m_HeaderSpacingX->setValue(m_SelectedImage->GetGeometry()->GetSpacing()[0]);
        m_Controls->m_HeaderSpacingY->setValue(m_SelectedImage->GetGeometry()->GetSpacing()[1]);
        m_Controls->m_HeaderSpacingZ->setValue(m_SelectedImage->GetGeometry()->GetSpacing()[2]);
        m_Controls->m_HeaderOriginX->setValue(m_SelectedImage->GetGeometry()->GetOrigin()[0]);
        m_Controls->m_HeaderOriginY->setValue(m_SelectedImage->GetGeometry()->GetOrigin()[1]);
        m_Controls->m_HeaderOriginZ->setValue(m_SelectedImage->GetGeometry()->GetOrigin()[2]);
        m_Controls->m_XstartBox->setMaximum(m_SelectedImage->GetGeometry()->GetExtent(0)-1);
        m_Controls->m_YstartBox->setMaximum(m_SelectedImage->GetGeometry()->GetExtent(1)-1);
        m_Controls->m_ZstartBox->setMaximum(m_SelectedImage->GetGeometry()->GetExtent(2)-1);
        m_Controls->m_XendBox->setMaximum(m_SelectedImage->GetGeometry()->GetExtent(0)-1);
        m_Controls->m_YendBox->setMaximum(m_SelectedImage->GetGeometry()->GetExtent(1)-1);
        m_Controls->m_ZendBox->setMaximum(m_SelectedImage->GetGeometry()->GetExtent(2)-1);

        AccessFixedDimensionByItk(m_SelectedImage, TemplatedUpdateGui,3);
    }
    else
    {
        for (int r=0; r<3; r++)
            for (int c=0; c<3; c++)
            {
                {
                    QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
                    delete item;
                    item = new QTableWidgetItem();
                    m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
                }
                {
                    QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item(r,c);
                    delete item;
                    item = new QTableWidgetItem();
                    m_Controls->m_DirectionMatrixTable->setItem(r,c,item);
                }
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

    MitkDwiType::Pointer newDwi = m_DiffusionImage->Clone();
    GradientDirectionContainerType::Pointer gradientContainer = newDwi->GetDirections();

    for (unsigned int j=0; j<gradientContainer->Size(); j++)
        if (gradientContainer->at(j)[0]<0)
            gradientContainer->at(j) = -gradientContainer->at(j);

    newDwi->SetDirections(gradientContainer);

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newDwi );
    QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();
    imageNode->SetName((name+"_halfsphere").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
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

    MitkDwiType::Pointer newDwi = m_DiffusionImage->Clone();
    newDwi->SetMeasurementFrame(mf);

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newDwi );
    QString name = m_SelectedDiffusionNodes.back()->GetName().c_str();
    imageNode->SetName((name+"_new-MF").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
}

void QmitkPreprocessingView::DoShowGradientDirections()
{
    if (m_DiffusionImage.IsNull())
        return;

    int maxIndex = 0;
    unsigned int maxSize = m_DiffusionImage->GetDimension(0);
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

    typedef mitk::DiffusionImage<short>::BValueMap BValueMap;
    typedef mitk::DiffusionImage<short>::BValueMap::iterator BValueMapIterator;
    BValueMap bValMap =  m_DiffusionImage->GetBValueMap();

    GradientDirectionContainerType::Pointer gradientContainer = m_DiffusionImage->GetDirections();
    mitk::BaseGeometry::Pointer geometry = m_DiffusionImage->GetGeometry();
    int shellCount = 1;
    for(BValueMapIterator it = bValMap.begin(); it!=bValMap.end(); ++it)
    {
        mitk::PointSet::Pointer pointset = mitk::PointSet::New();
        for (unsigned int j=0; j<it->second.size(); j++)
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
    BValueMap originalShellMap = m_DiffusionImage->GetBValueMap();
    std::vector<unsigned int> newNumGradientDirections;
    int shellCounter = 0;

    QString name = m_SelectedDiffusionNodes.front()->GetName().c_str();
    for (int i=0; i<m_Controls->m_B_ValueMap_TableWidget->rowCount(); i++)
    {
        double BValue = m_Controls->m_B_ValueMap_TableWidget->item(i,0)->text().toDouble();
        shellSlectionMap[BValue] = originalShellMap[BValue];
        unsigned int num = m_Controls->m_B_ValueMap_TableWidget->item(i,1)->text().toUInt();
        newNumGradientDirections.push_back(num);
        name += "_";
        name += QString::number(num);
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
    image->SetReferenceBValue(m_DiffusionImage->GetReferenceBValue());
    image->SetDirections(filter->GetGradientDirections());
    image->SetMeasurementFrame(m_DiffusionImage->GetMeasurementFrame());
    image->InitializeFromVectorImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );

    imageNode->SetName(name.toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, m_SelectedDiffusionNodes.back());
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
    for (unsigned int i=0; i<m_SelectedDiffusionNodes.size(); i++)
    {
        DiffusionImageType::Pointer dwi = dynamic_cast< mitk::DiffusionImage<DiffusionPixelType>* >( m_SelectedDiffusionNodes.at(i)->GetData() );
        if ( dwi.IsNotNull() )
        {
            imageContainer.push_back(dwi->GetVectorImage());
            gradientListContainer.push_back(dwi->GetDirections());
            bValueContainer.push_back(dwi->GetReferenceBValue());
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
    image->SetReferenceBValue(filter->GetB_Value());
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

        GetDefaultDataStorage()->Add(node, (*itemiter));

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

        GetDefaultDataStorage()->Add(node, (*itemiter));

        /*A reinitialization is needed to access the time channels via the ImageNavigationController
    The Global-Geometry can not recognize the time channel without a re-init.
    (for a new selection in datamanger a automatically updated of the Global-Geometry should be done - if it contains the time channel)*/
        mitk::RenderingManager::GetInstance()->InitializeViews(node->GetData()->GetTimeGeometry(),mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

        ++itemiter;
    }
}

void QmitkPreprocessingView::AverageGradients()
{
    int nrFiles = m_SelectedDiffusionNodes.size();
    if (!nrFiles) return;

    mitk::DataStorage::SetOfObjects::const_iterator itemiter( m_SelectedDiffusionNodes.begin() );
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( m_SelectedDiffusionNodes.end() );

    while ( itemiter != itemiterend ) // for all items
    {
        mitk::DiffusionImage<DiffusionPixelType>* mitkDwi =
                static_cast<mitk::DiffusionImage<DiffusionPixelType>*>(
                    (*itemiter)->GetData());

        MitkDwiType::Pointer newDwi = mitkDwi->Clone();
        newDwi->AverageRedundantGradients(m_Controls->m_Blur->value());

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( newDwi );
        QString name = (*itemiter)->GetName().c_str();
        imageNode->SetName((name+"_averaged").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, (*itemiter));

        ++itemiter;
    }
}
