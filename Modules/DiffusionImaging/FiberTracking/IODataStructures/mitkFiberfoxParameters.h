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

#ifndef _MITK_FiberfoxParameters_H
#define _MITK_FiberfoxParameters_H

#include <FiberTrackingExports.h>
#include <itkImageRegion.h>
#include <itkMatrix.h>
#include <mitkDiffusionNoiseModel.h>
#include <mitkDiffusionSignalModel.h>
#include <mitkDataNode.h>
#include <itkTractsToDWIImageFilter.h>

using namespace std;

namespace mitk {

/**
  * \brief Datastructure to manage the Fiberfox signal generation parameters
  *
  */

class FiberTracking_EXPORT FiberfoxParameters
{
public:

    typedef itk::Image<double, 3>           ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>    ItkUcharImgType;

    FiberfoxParameters();
    ~FiberfoxParameters();

    itk::ImageRegion<3>                 m_ImageRegion;
    itk::Vector<double,3>               m_ImageSpacing;
    itk::Point<double,3>                m_ImageOrigin;
    itk::Matrix<double, 3, 3>           m_ImageDirection;
    unsigned int                        m_NumGradients;
    double                              m_Bvalue;
    unsigned int                        m_Repetitions;
    double                              m_SignalScale;
    double                              m_tEcho;
    double                              m_tLine;
    double                              m_tInhom;
    double                              m_AxonRadius;
    unsigned int                        m_InterpolationShrink;
    double                              m_KspaceLineOffset;
    bool                                m_AddGibbsRinging;
    double                              m_EddyStrength;
    double                              m_Comp3Weight;
    double                              m_Comp4Weight;
    int                                 m_Spikes;
    double                              m_SpikeAmplitude;
    double                              m_Wrap;
    itk::Vector<double,3>               m_Translation;
    itk::Vector<double,3>               m_Rotation;
    bool                                m_DoSimulateRelaxation;
    bool                                m_DoSimulateEddyCurrents;
    bool                                m_DoDisablePartialVolume;
    bool                                m_DoAddMotion;
    bool                                m_RandomMotion;

    mitk::DiffusionNoiseModel<double>*  m_NoiseModel;
    mitk::DiffusionNoiseModel<short>*   m_NoiseModelShort;
    mitk::DiffusionSignalModel<double>::GradientListType  m_GradientDirections;
    itk::TractsToDWIImageFilter< short >::DiffusionModelList m_FiberModelList;
    itk::TractsToDWIImageFilter< short >::DiffusionModelList m_NonFiberModelList;
    ItkDoubleImgType::Pointer           m_FrequencyMap;
    ItkUcharImgType::Pointer            m_MaskImage;
    mitk::DataNode::Pointer             m_ResultNode;
    mitk::DataNode::Pointer             m_ParentNode;

    string m_SignalModelString;
    string m_ArtifactModelString;
    string m_OutputPath;

    void PrintSelf();

protected:

};

}

#endif

