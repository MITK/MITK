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
#include <mitkITKImageImport.h>
#include <mitkBValueMapProperty.h>
#include <mitkGradientDirectionsProperty.h>
#include <mitkMeasurementFrameProperty.h>
#include <itkImageDuplicator.h>
#include <itkFlipImageFilter.h>
#include <mitkITKImageImport.h>

const std::string QmitkPreprocessingView::VIEW_ID =
        "org.mitk.views.preprocessing";

#define DI_INFO MITK_INFO("DiffusionImaging")


typedef float TTensorPixelType;


QmitkPreprocessingView::QmitkPreprocessingView()
    : QmitkFunctionality(),
      m_Controls(NULL),
      m_MultiWidget(NULL)
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

        m_Controls->m_SelctedImageComboBox->SetDataStorage(this->GetDataStorage());
        m_Controls->m_MergeDwiBox->SetDataStorage(this->GetDataStorage());

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
        m_Controls->m_SelctedImageComboBox->SetPredicate(isMitkImage);
        m_Controls->m_MergeDwiBox->SetPredicate(isMitkImage);

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
        connect( (QObject*)(m_Controls->m_FlipAxis), SIGNAL(clicked()), this, SLOT(DoFlipAxis()) );
        connect( (QObject*)(m_Controls->m_SelctedImageComboBox), SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnImageSelectionChanged()) );

        m_Controls->m_NormalizationMaskBox->SetZeroEntryText("--");

    }
}

void QmitkPreprocessingView::DoFlipAxis()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(node) );

    if (isDiffusionImage)
    {
        ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
        mitk::CastToItkImage(image, itkVectorImagePointer);

        itk::FixedArray<bool, 3> flipAxes;
        flipAxes[0] = m_Controls->m_FlipX->isChecked();
        flipAxes[1] = m_Controls->m_FlipY->isChecked();
        flipAxes[2] = m_Controls->m_FlipZ->isChecked();

        itk::FlipImageFilter< ItkDwiType >::Pointer flipper = itk::FlipImageFilter< ItkDwiType >::New();
        flipper->SetInput(itkVectorImagePointer);
        flipper->SetFlipAxes(flipAxes);
        flipper->Update();

        mitk::GradientDirectionsProperty::GradientDirectionsContainerType::Pointer oldGradients = static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();
        mitk::GradientDirectionsProperty::GradientDirectionsContainerType::Pointer newGradients = mitk::GradientDirectionsProperty::GradientDirectionsContainerType::New();

        for (unsigned int i=0; i<oldGradients->Size(); i++)
        {
            mitk::GradientDirectionsProperty::GradientDirectionType g = oldGradients->GetElement(i);
            mitk::GradientDirectionsProperty::GradientDirectionType newG = g;
            if (flipAxes[0])
                newG[0] *= -1;
            if (flipAxes[1])
                newG[1] *= -1;
            if (flipAxes[2])
                newG[2] *= -1;
            newGradients->InsertElement(i, newG);
        }

        mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( flipper->GetOutput() );
        newImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( newGradients ) );
        newImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() ) );
        newImage->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );
        mitk::DiffusionPropertyHelper propertyHelper( newImage );
        propertyHelper.InitializeImage();
        newImage->GetGeometry()->SetOrigin(image->GetGeometry()->GetOrigin());

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( newImage );
        QString name = node->GetName().c_str();

        imageNode->SetName((name+"_flipped").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, node);

        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if( image->GetPixelType().GetNumberOfComponents() == 1 )
    {
        AccessFixedDimensionByItk(image, TemplatedFlipAxis,3);
    }
    else
    {
      QMessageBox::warning(NULL,"Warning", QString("Operation not supported in multi-component images") );
    }

}


template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedFlipAxis(itk::Image<TPixel, VImageDimension>* itkImage)
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    itk::FixedArray<bool, 3> flipAxes;
    flipAxes[0] = m_Controls->m_FlipX->isChecked();
    flipAxes[1] = m_Controls->m_FlipY->isChecked();
    flipAxes[2] = m_Controls->m_FlipZ->isChecked();

    typename itk::FlipImageFilter< itk::Image<TPixel, VImageDimension> >::Pointer flipper = itk::FlipImageFilter< itk::Image<TPixel, VImageDimension> >::New();
    flipper->SetInput(itkImage);
    flipper->SetFlipAxes(flipAxes);
    flipper->Update();

    mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( flipper->GetOutput() );
    newImage->GetGeometry()->SetOrigin(image->GetGeometry()->GetOrigin());

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_flipped").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);

    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoRemoveGradient()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( !isDiffusionImage )
    {
        return;
    }

    std::vector< unsigned int > channelsToRemove; channelsToRemove.push_back(m_Controls->m_RemoveGradientBox->value());

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    itk::RemoveDwiChannelFilter< short >::Pointer filter = itk::RemoveDwiChannelFilter< short >::New();
    filter->SetInput(itkVectorImagePointer);
    filter->SetChannelIndices(channelsToRemove);
    filter->SetDirections( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );
    filter->Update();

    mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( filter->GetNewDirections() ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );
    mitk::DiffusionPropertyHelper propertyHelper( newImage );
    propertyHelper.InitializeImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_removedgradients").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);

    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoExtractGradient()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( !isDiffusionImage )
    {
        return;
    }

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    unsigned int channel = m_Controls->m_ExtractGradientBox->value();
    itk::ExtractDwiChannelFilter< short >::Pointer filter = itk::ExtractDwiChannelFilter< short >::New();
    filter->SetInput( itkVectorImagePointer);
    filter->SetChannelIndex(channel);
    filter->Update();

    mitk::Image::Pointer newImage = mitk::Image::New();
    newImage->InitializeByItk( filter->GetOutput() );
    newImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();
    imageNode->SetName((name+"_direction-"+QString::number(channel)).toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);

    mitk::RenderingManager::GetInstance()->InitializeViews(imageNode->GetData()->GetTimeGeometry(),mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
}

