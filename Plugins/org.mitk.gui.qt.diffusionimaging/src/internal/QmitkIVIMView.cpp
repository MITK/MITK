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
#include "QmitkIVIMView.h"
#include "QmitkStdMultiWidget.h"

// qt
#include "qmessagebox.h"
#include "qclipboard.h"

// mitk
#include "mitkImage.h"
#include "mitkImageCast.h"

#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"

// itk
#include "itkScalarImageToHistogramGenerator.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"

// itk/mitk
#include "itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter.h"
#include "itkRegularizedIVIMReconstructionFilter.h"
#include "mitkImageCast.h"

const std::string QmitkIVIMView::VIEW_ID = "org.mitk.views.ivim";

QmitkIVIMView::QmitkIVIMView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
    , m_SliceObserverTag1(0), m_SliceObserverTag2(0), m_SliceObserverTag3(0)
    , m_DiffusionImageNode(NULL)
    , m_MaskImageNode(NULL)
    , m_Active(false)
    , m_HoldUpdate(false)
{
}

QmitkIVIMView::~QmitkIVIMView()
{
}

void QmitkIVIMView::CreateQtPartControl( QWidget *parent )
{
  // hold update untill all elements are set
  this->m_HoldUpdate = true;

    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkIVIMViewControls;
        m_Controls->setupUi( parent );

        connect( m_Controls->m_ButtonStart, SIGNAL(clicked()), this, SLOT(FittIVIMStart()) );
        connect( m_Controls->m_ButtonAutoThres, SIGNAL(clicked()), this, SLOT(AutoThreshold()) );

        connect( m_Controls->m_MethodCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(MethodCombo(int)) );

        connect( m_Controls->m_DStarSlider, SIGNAL(valueChanged(int)), this, SLOT(DStarSlider(int)) );
        connect( m_Controls->m_BThreshSlider, SIGNAL(valueChanged(int)), this, SLOT(BThreshSlider(int)) );
        connect( m_Controls->m_S0ThreshSlider, SIGNAL(valueChanged(int)), this, SLOT(S0ThreshSlider(int)) );
        connect( m_Controls->m_NumItSlider, SIGNAL(valueChanged(int)), this, SLOT(NumItsSlider(int)) );
        connect( m_Controls->m_LambdaSlider, SIGNAL(valueChanged(int)), this, SLOT(LambdaSlider(int)) );

        connect( m_Controls->m_CheckDStar, SIGNAL(clicked()), this, SLOT(Checkbox()) );
        connect( m_Controls->m_CheckD, SIGNAL(clicked()), this, SLOT(Checkbox()) );
        connect( m_Controls->m_Checkf, SIGNAL(clicked()), this, SLOT(Checkbox()) );

        connect( m_Controls->m_ChooseMethod, SIGNAL(clicked()), this, SLOT(ChooseMethod()) );
        connect( m_Controls->m_CurveClipboard, SIGNAL(clicked()), this, SLOT(ClipboardCurveButtonClicked()) );
        connect( m_Controls->m_ValuesClipboard, SIGNAL(clicked()), this, SLOT(ClipboardStatisticsButtonClicked()) );

        // connect all kurtosis actions to a recompute
        connect( m_Controls->m_KurtosisRangeWidget, SIGNAL( rangeChanged(double, double)), this, SLOT(OnKurtosisParamsChanged() ) );
        //connect( m_Controls->m_MaximalBValueWidget, SIGNAL( valueChanged(double)), this, SLOT( OnKurtosisParamsChanged() ) );
        connect( m_Controls->m_OmitBZeroCB, SIGNAL( stateChanged(int) ), this, SLOT( OnKurtosisParamsChanged() ) );
        connect( m_Controls->m_KurtosisFitScale, SIGNAL( currentIndexChanged(int)), this, SLOT( OnKurtosisParamsChanged() ) );
        connect( m_Controls->m_UseKurtosisBoundsCB, SIGNAL(clicked() ), this, SLOT( OnKurtosisParamsChanged() ) );
    }

    QString dstar = QString::number(m_Controls->m_DStarSlider->value()/1000.0);
    m_Controls->m_DStarLabel->setText(dstar);

    QString bthresh = QString::number(m_Controls->m_BThreshSlider->value()*5.0);
    m_Controls->m_BThreshLabel->setText(bthresh);

    QString s0thresh = QString::number(m_Controls->m_S0ThreshSlider->value()*0.5);
    m_Controls->m_S0ThreshLabel->setText(s0thresh);

    QString numits = QString::number(m_Controls->m_NumItSlider->value());
    m_Controls->m_NumItsLabel->setText(numits);

    QString lambda = QString::number(m_Controls->m_LambdaSlider->value()*.00001);
    m_Controls->m_LambdaLabel->setText(lambda);

    m_Controls->m_MethodCombo->setVisible(m_Controls->m_ChooseMethod->isChecked());
    m_Controls->m_Warning->setVisible(false);

    MethodCombo(m_Controls->m_MethodCombo->currentIndex());

    m_Controls->m_KurtosisRangeWidget->setSingleStep(0.1);
    m_Controls->m_KurtosisRangeWidget->setRange( 0.0, 10.0 );
    m_Controls->m_KurtosisRangeWidget->setMaximumValue( 5.0 );

    // LogScale not working yet, have to fix that first
    // m_Controls->m_KurtosisFitScale->setEnabled(false);


    //m_Controls->m_MaximalBValueWidget->setVisible( false );

    // release update block after the UI-elements were all set
    this->m_HoldUpdate = false;

}

