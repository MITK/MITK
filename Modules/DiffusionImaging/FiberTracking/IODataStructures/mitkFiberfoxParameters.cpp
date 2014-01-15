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
#include <mitkFiberfoxParameters.h>

using namespace mitk;

FiberfoxParameters::FiberfoxParameters()
    : m_AddGibbsRinging(false)
    , m_ArtifactModelString("")
    , m_AxonRadius(0)
    , m_Bvalue(1000)
    , m_Comp3Weight(1)
    , m_Comp4Weight(0)
    , m_DoAddMotion(false)
    , m_DoDisablePartialVolume(false)
    , m_DoSimulateEddyCurrents(false)
    , m_DoSimulateRelaxation(true)
    , m_EddyStrength(0)
    , m_InterpolationShrink(0)
    , m_KspaceLineOffset(0)
    , m_NumGradients(6)
    , m_OutputPath("")
    , m_RandomMotion(true)
    , m_Repetitions(1)
    , m_SignalModelString("")
    , m_SignalScale(100)
    , m_SpikeAmplitude(1)
    , m_Spikes(0)
    , m_tEcho(100)
    , m_tInhom(50)
    , m_tLine(1)
    , m_Wrap(1)
    , m_MaskImage(NULL)
    , m_FrequencyMap(NULL)
    , m_NoiseModel(NULL)
    , m_NoiseModelShort(NULL)
{
    m_ImageDirection.SetIdentity();
    m_ImageOrigin.Fill(0.0);
    m_ImageRegion.SetSize(0, 11);
    m_ImageRegion.SetSize(1, 11);
    m_ImageRegion.SetSize(2, 3);
    m_ImageSpacing.Fill(2.0);

    m_Translation.Fill(0.0);
    m_Rotation.Fill(0.0);

    m_ResultNode = mitk::DataNode::New();
    m_ParentNode = NULL;
}

FiberfoxParameters::~FiberfoxParameters()
{
//    if (m_NoiseModel!=NULL)
//        delete m_NoiseModel;
//    if (m_NoiseModelShort!=NULL)
//        delete m_NoiseModelShort;
}

void FiberfoxParameters::PrintSelf()
{
    MITK_INFO << m_ImageRegion;
    MITK_INFO << m_ImageSpacing;
    MITK_INFO << m_ImageOrigin;
    MITK_INFO << m_ImageDirection;
    MITK_INFO << m_NumGradients;
    MITK_INFO << m_Bvalue;
    MITK_INFO << m_Repetitions;
    MITK_INFO << m_SignalScale;
    MITK_INFO << m_tEcho;
    MITK_INFO << m_tLine;
    MITK_INFO << m_tInhom;
    MITK_INFO << m_AxonRadius;
    MITK_INFO << m_InterpolationShrink;
    MITK_INFO << m_KspaceLineOffset;
    MITK_INFO << m_AddGibbsRinging;
    MITK_INFO << m_EddyStrength;
    MITK_INFO << m_Comp3Weight;
    MITK_INFO << m_Comp4Weight;
    MITK_INFO << m_Spikes;
    MITK_INFO << m_SpikeAmplitude;
    MITK_INFO << m_Wrap;
    MITK_INFO << m_Translation;
    MITK_INFO << m_Rotation;
    MITK_INFO << m_DoSimulateRelaxation;
    MITK_INFO << m_DoSimulateEddyCurrents;
    MITK_INFO << m_DoDisablePartialVolume;
    MITK_INFO << m_DoAddMotion;
    MITK_INFO << "m_RandomMotion " << m_RandomMotion;

    MITK_INFO << m_NoiseModel;
    MITK_INFO << m_NoiseModelShort;
//    MITK_INFO << m_GradientDirections;
//    MITK_INFO << m_FiberModelList;
//    MITK_INFO << m_NonFiberModelList;
    if (m_FrequencyMap.IsNotNull())
        MITK_INFO << "m_FrequencyMap " << m_FrequencyMap;
    if (m_MaskImage.IsNotNull())
    MITK_INFO << "m_MaskImage " << m_MaskImage;
    if (m_ResultNode.IsNotNull())
    MITK_INFO << "m_ResultNode " << m_ResultNode;
    if (m_ParentNode.IsNotNull())
    MITK_INFO << "m_ParentNode " << m_ParentNode;

    MITK_INFO << m_SignalModelString;
    MITK_INFO << m_ArtifactModelString;
    MITK_INFO << m_OutputPath;
}