void QmitkPreprocessingView::DoCropImage()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( isDiffusionImage )
    {
        ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
        mitk::CastToItkImage(image, itkVectorImagePointer);

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
        cropper->SetInput( itkVectorImagePointer );
        cropper->Update();

        ItkDwiType::Pointer itkOutImage = cropper->GetOutput();
        itk::Point<double,3> origin = itkOutImage->GetOrigin();
        origin[0] += lower[0]*itkOutImage->GetSpacing()[0];
        origin[1] += lower[1]*itkOutImage->GetSpacing()[1];
        origin[2] += lower[2]*itkOutImage->GetSpacing()[2];
        itkOutImage->SetOrigin(origin);

        mitk::Image::Pointer newimage = mitk::GrabItkImageMemory( itkOutImage );
        newimage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() ) );
        newimage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() ) );
        newimage->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );
        mitk::DiffusionPropertyHelper propertyHelper( newimage );
        propertyHelper.InitializeImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( newimage );
        QString name = node->GetName().c_str();
        imageNode->SetName((name+"_cropped").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, node);
        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();

        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if( image->GetPixelType().GetNumberOfComponents() )
    {
        AccessFixedDimensionByItk(image, TemplatedCropImage,3);
    }
    else
    {
      QMessageBox::warning(NULL,"Warning", QString("Operation not supported in multi-component images") );
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedCropImage( itk::Image<TPixel, VImageDimension>* itkImage)
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;

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

    typename ImageType::Pointer itkOutImage = cropper->GetOutput();
    itk::Point<double,3> origin = itkOutImage->GetOrigin();
    origin[0] += lower[0]*itkOutImage->GetSpacing()[0];
    origin[1] += lower[1]*itkOutImage->GetSpacing()[1];
    origin[2] += lower[2]*itkOutImage->GetSpacing()[2];
    itkOutImage->SetOrigin(origin);
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk( itkOutImage.GetPointer() );
    image->SetVolume( itkOutImage->GetBufferPointer() );
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_cropped").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);

    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoApplySpacing()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( isDiffusionImage )
    {
        mitk::Vector3D spacing;
        spacing[0] = m_Controls->m_HeaderSpacingX->value();
        spacing[1] = m_Controls->m_HeaderSpacingY->value();
        spacing[2] = m_Controls->m_HeaderSpacingZ->value();

        mitk::Image::Pointer newImage = image->Clone();
        newImage->GetGeometry()->SetSpacing( spacing );
        mitk::DiffusionPropertyHelper propertyHelper( newImage );
        propertyHelper.InitializeImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( newImage );
        QString name = node->GetName().c_str();
        imageNode->SetName((name+"_newspacing").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, node);
        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if( image->GetPixelType().GetNumberOfComponents() )
    {
        AccessFixedDimensionByItk(image, TemplatedSetImageSpacing,3);
    }
    else
    {
        QMessageBox::warning(NULL,"Warning", QString("Operation not supported in multi-component images") );
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedSetImageSpacing( itk::Image<TPixel, VImageDimension>* itkImage)
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;

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
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_newspacing").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);
    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoApplyOrigin()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image));
    if ( isDiffusionImage )
    {
        mitk::Vector3D origin;
        origin[0] = m_Controls->m_HeaderOriginX->value();
        origin[1] = m_Controls->m_HeaderOriginY->value();
        origin[2] = m_Controls->m_HeaderOriginZ->value();

        mitk::Image::Pointer newImage = image->Clone();

        newImage->GetGeometry()->SetOrigin( origin );
        mitk::DiffusionPropertyHelper propertyHelper( newImage );
        propertyHelper.InitializeImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( newImage );
        QString name = node->GetName().c_str();
        imageNode->SetName((name+"_neworigin").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, node);
        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if( image->GetPixelType().GetNumberOfComponents() )
    {
        AccessFixedDimensionByItk(image, TemplatedSetImageOrigin,3);
    }
    else
    {
        QMessageBox::warning(NULL,"Warning", QString("Operation not supported in multi-component images") );
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedSetImageOrigin( itk::Image<TPixel, VImageDimension>* itkImage)
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;

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
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_neworigin").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);
    mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPreprocessingView::DoUpdateInterpolationGui(int i)
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

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

        mitk::BaseGeometry* geom = image->GetGeometry();
        m_Controls->m_ResampleDoubleX->setValue(geom->GetSpacing()[0]);
        m_Controls->m_ResampleDoubleY->setValue(geom->GetSpacing()[1]);
        m_Controls->m_ResampleDoubleZ->setValue(geom->GetSpacing()[2]);
        break;
    }
    case 2:
    {
        m_Controls->m_ResampleIntFrame->setVisible(true);
        m_Controls->m_ResampleDoubleFrame->setVisible(false);

        mitk::BaseGeometry* geom = image->GetGeometry();
        m_Controls->m_ResampleIntX->setValue(geom->GetExtent(0));
        m_Controls->m_ResampleIntY->setValue(geom->GetExtent(1));
        m_Controls->m_ResampleIntZ->setValue(geom->GetExtent(2));
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
}

void QmitkPreprocessingView::DoResampleImage()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( isDiffusionImage )
    {
        ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
        mitk::CastToItkImage(image, itkVectorImagePointer);

        typedef itk::ResampleDwiImageFilter< short > ResampleFilter;
        ResampleFilter::Pointer resampler = ResampleFilter::New();
        resampler->SetInput( itkVectorImagePointer );

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

        mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( resampler->GetOutput() );
        newImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() ) );
        newImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() ) );
        newImage->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );
        mitk::DiffusionPropertyHelper propertyHelper( newImage );
        propertyHelper.InitializeImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( newImage );
        QString name = node->GetName().c_str();

        imageNode->SetName((name+"_resampled_"+outAdd).toStdString().c_str());
        imageNode->SetVisibility(false);
        GetDefaultDataStorage()->Add(imageNode, node);
    }
    else if( image->GetPixelType().GetNumberOfComponents() )
    {
        AccessFixedDimensionByItk(image, TemplatedResampleImage,3);
    }
    else
    {
        QMessageBox::warning(NULL,"Warning", QString("Operation not supported in multi-component images") );
    }
}