void QmitkIVIMView::Checkbox()
{
    itk::StartEvent dummy;
    OnSliceChanged(dummy);
}

void QmitkIVIMView::MethodCombo(int val)
{
    switch(val)
    {
    case 0:
        m_Controls->m_DstarFrame->setVisible(false);
        m_Controls->m_NeglSiFrame->setVisible(true);
        m_Controls->m_NeglBframe->setVisible(false);
        m_Controls->m_IterationsFrame->setVisible(false);
        m_Controls->m_LambdaFrame->setVisible(false);
        break;
    case 1:
        m_Controls->m_DstarFrame->setVisible(true);
        m_Controls->m_NeglSiFrame->setVisible(true);
        m_Controls->m_NeglBframe->setVisible(false);
        m_Controls->m_IterationsFrame->setVisible(false);
        m_Controls->m_LambdaFrame->setVisible(false);
        break;
    case 2:
        m_Controls->m_DstarFrame->setVisible(false);
        m_Controls->m_NeglSiFrame->setVisible(true);
        m_Controls->m_NeglBframe->setVisible(true);
        m_Controls->m_IterationsFrame->setVisible(false);
        m_Controls->m_LambdaFrame->setVisible(false);
        break;
    case 3:
        m_Controls->m_DstarFrame->setVisible(false);
        m_Controls->m_NeglSiFrame->setVisible(true);
        m_Controls->m_NeglBframe->setVisible(true);
        m_Controls->m_IterationsFrame->setVisible(false);
        m_Controls->m_LambdaFrame->setVisible(false);
        break;
    case 4:
        m_Controls->m_DstarFrame->setVisible(false);
        m_Controls->m_NeglSiFrame->setVisible(false);
        m_Controls->m_NeglBframe->setVisible(false);
        m_Controls->m_IterationsFrame->setVisible(false);
        m_Controls->m_LambdaFrame->setVisible(false);
        break;
    }

    itk::StartEvent dummy;
    OnSliceChanged(dummy);
}

void QmitkIVIMView::DStarSlider (int val)
{
    QString sval = QString::number(val/1000.0);
    m_Controls->m_DStarLabel->setText(sval);

    itk::StartEvent dummy;
    OnSliceChanged(dummy);
}

void QmitkIVIMView::BThreshSlider (int val)
{
    QString sval = QString::number(val*5.0);
    m_Controls->m_BThreshLabel->setText(sval);

    itk::StartEvent dummy;
    OnSliceChanged(dummy);
}

void QmitkIVIMView::S0ThreshSlider (int val)
{
    QString sval = QString::number(val*0.5);
    m_Controls->m_S0ThreshLabel->setText(sval);

    itk::StartEvent dummy;
    OnSliceChanged(dummy);
}

void QmitkIVIMView::NumItsSlider (int val)
{
    QString sval = QString::number(val);
    m_Controls->m_NumItsLabel->setText(sval);

    itk::StartEvent dummy;
    OnSliceChanged(dummy);
}

void QmitkIVIMView::LambdaSlider (int val)
{
    QString sval = QString::number(val*.00001);
    m_Controls->m_LambdaLabel->setText(sval);

    itk::StartEvent dummy;
    OnSliceChanged(dummy);
}

void QmitkIVIMView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;

    {
        mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
        itk::ReceptorMemberCommand<QmitkIVIMView>::Pointer command = itk::ReceptorMemberCommand<QmitkIVIMView>::New();
        command->SetCallbackFunction( this, &QmitkIVIMView::OnSliceChanged );
        m_SliceObserverTag1 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
    }

    {
        mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
        itk::ReceptorMemberCommand<QmitkIVIMView>::Pointer command = itk::ReceptorMemberCommand<QmitkIVIMView>::New();
        command->SetCallbackFunction( this, &QmitkIVIMView::OnSliceChanged );
        m_SliceObserverTag2 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
    }

    {
        mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
        itk::ReceptorMemberCommand<QmitkIVIMView>::Pointer command = itk::ReceptorMemberCommand<QmitkIVIMView>::New();
        command->SetCallbackFunction( this, &QmitkIVIMView::OnSliceChanged );
        m_SliceObserverTag3 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
    }

}

