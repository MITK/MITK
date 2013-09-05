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
#define _USE_MATH_DEFINES
#include <math.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkFiberfoxView.h"

// MITK
#include <mitkImage.h>
#include <mitkDiffusionImage.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkDataStorage.h>
#include <itkFibersFromPlanarFiguresFilter.h>
#include <itkTractsToDWIImageFilter.h>
#include <mitkTensorImage.h>
#include <mitkILinkedRenderWindowPart.h>
#include <mitkGlobalInteraction.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkImageGenerator.h>
#include <mitkNodePredicateDataType.h>
#include <itkScalableAffineTransform.h>
#include <mitkLevelWindowProperty.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <itkAddArtifactsToDwiImageFilter.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <QFileDialog>
#include <QMessageBox>
#include "usModuleRegistry.h"

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkFiberfoxView::VIEW_ID = "org.mitk.views.fiberfoxview";

QmitkFiberfoxView::QmitkFiberfoxView()
    : QmitkAbstractView()
    , m_Controls( 0 )
    , m_SelectedImage( NULL )
{

}

// Destructor
QmitkFiberfoxView::~QmitkFiberfoxView()
{

}

void QmitkFiberfoxView::CreateQtPartControl( QWidget *parent )
{
    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkFiberfoxViewControls;
        m_Controls->setupUi( parent );

        m_Controls->m_StickWidget1->setVisible(true);
        m_Controls->m_StickWidget2->setVisible(false);
        m_Controls->m_ZeppelinWidget1->setVisible(false);
        m_Controls->m_ZeppelinWidget2->setVisible(false);
        m_Controls->m_TensorWidget1->setVisible(false);
        m_Controls->m_TensorWidget2->setVisible(false);

        m_Controls->m_BallWidget1->setVisible(true);
        m_Controls->m_BallWidget2->setVisible(false);
        m_Controls->m_AstrosticksWidget1->setVisible(false);
        m_Controls->m_AstrosticksWidget2->setVisible(false);
        m_Controls->m_DotWidget1->setVisible(false);
        m_Controls->m_DotWidget2->setVisible(false);

        m_Controls->m_Comp4FractionFrame->setVisible(false);
        m_Controls->m_DiffusionPropsMessage->setVisible(false);
        m_Controls->m_GeometryMessage->setVisible(false);
        m_Controls->m_AdvancedSignalOptionsFrame->setVisible(false);
        m_Controls->m_AdvancedFiberOptionsFrame->setVisible(false);
        m_Controls->m_VarianceBox->setVisible(false);
        m_Controls->m_GibbsRingingFrame->setVisible(false);
        m_Controls->m_NoiseFrame->setVisible(false);
        m_Controls->m_GhostFrame->setVisible(false);
        m_Controls->m_DistortionsFrame->setVisible(false);
        m_Controls->m_EddyFrame->setVisible(false);
        m_Controls->m_SpikeFrame->setVisible(false);

        m_Controls->m_FrequencyMapBox->SetDataStorage(this->GetDataStorage());
        mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
        mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
        mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
        mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");
        mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isDwi, isDti);
        isDiffusionImage = mitk::NodePredicateOr::New(isDiffusionImage, isQbi);
        mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
        mitk::NodePredicateAnd::Pointer finalPredicate = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
        m_Controls->m_FrequencyMapBox->SetPredicate(finalPredicate);

        connect((QObject*) m_Controls->m_GenerateImageButton, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateImage()));
        connect((QObject*) m_Controls->m_GenerateFibersButton, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateFibers()));
        connect((QObject*) m_Controls->m_CircleButton, SIGNAL(clicked()), (QObject*) this, SLOT(OnDrawROI()));
        connect((QObject*) m_Controls->m_FlipButton, SIGNAL(clicked()), (QObject*) this, SLOT(OnFlipButton()));
        connect((QObject*) m_Controls->m_JoinBundlesButton, SIGNAL(clicked()), (QObject*) this, SLOT(JoinBundles()));
        connect((QObject*) m_Controls->m_VarianceBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnVarianceChanged(double)));
        connect((QObject*) m_Controls->m_DistributionBox, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(OnDistributionChanged(int)));
        connect((QObject*) m_Controls->m_FiberDensityBox, SIGNAL(valueChanged(int)), (QObject*) this, SLOT(OnFiberDensityChanged(int)));
        connect((QObject*) m_Controls->m_FiberSamplingBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnFiberSamplingChanged(double)));
        connect((QObject*) m_Controls->m_TensionBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnTensionChanged(double)));
        connect((QObject*) m_Controls->m_ContinuityBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnContinuityChanged(double)));
        connect((QObject*) m_Controls->m_BiasBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnBiasChanged(double)));
        connect((QObject*) m_Controls->m_AddGibbsRinging, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddGibbsRinging(int)));
        connect((QObject*) m_Controls->m_AddNoise, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddNoise(int)));
        connect((QObject*) m_Controls->m_AddGhosts, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddGhosts(int)));
        connect((QObject*) m_Controls->m_AddDistortions, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddDistortions(int)));
        connect((QObject*) m_Controls->m_AddEddy, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddEddy(int)));
        connect((QObject*) m_Controls->m_AddSpikes, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddSpikes(int)));

        connect((QObject*) m_Controls->m_ConstantRadiusBox, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnConstantRadius(int)));
        connect((QObject*) m_Controls->m_CopyBundlesButton, SIGNAL(clicked()), (QObject*) this, SLOT(CopyBundles()));
        connect((QObject*) m_Controls->m_TransformBundlesButton, SIGNAL(clicked()), (QObject*) this, SLOT(ApplyTransform()));
        connect((QObject*) m_Controls->m_AlignOnGrid, SIGNAL(clicked()), (QObject*) this, SLOT(AlignOnGrid()));

        connect((QObject*) m_Controls->m_Compartment1Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp1ModelFrameVisibility(int)));
        connect((QObject*) m_Controls->m_Compartment2Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp2ModelFrameVisibility(int)));
        connect((QObject*) m_Controls->m_Compartment3Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp3ModelFrameVisibility(int)));
        connect((QObject*) m_Controls->m_Compartment4Box, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(Comp4ModelFrameVisibility(int)));

        connect((QObject*) m_Controls->m_AdvancedOptionsBox, SIGNAL( stateChanged(int)), (QObject*) this, SLOT(ShowAdvancedOptions(int)));
        connect((QObject*) m_Controls->m_AdvancedOptionsBox_2, SIGNAL( stateChanged(int)), (QObject*) this, SLOT(ShowAdvancedOptions(int)));

        connect((QObject*) m_Controls->m_SaveParametersButton, SIGNAL(clicked()), (QObject*) this, SLOT(SaveParameters()));
        connect((QObject*) m_Controls->m_LoadParametersButton, SIGNAL(clicked()), (QObject*) this, SLOT(LoadParameters()));
    }
}