template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedResampleImage( itk::Image<TPixel, VImageDimension>* itkImage)
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;

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
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_resampled_"+outAdd).toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);
}

void QmitkPreprocessingView::DoApplyDirectionMatrix()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( isDiffusionImage )
    {
        ItkDwiType::DirectionType newDirection;
        for (int r=0; r<3; r++)
        {
            for (int c=0; c<3; c++)
            {
                QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item(r,c);
                if (!item)
                    return;
                newDirection[r][c] = item->text().toDouble();
            }
        }
        ItkDwiType::Pointer itkDwi = ItkDwiType::New();
        mitk::CastToItkImage(image, itkDwi);

        itk::ImageDuplicator<ItkDwiType>::Pointer duplicator = itk::ImageDuplicator<ItkDwiType>::New();
        duplicator->SetInputImage(itkDwi);
        duplicator->Update();
        itkDwi = duplicator->GetOutput();

        vnl_matrix_fixed< double,3,3 > oldInverseDirection = itkDwi->GetDirection().GetInverse();
        mitk::GradientDirectionsProperty::GradientDirectionsContainerType::Pointer oldGradients = static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();
        mitk::GradientDirectionsProperty::GradientDirectionsContainerType::Pointer newGradients = mitk::GradientDirectionsProperty::GradientDirectionsContainerType::New();

        for (unsigned int i=0; i<oldGradients->Size(); i++)
        {
            mitk::GradientDirectionsProperty::GradientDirectionType g = oldGradients->GetElement(i);
            double mag = g.magnitude();
            g.normalize();
            mitk::GradientDirectionsProperty::GradientDirectionType newG = oldInverseDirection*g;
            newG = newDirection.GetVnlMatrix()*newG;
            newG.normalize();
            newGradients->InsertElement(i, newG*mag);
        }

        itkDwi->SetDirection(newDirection);
        mitk::Image::Pointer newDwi2 = mitk::GrabItkImageMemory( itkDwi.GetPointer() );
        newDwi2->SetProperty( mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( newGradients ) );
        newDwi2->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() ) );
        newDwi2->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );

        mitk::DiffusionPropertyHelper propertyHelper( newDwi2 );
        propertyHelper.InitializeImage();

        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( newDwi2 );
        QString name = node->GetName().c_str();
        imageNode->SetName((name+"_newdirection").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode, node);

        mitk::RenderingManager::GetInstance()->InitializeViews( imageNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else if( image->GetPixelType().GetNumberOfComponents() )
    {
        AccessFixedDimensionByItk(image, TemplatedApplyRotation,3);
    }
    else
    {
        QMessageBox::warning(NULL,"Warning", QString("Operation not supported in multi-component images") );
    }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedApplyRotation( itk::Image<TPixel, VImageDimension>* itkImage)
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;

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
    QString name = node->GetName().c_str();
    imageNode->SetName((name+"_newdirection").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);

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
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( ! isDiffusionImage )
        return;

    GradientDirectionContainerType::Pointer gradientContainer = static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

    int b0Index = -1;
    for (unsigned int i=0; i<gradientContainer->size(); i++)
    {
        GradientDirectionType g = gradientContainer->GetElement(i);
        if (g.magnitude()<0.001)
        {
            b0Index = i;
            break;
        }
    }
    if (b0Index==-1)
        return;

    typedef itk::DwiNormilzationFilter<short>  FilterType;

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkVectorImagePointer );
    filter->SetGradientDirections( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );

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
        dtFilter->SetGradientImage( gradientContainer, itkVectorImagePointer );
        dtFilter->SetBValue( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() );
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
                ref += itkVectorImagePointer->GetPixel(inIt.GetIndex())[b0Index];
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
        adcFilter->SetInput( itkVectorImagePointer );
        adcFilter->SetGradientDirections( gradientContainer);
        adcFilter->SetB_value( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() );
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
                ref += itkVectorImagePointer->GetPixel(inIt.GetIndex())[b0Index];
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

    mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );
    mitk::DiffusionPropertyHelper propertyHelper( newImage );
    propertyHelper.InitializeImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_normalized").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);
}