void QmitkIVIMView::StdMultiWidgetNotAvailable()
{

    {
        mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
        slicer->RemoveObserver( m_SliceObserverTag1 );
    }

    {
        mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
        slicer->RemoveObserver( m_SliceObserverTag2 );
    }

    {
        mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
        slicer->RemoveObserver( m_SliceObserverTag3 );
    }

    m_MultiWidget = NULL;
}

void QmitkIVIMView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    bool foundOneDiffusionImage = false;
    m_Controls->m_InputData->setTitle("Please Select Input Data");
    m_Controls->m_DiffusionImageLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_MaskImageLabel->setText("<font color='grey'>optional</font>");
    m_MaskImageNode = NULL;
    m_DiffusionImageNode = NULL;

    // iterate all selected objects, adjust warning visibility
    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;

        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
          bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image *>(node->GetData())) );

            if( isDiffusionImage )
            {
                m_DiffusionImageNode = node;
                foundOneDiffusionImage = true;
                m_Controls->m_DiffusionImageLabel->setText(node->GetName().c_str());
            }
            else
            {
                bool isBinary = false;
                node->GetPropertyValue<bool>("binary", isBinary);
                if (isBinary)
                {
                    m_MaskImageNode = node;
                    m_Controls->m_MaskImageLabel->setText(node->GetName().c_str());
                }
            }
        }
    }

    if (m_DiffusionImageNode.IsNotNull())
    {
        m_Controls->m_VisualizeResultsWidget->setVisible(true);
        m_Controls->m_KurtosisVisualizationWidget->setVisible(true);
        m_Controls->m_InputData->setTitle("Input Data");

        m_HoldUpdate = false;
    }
    else
    {
        m_Controls->m_VisualizeResultsWidget->setVisible(false);
        m_Controls->m_KurtosisVisualizationWidget->setVisible(false);
        m_Controls->m_DiffusionImageLabel->setText("<font color='red'>mandatory</font>");
    }

    m_Controls->m_ButtonStart->setEnabled( foundOneDiffusionImage );
    m_Controls->m_ButtonAutoThres->setEnabled( foundOneDiffusionImage );

    m_Controls->m_ControlsFrame->setEnabled( foundOneDiffusionImage );
    m_Controls->m_BottomControlsFrame->setEnabled( foundOneDiffusionImage );

    itk::StartEvent dummy;
    OnSliceChanged(dummy);
}