void QmitkFiberfoxView::UpdateImageParameters()
{
    m_ImageGenParameters.artifactList.clear();
    m_ImageGenParameters.nonFiberModelList.clear();
    m_ImageGenParameters.fiberModelList.clear();
    m_ImageGenParameters.signalModelString = "";
    m_ImageGenParameters.artifactModelString = "";
    m_ImageGenParameters.resultNode = mitk::DataNode::New();
    m_ImageGenParameters.tissueMaskImage = NULL;
    m_ImageGenParameters.frequencyMap = NULL;
    m_ImageGenParameters.gradientDirections.clear();
    m_ImageGenParameters.spikes = 0;
    m_ImageGenParameters.spikeAmplitude = 1;

    if (m_SelectedDWI.IsNotNull())  // use parameters of selected DWI
    {
        mitk::DiffusionImage<short>::Pointer dwi = dynamic_cast<mitk::DiffusionImage<short>*>(m_SelectedDWI->GetData());
        m_ImageGenParameters.imageRegion = dwi->GetVectorImage()->GetLargestPossibleRegion();
        m_ImageGenParameters.imageSpacing = dwi->GetVectorImage()->GetSpacing();
        m_ImageGenParameters.imageOrigin = dwi->GetVectorImage()->GetOrigin();
        m_ImageGenParameters.imageDirection = dwi->GetVectorImage()->GetDirection();
        m_ImageGenParameters.b_value = dwi->GetB_Value();
        mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer dirs = dwi->GetDirections();

        m_ImageGenParameters.numGradients = 0;
        for (int i=0; i<dirs->Size(); i++)
        {
            DiffusionSignalModel<double>::GradientType g;
            g[0] = dirs->at(i)[0];
            g[1] = dirs->at(i)[1];
            g[2] = dirs->at(i)[2];
            m_ImageGenParameters.gradientDirections.push_back(g);
            if (dirs->at(i).magnitude()>0.0001)
                m_ImageGenParameters.numGradients++;
        }
    }
    else if (m_SelectedImage.IsNotNull())   // use geometry of selected image
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_SelectedImage->GetData());
        itk::Image< float, 3 >::Pointer itkImg = itk::Image< float, 3 >::New();
        CastToItkImage< itk::Image< float, 3 > >(img, itkImg);

        m_ImageGenParameters.imageRegion = itkImg->GetLargestPossibleRegion();
        m_ImageGenParameters.imageSpacing = itkImg->GetSpacing();
        m_ImageGenParameters.imageOrigin = itkImg->GetOrigin();
        m_ImageGenParameters.imageDirection = itkImg->GetDirection();

        m_ImageGenParameters.numGradients = m_Controls->m_NumGradientsBox->value();
        m_ImageGenParameters.gradientDirections = GenerateHalfShell(m_Controls->m_NumGradientsBox->value());
        m_ImageGenParameters.b_value = m_Controls->m_BvalueBox->value();
    }
    else    // use GUI parameters
    {
        m_ImageGenParameters.imageRegion.SetSize(0, m_Controls->m_SizeX->value());
        m_ImageGenParameters.imageRegion.SetSize(1, m_Controls->m_SizeY->value());
        m_ImageGenParameters.imageRegion.SetSize(2, m_Controls->m_SizeZ->value());
        m_ImageGenParameters.imageSpacing[0] = m_Controls->m_SpacingX->value();
        m_ImageGenParameters.imageSpacing[1] = m_Controls->m_SpacingY->value();
        m_ImageGenParameters.imageSpacing[2] = m_Controls->m_SpacingZ->value();
        m_ImageGenParameters.imageOrigin[0] = m_ImageGenParameters.imageSpacing[0]/2;
        m_ImageGenParameters.imageOrigin[1] = m_ImageGenParameters.imageSpacing[1]/2;
        m_ImageGenParameters.imageOrigin[2] = m_ImageGenParameters.imageSpacing[2]/2;
        m_ImageGenParameters.imageDirection.SetIdentity();

        m_ImageGenParameters.numGradients = m_Controls->m_NumGradientsBox->value();
        m_ImageGenParameters.gradientDirections = GenerateHalfShell(m_Controls->m_NumGradientsBox->value());;
        m_ImageGenParameters.b_value = m_Controls->m_BvalueBox->value();
    }

    // signal relaxation
    m_ImageGenParameters.doSimulateRelaxation = m_Controls->m_RelaxationBox->isChecked();
    if (m_ImageGenParameters.doSimulateRelaxation)
        m_ImageGenParameters.artifactModelString += "_RELAX";

    // N/2 ghosts
    if (m_Controls->m_AddGhosts->isChecked())
    {
        m_ImageGenParameters.artifactModelString += "_GHOST";
        m_ImageGenParameters.kspaceLineOffset = m_Controls->m_kOffsetBox->value();
    }
    else
        m_ImageGenParameters.kspaceLineOffset = 0;

    m_ImageGenParameters.tLine = m_Controls->m_LineReadoutTimeBox->value();
    m_ImageGenParameters.tInhom = m_Controls->m_T2starBox->value();
    m_ImageGenParameters.tEcho = m_Controls->m_TEbox->value();
    m_ImageGenParameters.repetitions = m_Controls->m_RepetitionsBox->value();
    m_ImageGenParameters.doDisablePartialVolume = m_Controls->m_EnforcePureFiberVoxelsBox->isChecked();
    m_ImageGenParameters.interpolationShrink = m_Controls->m_InterpolationShrink->value();
    m_ImageGenParameters.axonRadius = m_Controls->m_FiberRadius->value();
    m_ImageGenParameters.signalScale = m_Controls->m_SignalScaleBox->value();

    if (m_Controls->m_AddSpikes->isChecked())
    {
        m_ImageGenParameters.spikes = m_Controls->m_SpikeNumBox->value();
        m_ImageGenParameters.spikeAmplitude = m_Controls->m_SpikeScaleBox->value();
        m_ImageGenParameters.artifactModelString += "_SPIKES";
    }

    // adjust echo time if needed
    if ( m_ImageGenParameters.tEcho < m_ImageGenParameters.imageRegion.GetSize(1)*m_ImageGenParameters.tLine )
    {
        this->m_Controls->m_TEbox->setValue( m_ImageGenParameters.imageRegion.GetSize(1)*m_ImageGenParameters.tLine );
        m_ImageGenParameters.tEcho = m_Controls->m_TEbox->value();
        QMessageBox::information( NULL, "Warning", "Echo time is too short! Time not sufficient to read slice. Automaticall adjusted to "+QString::number(m_ImageGenParameters.tEcho)+" ms");
    }

    // check tissue mask
    if (m_TissueMask.IsNotNull())
    {
        m_ImageGenParameters.tissueMaskImage = ItkUcharImgType::New();
        mitk::CastToItkImage<ItkUcharImgType>(m_TissueMask, m_ImageGenParameters.tissueMaskImage);
    }

    // rician noise
    if (m_Controls->m_AddNoise->isChecked())
    {
        double noiseVariance = m_Controls->m_NoiseLevel->value();
        m_ImageGenParameters.ricianNoiseModel.SetNoiseVariance(noiseVariance);
        m_ImageGenParameters.artifactModelString += "_NOISE";
        m_ImageGenParameters.artifactModelString += QString::number(noiseVariance);
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Noise-Variance", DoubleProperty::New(noiseVariance));
    }
    else
        m_ImageGenParameters.ricianNoiseModel.SetNoiseVariance(0);

    // gibbs ringing
    m_ImageGenParameters.upsampling = 1;
    if (m_Controls->m_AddGibbsRinging->isChecked())
    {
        m_ImageGenParameters.artifactModelString += "_RINGING";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Ringing-Upsampling", DoubleProperty::New(m_Controls->m_ImageUpsamplingBox->value()));
        m_ImageGenParameters.upsampling = m_Controls->m_ImageUpsamplingBox->value();
    }

    // adjusting line readout time to the adapted image size needed for the DFT
    int y = m_ImageGenParameters.imageRegion.GetSize(1);
    if ( y%2 == 1 )
        y += 1;
    if ( y>m_ImageGenParameters.imageRegion.GetSize(1) )
        m_ImageGenParameters.tLine *= (double)m_ImageGenParameters.imageRegion.GetSize(1)/y;

    // add distortions
    if (m_Controls->m_AddDistortions->isChecked() && m_Controls->m_FrequencyMapBox->GetSelectedNode().IsNotNull())
    {
        mitk::DataNode::Pointer fMapNode = m_Controls->m_FrequencyMapBox->GetSelectedNode();
        mitk::Image* img = dynamic_cast<mitk::Image*>(fMapNode->GetData());
        ItkDoubleImgType::Pointer itkImg = ItkDoubleImgType::New();
        CastToItkImage< ItkDoubleImgType >(img, itkImg);

        if (m_ImageGenParameters.imageRegion.GetSize(0)==itkImg->GetLargestPossibleRegion().GetSize(0) &&
                m_ImageGenParameters.imageRegion.GetSize(1)==itkImg->GetLargestPossibleRegion().GetSize(1) &&
                m_ImageGenParameters.imageRegion.GetSize(2)==itkImg->GetLargestPossibleRegion().GetSize(2))
        {
            m_ImageGenParameters.frequencyMap = itkImg;
            m_ImageGenParameters.artifactModelString += "_DISTORTED";
        }
    }

    m_ImageGenParameters.doSimulateEddyCurrents = m_Controls->m_AddEddy->isChecked();
    m_ImageGenParameters.eddyStrength = 0;
    if (m_Controls->m_AddEddy->isChecked())
    {
        m_ImageGenParameters.eddyStrength = m_Controls->m_EddyGradientStrength->value();
        m_ImageGenParameters.artifactModelString += "_EDDY";
    }

    // signal models
    m_ImageGenParameters.comp3Weight = 1;
    m_ImageGenParameters.comp4Weight = 0;
    if (m_Controls->m_Compartment4Box->currentIndex()>0)
    {
        m_ImageGenParameters.comp4Weight = m_Controls->m_Comp4FractionBox->value();
        m_ImageGenParameters.comp3Weight -= m_ImageGenParameters.comp4Weight;
    }

    // compartment 1
    switch (m_Controls->m_Compartment1Box->currentIndex())
    {
    case 0:
        m_StickModel1.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_StickModel1.SetBvalue(m_ImageGenParameters.b_value);
        m_StickModel1.SetDiffusivity(m_Controls->m_StickWidget1->GetD());
        m_StickModel1.SetT2(m_Controls->m_StickWidget1->GetT2());
        m_ImageGenParameters.fiberModelList.push_back(&m_StickModel1);
        m_ImageGenParameters.signalModelString += "Stick";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Stick") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.D", DoubleProperty::New(m_Controls->m_StickWidget1->GetD()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.T2", DoubleProperty::New(m_StickModel1.GetT2()) );
        break;
    case 1:
        m_ZeppelinModel1.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_ZeppelinModel1.SetBvalue(m_ImageGenParameters.b_value);
        m_ZeppelinModel1.SetDiffusivity1(m_Controls->m_ZeppelinWidget1->GetD1());
        m_ZeppelinModel1.SetDiffusivity2(m_Controls->m_ZeppelinWidget1->GetD2());
        m_ZeppelinModel1.SetDiffusivity3(m_Controls->m_ZeppelinWidget1->GetD2());
        m_ZeppelinModel1.SetT2(m_Controls->m_ZeppelinWidget1->GetT2());
        m_ImageGenParameters.fiberModelList.push_back(&m_ZeppelinModel1);
        m_ImageGenParameters.signalModelString += "Zeppelin";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Zeppelin") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.D1", DoubleProperty::New(m_Controls->m_ZeppelinWidget1->GetD1()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.D2", DoubleProperty::New(m_Controls->m_ZeppelinWidget1->GetD2()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.T2", DoubleProperty::New(m_ZeppelinModel1.GetT2()) );
        break;
    case 2:
        m_TensorModel1.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_TensorModel1.SetBvalue(m_ImageGenParameters.b_value);
        m_TensorModel1.SetDiffusivity1(m_Controls->m_TensorWidget1->GetD1());
        m_TensorModel1.SetDiffusivity2(m_Controls->m_TensorWidget1->GetD2());
        m_TensorModel1.SetDiffusivity3(m_Controls->m_TensorWidget1->GetD3());
        m_TensorModel1.SetT2(m_Controls->m_TensorWidget1->GetT2());
        m_ImageGenParameters.fiberModelList.push_back(&m_TensorModel1);
        m_ImageGenParameters.signalModelString += "Tensor";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.Description", StringProperty::New("Intra-axonal compartment") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.Model", StringProperty::New("Tensor") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.D1", DoubleProperty::New(m_Controls->m_TensorWidget1->GetD1()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.D2", DoubleProperty::New(m_Controls->m_TensorWidget1->GetD2()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.D3", DoubleProperty::New(m_Controls->m_TensorWidget1->GetD3()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment1.T2", DoubleProperty::New(m_ZeppelinModel1.GetT2()) );
        break;
    }

    // compartment 2
    switch (m_Controls->m_Compartment2Box->currentIndex())
    {
    case 0:
        break;
    case 1:
        m_StickModel2.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_StickModel2.SetBvalue(m_ImageGenParameters.b_value);
        m_StickModel2.SetDiffusivity(m_Controls->m_StickWidget2->GetD());
        m_StickModel2.SetT2(m_Controls->m_StickWidget2->GetT2());
        m_ImageGenParameters.fiberModelList.push_back(&m_StickModel2);
        m_ImageGenParameters.signalModelString += "Stick";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.Description", StringProperty::New("Inter-axonal compartment") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.Model", StringProperty::New("Stick") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.D", DoubleProperty::New(m_Controls->m_StickWidget2->GetD()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.T2", DoubleProperty::New(m_StickModel2.GetT2()) );
        break;
    case 2:
        m_ZeppelinModel2.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_ZeppelinModel2.SetBvalue(m_ImageGenParameters.b_value);
        m_ZeppelinModel2.SetDiffusivity1(m_Controls->m_ZeppelinWidget2->GetD1());
        m_ZeppelinModel2.SetDiffusivity2(m_Controls->m_ZeppelinWidget2->GetD2());
        m_ZeppelinModel2.SetDiffusivity3(m_Controls->m_ZeppelinWidget2->GetD2());
        m_ZeppelinModel2.SetT2(m_Controls->m_ZeppelinWidget2->GetT2());
        m_ImageGenParameters.fiberModelList.push_back(&m_ZeppelinModel2);
        m_ImageGenParameters.signalModelString += "Zeppelin";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.Description", StringProperty::New("Inter-axonal compartment") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.Model", StringProperty::New("Zeppelin") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.D1", DoubleProperty::New(m_Controls->m_ZeppelinWidget2->GetD1()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.D2", DoubleProperty::New(m_Controls->m_ZeppelinWidget2->GetD2()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.T2", DoubleProperty::New(m_ZeppelinModel2.GetT2()) );
        break;
    case 3:
        m_TensorModel2.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_TensorModel2.SetBvalue(m_ImageGenParameters.b_value);
        m_TensorModel2.SetDiffusivity1(m_Controls->m_TensorWidget2->GetD1());
        m_TensorModel2.SetDiffusivity2(m_Controls->m_TensorWidget2->GetD2());
        m_TensorModel2.SetDiffusivity3(m_Controls->m_TensorWidget2->GetD3());
        m_TensorModel2.SetT2(m_Controls->m_TensorWidget2->GetT2());
        m_ImageGenParameters.fiberModelList.push_back(&m_TensorModel2);
        m_ImageGenParameters.signalModelString += "Tensor";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.Description", StringProperty::New("Inter-axonal compartment") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.Model", StringProperty::New("Tensor") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.D1", DoubleProperty::New(m_Controls->m_TensorWidget2->GetD1()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.D2", DoubleProperty::New(m_Controls->m_TensorWidget2->GetD2()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.D3", DoubleProperty::New(m_Controls->m_TensorWidget2->GetD3()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment2.T2", DoubleProperty::New(m_ZeppelinModel2.GetT2()) );
        break;
    }

    // compartment 3
    switch (m_Controls->m_Compartment3Box->currentIndex())
    {
    case 0:
        m_BallModel1.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_BallModel1.SetBvalue(m_ImageGenParameters.b_value);
        m_BallModel1.SetDiffusivity(m_Controls->m_BallWidget1->GetD());
        m_BallModel1.SetT2(m_Controls->m_BallWidget1->GetT2());
        m_BallModel1.SetWeight(m_ImageGenParameters.comp3Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(&m_BallModel1);
        m_ImageGenParameters.signalModelString += "Ball";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Ball") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.D", DoubleProperty::New(m_Controls->m_BallWidget1->GetD()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.T2", DoubleProperty::New(m_BallModel1.GetT2()) );
        break;
    case 1:
        m_AstrosticksModel1.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_AstrosticksModel1.SetBvalue(m_ImageGenParameters.b_value);
        m_AstrosticksModel1.SetDiffusivity(m_Controls->m_AstrosticksWidget1->GetD());
        m_AstrosticksModel1.SetT2(m_Controls->m_AstrosticksWidget1->GetT2());
        m_AstrosticksModel1.SetRandomizeSticks(m_Controls->m_AstrosticksWidget1->GetRandomizeSticks());
        m_AstrosticksModel1.SetWeight(m_ImageGenParameters.comp3Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(&m_AstrosticksModel1);
        m_ImageGenParameters.signalModelString += "Astrosticks";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Astrosticks") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.D", DoubleProperty::New(m_Controls->m_AstrosticksWidget1->GetD()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.T2", DoubleProperty::New(m_AstrosticksModel1.GetT2()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.RandomSticks", BoolProperty::New(m_Controls->m_AstrosticksWidget1->GetRandomizeSticks()) );
        break;
    case 2:
        m_DotModel1.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_DotModel1.SetT2(m_Controls->m_DotWidget1->GetT2());
        m_DotModel1.SetWeight(m_ImageGenParameters.comp3Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(&m_DotModel1);
        m_ImageGenParameters.signalModelString += "Dot";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.Description", StringProperty::New("Extra-axonal compartment 1") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.Model", StringProperty::New("Dot") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment3.T2", DoubleProperty::New(m_DotModel1.GetT2()) );
        break;
    }

    // compartment 4
    switch (m_Controls->m_Compartment4Box->currentIndex())
    {
    case 0:
        break;
    case 1:
        m_BallModel2.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_BallModel2.SetBvalue(m_ImageGenParameters.b_value);
        m_BallModel2.SetDiffusivity(m_Controls->m_BallWidget2->GetD());
        m_BallModel2.SetT2(m_Controls->m_BallWidget2->GetT2());
        m_BallModel2.SetWeight(m_ImageGenParameters.comp4Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(&m_BallModel2);
        m_ImageGenParameters.signalModelString += "Ball";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Ball") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.D", DoubleProperty::New(m_Controls->m_BallWidget2->GetD()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.T2", DoubleProperty::New(m_BallModel2.GetT2()) );
        break;
    case 2:
        m_AstrosticksModel2.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_AstrosticksModel2.SetBvalue(m_ImageGenParameters.b_value);
        m_AstrosticksModel2.SetDiffusivity(m_Controls->m_AstrosticksWidget2->GetD());
        m_AstrosticksModel2.SetT2(m_Controls->m_AstrosticksWidget2->GetT2());
        m_AstrosticksModel2.SetRandomizeSticks(m_Controls->m_AstrosticksWidget2->GetRandomizeSticks());
        m_AstrosticksModel2.SetWeight(m_ImageGenParameters.comp4Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(&m_AstrosticksModel2);
        m_ImageGenParameters.signalModelString += "Astrosticks";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Astrosticks") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.D", DoubleProperty::New(m_Controls->m_AstrosticksWidget2->GetD()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.T2", DoubleProperty::New(m_AstrosticksModel2.GetT2()) );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.RandomSticks", BoolProperty::New(m_Controls->m_AstrosticksWidget2->GetRandomizeSticks()) );
        break;
    case 3:
        m_DotModel2.SetGradientList(m_ImageGenParameters.gradientDirections);
        m_DotModel2.SetT2(m_Controls->m_DotWidget2->GetT2());
        m_DotModel2.SetWeight(m_ImageGenParameters.comp4Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(&m_DotModel2);
        m_ImageGenParameters.signalModelString += "Dot";
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.Description", StringProperty::New("Extra-axonal compartment 2") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.Model", StringProperty::New("Dot") );
        m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Compartment4.T2", DoubleProperty::New(m_DotModel2.GetT2()) );
        break;
    }

    m_ImageGenParameters.resultNode->AddProperty("Fiberfox.InterpolationShrink", IntProperty::New(m_ImageGenParameters.interpolationShrink));
    m_ImageGenParameters.resultNode->AddProperty("Fiberfox.SignalScale", IntProperty::New(m_ImageGenParameters.signalScale));
    m_ImageGenParameters.resultNode->AddProperty("Fiberfox.FiberRadius", IntProperty::New(m_ImageGenParameters.axonRadius));
    m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Tinhom", IntProperty::New(m_ImageGenParameters.tInhom));
    m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Repetitions", IntProperty::New(m_ImageGenParameters.repetitions));
    m_ImageGenParameters.resultNode->AddProperty("Fiberfox.b-value", DoubleProperty::New(m_ImageGenParameters.b_value));
    m_ImageGenParameters.resultNode->AddProperty("Fiberfox.Model", StringProperty::New(m_ImageGenParameters.signalModelString.toStdString()));
    m_ImageGenParameters.resultNode->AddProperty("Fiberfox.PureFiberVoxels", BoolProperty::New(m_ImageGenParameters.doDisablePartialVolume));
    m_ImageGenParameters.resultNode->AddProperty("binary", BoolProperty::New(false));
}

void QmitkFiberfoxView::SaveParameters()
{
    UpdateImageParameters();

    QString filename = QFileDialog::getSaveFileName(
                0,
                tr("Save Parameters"),
                QDir::currentPath()+"/param.ffp",
                tr("Fiberfox Parameters (*.ffp)") );

    if(filename.isEmpty() || filename.isNull())
        return;
    if(!filename.endsWith(".ffp"))
        filename += ".ffp";

    boost::property_tree::ptree parameters;

    // fiber generation parameters
    parameters.put("fiberfox.fibers.realtime", m_Controls->m_RealTimeFibers->isChecked());
    parameters.put("fiberfox.fibers.showadvanced", m_Controls->m_AdvancedOptionsBox->isChecked());
    parameters.put("fiberfox.fibers.distribution", m_Controls->m_DistributionBox->currentIndex());
    parameters.put("fiberfox.fibers.variance", m_Controls->m_VarianceBox->value());
    parameters.put("fiberfox.fibers.density", m_Controls->m_FiberDensityBox->value());
    parameters.put("fiberfox.fibers.spline.sampling", m_Controls->m_FiberSamplingBox->value());
    parameters.put("fiberfox.fibers.spline.tension", m_Controls->m_TensionBox->value());
    parameters.put("fiberfox.fibers.spline.continuity", m_Controls->m_ContinuityBox->value());
    parameters.put("fiberfox.fibers.spline.bias", m_Controls->m_BiasBox->value());
    parameters.put("fiberfox.fibers.constantradius", m_Controls->m_ConstantRadiusBox->isChecked());
    parameters.put("fiberfox.fibers.rotation.x", m_Controls->m_XrotBox->value());
    parameters.put("fiberfox.fibers.rotation.y", m_Controls->m_YrotBox->value());
    parameters.put("fiberfox.fibers.rotation.z", m_Controls->m_ZrotBox->value());
    parameters.put("fiberfox.fibers.translation.x", m_Controls->m_XtransBox->value());
    parameters.put("fiberfox.fibers.translation.y", m_Controls->m_YtransBox->value());
    parameters.put("fiberfox.fibers.translation.z", m_Controls->m_ZtransBox->value());
    parameters.put("fiberfox.fibers.scale.x", m_Controls->m_XscaleBox->value());
    parameters.put("fiberfox.fibers.scale.y", m_Controls->m_YscaleBox->value());
    parameters.put("fiberfox.fibers.scale.z", m_Controls->m_ZscaleBox->value());
    parameters.put("fiberfox.fibers.includeFiducials", m_Controls->m_IncludeFiducials->isChecked());
    parameters.put("fiberfox.fibers.includeFiducials", m_Controls->m_IncludeFiducials->isChecked());

    // image generation parameters
    parameters.put("fiberfox.image.basic.size.x", m_ImageGenParameters.imageRegion.GetSize(0));
    parameters.put("fiberfox.image.basic.size.y", m_ImageGenParameters.imageRegion.GetSize(1));
    parameters.put("fiberfox.image.basic.size.z", m_ImageGenParameters.imageRegion.GetSize(2));
    parameters.put("fiberfox.image.basic.spacing.x", m_ImageGenParameters.imageSpacing[0]);
    parameters.put("fiberfox.image.basic.spacing.y", m_ImageGenParameters.imageSpacing[1]);
    parameters.put("fiberfox.image.basic.spacing.z", m_ImageGenParameters.imageSpacing[2]);
    parameters.put("fiberfox.image.basic.numgradients", m_ImageGenParameters.numGradients);
    parameters.put("fiberfox.image.basic.bvalue", m_ImageGenParameters.b_value);
    parameters.put("fiberfox.image.showadvanced", m_Controls->m_AdvancedOptionsBox_2->isChecked());
    parameters.put("fiberfox.image.repetitions", m_ImageGenParameters.repetitions);
    parameters.put("fiberfox.image.signalScale", m_ImageGenParameters.signalScale);
    parameters.put("fiberfox.image.tEcho", m_ImageGenParameters.tEcho);
    parameters.put("fiberfox.image.tLine", m_Controls->m_LineReadoutTimeBox->value());
    parameters.put("fiberfox.image.tInhom", m_ImageGenParameters.tInhom);
    parameters.put("fiberfox.image.axonRadius", m_ImageGenParameters.axonRadius);
    parameters.put("fiberfox.image.interpolationShrink", m_ImageGenParameters.interpolationShrink);
    parameters.put("fiberfox.image.doSimulateRelaxation", m_ImageGenParameters.doSimulateRelaxation);
    parameters.put("fiberfox.image.doDisablePartialVolume", m_ImageGenParameters.doDisablePartialVolume);
    parameters.put("fiberfox.image.outputvolumefractions", m_Controls->m_VolumeFractionsBox->isChecked());

    parameters.put("fiberfox.image.artifacts.addnoise", m_Controls->m_AddNoise->isChecked());
    parameters.put("fiberfox.image.artifacts.noisevariance", m_Controls->m_NoiseLevel->value());
    parameters.put("fiberfox.image.artifacts.addghost", m_Controls->m_AddGhosts->isChecked());
    parameters.put("fiberfox.image.artifacts.kspaceLineOffset", m_Controls->m_kOffsetBox->value());
    parameters.put("fiberfox.image.artifacts.distortions", m_Controls->m_AddDistortions->isChecked());
    parameters.put("fiberfox.image.artifacts.addeddy", m_Controls->m_AddEddy->isChecked());
    parameters.put("fiberfox.image.artifacts.eddyStrength", m_Controls->m_EddyGradientStrength->value());
    parameters.put("fiberfox.image.artifacts.addringing", m_Controls->m_AddGibbsRinging->isChecked());
    parameters.put("fiberfox.image.artifacts.ringingupsampling", m_Controls->m_ImageUpsamplingBox->value());
    parameters.put("fiberfox.image.artifacts.addspikes", m_Controls->m_AddSpikes->isChecked());
    parameters.put("fiberfox.image.artifacts.spikesnum", m_Controls->m_SpikeNumBox->value());
    parameters.put("fiberfox.image.artifacts.spikesscale", m_Controls->m_SpikeScaleBox->value());

    parameters.put("fiberfox.image.compartment1.index", m_Controls->m_Compartment1Box->currentIndex());
    parameters.put("fiberfox.image.compartment2.index", m_Controls->m_Compartment2Box->currentIndex());
    parameters.put("fiberfox.image.compartment3.index", m_Controls->m_Compartment3Box->currentIndex());
    parameters.put("fiberfox.image.compartment4.index", m_Controls->m_Compartment4Box->currentIndex());

    parameters.put("fiberfox.image.compartment1.stick.d", m_Controls->m_StickWidget1->GetD());
    parameters.put("fiberfox.image.compartment1.stick.t2", m_Controls->m_StickWidget1->GetT2());
    parameters.put("fiberfox.image.compartment1.zeppelin.d1", m_Controls->m_ZeppelinWidget1->GetD1());
    parameters.put("fiberfox.image.compartment1.zeppelin.d2", m_Controls->m_ZeppelinWidget1->GetD2());
    parameters.put("fiberfox.image.compartment1.zeppelin.t2", m_Controls->m_ZeppelinWidget1->GetT2());
    parameters.put("fiberfox.image.compartment1.tensor.d1", m_Controls->m_TensorWidget1->GetD1());
    parameters.put("fiberfox.image.compartment1.tensor.d2", m_Controls->m_TensorWidget1->GetD2());
    parameters.put("fiberfox.image.compartment1.tensor.d3", m_Controls->m_TensorWidget1->GetD3());
    parameters.put("fiberfox.image.compartment1.tensor.t2", m_Controls->m_TensorWidget1->GetT2());

    parameters.put("fiberfox.image.compartment2.stick.d", m_Controls->m_StickWidget2->GetD());
    parameters.put("fiberfox.image.compartment2.stick.t2", m_Controls->m_StickWidget2->GetT2());
    parameters.put("fiberfox.image.compartment2.zeppelin.d1", m_Controls->m_ZeppelinWidget2->GetD1());
    parameters.put("fiberfox.image.compartment2.zeppelin.d2", m_Controls->m_ZeppelinWidget2->GetD2());
    parameters.put("fiberfox.image.compartment2.zeppelin.t2", m_Controls->m_ZeppelinWidget2->GetT2());
    parameters.put("fiberfox.image.compartment2.tensor.d1", m_Controls->m_TensorWidget2->GetD1());
    parameters.put("fiberfox.image.compartment2.tensor.d2", m_Controls->m_TensorWidget2->GetD2());
    parameters.put("fiberfox.image.compartment2.tensor.d3", m_Controls->m_TensorWidget2->GetD3());
    parameters.put("fiberfox.image.compartment2.tensor.t2", m_Controls->m_TensorWidget2->GetT2());

    parameters.put("fiberfox.image.compartment3.ball.d", m_Controls->m_BallWidget1->GetD());
    parameters.put("fiberfox.image.compartment3.ball.t2", m_Controls->m_BallWidget1->GetT2());
    parameters.put("fiberfox.image.compartment3.astrosticks.d", m_Controls->m_AstrosticksWidget1->GetD());
    parameters.put("fiberfox.image.compartment3.astrosticks.t2", m_Controls->m_AstrosticksWidget1->GetT2());
    parameters.put("fiberfox.image.compartment3.astrosticks.randomize", m_Controls->m_AstrosticksWidget1->GetRandomizeSticks());
    parameters.put("fiberfox.image.compartment3.dot.t2", m_Controls->m_DotWidget1->GetT2());

    parameters.put("fiberfox.image.compartment4.ball.d", m_Controls->m_BallWidget2->GetD());
    parameters.put("fiberfox.image.compartment4.ball.t2", m_Controls->m_BallWidget2->GetT2());
    parameters.put("fiberfox.image.compartment4.astrosticks.d", m_Controls->m_AstrosticksWidget2->GetD());
    parameters.put("fiberfox.image.compartment4.astrosticks.t2", m_Controls->m_AstrosticksWidget2->GetT2());
    parameters.put("fiberfox.image.compartment4.astrosticks.randomize", m_Controls->m_AstrosticksWidget2->GetRandomizeSticks());
    parameters.put("fiberfox.image.compartment4.dot.t2", m_Controls->m_DotWidget2->GetT2());

    parameters.put("fiberfox.image.compartment4.weight", m_Controls->m_Comp4FractionBox->value());

    boost::property_tree::xml_parser::write_xml(filename.toStdString(), parameters);
}

void QmitkFiberfoxView::LoadParameters()
{
    QString filename = QFileDialog::getOpenFileName(0, tr("Load Parameters"), QDir::currentPath(), tr("Fiberfox Parameters (*.ffp)") );
    if(filename.isEmpty() || filename.isNull())
        return;

    boost::property_tree::ptree parameters;
    boost::property_tree::xml_parser::read_xml(filename.toStdString(), parameters);

    BOOST_FOREACH( boost::property_tree::ptree::value_type const& v1, parameters.get_child("fiberfox") )
    {
        if( v1.first == "fibers" )
        {
            m_Controls->m_RealTimeFibers->setChecked(v1.second.get<bool>("realtime"));
            m_Controls->m_AdvancedOptionsBox->setChecked(v1.second.get<bool>("showadvanced"));
            m_Controls->m_DistributionBox->setCurrentIndex(v1.second.get<int>("distribution"));
            m_Controls->m_VarianceBox->setValue(v1.second.get<double>("variance"));
            m_Controls->m_FiberDensityBox->setValue(v1.second.get<int>("density"));
            m_Controls->m_IncludeFiducials->setChecked(v1.second.get<bool>("includeFiducials"));
            m_Controls->m_ConstantRadiusBox->setChecked(v1.second.get<bool>("constantradius"));

            BOOST_FOREACH( boost::property_tree::ptree::value_type const& v2, v1.second )
            {
                if( v2.first == "spline" )
                {
                    m_Controls->m_FiberSamplingBox->setValue(v2.second.get<double>("sampling"));
                    m_Controls->m_TensionBox->setValue(v2.second.get<double>("tension"));
                    m_Controls->m_ContinuityBox->setValue(v2.second.get<double>("continuity"));
                    m_Controls->m_BiasBox->setValue(v2.second.get<double>("bias"));
                }
                if( v2.first == "rotation" )
                {
                    m_Controls->m_XrotBox->setValue(v2.second.get<double>("x"));
                    m_Controls->m_YrotBox->setValue(v2.second.get<double>("y"));
                    m_Controls->m_ZrotBox->setValue(v2.second.get<double>("z"));
                }
                if( v2.first == "translation" )
                {
                    m_Controls->m_XtransBox->setValue(v2.second.get<double>("x"));
                    m_Controls->m_YtransBox->setValue(v2.second.get<double>("y"));
                    m_Controls->m_ZtransBox->setValue(v2.second.get<double>("z"));
                }
                if( v2.first == "scale" )
                {
                    m_Controls->m_XscaleBox->setValue(v2.second.get<double>("x"));
                    m_Controls->m_YscaleBox->setValue(v2.second.get<double>("y"));
                    m_Controls->m_ZscaleBox->setValue(v2.second.get<double>("z"));
                }
            }
        }
        if( v1.first == "image" )
        {
            m_Controls->m_SizeX->setValue(v1.second.get<int>("basic.size.x"));
            m_Controls->m_SizeY->setValue(v1.second.get<int>("basic.size.y"));
            m_Controls->m_SizeZ->setValue(v1.second.get<int>("basic.size.z"));
            m_Controls->m_SpacingX->setValue(v1.second.get<double>("basic.spacing.x"));
            m_Controls->m_SpacingY->setValue(v1.second.get<double>("basic.spacing.y"));
            m_Controls->m_SpacingZ->setValue(v1.second.get<double>("basic.spacing.z"));
            m_Controls->m_NumGradientsBox->setValue(v1.second.get<int>("basic.numgradients"));
            m_Controls->m_BvalueBox->setValue(v1.second.get<int>("basic.bvalue"));
            m_Controls->m_AdvancedOptionsBox_2->setChecked(v1.second.get<bool>("showadvanced"));
            m_Controls->m_RepetitionsBox->setValue(v1.second.get<int>("repetitions"));
            m_Controls->m_SignalScaleBox->setValue(v1.second.get<int>("signalScale"));
            m_Controls->m_TEbox->setValue(v1.second.get<double>("tEcho"));
            m_Controls->m_LineReadoutTimeBox->setValue(v1.second.get<double>("tLine"));
            m_Controls->m_T2starBox->setValue(v1.second.get<double>("tInhom"));
            m_Controls->m_FiberRadius->setValue(v1.second.get<double>("axonRadius"));
            m_Controls->m_InterpolationShrink->setValue(v1.second.get<int>("interpolationShrink"));
            m_Controls->m_RelaxationBox->setChecked(v1.second.get<bool>("doSimulateRelaxation"));
            m_Controls->m_EnforcePureFiberVoxelsBox->setChecked(v1.second.get<bool>("doDisablePartialVolume"));
            m_Controls->m_VolumeFractionsBox->setChecked(v1.second.get<bool>("outputvolumefractions"));

            m_Controls->m_AddNoise->setChecked(v1.second.get<bool>("artifacts.addnoise"));
            m_Controls->m_NoiseLevel->setValue(v1.second.get<double>("artifacts.noisevariance"));
            m_Controls->m_AddGhosts->setChecked(v1.second.get<bool>("artifacts.addghost"));
            m_Controls->m_kOffsetBox->setValue(v1.second.get<double>("artifacts.kspaceLineOffset"));
            m_Controls->m_AddDistortions->setChecked(v1.second.get<bool>("artifacts.distortions"));

            m_Controls->m_AddSpikes->setChecked(v1.second.get<bool>("artifacts.addspikes"));
            m_Controls->m_SpikeNumBox->setValue(v1.second.get<int>("artifacts.spikesnum"));
            m_Controls->m_SpikeScaleBox->setValue(v1.second.get<double>("artifacts.spikesscale"));
            m_Controls->m_AddEddy->setChecked(v1.second.get<bool>("artifacts.addeddy"));
            m_Controls->m_EddyGradientStrength->setValue(v1.second.get<double>("artifacts.eddyStrength"));
            m_Controls->m_AddGibbsRinging->setChecked(v1.second.get<bool>("artifacts.addringing"));
            m_Controls->m_ImageUpsamplingBox->setValue(v1.second.get<double>("artifacts.ringingupsampling"));

            m_Controls->m_Compartment1Box->setCurrentIndex(v1.second.get<int>("compartment1.index"));
            m_Controls->m_Compartment2Box->setCurrentIndex(v1.second.get<int>("compartment2.index"));
            m_Controls->m_Compartment3Box->setCurrentIndex(v1.second.get<int>("compartment3.index"));
            m_Controls->m_Compartment4Box->setCurrentIndex(v1.second.get<int>("compartment4.index"));


            m_Controls->m_StickWidget1->SetD(v1.second.get<double>("compartment1.stick.d"));
            m_Controls->m_StickWidget1->SetT2(v1.second.get<double>("compartment1.stick.t2"));
            m_Controls->m_ZeppelinWidget1->SetD1(v1.second.get<double>("compartment1.zeppelin.d1"));
            m_Controls->m_ZeppelinWidget1->SetD2(v1.second.get<double>("compartment1.zeppelin.d2"));
            m_Controls->m_ZeppelinWidget1->SetT2(v1.second.get<double>("compartment1.zeppelin.t2"));
            m_Controls->m_TensorWidget1->SetD1(v1.second.get<double>("compartment1.tensor.d1"));
            m_Controls->m_TensorWidget1->SetD2(v1.second.get<double>("compartment1.tensor.d2"));
            m_Controls->m_TensorWidget1->SetD3(v1.second.get<double>("compartment1.tensor.d3"));
            m_Controls->m_TensorWidget1->SetT2(v1.second.get<double>("compartment1.tensor.t2"));

            m_Controls->m_StickWidget2->SetD(v1.second.get<double>("compartment2.stick.d"));
            m_Controls->m_StickWidget2->SetT2(v1.second.get<double>("compartment2.stick.t2"));
            m_Controls->m_ZeppelinWidget2->SetD1(v1.second.get<double>("compartment2.zeppelin.d1"));
            m_Controls->m_ZeppelinWidget2->SetD2(v1.second.get<double>("compartment2.zeppelin.d2"));
            m_Controls->m_ZeppelinWidget2->SetT2(v1.second.get<double>("compartment2.zeppelin.t2"));
            m_Controls->m_TensorWidget2->SetD1(v1.second.get<double>("compartment2.tensor.d1"));
            m_Controls->m_TensorWidget2->SetD2(v1.second.get<double>("compartment2.tensor.d2"));
            m_Controls->m_TensorWidget2->SetD3(v1.second.get<double>("compartment2.tensor.d3"));
            m_Controls->m_TensorWidget2->SetT2(v1.second.get<double>("compartment2.tensor.t2"));

            m_Controls->m_BallWidget1->SetD(v1.second.get<double>("compartment3.ball.d"));
            m_Controls->m_BallWidget1->SetT2(v1.second.get<double>("compartment3.ball.t2"));
            m_Controls->m_AstrosticksWidget1->SetD(v1.second.get<double>("compartment3.astrosticks.d"));
            m_Controls->m_AstrosticksWidget1->SetT2(v1.second.get<double>("compartment3.astrosticks.t2"));
            m_Controls->m_AstrosticksWidget1->SetRandomizeSticks(v1.second.get<bool>("compartment3.astrosticks.randomize"));
            m_Controls->m_DotWidget1->SetT2(v1.second.get<double>("compartment3.dot.t2"));

            m_Controls->m_BallWidget2->SetD(v1.second.get<double>("compartment4.ball.d"));
            m_Controls->m_BallWidget2->SetT2(v1.second.get<double>("compartment4.ball.t2"));
            m_Controls->m_AstrosticksWidget2->SetD(v1.second.get<double>("compartment4.astrosticks.d"));
            m_Controls->m_AstrosticksWidget2->SetT2(v1.second.get<double>("compartment4.astrosticks.t2"));
            m_Controls->m_AstrosticksWidget2->SetRandomizeSticks(v1.second.get<bool>("compartment4.astrosticks.randomize"));
            m_Controls->m_DotWidget2->SetT2(v1.second.get<double>("compartment4.dot.t2"));

            m_Controls->m_Comp4FractionBox->setValue(v1.second.get<double>("compartment4.weight"));
        }
    }
    UpdateImageParameters();
}

void QmitkFiberfoxView::ShowAdvancedOptions(int state)
{
    if (state)
    {
        m_Controls->m_AdvancedFiberOptionsFrame->setVisible(true);
        m_Controls->m_AdvancedSignalOptionsFrame->setVisible(true);
        m_Controls->m_AdvancedOptionsBox->setChecked(true);
        m_Controls->m_AdvancedOptionsBox_2->setChecked(true);
    }
    else
    {
        m_Controls->m_AdvancedFiberOptionsFrame->setVisible(false);
        m_Controls->m_AdvancedSignalOptionsFrame->setVisible(false);
        m_Controls->m_AdvancedOptionsBox->setChecked(false);
        m_Controls->m_AdvancedOptionsBox_2->setChecked(false);
    }
}

void QmitkFiberfoxView::Comp1ModelFrameVisibility(int index)
{
    m_Controls->m_StickWidget1->setVisible(false);
    m_Controls->m_ZeppelinWidget1->setVisible(false);
    m_Controls->m_TensorWidget1->setVisible(false);

    switch (index)
    {
    case 0:
        m_Controls->m_StickWidget1->setVisible(true);
        break;
    case 1:
        m_Controls->m_ZeppelinWidget1->setVisible(true);
        break;
    case 2:
        m_Controls->m_TensorWidget1->setVisible(true);
        break;
    }
}

void QmitkFiberfoxView::Comp2ModelFrameVisibility(int index)
{
    m_Controls->m_StickWidget2->setVisible(false);
    m_Controls->m_ZeppelinWidget2->setVisible(false);
    m_Controls->m_TensorWidget2->setVisible(false);

    switch (index)
    {
    case 0:
        break;
    case 1:
        m_Controls->m_StickWidget2->setVisible(true);
        break;
    case 2:
        m_Controls->m_ZeppelinWidget2->setVisible(true);
        break;
    case 3:
        m_Controls->m_TensorWidget2->setVisible(true);
        break;
    }
}

void QmitkFiberfoxView::Comp3ModelFrameVisibility(int index)
{
    m_Controls->m_BallWidget1->setVisible(false);
    m_Controls->m_AstrosticksWidget1->setVisible(false);
    m_Controls->m_DotWidget1->setVisible(false);

    switch (index)
    {
    case 0:
        m_Controls->m_BallWidget1->setVisible(true);
        break;
    case 1:
        m_Controls->m_AstrosticksWidget1->setVisible(true);
        break;
    case 2:
        m_Controls->m_DotWidget1->setVisible(true);
        break;
    }
}

void QmitkFiberfoxView::Comp4ModelFrameVisibility(int index)
{
    m_Controls->m_BallWidget2->setVisible(false);
    m_Controls->m_AstrosticksWidget2->setVisible(false);
    m_Controls->m_DotWidget2->setVisible(false);
    m_Controls->m_Comp4FractionFrame->setVisible(false);

    switch (index)
    {
    case 0:
        break;
    case 1:
        m_Controls->m_BallWidget2->setVisible(true);
        m_Controls->m_Comp4FractionFrame->setVisible(true);
        break;
    case 2:
        m_Controls->m_AstrosticksWidget2->setVisible(true);
        m_Controls->m_Comp4FractionFrame->setVisible(true);
        break;
    case 3:
        m_Controls->m_DotWidget2->setVisible(true);
        m_Controls->m_Comp4FractionFrame->setVisible(true);
        break;
    }
}

void QmitkFiberfoxView::OnConstantRadius(int value)
{
    if (value>0 && m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnAddSpikes(int value)
{
    if (value>0)
        m_Controls->m_SpikeFrame->setVisible(true);
    else
        m_Controls->m_SpikeFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddEddy(int value)
{
    if (value>0)
        m_Controls->m_EddyFrame->setVisible(true);
    else
        m_Controls->m_EddyFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddDistortions(int value)
{
    if (value>0)
        m_Controls->m_DistortionsFrame->setVisible(true);
    else
        m_Controls->m_DistortionsFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddGhosts(int value)
{
    if (value>0)
        m_Controls->m_GhostFrame->setVisible(true);
    else
        m_Controls->m_GhostFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddNoise(int value)
{
    if (value>0)
        m_Controls->m_NoiseFrame->setVisible(true);
    else
        m_Controls->m_NoiseFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddGibbsRinging(int value)
{
    if (value>0)
        m_Controls->m_GibbsRingingFrame->setVisible(true);
    else
        m_Controls->m_GibbsRingingFrame->setVisible(false);
}

void QmitkFiberfoxView::OnDistributionChanged(int value)
{
    if (value==1)
        m_Controls->m_VarianceBox->setVisible(true);
    else
        m_Controls->m_VarianceBox->setVisible(false);

    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnVarianceChanged(double value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnFiberDensityChanged(int value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnFiberSamplingChanged(double value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnTensionChanged(double value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnContinuityChanged(double value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnBiasChanged(double value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::AlignOnGrid()
{
    for (int i=0; i<m_SelectedFiducials.size(); i++)
    {
        mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(m_SelectedFiducials.at(i)->GetData());
        mitk::Point3D wc0 = pe->GetWorldControlPoint(0);

        mitk::DataStorage::SetOfObjects::ConstPointer parentFibs = GetDataStorage()->GetSources(m_SelectedFiducials.at(i));
        for( mitk::DataStorage::SetOfObjects::const_iterator it = parentFibs->begin(); it != parentFibs->end(); ++it )
        {
            mitk::DataNode::Pointer pFibNode = *it;
            if ( pFibNode.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(pFibNode->GetData()) )
            {
                mitk::DataStorage::SetOfObjects::ConstPointer parentImgs = GetDataStorage()->GetSources(pFibNode);
                for( mitk::DataStorage::SetOfObjects::const_iterator it2 = parentImgs->begin(); it2 != parentImgs->end(); ++it2 )
                {
                    mitk::DataNode::Pointer pImgNode = *it2;
                    if ( pImgNode.IsNotNull() && dynamic_cast<mitk::Image*>(pImgNode->GetData()) )
                    {
                        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(pImgNode->GetData());
                        mitk::Geometry3D::Pointer geom = img->GetGeometry();
                        itk::Index<3> idx;
                        geom->WorldToIndex(wc0, idx);

                        mitk::Point3D cIdx; cIdx[0]=idx[0]; cIdx[1]=idx[1]; cIdx[2]=idx[2];
                        mitk::Point3D world;
                        geom->IndexToWorld(cIdx,world);

                        mitk::Vector3D trans = world - wc0;
                        pe->GetGeometry()->Translate(trans);

                        break;
                    }
                }
                break;
            }
        }
    }

    for( int i=0; i<m_SelectedBundles2.size(); i++ )
    {
        mitk::DataNode::Pointer fibNode = m_SelectedBundles2.at(i);

        mitk::DataStorage::SetOfObjects::ConstPointer sources = GetDataStorage()->GetSources(fibNode);
        for( mitk::DataStorage::SetOfObjects::const_iterator it = sources->begin(); it != sources->end(); ++it )
        {
            mitk::DataNode::Pointer imgNode = *it;
            if ( imgNode.IsNotNull() && dynamic_cast<mitk::Image*>(imgNode->GetData()) )
            {
                mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(fibNode);
                for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations->begin(); it2 != derivations->end(); ++it2 )
                {
                    mitk::DataNode::Pointer fiducialNode = *it2;
                    if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
                    {
                        mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData());
                        mitk::Point3D wc0 = pe->GetWorldControlPoint(0);

                        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(imgNode->GetData());
                        mitk::Geometry3D::Pointer geom = img->GetGeometry();
                        itk::Index<3> idx;
                        geom->WorldToIndex(wc0, idx);
                        mitk::Point3D cIdx; cIdx[0]=idx[0]; cIdx[1]=idx[1]; cIdx[2]=idx[2];
                        mitk::Point3D world;
                        geom->IndexToWorld(cIdx,world);

                        mitk::Vector3D trans = world - wc0;
                        pe->GetGeometry()->Translate(trans);
                    }
                }

                break;
            }
        }
    }

    for( int i=0; i<m_SelectedImages.size(); i++ )
    {
        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_SelectedImages.at(i)->GetData());

        mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(m_SelectedImages.at(i));
        for( mitk::DataStorage::SetOfObjects::const_iterator it = derivations->begin(); it != derivations->end(); ++it )
        {
            mitk::DataNode::Pointer fibNode = *it;
            if ( fibNode.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(fibNode->GetData()) )
            {
                mitk::DataStorage::SetOfObjects::ConstPointer derivations2 = GetDataStorage()->GetDerivations(fibNode);
                for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations2->begin(); it2 != derivations2->end(); ++it2 )
                {
                    mitk::DataNode::Pointer fiducialNode = *it2;
                    if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
                    {
                        mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData());
                        mitk::Point3D wc0 = pe->GetWorldControlPoint(0);

                        mitk::Geometry3D::Pointer geom = img->GetGeometry();
                        itk::Index<3> idx;
                        geom->WorldToIndex(wc0, idx);
                        mitk::Point3D cIdx; cIdx[0]=idx[0]; cIdx[1]=idx[1]; cIdx[2]=idx[2];
                        mitk::Point3D world;
                        geom->IndexToWorld(cIdx,world);

                        mitk::Vector3D trans = world - wc0;
                        pe->GetGeometry()->Translate(trans);
                    }
                }
            }
        }
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnFlipButton()
{
    if (m_SelectedFiducial.IsNull())
        return;

    std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(m_SelectedFiducial.GetPointer());
    if( it != m_DataNodeToPlanarFigureData.end() )
    {
        QmitkPlanarFigureData& data = it->second;
        data.m_Flipped += 1;
        data.m_Flipped %= 2;
    }

    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

QmitkFiberfoxView::GradientListType QmitkFiberfoxView::GenerateHalfShell(int NPoints)
{
    NPoints *= 2;
    GradientListType pointshell;

    int numB0 = NPoints/20;
    if (numB0==0)
        numB0=1;
    GradientType g;
    g.Fill(0.0);
    for (int i=0; i<numB0; i++)
        pointshell.push_back(g);

    if (NPoints==0)
        return pointshell;

    vnl_vector<double> theta; theta.set_size(NPoints);
    vnl_vector<double> phi; phi.set_size(NPoints);
    double C = sqrt(4*M_PI);
    phi(0) = 0.0;
    phi(NPoints-1) = 0.0;
    for(int i=0; i<NPoints; i++)
    {
        theta(i) = acos(-1.0+2.0*i/(NPoints-1.0)) - M_PI / 2.0;
        if( i>0 && i<NPoints-1)
        {
            phi(i) = (phi(i-1) + C /
                      sqrt(NPoints*(1-(-1.0+2.0*i/(NPoints-1.0))*(-1.0+2.0*i/(NPoints-1.0)))));
            // % (2*DIST_POINTSHELL_PI);
        }
    }

    for(int i=0; i<NPoints; i++)
    {
        g[2] = sin(theta(i));
        if (g[2]<0)
            continue;
        g[0] = cos(theta(i)) * cos(phi(i));
        g[1] = cos(theta(i)) * sin(phi(i));
        pointshell.push_back(g);
    }

    return pointshell;
}

template<int ndirs>
std::vector<itk::Vector<double,3> > QmitkFiberfoxView::MakeGradientList()
{
    std::vector<itk::Vector<double,3> > retval;
    vnl_matrix_fixed<double, 3, ndirs>* U =
            itk::PointShell<ndirs, vnl_matrix_fixed<double, 3, ndirs> >::DistributePointShell();


    // Add 0 vector for B0
    int numB0 = ndirs/10;
    if (numB0==0)
        numB0=1;
    itk::Vector<double,3> v;
    v.Fill(0.0);
    for (int i=0; i<numB0; i++)
    {
        retval.push_back(v);
    }

    for(int i=0; i<ndirs;i++)
    {
        itk::Vector<double,3> v;
        v[0] = U->get(0,i); v[1] = U->get(1,i); v[2] = U->get(2,i);
        retval.push_back(v);
    }

    return retval;
}

void QmitkFiberfoxView::OnAddBundle()
{
    if (m_SelectedImage.IsNull())
        return;

    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedImage);

    mitk::FiberBundleX::Pointer bundle = mitk::FiberBundleX::New();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( bundle );
    QString name = QString("Bundle_%1").arg(children->size());
    node->SetName(name.toStdString());
    m_SelectedBundles.push_back(node);
    UpdateGui();

    GetDataStorage()->Add(node, m_SelectedImage);
}

void QmitkFiberfoxView::OnDrawROI()
{
    if (m_SelectedBundles.empty())
        OnAddBundle();
    if (m_SelectedBundles.empty())
        return;

    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundles.at(0));

    mitk::PlanarEllipse::Pointer figure = mitk::PlanarEllipse::New();

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( figure );

    QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
    for( int i=0; i<nodes.size(); i++)
        nodes.at(i)->SetSelected(false);

    m_SelectedFiducial = node;

    QString name = QString("Fiducial_%1").arg(children->size());
    node->SetName(name.toStdString());
    node->SetSelected(true);

    this->DisableCrosshairNavigation();

    mitk::PlanarFigureInteractor::Pointer figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetDataInteractor().GetPointer());
    if(figureInteractor.IsNull())
    {
      figureInteractor = mitk::PlanarFigureInteractor::New();
      us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "PlanarFigure" );
      figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
      figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
      figureInteractor->SetDataNode( node );
    }

    UpdateGui();
    GetDataStorage()->Add(node, m_SelectedBundles.at(0));
}

bool CompareLayer(mitk::DataNode::Pointer i,mitk::DataNode::Pointer j)
{
    int li = -1;
    i->GetPropertyValue("layer", li);
    int lj = -1;
    j->GetPropertyValue("layer", lj);
    return li<lj;
}

void QmitkFiberfoxView::GenerateFibers()
{
    if (m_SelectedBundles.empty())
    {
        if (m_SelectedFiducial.IsNull())
            return;

        mitk::DataStorage::SetOfObjects::ConstPointer parents = GetDataStorage()->GetSources(m_SelectedFiducial);
        for( mitk::DataStorage::SetOfObjects::const_iterator it = parents->begin(); it != parents->end(); ++it )
            if(dynamic_cast<mitk::FiberBundleX*>((*it)->GetData()))
                m_SelectedBundles.push_back(*it);

        if (m_SelectedBundles.empty())
            return;
    }

    vector< vector< mitk::PlanarEllipse::Pointer > > fiducials;
    vector< vector< unsigned int > > fliplist;
    for (int i=0; i<m_SelectedBundles.size(); i++)
    {
        mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundles.at(i));
        std::vector< mitk::DataNode::Pointer > childVector;
        for( mitk::DataStorage::SetOfObjects::const_iterator it = children->begin(); it != children->end(); ++it )
            childVector.push_back(*it);
        sort(childVector.begin(), childVector.end(), CompareLayer);

        vector< mitk::PlanarEllipse::Pointer > fib;
        vector< unsigned int > flip;
        float radius = 1;
        int count = 0;
        for( std::vector< mitk::DataNode::Pointer >::const_iterator it = childVector.begin(); it != childVector.end(); ++it )
        {
            mitk::DataNode::Pointer node = *it;

            if ( node.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(node->GetData()) )
            {
                mitk::PlanarEllipse* ellipse = dynamic_cast<mitk::PlanarEllipse*>(node->GetData());
                if (m_Controls->m_ConstantRadiusBox->isChecked())
                {
                    ellipse->SetTreatAsCircle(true);
                    mitk::Point2D c = ellipse->GetControlPoint(0);
                    mitk::Point2D p = ellipse->GetControlPoint(1);
                    mitk::Vector2D v = p-c;
                    if (count==0)
                    {
                        radius = v.GetVnlVector().magnitude();
                        ellipse->SetControlPoint(1, p);
                    }
                    else
                    {
                        v.Normalize();
                        v *= radius;
                        ellipse->SetControlPoint(1, c+v);
                    }
                }
                fib.push_back(ellipse);

                std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(node.GetPointer());
                if( it != m_DataNodeToPlanarFigureData.end() )
                {
                    QmitkPlanarFigureData& data = it->second;
                    flip.push_back(data.m_Flipped);
                }
                else
                    flip.push_back(0);
            }
            count++;
        }
        if (fib.size()>1)
        {
            fiducials.push_back(fib);
            fliplist.push_back(flip);
        }
        else if (fib.size()>0)
            m_SelectedBundles.at(i)->SetData( mitk::FiberBundleX::New() );

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    itk::FibersFromPlanarFiguresFilter::Pointer filter = itk::FibersFromPlanarFiguresFilter::New();
    filter->SetFiducials(fiducials);
    filter->SetFlipList(fliplist);

    switch(m_Controls->m_DistributionBox->currentIndex()){
    case 0:
        filter->SetFiberDistribution(itk::FibersFromPlanarFiguresFilter::DISTRIBUTE_UNIFORM);
        break;
    case 1:
        filter->SetFiberDistribution(itk::FibersFromPlanarFiguresFilter::DISTRIBUTE_GAUSSIAN);
        filter->SetVariance(m_Controls->m_VarianceBox->value());
        break;
    }

    filter->SetDensity(m_Controls->m_FiberDensityBox->value());
    filter->SetTension(m_Controls->m_TensionBox->value());
    filter->SetContinuity(m_Controls->m_ContinuityBox->value());
    filter->SetBias(m_Controls->m_BiasBox->value());
    filter->SetFiberSampling(m_Controls->m_FiberSamplingBox->value());
    filter->Update();
    vector< mitk::FiberBundleX::Pointer > fiberBundles = filter->GetFiberBundles();

    for (int i=0; i<fiberBundles.size(); i++)
    {
        m_SelectedBundles.at(i)->SetData( fiberBundles.at(i) );
        if (fiberBundles.at(i)->GetNumFibers()>50000)
            m_SelectedBundles.at(i)->SetVisibility(false);
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::GenerateImage()
{
    UpdateImageParameters();
    if (m_SelectedBundles.empty())
    {
        if (m_SelectedDWI.IsNotNull()) // add artifacts to existing diffusion weighted image
        {
            for (int i=0; i<m_SelectedImages.size(); i++)
            {
                if (!dynamic_cast<mitk::DiffusionImage<short>*>(m_SelectedImages.at(i)->GetData()))
                    continue;

                m_SelectedDWI = m_SelectedImages.at(i);
                UpdateImageParameters();

                mitk::DiffusionImage<short>::Pointer diffImg = dynamic_cast<mitk::DiffusionImage<short>*>(m_SelectedImages.at(i)->GetData());

                mitk::RicianNoiseModel<short> noiseModel;
                noiseModel.SetNoiseVariance(m_ImageGenParameters.ricianNoiseModel.GetNoiseVariance());

                itk::AddArtifactsToDwiImageFilter< short >::Pointer filter = itk::AddArtifactsToDwiImageFilter< short >::New();
                filter->SetInput(diffImg->GetVectorImage());
                filter->SettLine(m_ImageGenParameters.tLine);
                filter->SetkOffset(m_ImageGenParameters.kspaceLineOffset);
                filter->SetNoiseModel(&noiseModel);
                filter->SetGradientList(m_ImageGenParameters.gradientDirections);
                filter->SetTE(m_ImageGenParameters.tEcho);
                filter->SetSimulateEddyCurrents(m_ImageGenParameters.doSimulateEddyCurrents);
                filter->SetEddyGradientStrength(m_ImageGenParameters.eddyStrength);
                filter->SetUpsampling(m_ImageGenParameters.upsampling);
                filter->SetFrequencyMap(m_ImageGenParameters.frequencyMap);
                filter->SetSpikeAmplitude(m_ImageGenParameters.spikeAmplitude);
                filter->SetSpikes(m_ImageGenParameters.spikes);
                filter->Update();

                mitk::DiffusionImage<short>::Pointer image = mitk::DiffusionImage<short>::New();
                image->SetVectorImage( filter->GetOutput() );
                image->SetB_Value(diffImg->GetB_Value());
                image->SetDirections(diffImg->GetDirections());
                image->InitializeFromVectorImage();
                m_ImageGenParameters.resultNode->SetData( image );
                m_ImageGenParameters.resultNode->SetName(m_SelectedImages.at(i)->GetName()+m_ImageGenParameters.artifactModelString.toStdString());
                GetDataStorage()->Add(m_ImageGenParameters.resultNode);
            }
            m_SelectedDWI = m_SelectedImages.front();
            return;
        }

        mitk::Image::Pointer image = mitk::ImageGenerator::GenerateGradientImage<unsigned int>(
                    m_Controls->m_SizeX->value(),
                    m_Controls->m_SizeY->value(),
                    m_Controls->m_SizeZ->value(),
                    m_Controls->m_SpacingX->value(),
                    m_Controls->m_SpacingY->value(),
                    m_Controls->m_SpacingZ->value());

        mitk::Geometry3D* geom = image->GetGeometry();
        geom->SetOrigin(m_ImageGenParameters.imageOrigin);

        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData( image );
        node->SetName("Dummy");
        unsigned int window = m_Controls->m_SizeX->value()*m_Controls->m_SizeY->value()*m_Controls->m_SizeZ->value();
        unsigned int level = window/2;
        mitk::LevelWindow lw; lw.SetLevelWindow(level, window);
        node->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( lw ) );
        GetDataStorage()->Add(node);
        m_SelectedImage = node;

        mitk::BaseData::Pointer basedata = node->GetData();
        if (basedata.IsNotNull())
        {
            mitk::RenderingManager::GetInstance()->InitializeViews( basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
        UpdateGui();

        return;
    }

    for (int i=0; i<m_SelectedBundles.size(); i++)
    {
        mitk::FiberBundleX::Pointer fiberBundle = dynamic_cast<mitk::FiberBundleX*>(m_SelectedBundles.at(i)->GetData());
        if (fiberBundle->GetNumFibers()<=0)
            continue;

        itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();

        tractsToDwiFilter->SetSimulateEddyCurrents(m_ImageGenParameters.doSimulateEddyCurrents);
        tractsToDwiFilter->SetEddyGradientStrength(m_ImageGenParameters.eddyStrength);
        tractsToDwiFilter->SetUpsampling(m_ImageGenParameters.upsampling);
        tractsToDwiFilter->SetSimulateRelaxation(m_ImageGenParameters.doSimulateRelaxation);
        tractsToDwiFilter->SetImageRegion(m_ImageGenParameters.imageRegion);
        tractsToDwiFilter->SetSpacing(m_ImageGenParameters.imageSpacing);
        tractsToDwiFilter->SetOrigin(m_ImageGenParameters.imageOrigin);
        tractsToDwiFilter->SetDirectionMatrix(m_ImageGenParameters.imageDirection);
        tractsToDwiFilter->SetFiberBundle(fiberBundle);
        tractsToDwiFilter->SetFiberModels(m_ImageGenParameters.fiberModelList);
        tractsToDwiFilter->SetNonFiberModels(m_ImageGenParameters.nonFiberModelList);
        tractsToDwiFilter->SetNoiseModel(&m_ImageGenParameters.ricianNoiseModel);
        tractsToDwiFilter->SetKspaceArtifacts(m_ImageGenParameters.artifactList);
        tractsToDwiFilter->SetkOffset(m_ImageGenParameters.kspaceLineOffset);
        tractsToDwiFilter->SettLine(m_ImageGenParameters.tLine);
        tractsToDwiFilter->SettInhom(m_ImageGenParameters.tInhom);
        tractsToDwiFilter->SetTE(m_ImageGenParameters.tEcho);
        tractsToDwiFilter->SetNumberOfRepetitions(m_ImageGenParameters.repetitions);
        tractsToDwiFilter->SetEnforcePureFiberVoxels(m_ImageGenParameters.doDisablePartialVolume);
        tractsToDwiFilter->SetInterpolationShrink(m_ImageGenParameters.interpolationShrink);
        tractsToDwiFilter->SetFiberRadius(m_ImageGenParameters.axonRadius);
        tractsToDwiFilter->SetSignalScale(m_ImageGenParameters.signalScale);
        if (m_ImageGenParameters.interpolationShrink>0)
            tractsToDwiFilter->SetUseInterpolation(true);
        tractsToDwiFilter->SetTissueMask(m_ImageGenParameters.tissueMaskImage);
        tractsToDwiFilter->SetFrequencyMap(m_ImageGenParameters.frequencyMap);
        tractsToDwiFilter->SetSpikeAmplitude(m_ImageGenParameters.spikeAmplitude);
        tractsToDwiFilter->SetSpikes(m_ImageGenParameters.spikes);
        tractsToDwiFilter->Update();

        mitk::DiffusionImage<short>::Pointer image = mitk::DiffusionImage<short>::New();
        image->SetVectorImage( tractsToDwiFilter->GetOutput() );
        image->SetB_Value(m_ImageGenParameters.b_value);
        image->SetDirections(m_ImageGenParameters.gradientDirections);
        image->InitializeFromVectorImage();
        m_ImageGenParameters.resultNode->SetData( image );
        m_ImageGenParameters.resultNode->SetName(m_SelectedBundles.at(i)->GetName()
                                                 +"_D"+QString::number(m_ImageGenParameters.imageRegion.GetSize(0)).toStdString()
                                                 +"-"+QString::number(m_ImageGenParameters.imageRegion.GetSize(1)).toStdString()
                                                 +"-"+QString::number(m_ImageGenParameters.imageRegion.GetSize(2)).toStdString()
                                                 +"_S"+QString::number(m_ImageGenParameters.imageSpacing[0]).toStdString()
                                                 +"-"+QString::number(m_ImageGenParameters.imageSpacing[1]).toStdString()
                                                 +"-"+QString::number(m_ImageGenParameters.imageSpacing[2]).toStdString()
                                                 +"_b"+QString::number(m_ImageGenParameters.b_value).toStdString()
                                                 +"_"+m_ImageGenParameters.signalModelString.toStdString()
                                                 +m_ImageGenParameters.artifactModelString.toStdString());

        GetDataStorage()->Add(m_ImageGenParameters.resultNode, m_SelectedBundles.at(i));

        m_ImageGenParameters.resultNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New(tractsToDwiFilter->GetLevelWindow()) );

        if (m_Controls->m_VolumeFractionsBox->isChecked())
        {
            std::vector< itk::TractsToDWIImageFilter< short >::ItkDoubleImgType::Pointer > volumeFractions = tractsToDwiFilter->GetVolumeFractions();
            for (int k=0; k<volumeFractions.size(); k++)
            {
                mitk::Image::Pointer image = mitk::Image::New();
                image->InitializeByItk(volumeFractions.at(k).GetPointer());
                image->SetVolume(volumeFractions.at(k)->GetBufferPointer());

                mitk::DataNode::Pointer node = mitk::DataNode::New();
                node->SetData( image );
                node->SetName(m_SelectedBundles.at(i)->GetName()+"_CompartmentVolume-"+QString::number(k).toStdString());
                GetDataStorage()->Add(node, m_SelectedBundles.at(i));
            }
        }

        mitk::BaseData::Pointer basedata = m_ImageGenParameters.resultNode->GetData();
        if (basedata.IsNotNull())
        {
            mitk::RenderingManager::GetInstance()->InitializeViews(
                        basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }
}

void QmitkFiberfoxView::ApplyTransform()
{
    vector< mitk::DataNode::Pointer > selectedBundles;
    for( int i=0; i<m_SelectedImages.size(); i++ )
    {
        mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(m_SelectedImages.at(i));
        for( mitk::DataStorage::SetOfObjects::const_iterator it = derivations->begin(); it != derivations->end(); ++it )
        {
            mitk::DataNode::Pointer fibNode = *it;
            if ( fibNode.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(fibNode->GetData()) )
                selectedBundles.push_back(fibNode);
        }
    }
    if (selectedBundles.empty())
        selectedBundles = m_SelectedBundles2;

    if (!selectedBundles.empty())
    {
        std::vector<mitk::DataNode::Pointer>::const_iterator it = selectedBundles.begin();
        for (it; it!=selectedBundles.end(); ++it)
        {
            mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
            fib->RotateAroundAxis(m_Controls->m_XrotBox->value(), m_Controls->m_YrotBox->value(), m_Controls->m_ZrotBox->value());
            fib->TranslateFibers(m_Controls->m_XtransBox->value(), m_Controls->m_YtransBox->value(), m_Controls->m_ZtransBox->value());
            fib->ScaleFibers(m_Controls->m_XscaleBox->value(), m_Controls->m_YscaleBox->value(), m_Controls->m_ZscaleBox->value());

            // handle child fiducials
            if (m_Controls->m_IncludeFiducials->isChecked())
            {
                mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(*it);
                for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations->begin(); it2 != derivations->end(); ++it2 )
                {
                    mitk::DataNode::Pointer fiducialNode = *it2;
                    if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
                    {
                        mitk::PlanarEllipse* pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData());
                        mitk::Geometry3D* geom = pe->GetGeometry();

                        // translate
                        mitk::Vector3D world;
                        world[0] = m_Controls->m_XtransBox->value();
                        world[1] = m_Controls->m_YtransBox->value();
                        world[2] = m_Controls->m_ZtransBox->value();
                        geom->Translate(world);

                        // calculate rotation matrix
                        double x = m_Controls->m_XrotBox->value()*M_PI/180;
                        double y = m_Controls->m_YrotBox->value()*M_PI/180;
                        double z = m_Controls->m_ZrotBox->value()*M_PI/180;

                        itk::Matrix< float, 3, 3 > rotX; rotX.SetIdentity();
                        rotX[1][1] = cos(x);
                        rotX[2][2] = rotX[1][1];
                        rotX[1][2] = -sin(x);
                        rotX[2][1] = -rotX[1][2];

                        itk::Matrix< float, 3, 3 > rotY; rotY.SetIdentity();
                        rotY[0][0] = cos(y);
                        rotY[2][2] = rotY[0][0];
                        rotY[0][2] = sin(y);
                        rotY[2][0] = -rotY[0][2];

                        itk::Matrix< float, 3, 3 > rotZ; rotZ.SetIdentity();
                        rotZ[0][0] = cos(z);
                        rotZ[1][1] = rotZ[0][0];
                        rotZ[0][1] = -sin(z);
                        rotZ[1][0] = -rotZ[0][1];

                        itk::Matrix< float, 3, 3 > rot = rotZ*rotY*rotX;

                        // transform control point coordinate into geometry translation
                        geom->SetOrigin(pe->GetWorldControlPoint(0));
                        mitk::Point2D cp; cp.Fill(0.0);
                        pe->SetControlPoint(0, cp);

                        // rotate fiducial
                        geom->GetIndexToWorldTransform()->SetMatrix(rot*geom->GetIndexToWorldTransform()->GetMatrix());

                        // implicit translation
                        mitk::Vector3D trans;
                        trans[0] = geom->GetOrigin()[0]-fib->GetGeometry()->GetCenter()[0];
                        trans[1] = geom->GetOrigin()[1]-fib->GetGeometry()->GetCenter()[1];
                        trans[2] = geom->GetOrigin()[2]-fib->GetGeometry()->GetCenter()[2];
                        mitk::Vector3D newWc = rot*trans;
                        newWc = newWc-trans;
                        geom->Translate(newWc);
                    }
                }
            }
        }
    }
    else
    {
        for (int i=0; i<m_SelectedFiducials.size(); i++)
        {
            mitk::PlanarEllipse* pe = dynamic_cast<mitk::PlanarEllipse*>(m_SelectedFiducials.at(i)->GetData());
            mitk::Geometry3D* geom = pe->GetGeometry();

            // translate
            mitk::Vector3D world;
            world[0] = m_Controls->m_XtransBox->value();
            world[1] = m_Controls->m_YtransBox->value();
            world[2] = m_Controls->m_ZtransBox->value();
            geom->Translate(world);

            // calculate rotation matrix
            double x = m_Controls->m_XrotBox->value()*M_PI/180;
            double y = m_Controls->m_YrotBox->value()*M_PI/180;
            double z = m_Controls->m_ZrotBox->value()*M_PI/180;
            itk::Matrix< float, 3, 3 > rotX; rotX.SetIdentity();
            rotX[1][1] = cos(x);
            rotX[2][2] = rotX[1][1];
            rotX[1][2] = -sin(x);
            rotX[2][1] = -rotX[1][2];
            itk::Matrix< float, 3, 3 > rotY; rotY.SetIdentity();
            rotY[0][0] = cos(y);
            rotY[2][2] = rotY[0][0];
            rotY[0][2] = sin(y);
            rotY[2][0] = -rotY[0][2];
            itk::Matrix< float, 3, 3 > rotZ; rotZ.SetIdentity();
            rotZ[0][0] = cos(z);
            rotZ[1][1] = rotZ[0][0];
            rotZ[0][1] = -sin(z);
            rotZ[1][0] = -rotZ[0][1];
            itk::Matrix< float, 3, 3 > rot = rotZ*rotY*rotX;

            // transform control point coordinate into geometry translation
            geom->SetOrigin(pe->GetWorldControlPoint(0));
            mitk::Point2D cp; cp.Fill(0.0);
            pe->SetControlPoint(0, cp);

            // rotate fiducial
            geom->GetIndexToWorldTransform()->SetMatrix(rot*geom->GetIndexToWorldTransform()->GetMatrix());
        }
        if (m_Controls->m_RealTimeFibers->isChecked())
            GenerateFibers();
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::CopyBundles()
{
    if ( m_SelectedBundles.size()<1 ){
        QMessageBox::information( NULL, "Warning", "Select at least one fiber bundle!");
        MITK_WARN("QmitkFiberProcessingView") << "Select at least one fiber bundle!";
        return;
    }

    std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedBundles.begin();
    for (it; it!=m_SelectedBundles.end(); ++it)
    {
        // find parent image
        mitk::DataNode::Pointer parentNode;
        mitk::DataStorage::SetOfObjects::ConstPointer parentImgs = GetDataStorage()->GetSources(*it);
        for( mitk::DataStorage::SetOfObjects::const_iterator it2 = parentImgs->begin(); it2 != parentImgs->end(); ++it2 )
        {
            mitk::DataNode::Pointer pImgNode = *it2;
            if ( pImgNode.IsNotNull() && dynamic_cast<mitk::Image*>(pImgNode->GetData()) )
            {
                parentNode = pImgNode;
                break;
            }
        }

        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
        mitk::FiberBundleX::Pointer newBundle = fib->GetDeepCopy();
        QString name((*it)->GetName().c_str());
        name += "_copy";

        mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
        fbNode->SetData(newBundle);
        fbNode->SetName(name.toStdString());
        fbNode->SetVisibility(true);
        if (parentNode.IsNotNull())
            GetDataStorage()->Add(fbNode, parentNode);
        else
            GetDataStorage()->Add(fbNode);

        // copy child fiducials
        if (m_Controls->m_IncludeFiducials->isChecked())
        {
            mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(*it);
            for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations->begin(); it2 != derivations->end(); ++it2 )
            {
                mitk::DataNode::Pointer fiducialNode = *it2;
                if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
                {
                    mitk::PlanarEllipse::Pointer pe = mitk::PlanarEllipse::New();
                    pe->DeepCopy(dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()));
                    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
                    newNode->SetData(pe);
                    newNode->SetName(fiducialNode->GetName());
                    GetDataStorage()->Add(newNode, fbNode);
                }
            }
        }
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::JoinBundles()
{
    if ( m_SelectedBundles.size()<2 ){
        QMessageBox::information( NULL, "Warning", "Select at least two fiber bundles!");
        MITK_WARN("QmitkFiberProcessingView") << "Select at least two fiber bundles!";
        return;
    }

    std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedBundles.begin();
    mitk::FiberBundleX::Pointer newBundle = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
    QString name("");
    name += QString((*it)->GetName().c_str());
    ++it;
    for (it; it!=m_SelectedBundles.end(); ++it)
    {
        newBundle = newBundle->AddBundle(dynamic_cast<mitk::FiberBundleX*>((*it)->GetData()));
        name += "+"+QString((*it)->GetName().c_str());
    }

    mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
    fbNode->SetData(newBundle);
    fbNode->SetName(name.toStdString());
    fbNode->SetVisibility(true);
    GetDataStorage()->Add(fbNode);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::UpdateGui()
{
    m_Controls->m_FiberBundleLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_GeometryFrame->setEnabled(true);
    m_Controls->m_GeometryMessage->setVisible(false);
    m_Controls->m_DiffusionPropsMessage->setVisible(false);
    m_Controls->m_FiberGenMessage->setVisible(true);

    m_Controls->m_TransformBundlesButton->setEnabled(false);
    m_Controls->m_CopyBundlesButton->setEnabled(false);
    m_Controls->m_GenerateFibersButton->setEnabled(false);
    m_Controls->m_FlipButton->setEnabled(false);
    m_Controls->m_CircleButton->setEnabled(false);
    m_Controls->m_BvalueBox->setEnabled(true);
    m_Controls->m_NumGradientsBox->setEnabled(true);
    m_Controls->m_JoinBundlesButton->setEnabled(false);
    m_Controls->m_AlignOnGrid->setEnabled(false);

    if (m_SelectedFiducial.IsNotNull())
    {
        m_Controls->m_TransformBundlesButton->setEnabled(true);
        m_Controls->m_FlipButton->setEnabled(true);
        m_Controls->m_AlignOnGrid->setEnabled(true);
    }

    if (m_SelectedImage.IsNotNull() || !m_SelectedBundles.empty())
    {
        m_Controls->m_TransformBundlesButton->setEnabled(true);
        m_Controls->m_CircleButton->setEnabled(true);
        m_Controls->m_FiberGenMessage->setVisible(false);
        m_Controls->m_AlignOnGrid->setEnabled(true);
    }

    if (m_TissueMask.IsNotNull() || m_SelectedImage.IsNotNull())
    {
        m_Controls->m_GeometryMessage->setVisible(true);
        m_Controls->m_GeometryFrame->setEnabled(false);
    }

    if (m_SelectedDWI.IsNotNull())
    {
        m_Controls->m_DiffusionPropsMessage->setVisible(true);
        m_Controls->m_BvalueBox->setEnabled(false);
        m_Controls->m_NumGradientsBox->setEnabled(false);
        m_Controls->m_GeometryMessage->setVisible(true);
        m_Controls->m_GeometryFrame->setEnabled(false);
    }

    if (!m_SelectedBundles.empty())
    {
        m_Controls->m_CopyBundlesButton->setEnabled(true);
        m_Controls->m_GenerateFibersButton->setEnabled(true);
        m_Controls->m_FiberBundleLabel->setText(m_SelectedBundles.at(0)->GetName().c_str());

        if (m_SelectedBundles.size()>1)
            m_Controls->m_JoinBundlesButton->setEnabled(true);
    }
}

void QmitkFiberfoxView::OnSelectionChanged( berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes )
{
    m_SelectedBundles2.clear();
    m_SelectedImages.clear();
    m_SelectedFiducials.clear();
    m_SelectedFiducial = NULL;
    m_TissueMask = NULL;
    m_SelectedBundles.clear();
    m_SelectedImage = NULL;
    m_SelectedDWI = NULL;
    m_Controls->m_TissueMaskLabel->setText("<font color='grey'>optional</font>");

    // iterate all selected objects, adjust warning visibility
    for( int i=0; i<nodes.size(); i++)
    {
        mitk::DataNode::Pointer node = nodes.at(i);

        if ( node.IsNotNull() && dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData()) )
        {
            m_SelectedDWI = node;
            m_SelectedImage = node;
            m_SelectedImages.push_back(node);
        }
        else if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            m_SelectedImages.push_back(node);
            m_SelectedImage = node;
            bool isBinary = false;
            node->GetPropertyValue<bool>("binary", isBinary);
            if (isBinary)
            {
                m_TissueMask = dynamic_cast<mitk::Image*>(node->GetData());
                m_Controls->m_TissueMaskLabel->setText(node->GetName().c_str());
            }
        }
        else if ( node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
        {
            m_SelectedBundles2.push_back(node);
            if (m_Controls->m_RealTimeFibers->isChecked())
            {
                m_SelectedBundles.push_back(node);
                mitk::FiberBundleX::Pointer newFib = dynamic_cast<mitk::FiberBundleX*>(node->GetData());
                if (newFib->GetNumFibers()!=m_Controls->m_FiberDensityBox->value())
                    GenerateFibers();
            }
            else
                m_SelectedBundles.push_back(node);
        }
        else if ( node.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(node->GetData()) )
        {
            m_SelectedFiducials.push_back(node);
            m_SelectedFiducial = node;
            m_SelectedBundles.clear();
            mitk::DataStorage::SetOfObjects::ConstPointer parents = GetDataStorage()->GetSources(node);
            for( mitk::DataStorage::SetOfObjects::const_iterator it = parents->begin(); it != parents->end(); ++it )
            {
                mitk::DataNode::Pointer pNode = *it;
                if ( pNode.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(pNode->GetData()) )
                    m_SelectedBundles.push_back(pNode);
            }
        }
    }
    UpdateGui();
}


void QmitkFiberfoxView::EnableCrosshairNavigation()
{
    MITK_DEBUG << "EnableCrosshairNavigation";

    // enable the crosshair navigation
    if (mitk::ILinkedRenderWindowPart* linkedRenderWindow =
            dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart()))
    {
        MITK_DEBUG << "enabling linked navigation";
        linkedRenderWindow->EnableLinkedNavigation(true);
        //        linkedRenderWindow->EnableSlicingPlanes(true);
    }

    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::DisableCrosshairNavigation()
{
    MITK_DEBUG << "DisableCrosshairNavigation";

    // disable the crosshair navigation during the drawing
    if (mitk::ILinkedRenderWindowPart* linkedRenderWindow =
            dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart()))
    {
        MITK_DEBUG << "disabling linked navigation";
        linkedRenderWindow->EnableLinkedNavigation(false);
        //        linkedRenderWindow->EnableSlicingPlanes(false);
    }
}

void QmitkFiberfoxView::NodeRemoved(const mitk::DataNode* node)
{
    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
    std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(nonConstNode);

    if (dynamic_cast<FiberBundleX*>(node->GetData()))
    {
        m_SelectedBundles.clear();
        m_SelectedBundles2.clear();
    }
    else if (dynamic_cast<Image*>(node->GetData()))
        m_SelectedImages.clear();

    if( it != m_DataNodeToPlanarFigureData.end() )
    {
        QmitkPlanarFigureData& data = it->second;

        // remove observers
        data.m_Figure->RemoveObserver( data.m_EndPlacementObserverTag );
        data.m_Figure->RemoveObserver( data.m_SelectObserverTag );
        data.m_Figure->RemoveObserver( data.m_StartInteractionObserverTag );
        data.m_Figure->RemoveObserver( data.m_EndInteractionObserverTag );

        m_DataNodeToPlanarFigureData.erase( it );
    }
}

void QmitkFiberfoxView::NodeAdded( const mitk::DataNode* node )
{
    // add observer for selection in renderwindow
    mitk::PlanarFigure* figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
    bool isPositionMarker (false);
    node->GetBoolProperty("isContourMarker", isPositionMarker);
    if( figure && !isPositionMarker )
    {
        MITK_DEBUG << "figure added. will add interactor if needed.";
        mitk::PlanarFigureInteractor::Pointer figureInteractor
                = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetDataInteractor().GetPointer());

        mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>( node );
        if(figureInteractor.IsNull())
        {
          figureInteractor = mitk::PlanarFigureInteractor::New();
          us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "PlanarFigure" );
          figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
          figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
          figureInteractor->SetDataNode( nonConstNode );
        }

        MITK_DEBUG << "will now add observers for planarfigure";
        QmitkPlanarFigureData data;
        data.m_Figure = figure;

        //        // add observer for event when figure has been placed
        typedef itk::SimpleMemberCommand< QmitkFiberfoxView > SimpleCommandType;
        //        SimpleCommandType::Pointer initializationCommand = SimpleCommandType::New();
        //        initializationCommand->SetCallbackFunction( this, &QmitkFiberfoxView::PlanarFigureInitialized );
        //        data.m_EndPlacementObserverTag = figure->AddObserver( mitk::EndPlacementPlanarFigureEvent(), initializationCommand );

        // add observer for event when figure is picked (selected)
        typedef itk::MemberCommand< QmitkFiberfoxView > MemberCommandType;
        MemberCommandType::Pointer selectCommand = MemberCommandType::New();
        selectCommand->SetCallbackFunction( this, &QmitkFiberfoxView::PlanarFigureSelected );
        data.m_SelectObserverTag = figure->AddObserver( mitk::SelectPlanarFigureEvent(), selectCommand );

        // add observer for event when interaction with figure starts
        SimpleCommandType::Pointer startInteractionCommand = SimpleCommandType::New();
        startInteractionCommand->SetCallbackFunction( this, &QmitkFiberfoxView::DisableCrosshairNavigation);
        data.m_StartInteractionObserverTag = figure->AddObserver( mitk::StartInteractionPlanarFigureEvent(), startInteractionCommand );

        // add observer for event when interaction with figure starts
        SimpleCommandType::Pointer endInteractionCommand = SimpleCommandType::New();
        endInteractionCommand->SetCallbackFunction( this, &QmitkFiberfoxView::EnableCrosshairNavigation);
        data.m_EndInteractionObserverTag = figure->AddObserver( mitk::EndInteractionPlanarFigureEvent(), endInteractionCommand );

        m_DataNodeToPlanarFigureData[nonConstNode] = data;
    }
}

void QmitkFiberfoxView::PlanarFigureSelected( itk::Object* object, const itk::EventObject& )
{
    mitk::TNodePredicateDataType<mitk::PlanarFigure>::Pointer isPf = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();

    mitk::DataStorage::SetOfObjects::ConstPointer allPfs = this->GetDataStorage()->GetSubset( isPf );
    for ( mitk::DataStorage::SetOfObjects::const_iterator it = allPfs->begin(); it!=allPfs->end(); ++it)
    {
        mitk::DataNode* node = *it;

        if( node->GetData() == object )
        {
            node->SetSelected(true);
            m_SelectedFiducial = node;
        }
        else
            node->SetSelected(false);
    }
    UpdateGui();
    this->RequestRenderWindowUpdate();
}

void QmitkFiberfoxView::SetFocus()
{
    m_Controls->m_CircleButton->setFocus();
}