void QmitkPreprocessingView::DoLengthCorrection()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( ! isDiffusionImage )
        return;

    typedef itk::DwiGradientLengthCorrectionFilter  FilterType;

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    FilterType::Pointer filter = FilterType::New();
    filter->SetRoundingValue( m_Controls->m_B_ValueMap_Rounder_SpinBox->value());
    filter->SetReferenceBValue( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() );
    filter->SetReferenceGradientDirectionContainer( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );
    filter->Update();

    mitk::Image::Pointer newImage = mitk::ImportItkImage( itkVectorImagePointer );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( filter->GetOutputGradientDirectionContainer() ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( filter->GetNewBValue() ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );
    mitk::DiffusionPropertyHelper propertyHelper( newImage );
    propertyHelper.InitializeImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_rounded").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);
}

void QmitkPreprocessingView::UpdateDwiBValueMapRounder(int i)
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(node) );
    if ( ! isDiffusionImage )
        return;

    UpdateBValueTableWidget(i);
}

void QmitkPreprocessingView::CallMultishellToSingleShellFilter(itk::DWIVoxelFunctor * functor, mitk::Image::Pointer image, QString imageName, mitk::DataNode* parent)
{
    typedef itk::RadialMultishellToSingleshellImageFilter<DiffusionPixelType, DiffusionPixelType> FilterType;

    // filter input parameter
    const mitk::BValueMapProperty::BValueMap
            &originalShellMap  = static_cast<mitk::BValueMapProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);
    ItkDwiType
            *vectorImage       = itkVectorImagePointer.GetPointer();

    const mitk::GradientDirectionsProperty::GradientDirectionsContainerType::Pointer
            gradientContainer = static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

    const unsigned int
            &bValue            = static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue();

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
    mitk::Image::Pointer outImage = mitk::GrabItkImageMemory( filter->GetOutput() );
    outImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( filter->GetTargetGradientDirections() ) );
    outImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( m_Controls->m_targetBValueSpinBox->value() ) );
    outImage->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );
    mitk::DiffusionPropertyHelper propertyHelper( outImage );
    propertyHelper.InitializeImage();

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
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( ! isDiffusionImage )
        return;

    itk::BiExpFitFunctor::Pointer functor = itk::BiExpFitFunctor::New();

    QString name(node->GetName().c_str());

    const mitk::BValueMapProperty::BValueMap
            &originalShellMap  = static_cast<mitk::BValueMapProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();

    mitk::BValueMapProperty::BValueMap::const_iterator it = originalShellMap.begin();
    ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
    vnl_vector<double> bValueList(originalShellMap.size()-1);
    while(it != originalShellMap.end())
        bValueList.put(s++,(it++)->first);

    const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
    functor->setListOfBValues(bValueList);
    functor->setTargetBValue(targetBValue);
    CallMultishellToSingleShellFilter(functor,image,name + "_BiExp", node);
}

void QmitkPreprocessingView::DoAKCFit()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( ! isDiffusionImage )
        return;

    itk::KurtosisFitFunctor::Pointer functor = itk::KurtosisFitFunctor::New();

    QString name(node->GetName().c_str());

    const mitk::BValueMapProperty::BValueMap
            &originalShellMap  = static_cast<mitk::BValueMapProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();

    mitk::BValueMapProperty::BValueMap::const_iterator it = originalShellMap.begin();
    ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
    vnl_vector<double> bValueList(originalShellMap.size()-1);
    while(it != originalShellMap.end())
        bValueList.put(s++,(it++)->first);

    const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
    functor->setListOfBValues(bValueList);
    functor->setTargetBValue(targetBValue);
    CallMultishellToSingleShellFilter(functor,image,name + "_AKC", node);
}

void QmitkPreprocessingView::DoADCFit()
{
    // later
}