void QmitkIVIMView::AutoThreshold()
{
    std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
    if (nodes.empty()) return;

    if (!nodes.front())
    {
        // Nothing selected. Inform the user and return
        QMessageBox::information( NULL, "Template", "Please load and select a diffusion image before starting image processing.");
        return;
    }

    mitk::Image* dimg = dynamic_cast<mitk::Image*>(nodes.front()->GetData());

    if (!dimg)
    {
        // Nothing selected. Inform the user and return
        QMessageBox::information( NULL, "Template", "No valid diffusion image was found.");
        return;
    }

    // find bzero index
    int index = -1;
    DirContainerType::Pointer directions = static_cast<mitk::GradientDirectionsProperty*>( dimg->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();
    for(DirContainerType::ConstIterator it = directions->Begin();
        it != directions->End(); ++it)
    {
        index++;
        GradientDirectionType g = it.Value();
        if(g[0] == 0 && g[1] == 0 && g[2] == 0 )
            break;
    }

    VecImgType::Pointer vecimg = VecImgType::New();
    mitk::CastToItkImage(dimg, vecimg);

    int vecLength = vecimg->GetVectorLength();
    index = index > vecLength-1 ? vecLength-1 : index;

    MITK_INFO << "Performing Histogram Analysis on Channel" << index;

    typedef itk::Image<short,3> ImgType;
    ImgType::Pointer img = ImgType::New();
    mitk::CastToItkImage(dimg, img);

    itk::ImageRegionIterator<ImgType> itw (img, img->GetLargestPossibleRegion() );
    itw.GoToBegin();

    itk::ImageRegionConstIterator<VecImgType> itr (vecimg, vecimg->GetLargestPossibleRegion() );
    itr.GoToBegin();

    while(!itr.IsAtEnd())
    {
        itw.Set(itr.Get().GetElement(index));
        ++itr;
        ++itw;
    }

    typedef itk::Statistics::ScalarImageToHistogramGenerator< ImgType >
            HistogramGeneratorType;
    typedef HistogramGeneratorType::HistogramType HistogramType;

    HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();
    histogramGenerator->SetInput( img );
    histogramGenerator->SetMarginalScale( 10 ); // Defines y-margin width of histogram
    histogramGenerator->SetNumberOfBins( 100 ); // CT range [-1024, +2048] --> bin size 4 values
    histogramGenerator->SetHistogramMin(  dimg->GetScalarValueMin() );
    histogramGenerator->SetHistogramMax(  dimg->GetScalarValueMax() * .5 );
    histogramGenerator->Compute();

    HistogramType::ConstIterator iter = histogramGenerator->GetOutput()->Begin();
    float maxFreq = 0;
    float maxValue = 0;
    while ( iter != histogramGenerator->GetOutput()->End() )
    {
        if(iter.GetFrequency() > maxFreq)
        {
            maxFreq = iter.GetFrequency();
            maxValue = iter.GetMeasurementVector()[0];
        }
        ++iter;
    }

    maxValue *= 2;

    int sliderPos = maxValue * 2;
    m_Controls->m_S0ThreshSlider->setValue(sliderPos);
    S0ThreshSlider(sliderPos);
}

void QmitkIVIMView::FittIVIMStart()
{
    std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();

    mitk::Image* img = 0;
    for ( unsigned int i=0; i<nodes.size(); i++ )
    {
        img = dynamic_cast<mitk::Image*>(nodes.at(i)->GetData());

        bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( dynamic_cast<mitk::Image *>(nodes.at(i)->GetData())) );

        if (img && isDiffusionImage)
            break;
    }

    if (!img)
    {
        QMessageBox::information( NULL, "Template", "No valid diffusion image was found.");
        return;
    }


    VecImgType::Pointer vecimg = VecImgType::New();
    mitk::CastToItkImage(img, vecimg);

    OutImgType::IndexType dummy;

    if( m_Controls->m_ModelTabSelectionWidget->currentIndex() )
    {
      // KURTOSIS
      KurtosisFilterType::Pointer filter = KurtosisFilterType::New();
      filter->SetInput(vecimg);
      filter->SetReferenceBValue( static_cast<mitk::FloatProperty*>(img->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue() );
      filter->SetGradientDirections( static_cast<mitk::GradientDirectionsProperty*>( img->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer() );
      filter->SetSmoothingSigma( this->m_Controls->m_SigmaSpinBox->value() );

      if( this->m_Controls->m_UseKurtosisBoundsCB->isChecked() )
        filter->SetBoundariesForKurtosis( this->m_Controls->m_KurtosisRangeWidget->minimumValue(), this->m_Controls->m_KurtosisRangeWidget->maximumValue() );

      filter->SetFittingScale( static_cast<itk::FitScale>(this->m_Controls->m_KurtosisFitScale->currentIndex() ) );

      if( m_MaskImageNode.IsNotNull() )
      {
        mitk::Image::Pointer maskImg = dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData());
        typedef itk::Image<short, 3> MaskImgType;

        MaskImgType::Pointer maskItk;
        CastToItkImage( maskImg, maskItk );

        filter->SetImageMask( maskItk );
      }

      filter->Update();

      mitk::LookupTable::Pointer kurt_map_lut = mitk::LookupTable::New();
      kurt_map_lut->SetType( mitk::LookupTable::JET_TRANSPARENT );
      mitk::LookupTableProperty::Pointer kurt_lut_prop =
          mitk::LookupTableProperty::New();
      kurt_lut_prop->SetLookupTable( kurt_map_lut );

      mitk::Image::Pointer dimage = mitk::Image::New();
      dimage->InitializeByItk( filter->GetOutput(0) );
      dimage->SetVolume( filter->GetOutput(0)->GetBufferPointer());

      mitk::Image::Pointer kimage = mitk::Image::New();
      kimage->InitializeByItk( filter->GetOutput(1) );
      kimage->SetVolume( filter->GetOutput(1)->GetBufferPointer());

      QString basename(nodes.front()->GetName().c_str());
      QString new_dname = basename; new_dname = new_dname.append("KurtFit_DMap");

      QString new_kname = basename; new_kname = new_kname.append("KurtFit_KMap");

      if( this->m_Controls->m_CheckKurtD->isChecked() )
      {
        mitk::DataNode::Pointer dnode = mitk::DataNode::New();
        dnode->SetData( dimage );
        dnode->SetName(new_dname.toLatin1());
        dnode->SetProperty("LookupTable", kurt_lut_prop );
        GetDefaultDataStorage()->Add(dnode);
      }

      if( this->m_Controls->m_CheckKurtK->isChecked() )
      {
        mitk::DataNode::Pointer knode = mitk::DataNode::New();
        knode->SetData( kimage );
        knode->SetName(new_kname.toLatin1());
        knode->SetProperty("LookupTable", kurt_lut_prop );
        GetDefaultDataStorage()->Add(knode);
      }

    }
    else
    {
      FittIVIM(vecimg,
             static_cast<mitk::GradientDirectionsProperty*>( img->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer(),
             static_cast<mitk::FloatProperty*>(img->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue(),
             true,
             dummy);

      OutputToDatastorage(nodes);
    }
}

void QmitkIVIMView::OnKurtosisParamsChanged()
{
  itk::StartEvent dummy;
  OnSliceChanged(dummy);
}

void QmitkIVIMView::OnSliceChanged(const itk::EventObject& /*e*/)
{
    if(!m_Visible)
        return;

    if(m_HoldUpdate)
      return;

    m_Controls->m_Warning->setVisible(false);
    if(!m_Controls || m_DiffusionImageNode.IsNull())
        return;

    m_Controls->m_VisualizeResultsWidget->setVisible(false);
    m_Controls->m_KurtosisVisualizationWidget->setVisible(false);

    mitk::Image::Pointer diffusionImg = dynamic_cast<mitk::Image*>(m_DiffusionImageNode->GetData());
    mitk::Image::Pointer maskImg = NULL;
    if (m_MaskImageNode.IsNotNull())
        maskImg = dynamic_cast<mitk::Image*>(m_MaskImageNode->GetData());

    if (!m_MultiWidget) return;

    VecImgType::Pointer vecimg = VecImgType::New();
    mitk::CastToItkImage(diffusionImg, vecimg);

    VecImgType::Pointer roiImage = VecImgType::New();

    bool success = false;
    if(maskImg.IsNull())
    {
        int roisize = 0;
        if(m_Controls->m_MethodCombo->currentIndex() == 4)
            roisize = 5;

        mitk::Point3D pos = m_MultiWidget->GetCrossPosition();

        VecImgType::IndexType crosspos;
        diffusionImg->GetGeometry()->WorldToIndex(pos, crosspos);
        if (!vecimg->GetLargestPossibleRegion().IsInside(crosspos))
        {
            m_Controls->m_Warning->setText(QString("Crosshair position not inside of selected diffusion weighted image. Reinit needed!"));
            m_Controls->m_Warning->setVisible(true);
            return;
        }
        else
            m_Controls->m_Warning->setVisible(false);

        VecImgType::IndexType index;
        index[0] = crosspos[0] - roisize; index[0] = index[0] < 0 ? 0 : index[0];
        index[1] = crosspos[1] - roisize; index[1] = index[1] < 0 ? 0 : index[1];
        index[2] = crosspos[2] - roisize; index[2] = index[2] < 0 ? 0 : index[2];

        VecImgType::SizeType size;
        size[0] = roisize*2+1;
        size[1] = roisize*2+1;
        size[2] = roisize*2+1;

        VecImgType::SizeType maxSize = vecimg->GetLargestPossibleRegion().GetSize();
        size[0] = index[0]+size[0] > maxSize[0] ? maxSize[0]-index[0] : size[0];
        size[1] = index[1]+size[1] > maxSize[1] ? maxSize[1]-index[1] : size[1];
        size[2] = index[2]+size[2] > maxSize[2] ? maxSize[2]-index[2] : size[2];

        VecImgType::RegionType region;
        region.SetSize( size );
        region.SetIndex( index );
        vecimg->SetRequestedRegion( region );

        VecImgType::IndexType  newstart;
        newstart.Fill(0);

        VecImgType::RegionType newregion;
        newregion.SetSize( size );
        newregion.SetIndex( newstart );

        roiImage->CopyInformation( vecimg );
        roiImage->SetRegions( newregion );
        roiImage->SetOrigin( pos );
        roiImage->Allocate();
        roiImage->SetPixel(newstart, vecimg->GetPixel(index));

        if( m_Controls->m_ModelTabSelectionWidget->currentIndex() )
        {
            success = FitKurtosis(roiImage,
                             static_cast<mitk::GradientDirectionsProperty*>( diffusionImg->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer(),
                             static_cast<mitk::FloatProperty*>(diffusionImg->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue(),
                             newstart);
        }
        else
        {
            success = FittIVIM(roiImage,
                             static_cast<mitk::GradientDirectionsProperty*>( diffusionImg->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer(),
                             static_cast<mitk::FloatProperty*>(diffusionImg->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue(),
                             false,
                             crosspos);
        }
    }
    else
    {
        typedef itk::Image<float,3> MaskImgType;

        MaskImgType::Pointer maskItk;
        CastToItkImage( maskImg, maskItk );

        mitk::Point3D pos;
        pos[0] = 0;
        pos[1] = 0;
        pos[2] = 0;

        VecImgType::IndexType index;
        index[0] = 0;
        index[1] = 0;
        index[2] = 0;

        VecImgType::SizeType size;
        size[0] = 1;
        size[1] = 1;
        size[2] = 1;

        VecImgType::RegionType region;
        region.SetSize( size );
        region.SetIndex( index );
        vecimg->SetRequestedRegion( region );

        // iterators over output and input
        itk::ImageRegionConstIteratorWithIndex<VecImgType>
                vecit(vecimg, vecimg->GetLargestPossibleRegion());

        itk::VariableLengthVector<double> avg(vecimg->GetVectorLength());
        avg.Fill(0);

        float numPixels = 0;
        while ( ! vecit.IsAtEnd() )
        {
            VecImgType::PointType point;
            vecimg->TransformIndexToPhysicalPoint(vecit.GetIndex(), point);

            MaskImgType::IndexType index;
            maskItk->TransformPhysicalPointToIndex(point, index);

            if(maskItk->GetPixel(index) != 0)
            {
                avg += vecit.Get();
                numPixels += 1.0;
            }

            // update iterators
            ++vecit;

        }

        avg /= numPixels;

        m_Controls->m_Warning->setText(QString("Averaging ")+QString::number((int)numPixels)+QString(" voxels!"));
        m_Controls->m_Warning->setVisible(true);

        roiImage->CopyInformation( vecimg );
        roiImage->SetRegions( region );
        roiImage->SetOrigin( pos );
        roiImage->Allocate();
        roiImage->SetPixel(index, avg);

        if( m_Controls->m_ModelTabSelectionWidget->currentIndex() )
        {
            success = FitKurtosis(roiImage,
                             static_cast<mitk::GradientDirectionsProperty*>( diffusionImg->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer(),
                             static_cast<mitk::FloatProperty*>(diffusionImg->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue(),
                             index);
        }
        else
        {
            success = FittIVIM(roiImage,
                             static_cast<mitk::GradientDirectionsProperty*>( diffusionImg->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer(),
                             static_cast<mitk::FloatProperty*>(diffusionImg->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue(),
                             false,
                             index);
        }

        // do not update until selection changed, the values will remain the same as long as the mask is selected!
        m_HoldUpdate = true;

    }

    vecimg->SetRegions( vecimg->GetLargestPossibleRegion() );

    if (success)
    {
        // 0 - IVIM, 1 - Kurtosis
        if( m_Controls->m_ModelTabSelectionWidget->currentIndex() )
        {
           m_Controls->m_KurtosisVisualizationWidget->setVisible(true);
           m_Controls->m_KurtosisVisualizationWidget->SetData(m_KurtosisSnap);
        }
        else
        {
           m_Controls->m_VisualizeResultsWidget->setVisible(true);
           m_Controls->m_VisualizeResultsWidget->SetParameters(m_Snap);
        }
    }
}

bool QmitkIVIMView::FitKurtosis( itk::VectorImage<short, 3> *vecimg, DirContainerType *dirs, float bval, OutImgType::IndexType &crosspos )
{
  KurtosisFilterType::Pointer filter = KurtosisFilterType::New();

  itk::KurtosisFitConfiguration fit_config;
  fit_config.omit_bzero =  this->m_Controls->m_OmitBZeroCB->isChecked();
  if( this->m_Controls->m_UseKurtosisBoundsCB->isChecked() )
  {
    fit_config.use_K_limits = true;
    vnl_vector_fixed<double, 2> k_limits;
    k_limits[0] = this->m_Controls->m_KurtosisRangeWidget->minimumValue();
    k_limits[1] = this->m_Controls->m_KurtosisRangeWidget->maximumValue();

    fit_config.K_limits = k_limits;

  }
  fit_config.fit_scale = static_cast<itk::FitScale>(this->m_Controls->m_KurtosisFitScale->currentIndex() );

  m_KurtosisSnap = filter->GetSnapshot( vecimg->GetPixel( crosspos ), dirs, bval, fit_config );

  return true;
}


bool QmitkIVIMView::FittIVIM(itk::VectorImage<short,3>* vecimg, DirContainerType* dirs, float bval, bool multivoxel, OutImgType::IndexType &crosspos)
{
    IVIMFilterType::Pointer filter = IVIMFilterType::New();
    filter->SetInput(vecimg);
    filter->SetGradientDirections(dirs);
    filter->SetBValue(bval);

    switch(m_Controls->m_MethodCombo->currentIndex())
    {

    case 0:
        filter->SetMethod(IVIMFilterType::IVIM_FIT_ALL);
        filter->SetS0Thres(m_Controls->m_S0ThreshLabel->text().toDouble());
        break;

    case 1:
        filter->SetMethod(IVIMFilterType::IVIM_DSTAR_FIX);
        filter->SetDStar(m_Controls->m_DStarLabel->text().toDouble());
        filter->SetS0Thres(m_Controls->m_S0ThreshLabel->text().toDouble());
        break;

    case 2:
        filter->SetMethod(IVIMFilterType::IVIM_D_THEN_DSTAR);
        filter->SetBThres(m_Controls->m_BThreshLabel->text().toDouble());
        filter->SetS0Thres(m_Controls->m_S0ThreshLabel->text().toDouble());
        filter->SetFitDStar(m_Controls->m_CheckDStar->isChecked());
        break;

    case 3:
        filter->SetMethod(IVIMFilterType::IVIM_LINEAR_D_THEN_F);
        filter->SetBThres(m_Controls->m_BThreshLabel->text().toDouble());
        filter->SetS0Thres(m_Controls->m_S0ThreshLabel->text().toDouble());
        filter->SetFitDStar(m_Controls->m_CheckDStar->isChecked());
        break;

    case 4:
        filter->SetMethod(IVIMFilterType::IVIM_REGULARIZED);
        filter->SetBThres(m_Controls->m_BThreshLabel->text().toDouble());
        filter->SetS0Thres(m_Controls->m_S0ThreshLabel->text().toDouble());
        filter->SetNumberIterations(m_Controls->m_NumItsLabel->text().toInt());
        filter->SetLambda(m_Controls->m_LambdaLabel->text().toDouble());
        filter->SetFitDStar(m_Controls->m_CheckDStar->isChecked());
        break;
    }

    if(!multivoxel)
    {
        filter->SetFitDStar(true);
    }

    filter->SetNumberOfThreads(1);
    filter->SetVerbose(false);
    filter->SetCrossPosition(crosspos);

    try{
        filter->Update();
        m_Snap = filter->GetSnapshot();
        m_DStarMap = filter->GetOutput(2);
        m_DMap = filter->GetOutput(1);
        m_fMap = filter->GetOutput();
    }
    catch (itk::ExceptionObject &ex)
    {
        MITK_INFO << ex ;
        m_Controls->m_Warning->setText(QString("IVIM fit not possible: ")+ex.GetDescription());
        m_Controls->m_Warning->setVisible(true);
        return false;
    }
    return true;
}

void QmitkIVIMView::OutputToDatastorage(std::vector<mitk::DataNode*> nodes)
{
    // Outputs to Datastorage
    QString basename(nodes.front()->GetName().c_str());

    if(m_Controls->m_CheckDStar->isChecked())
    {
        mitk::Image::Pointer dstarimage = mitk::Image::New();
        dstarimage->InitializeByItk(m_DStarMap.GetPointer());
        dstarimage->SetVolume(m_DStarMap->GetBufferPointer());
        QString newname2 = basename; newname2 = newname2.append("_DStarMap_%1").arg(m_Controls->m_MethodCombo->currentText());
        mitk::DataNode::Pointer node2=mitk::DataNode::New();
        node2->SetData( dstarimage );
        node2->SetName(newname2.toLatin1());
        GetDefaultDataStorage()->Add(node2);
    }

    if(m_Controls->m_CheckD->isChecked())
    {
        mitk::Image::Pointer dimage = mitk::Image::New();
        dimage->InitializeByItk(m_DMap.GetPointer());
        dimage->SetVolume(m_DMap->GetBufferPointer());
        QString newname1 = basename; newname1 = newname1.append("_DMap_%1").arg(m_Controls->m_MethodCombo->currentText());
        mitk::DataNode::Pointer node1=mitk::DataNode::New();
        node1->SetData( dimage );
        node1->SetName(newname1.toLatin1());
        GetDefaultDataStorage()->Add(node1);
    }

    if(m_Controls->m_Checkf->isChecked())
    {
        mitk::Image::Pointer image = mitk::Image::New();
        image->InitializeByItk(m_fMap.GetPointer());
        image->SetVolume(m_fMap->GetBufferPointer());
        QString newname0 = basename; newname0 = newname0.append("_fMap_%1").arg(m_Controls->m_MethodCombo->currentText());
        mitk::DataNode::Pointer node=mitk::DataNode::New();
        node->SetData( image );
        node->SetName(newname0.toLatin1());
        GetDefaultDataStorage()->Add(node);
    }

    m_MultiWidget->RequestUpdate();

    // reset the data node labels, the selection in DataManager is lost after adding
    // a new node -> we cannot directly proceed twice, the DWI ( and MASK) image have to be selected again
    m_Controls->m_InputData->setTitle("Please Select Input Data");
    m_Controls->m_DiffusionImageLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_MaskImageLabel->setText("<font color='grey'>optional</font>");
    m_MaskImageNode = NULL;
    m_DiffusionImageNode = NULL;

}

void QmitkIVIMView::ChooseMethod()
{
    m_Controls->m_MethodCombo->setVisible(m_Controls->m_ChooseMethod->isChecked());
}

void QmitkIVIMView::ClipboardCurveButtonClicked()
{
  // Kurtosis
    if ( m_Controls->m_ModelTabSelectionWidget->currentIndex() )
    {
      std::stringstream ss;
      QString clipboard("Measurement Points\n");

      ss << m_KurtosisSnap.bvalues << "\n" << m_KurtosisSnap.measurements << "\n\n";
      ss << "Fitted Values ( D  K  [b_0] ) \n" << m_KurtosisSnap.m_D << " " << m_KurtosisSnap.m_K;
      if( m_KurtosisSnap.m_fittedBZero )
        ss << " " << m_KurtosisSnap.m_BzeroFit;

      ss << "\n\n";
      clipboard.append( QString( ss.str().c_str() ));

      ss.str( std::string() );
      ss.clear();

      QApplication::clipboard()->setText(
                  clipboard, QClipboard::Clipboard );
    }
    else
    {

        QString clipboard("Measurement Points\n");
        for ( unsigned int i=0; i<m_Snap.bvalues.size(); i++)
        {
            clipboard = clipboard.append( "%L1 \t" )
                    .arg( m_Snap.bvalues[i], 0, 'f', 2 );
        }
        clipboard = clipboard.append( "\n" );

        for ( unsigned int i=0; i<m_Snap.allmeas.size(); i++)
        {
            clipboard = clipboard.append( "%L1 \t" )
                    .arg( m_Snap.allmeas[i], 0, 'f', 2 );
        }
        clipboard = clipboard.append( "\n" );


        clipboard = clipboard.append( "1st Linear Fit of D and f \n" );
        double maxb = m_Snap.bvalues.max_value();
        clipboard = clipboard.append( "%L1 \t %L2 \n %L3 \t %L4 \n" )
                .arg( (float) 0, 0, 'f', 2 )
                .arg( maxb, 0, 'f', 2 )
                .arg(1-m_Snap.currentFunceiled, 0, 'f', 2 )
                .arg((1-m_Snap.currentFunceiled)*exp(-maxb * m_Snap.currentD), 0, 'f', 2 );


        int nsampling = 50;
        double f = 1-m_Snap.currentFunceiled;
        clipboard = clipboard.append("Final Model\n");
        for(int i=0; i<nsampling; i++)
        {
            double x = (((1.0)*i)/(1.0*nsampling))*maxb;
            clipboard = clipboard.append( "%L1 \t" )
                    .arg( x, 0, 'f', 2 );
        }
        clipboard = clipboard.append( "\n" );

        for(int i=0; i<nsampling; i++)
        {
            double x = (((1.0)*i)/(1.0*nsampling))*maxb;
            double y = f*exp(- x * m_Snap.currentD) + (1-f)*exp(- x * (m_Snap.currentD+m_Snap.currentDStar));
            clipboard = clipboard.append( "%L1 \t" )
                    .arg( y, 0, 'f', 2 );
        }
        clipboard = clipboard.append( "\n" );

        QApplication::clipboard()->setText(
                    clipboard, QClipboard::Clipboard );
    }
}


void QmitkIVIMView::ClipboardStatisticsButtonClicked()
{
    // Kurtosis
    if ( m_Controls->m_ModelTabSelectionWidget->currentIndex() )
    {

        QString clipboard( "D \t K \n" );
        clipboard = clipboard.append( "%L1 \t %L2" )
                .arg( m_KurtosisSnap.m_D, 0, 'f', 10 )
                .arg( m_KurtosisSnap.m_K, 0, 'f', 10 ) ;

        QApplication::clipboard()->setText(
                    clipboard, QClipboard::Clipboard );
    }
    else
    {
      QString clipboard( "f \t D \t D* \n" );
      clipboard = clipboard.append( "%L1 \t %L2 \t %L3" )
              .arg( m_Snap.currentF, 0, 'f', 10 )
              .arg( m_Snap.currentD, 0, 'f', 10 )
              .arg( m_Snap.currentDStar, 0, 'f', 10 ) ;

      QApplication::clipboard()->setText(
                  clipboard, QClipboard::Clipboard );
    }
}

void QmitkIVIMView::Activated()
{
    m_Active = true;
}

void QmitkIVIMView::Deactivated()
{
    m_Active = false;
}
