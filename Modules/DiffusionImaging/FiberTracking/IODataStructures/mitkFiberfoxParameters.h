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

#include <itkImageRegion.h>
#include <itkMatrix.h>
#include <mitkDiffusionNoiseModel.h>
#include <mitkDiffusionSignalModel.h>
#include <mitkDataNode.h>
#include <mitkRicianNoiseModel.h>
#include <mitkChiSquareNoiseModel.h>
#include <mitkDiffusionImage.h>

using namespace std;

namespace mitk {

/**
  * \brief Datastructure to manage the Fiberfox signal generation parameters
  *
  */

template< class ScalarType >
class FiberfoxParameters
{
public:

    typedef itk::Image<double, 3>           ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
    typedef std::vector< DiffusionSignalModel<double>* > DiffusionModelListType;
    typedef DiffusionSignalModel<double>::GradientListType GradientListType;
    typedef DiffusionSignalModel<double>::GradientType GradientType;
    typedef DiffusionNoiseModel<ScalarType> NoiseModelType;
    typedef DiffusionSignalModel<double>*   DiffusionModelType;

    FiberfoxParameters();
    ~FiberfoxParameters();

    template< class OutType > FiberfoxParameters< OutType > CopyParameters()
    {
        FiberfoxParameters< OutType > out;

        out.m_ImageRegion = m_ImageRegion;
        out.m_ImageSpacing = m_ImageSpacing;
        out.m_ImageOrigin = m_ImageOrigin;
        out.m_ImageDirection = m_ImageDirection;
        out.SetNumWeightedGradients(m_NumGradients);
        out.m_Bvalue = m_Bvalue;
        out.m_Repetitions = m_Repetitions;
        out.m_SignalScale = m_SignalScale;
        out.m_tEcho = m_tEcho;
        out.m_tLine = m_tLine;
        out.m_tInhom = m_tInhom;
        out.m_AxonRadius = m_AxonRadius;
        out.m_KspaceLineOffset = m_KspaceLineOffset;
        out.m_DoAddGibbsRinging = m_DoAddGibbsRinging;
        out.m_EddyStrength = m_EddyStrength;
        out.m_Spikes = m_Spikes;
        out.m_SpikeAmplitude = m_SpikeAmplitude;
        out.m_Wrap = m_Wrap;
        out.m_DoSimulateRelaxation = m_DoSimulateRelaxation;
        out.m_DoDisablePartialVolume = m_DoDisablePartialVolume;
        out.m_DoAddMotion = m_DoAddMotion;
        out.m_DoRandomizeMotion = m_DoRandomizeMotion;
        out.m_Translation = m_Translation;
        out.m_Rotation = m_Rotation;
        if (m_NoiseModel!=NULL)
        {
            if (dynamic_cast<mitk::RicianNoiseModel<ScalarType>*>(m_NoiseModel))
                out.m_NoiseModel = new mitk::RicianNoiseModel<OutType>();
            else if (dynamic_cast<mitk::ChiSquareNoiseModel<ScalarType>*>(m_NoiseModel))
                out.m_NoiseModel = new mitk::ChiSquareNoiseModel<OutType>();
            out.m_NoiseModel->SetNoiseVariance(m_NoiseModel->GetNoiseVariance());
        }
        out.m_FrequencyMap = m_FrequencyMap;
        out.m_MaskImage = m_MaskImage;
        out.m_ResultNode = m_ResultNode;
        out.m_ParentNode = m_ParentNode;
        out.m_SignalModelString = m_SignalModelString;
        out.m_ArtifactModelString = m_ArtifactModelString;
        out.m_OutputPath = m_OutputPath;

        return out;
    }

    /** Output image specifications */
    itk::ImageRegion<3>                 m_ImageRegion;
    itk::Vector<double,3>               m_ImageSpacing;
    itk::Point<double,3>                m_ImageOrigin;
    itk::Matrix<double, 3, 3>           m_ImageDirection;

    /** Other acquisitions parameters */
    unsigned int                        m_Repetitions;
    double                              m_SignalScale;
    double                              m_tEcho;
    double                              m_tLine;
    double                              m_tInhom;
    double                              m_Bvalue;

    /** Signal generation */
    DiffusionModelListType              m_FiberModelList;       ///< Intra- and inter-axonal compartments
    DiffusionModelListType              m_NonFiberModelList;    ///< Extra-axonal compartments
    double                              m_AxonRadius;           ///< Determines compartment volume fractions (0 == automatic axon radius estimation)

    /** Artifacts */
    int                                 m_Spikes;
    double                              m_SpikeAmplitude;
    double                              m_KspaceLineOffset;
    double                              m_EddyStrength;
    double                              m_Wrap;
    bool                                m_DoAddGibbsRinging;
    bool                                m_DoSimulateRelaxation;
    bool                                m_DoDisablePartialVolume;
    bool                                m_DoAddMotion;
    bool                                m_DoRandomizeMotion;
    itk::Vector<double,3>               m_Translation;
    itk::Vector<double,3>               m_Rotation;
    NoiseModelType*                     m_NoiseModel;
    ItkDoubleImgType::Pointer           m_FrequencyMap;
    ItkUcharImgType::Pointer            m_MaskImage;

    /** Output parameters */
    mitk::DataNode::Pointer             m_ResultNode;
    mitk::DataNode::Pointer             m_ParentNode;
    string                              m_SignalModelString;
    string                              m_ArtifactModelString;
    string                              m_OutputPath;

    void PrintSelf();
    void LoadParameters(string filename);
    void GenerateHalfShell();

    std::vector< int > GetBaselineIndices();
    unsigned int GetFirstBaselineIndex();
    bool IsBaselineIndex(unsigned int idx);

    unsigned int GetNumWeightedVolumes();
    unsigned int GetNumBaselineVolumes();
    unsigned int GetNumVolumes();
    GradientListType GetGradientDirections();
    GradientType GetGradientDirection(unsigned int i);

    void SetNumWeightedGradients(int numGradients);
    void SetGradienDirections(GradientListType gradientList);
    void SetGradienDirections(mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer gradientList);

protected:

    unsigned int                        m_NumBaseline;
    unsigned int                        m_NumGradients;
    GradientListType                    m_GradientDirections;

};
}

#include "mitkFiberfoxParameters.cpp"

#endif