void QmitkPreprocessingView::DoADCAverage()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( ! isDiffusionImage )
        return;

    itk::ADCAverageFunctor::Pointer functor = itk::ADCAverageFunctor::New();

    QString name(node->GetName().c_str());

    const mitk::BValueMapProperty::BValueMap
            &originalShellMap  = static_cast<mitk::BValueMapProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();

    mitk::BValueMapProperty::BValueMap::const_iterator it = originalShellMap.begin();
    ++it;/* skip b=0*/ unsigned int s = 0; /*shell index */
    vnl_vector<double> bValueList(originalShellMap.size()-1);
    while(it != originalShellMap.end())
        bValueList.put(s++,(it++)->first);

    const double targetBValue = m_Controls->m_targetBValueSpinBox->value();
    functor->setListOfBValues(bValueList);
    functor->setTargetBValue(targetBValue);
    CallMultishellToSingleShellFilter(functor,image,name + "_ADC", node);
}

void QmitkPreprocessingView::DoAdcCalculation()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( ! isDiffusionImage )
        return;

    typedef itk::AdcImageFilter< DiffusionPixelType, double >     FilterType;

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkVectorImagePointer );
    filter->SetGradientDirections( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );
    filter->SetB_value( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() );
    filter->Update();

    mitk::Image::Pointer newImage = mitk::Image::New();
    newImage->InitializeByItk( filter->GetOutput() );
    newImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    QString name = node->GetName().c_str();

    imageNode->SetName((name+"_ADC").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);
}

void QmitkPreprocessingView::CleanBValueTableWidget()
{
  m_Controls->m_B_ValueMap_TableWidget->clear();
  m_Controls->m_B_ValueMap_TableWidget->setRowCount(1);
  QStringList headerList;
  headerList << "b-Value" << "Number of gradients";
  m_Controls->m_B_ValueMap_TableWidget->setHorizontalHeaderLabels(headerList);
  m_Controls->m_B_ValueMap_TableWidget->setItem(0,0,new QTableWidgetItem("-"));
  m_Controls->m_B_ValueMap_TableWidget->setItem(0,1,new QTableWidgetItem("-"));
}

