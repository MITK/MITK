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
{
    m_NoiseModel = NULL;
    m_NoiseModelShort = NULL;

    m_ImageDirection.SetIdentity();
    m_ImageOrigin.Fill(0.0);
    m_ImageRegion.SetSize(0, 11);
    m_ImageRegion.SetSize(1, 11);
    m_ImageRegion.SetSize(2, 3);
    m_ImageSpacing.Fill(2.0);

    m_Translation.Fill(0.0);
    m_Rotation.Fill(0.0);
}

FiberfoxParameters::~FiberfoxParameters()
{
    if (m_NoiseModel!=NULL)
        delete m_NoiseModel;
    if (m_NoiseModelShort!=NULL)
        delete m_NoiseModelShort;
}
