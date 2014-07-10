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

        m_Controls->m_MeasurementFrameTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        m_Controls->m_MeasurementFrameTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);

        m_Controls->m_DirectionMatrixTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        m_Controls->m_DirectionMatrixTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
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
        connect( (QObject*)(m_Controls->m_ResampleImageButton), SIGNAL(clicked()), this, SLOT(DoResampleImage()) );
        connect( (QObject*)(m_Controls->m_ResampleTypeBox), SIGNAL(currentIndexChanged(int)), this, SLOT(DoUpdateInterpolationGui(int)) );
        //        connect( (QObject*)(m_Controls->m_ExtractBrainMask), SIGNAL(clicked()), this, SLOT(DoExtractBrainMask()) );
    }
}

void QmitkPreprocessingView::DoUpdateInterpolationGui(int i)
{
    switch (i)
    {
    case 0:
    {
        m_Controls->m_ResampleIntFrame->setVisible(false);
        m_Controls->m_ResampleDoubleFrame->setVisible(true);

        m_Controls->m_ResampleDoubleX->setValue(2);
        m_Controls->m_ResampleDoubleY->setValue(2);
        m_Controls->m_ResampleDoubleZ->setValue(2);
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
        m_Controls->m_ResampleDoubleX->setValue(2);
        m_Controls->m_ResampleDoubleY->setValue(2);
        m_Controls->m_ResampleDoubleZ->setValue(2);
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
        typename ResampleFilter::Pointer resampler = ResampleFilter::New();
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
        GetDefaultDataStorage()->Add(imageNode);
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

    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typename ImageType::Pointer outImage = ImageType::New();
    outImage->SetSpacing( newSpacing );
    outImage->SetOrigin( itkImage->GetOrigin() );
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
    GetDefaultDataStorage()->Add(imageNode);
}

void QmitkPreprocessingView::DoApplyDirectionMatrix()
{
    if (m_DiffusionImage.IsNotNull())
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

        ItkDwiType::Pointer itkDwi = m_DiffusionImage->GetVectorImage();

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

        m_DiffusionImage->SetDirections(newGradients);
        itkDwi->SetDirection(newDirection);
        m_DiffusionImage->InitializeFromVectorImage();

        mitk::RenderingManager::GetInstance()->InitializeViews( m_SelectedDiffusionNodes.back()->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
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

    m_SelectedImage->InitializeByItk(newImage.GetPointer());
    m_SelectedImage->SetVolume(newImage->GetBufferPointer());

    mitk::RenderingManager::GetInstance()->InitializeViews( m_SelectedImageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
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

    typedef mitk::DiffusionImage<DiffusionPixelType>  DiffusionImageType;
    typedef itk::DwiNormilzationFilter<short>  FilterType;

    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(m_DiffusionImage->GetVectorImage());
    filter->SetGradientDirections(m_DiffusionImage->GetDirections());
    if (m_Controls->m_NormalizationMethodBox->currentIndex()==1)
        filter->SetUseGlobalMax(true);
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
    GetDefaultDataStorage()->Add(imageNode);
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
    QString name = m_SelectedDiffusionNodes.front()->GetName().c_str();

    imageNode->SetName((name+"_rounded").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode);
}

void QmitkPreprocessingView::UpdateDwiBValueMapRounder(int i)
{
    if (m_DiffusionImage.IsNull())
        return;
    //m_DiffusionImage->UpdateBValueMap();
    UpdateBValueTableWidget(i);
}

void QmitkPreprocessingView::CallMultishellToSingleShellFilter(itk::DWIVoxelFunctor * functor, mitk::DiffusionImage<DiffusionPixelType>::Pointer ImPtr, QString imageName)
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
    GetDefaultDataStorage()->Add(imageNode);

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
        CallMultishellToSingleShellFilter(functor,inImage,name + "_BiExp");
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
        CallMultishellToSingleShellFilter(functor,inImage,name + "_ADC");
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
        GetDefaultDataStorage()->Add(imageNode);
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
    GradientDirectionContainerType::Pointer gradientContainer = m_DiffusionImage->GetDirections();

    for (unsigned int j=0; j<gradientContainer->Size(); j++)
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