void QmitkPreprocessingView::UpdateBValueTableWidget(int)
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();

    if (node.IsNull())
    {
        CleanBValueTableWidget();
        return;
    }

    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage(false);

    isDiffusionImage = mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image);

    if ( ! isDiffusionImage )
    {
        CleanBValueTableWidget();
    }
    else
    {
        typedef mitk::BValueMapProperty::BValueMap BValueMap;
        typedef mitk::BValueMapProperty::BValueMap::iterator BValueMapIterator;

        BValueMapIterator it;

        BValueMap roundedBValueMap = static_cast<mitk::BValueMapProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();

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

template < typename TPixel, unsigned int VImageDimension >
void QmitkPreprocessingView::TemplatedUpdateGui( itk::VectorImage<TPixel, VImageDimension>* itkImage)
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

void QmitkPreprocessingView::OnImageSelectionChanged()
{
    bool foundImageVolume = true;
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    bool foundDwiVolume( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( node ) );

    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
    bool multiComponentVolume = (image->GetPixelType().GetNumberOfComponents() > 1);

    bool foundSingleImageVolume = foundDwiVolume || (foundImageVolume && (!multiComponentVolume));

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
    m_Controls->m_DirectionMatrixTable->setEnabled(foundSingleImageVolume);
    m_Controls->m_ModifyDirection->setEnabled(foundSingleImageVolume);
    m_Controls->m_ExtractBrainMask->setEnabled(foundSingleImageVolume);
    m_Controls->m_ResampleImageButton->setEnabled(foundSingleImageVolume);
    m_Controls->m_ModifySpacingButton->setEnabled(foundSingleImageVolume);
    m_Controls->m_ModifyOriginButton->setEnabled(foundSingleImageVolume);
    m_Controls->m_CropImageButton->setEnabled(foundSingleImageVolume);
    m_Controls->m_RemoveGradientButton->setEnabled(foundDwiVolume);
    m_Controls->m_ExtractGradientButton->setEnabled(foundDwiVolume);
    m_Controls->m_FlipAxis->setEnabled(foundSingleImageVolume);

    // reset sampling frame to 1 and update all ealted components
    m_Controls->m_B_ValueMap_Rounder_SpinBox->setValue(1);

    UpdateBValueTableWidget(m_Controls->m_B_ValueMap_Rounder_SpinBox->value());
    DoUpdateInterpolationGui(m_Controls->m_ResampleTypeBox->currentIndex());


    if (foundDwiVolume)
    {
        m_Controls->m_InputData->setTitle("Input Data");
        vnl_matrix_fixed< double, 3, 3 > mf = static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame();
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
                    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
                    mitk::CastToItkImage(image, itkVectorImagePointer);

                    QTableWidgetItem* item = m_Controls->m_DirectionMatrixTable->item(r,c);
                    delete item;
                    item = new QTableWidgetItem();
                    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                    item->setText(QString::number(itkVectorImagePointer->GetDirection()[r][c]));
                    m_Controls->m_DirectionMatrixTable->setItem(r,c,item);
                }
            }

        //calculate target bValue for MultishellToSingleShellfilter
        const mitk::BValueMapProperty::BValueMap & bValMap = static_cast<mitk::BValueMapProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();
        mitk::BValueMapProperty::BValueMap::const_iterator it = bValMap.begin();
        unsigned int targetBVal = 0;
        while(it != bValMap.end())
            targetBVal += (it++)->first;
        targetBVal /= (float)bValMap.size()-1;
        m_Controls->m_targetBValueSpinBox->setValue(targetBVal);

        m_Controls->m_HeaderSpacingX->setValue(image->GetGeometry()->GetSpacing()[0]);
        m_Controls->m_HeaderSpacingY->setValue(image->GetGeometry()->GetSpacing()[1]);
        m_Controls->m_HeaderSpacingZ->setValue(image->GetGeometry()->GetSpacing()[2]);
        m_Controls->m_HeaderOriginX->setValue(image->GetGeometry()->GetOrigin()[0]);
        m_Controls->m_HeaderOriginY->setValue(image->GetGeometry()->GetOrigin()[1]);
        m_Controls->m_HeaderOriginZ->setValue(image->GetGeometry()->GetOrigin()[2]);
        m_Controls->m_XstartBox->setMaximum(image->GetGeometry()->GetExtent(0)-1);
        m_Controls->m_YstartBox->setMaximum(image->GetGeometry()->GetExtent(1)-1);
        m_Controls->m_ZstartBox->setMaximum(image->GetGeometry()->GetExtent(2)-1);
        m_Controls->m_XendBox->setMaximum(image->GetGeometry()->GetExtent(0)-1);
        m_Controls->m_YendBox->setMaximum(image->GetGeometry()->GetExtent(1)-1);
        m_Controls->m_ZendBox->setMaximum(image->GetGeometry()->GetExtent(2)-1);
        m_Controls->m_RemoveGradientBox->setMaximum(static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer()->Size()-1);
        m_Controls->m_ExtractGradientBox->setMaximum(static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer()->Size()-1);
    }
    else if (foundSingleImageVolume)
    {
        for (int r=0; r<3; r++)
            for (int c=0; c<3; c++)
            {
                QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
                delete item;
                item = new QTableWidgetItem();
                m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
            }

        m_Controls->m_HeaderSpacingX->setValue(image->GetGeometry()->GetSpacing()[0]);
        m_Controls->m_HeaderSpacingY->setValue(image->GetGeometry()->GetSpacing()[1]);
        m_Controls->m_HeaderSpacingZ->setValue(image->GetGeometry()->GetSpacing()[2]);
        m_Controls->m_HeaderOriginX->setValue(image->GetGeometry()->GetOrigin()[0]);
        m_Controls->m_HeaderOriginY->setValue(image->GetGeometry()->GetOrigin()[1]);
        m_Controls->m_HeaderOriginZ->setValue(image->GetGeometry()->GetOrigin()[2]);
        m_Controls->m_XstartBox->setMaximum(image->GetGeometry()->GetExtent(0)-1);
        m_Controls->m_YstartBox->setMaximum(image->GetGeometry()->GetExtent(1)-1);
        m_Controls->m_ZstartBox->setMaximum(image->GetGeometry()->GetExtent(2)-1);
        m_Controls->m_XendBox->setMaximum(image->GetGeometry()->GetExtent(0)-1);
        m_Controls->m_YendBox->setMaximum(image->GetGeometry()->GetExtent(1)-1);
        m_Controls->m_ZendBox->setMaximum(image->GetGeometry()->GetExtent(2)-1);

        AccessFixedDimensionByItk(image, TemplatedUpdateGui,3);
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
    }
}

void QmitkPreprocessingView::Visible()
{
    QmitkFunctionality::Visible();
    OnImageSelectionChanged();
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
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    mitk::Image::Pointer newDwi = image->Clone();
    GradientDirectionContainerType::Pointer gradientContainer =   static_cast<mitk::GradientDirectionsProperty*>( newDwi->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

    for (unsigned int j=0; j<gradientContainer->Size(); j++)
        if (gradientContainer->at(j)[0]<0)
            gradientContainer->at(j) = -gradientContainer->at(j);

    newDwi->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( gradientContainer ) );
    mitk::DiffusionPropertyHelper propertyHelper( newDwi );
    propertyHelper.InitializeImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newDwi );
    QString name = node->GetName().c_str();
    imageNode->SetName((name+"_halfsphere").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);
}

void QmitkPreprocessingView::DoApplyMesurementFrame()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( !isDiffusionImage )
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

    mitk::Image::Pointer newDwi = image->Clone();
    newDwi->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( mf ) );
    mitk::DiffusionPropertyHelper propertyHelper( newDwi );
    propertyHelper.InitializeImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newDwi );
    QString name = node->GetName().c_str();
    imageNode->SetName((name+"_new-MF").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);
}

void QmitkPreprocessingView::DoShowGradientDirections()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( !isDiffusionImage )
        return;

    int maxIndex = 0;
    unsigned int maxSize = image->GetDimension(0);
    if (maxSize<image->GetDimension(1))
    {
        maxSize = image->GetDimension(1);
        maxIndex = 1;
    }
    if (maxSize<image->GetDimension(2))
    {
        maxSize = image->GetDimension(2);
        maxIndex = 2;
    }

    mitk::Point3D origin = image->GetGeometry()->GetOrigin();
    mitk::PointSet::Pointer originSet = mitk::PointSet::New();

    typedef mitk::BValueMapProperty::BValueMap BValueMap;
    typedef mitk::BValueMapProperty::BValueMap::iterator BValueMapIterator;
    BValueMap bValMap =  static_cast<mitk::BValueMapProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();

    GradientDirectionContainerType::Pointer gradientContainer = static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();
    mitk::BaseGeometry::Pointer geometry = image->GetGeometry();
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
                ip[0] = v[0]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[0]-0.5*geometry->GetSpacing()[0] + geometry->GetSpacing()[0]*image->GetDimension(0)/2;
                ip[1] = v[1]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[1]-0.5*geometry->GetSpacing()[1] + geometry->GetSpacing()[1]*image->GetDimension(1)/2;
                ip[2] = v[2]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[2]-0.5*geometry->GetSpacing()[2] + geometry->GetSpacing()[2]*image->GetDimension(2)/2;

                pointset->InsertPoint(j, ip);
            }
            else if (originSet->IsEmpty())
            {
                ip[0] = v[0]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[0]-0.5*geometry->GetSpacing()[0] + geometry->GetSpacing()[0]*image->GetDimension(0)/2;
                ip[1] = v[1]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[1]-0.5*geometry->GetSpacing()[1] + geometry->GetSpacing()[1]*image->GetDimension(1)/2;
                ip[2] = v[2]*maxSize*geometry->GetSpacing()[maxIndex]/2 + origin[2]-0.5*geometry->GetSpacing()[2] + geometry->GetSpacing()[2]*image->GetDimension(2)/2;

                originSet->InsertPoint(j, ip);
            }
        }
        if (it->first<mitk::eps)
            continue;

        // add shell to datastorage
        mitk::DataNode::Pointer newNode = mitk::DataNode::New();
        newNode->SetData(pointset);
        QString name = node->GetName().c_str();
        name += "_Shell_";
        name += QString::number(it->first);
        newNode->SetName(name.toStdString().c_str());
        newNode->SetProperty("pointsize", mitk::FloatProperty::New((float)maxSize/50));
        int b0 = shellCount%2;
        int b1 = 0;
        int b2 = 0;
        if (shellCount>4)
            b2 = 1;
        if (shellCount%4 >= 2)
            b1 = 1;

        newNode->SetProperty("color", mitk::ColorProperty::New(b2, b1, b0));
        GetDefaultDataStorage()->Add(newNode, node);
        shellCount++;
    }

    // add origin to datastorage
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    newNode->SetData(originSet);
    QString name = node->GetName().c_str();
    name += "_Origin";
    newNode->SetName(name.toStdString().c_str());
    newNode->SetProperty("pointsize", mitk::FloatProperty::New((float)maxSize/50));
    newNode->SetProperty("color", mitk::ColorProperty::New(1,1,1));
    GetDefaultDataStorage()->Add(newNode, node);
}

void QmitkPreprocessingView::DoReduceGradientDirections()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( !isDiffusionImage )
        return;

    typedef itk::ElectrostaticRepulsionDiffusionGradientReductionFilter<DiffusionPixelType, DiffusionPixelType> FilterType;
    typedef mitk::BValueMapProperty::BValueMap BValueMap;

    // GetShellSelection from GUI
    BValueMap shellSlectionMap;
    BValueMap originalShellMap = static_cast<mitk::BValueMapProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();
    std::vector<unsigned int> newNumGradientDirections;
    int shellCounter = 0;

    QString name = node->GetName().c_str();
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

    GradientDirectionContainerType::Pointer gradientContainer = static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkVectorImagePointer );
    filter->SetOriginalGradientDirections(gradientContainer);
    filter->SetNumGradientDirections(newNumGradientDirections);
    filter->SetOriginalBValueMap(originalShellMap);
    filter->SetShellSelectionBValueMap(shellSlectionMap);
    filter->Update();

    mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( filter->GetGradientDirections() ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( static_cast<mitk::MeasurementFrameProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer() )->GetMeasurementFrame() ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() ) );
    mitk::DiffusionPropertyHelper propertyHelper( newImage );
    propertyHelper.InitializeImage();


    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );

    imageNode->SetName(name.toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);

    // update the b-value widget to remove the modified number of gradients used for extraction
    this->CleanBValueTableWidget();
    this->UpdateBValueTableWidget(0);
}

void QmitkPreprocessingView::MergeDwis()
{
    typedef mitk::GradientDirectionsProperty::GradientDirectionsContainerType    GradientContainerType;

    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( !isDiffusionImage )
        return;

    mitk::DataNode::Pointer node2 = m_Controls->m_MergeDwiBox->GetSelectedNode();
    if (node2.IsNull())
        return;
    mitk::Image::Pointer image2 = dynamic_cast<mitk::Image*>(node2->GetData());

    typedef itk::VectorImage<DiffusionPixelType,3>                  DwiImageType;
    typedef DwiImageType::PixelType                        DwiPixelType;
    typedef DwiImageType::RegionType                       DwiRegionType;
    typedef std::vector< DwiImageType::Pointer >  DwiImageContainerType;

    typedef std::vector< GradientContainerType::Pointer >  GradientListContainerType;

    DwiImageContainerType       imageContainer;
    GradientListContainerType   gradientListContainer;
    std::vector< double >       bValueContainer;

    QString name = "";
    {
        ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
        mitk::CastToItkImage(image, itkVectorImagePointer);

        imageContainer.push_back( itkVectorImagePointer );
        gradientListContainer.push_back( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );
        bValueContainer.push_back( static_cast<mitk::FloatProperty*>(image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() );
        name += node->GetName().c_str();
    }

    {
        ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
        mitk::CastToItkImage(image2, itkVectorImagePointer);

        imageContainer.push_back( itkVectorImagePointer );
        gradientListContainer.push_back( static_cast<mitk::GradientDirectionsProperty*>( image2->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );
        bValueContainer.push_back( static_cast<mitk::FloatProperty*>(image2->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() );
        name += "+";
        name += node2->GetName().c_str();
    }

    typedef itk::MergeDiffusionImagesFilter<short> FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetImageVolumes(imageContainer);
    filter->SetGradientLists(gradientListContainer);
    filter->SetBValues(bValueContainer);
    filter->Update();

    vnl_matrix_fixed< double, 3, 3 > mf; mf.set_identity();
    mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );

    newImage->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( filter->GetOutputGradients() ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( mf ) );
    newImage->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( filter->GetB_Value() ) );
    mitk::DiffusionPropertyHelper propertyHelper( newImage );
    propertyHelper.InitializeImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newImage );
    imageNode->SetName(name.toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode);
}

void QmitkPreprocessingView::ExtractB0()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( !isDiffusionImage )
        return;

    typedef mitk::GradientDirectionsProperty::GradientDirectionsContainerType    GradientContainerType;

    // call the extraction withou averaging if the check-box is checked
    if( this->m_Controls->m_CheckExtractAll->isChecked() )
    {
        DoExtractBOWithoutAveraging();
        return;
    }

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    // Extract image using found index
    typedef itk::B0ImageExtractionImageFilter<short, short> FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkVectorImagePointer );
    filter->SetDirections( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );
    filter->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( filter->GetOutput() );
    mitkImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer newNode=mitk::DataNode::New();
    newNode->SetData( mitkImage );
    newNode->SetProperty( "name", mitk::StringProperty::New(node->GetName() + "_B0"));

    GetDefaultDataStorage()->Add(newNode, node);
}

void QmitkPreprocessingView::DoExtractBOWithoutAveraging()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( !isDiffusionImage )
        return;

    // typedefs
    typedef mitk::GradientDirectionsProperty::GradientDirectionsContainerType    GradientContainerType;
    typedef itk::B0ImageExtractionToSeparateImageFilter< short, short> FilterType;

    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(image, itkVectorImagePointer);

    // Extract image using found index
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkVectorImagePointer );
    filter->SetDirections( static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );
    filter->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( filter->GetOutput() );
    mitkImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer newNode=mitk::DataNode::New();
    newNode->SetData( mitkImage );
    newNode->SetProperty( "name", mitk::StringProperty::New(node->GetName() + "_B0_ALL"));

    GetDefaultDataStorage()->Add(newNode, node);

    /*A reinitialization is needed to access the time channels via the ImageNavigationController
    The Global-Geometry can not recognize the time channel without a re-init.
    (for a new selection in datamanger a automatically updated of the Global-Geometry should be done - if it contains the time channel)*/
    mitk::RenderingManager::GetInstance()->InitializeViews(newNode->GetData()->GetTimeGeometry(),mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
}

void QmitkPreprocessingView::AverageGradients()
{
    mitk::DataNode::Pointer node = m_Controls->m_SelctedImageComboBox->GetSelectedNode();
    if (node.IsNull())
        return;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

    bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
    if ( !isDiffusionImage )
        return;

    mitk::Image::Pointer newDwi = image->Clone();
    newDwi->SetPropertyList(image->GetPropertyList()->Clone());

    mitk::DiffusionPropertyHelper propertyHelper(newDwi);
    propertyHelper.AverageRedundantGradients(m_Controls->m_Blur->value());
    propertyHelper.InitializeImage();

    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( newDwi );
    QString name = node->GetName().c_str();
    imageNode->SetName((name+"_averaged").toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode, node);
}
