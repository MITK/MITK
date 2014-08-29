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
  * \brief Datastructure to manage the Fiberfox signal generation parameters.
  *
  */

template< class ScalarType = double >
class FiberfoxParameters
{
public:

    enum DiffusionDirectionMode {
        FIBER_TANGENT_DIRECTIONS,
        MAIN_FIBER_DIRECTIONS,
        RANDOM_DIRECTIONS
    };

    typedef itk::Image<double, 3>                           ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>                    ItkUcharImgType;
    typedef std::vector< DiffusionSignalModel<double>* >    DiffusionModelListType;
    typedef DiffusionSignalModel<double>::GradientListType  GradientListType;
    typedef DiffusionSignalModel<double>::GradientType      GradientType;
    typedef DiffusionNoiseModel<ScalarType>                 NoiseModelType;
    typedef DiffusionSignalModel<double>*                   DiffusionModelType;

    FiberfoxParameters();
    ~FiberfoxParameters();

    /** Get same parameter object with different template parameter */
    template< class OutType > FiberfoxParameters< OutType > CopyParameters()
    {
        FiberfoxParameters< OutType > out;

//        out.m_FiberGenerationParameters = m_FiberGenerationParameters;

        out.m_ImageRegion = m_ImageRegion;
        out.m_ImageSpacing = m_ImageSpacing;
        out.m_ImageOrigin = m_ImageOrigin;
        out.m_ImageDirection = m_ImageDirection;
        out.SetNumWeightedGradients(m_NumGradients);
        out.m_Bvalue = m_Bvalue;
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
        out.m_CroppingFactor = m_CroppingFactor;
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
        //        out.m_DiffusionDirectionMode = m_DiffusionDirectionMode;
        //        out.m_SignalGenerationMode = m_SignalGenerationMode;
        out.m_SimulateKspaceAcquisition = m_SimulateKspaceAcquisition;

        // TODO: copy constructor für singalmodelle und rauschen

        return out;
    }

    /** Fiber generation */
    struct FiberGenerationParameters
    {
        bool            m_RealTimeFibers;
        bool            m_AdvancedOptions;
        int             m_Distribution;
        double          m_Variance;
        unsigned int    m_FiberDensity;
        double          m_Sampling;
        double          m_Tension;
        double          m_Continuity;
        double          m_Bias;
        bool            m_ConstantRadius;
        mitk::Vector3D  m_Rotation;
        mitk::Vector3D  m_Translation;
        mitk::Vector3D  m_Scale;
        bool            m_IncludeFiducials;
    } m_FiberGenerationParameters;

    /** Output image specifications */
    itk::ImageRegion<3>                 m_ImageRegion;              ///< Image size.
    itk::Vector<double,3>               m_ImageSpacing;             ///< Image voxel size.
    itk::Point<double,3>                m_ImageOrigin;              ///< Image origin.
    itk::Matrix<double, 3, 3>           m_ImageDirection;           ///< Image rotation matrix.

    /** Other acquisitions parameters */
    double                              m_SignalScale;              ///< Scaling factor for output signal (before noise is added).
    double                              m_tEcho;                    ///< Echo time TE.
    double                              m_tLine;                    ///< k-space line readout time.
    double                              m_tInhom;                   ///< T2'
    double                              m_Bvalue;                   ///< Acquisition b-value
    bool                                m_SimulateKspaceAcquisition;///<

    /** Signal generation */
    DiffusionModelListType              m_FiberModelList;           ///< Intra- and inter-axonal compartments.
    DiffusionModelListType              m_NonFiberModelList;        ///< Extra-axonal compartments.
    double                              m_AxonRadius;               ///< Determines compartment volume fractions (0 == automatic axon radius estimation)
    DiffusionDirectionMode              m_DiffusionDirectionMode;   ///< Determines how the main diffusion direction of the signal models is selected
    double                              m_FiberSeparationThreshold; ///< Used for random and and mein fiber deriction DiffusionDirectionMode

    /** Artifacts */
    unsigned int                        m_Spikes;                   ///< Number of spikes randomly appearing in the image
    double                              m_SpikeAmplitude;           ///< amplitude of spikes relative to the largest signal intensity (magnitude of complex)
    double                              m_KspaceLineOffset;         ///< Causes N/2 ghosts. Larger offset means stronger ghost.
    double                              m_EddyStrength;             ///< Strength of eddy current induced gradients in mT/m.
    double                              m_Tau;                      ///< Eddy current decay constant (in ms)
    double                              m_CroppingFactor;           ///< FOV size in y-direction is multiplied by this factor. Causes aliasing artifacts.

    bool                                m_DoAddNoise;
    bool                                m_DoAddGhosts;
    bool                                m_DoAddAliasing;
    bool                                m_DoAddSpikes;
    bool                                m_DoAddEddyCurrents;
    bool                                m_DoAddGibbsRinging;        ///< Add Gibbs ringing artifact
    bool                                m_DoSimulateRelaxation;     ///< Add T2 relaxation effects
    bool                                m_DoAddDistortions;         ///< Add magnetic field distortions
    bool                                m_DoDisablePartialVolume;   ///< Disable partial volume effects. Each voxel is either all fiber or all non-fiber.
    bool                                m_DoAddMotion;              ///< Enable motion artifacts.
    bool                                m_DoRandomizeMotion;        ///< Toggles between random and linear motion.
    itk::Vector<double,3>               m_Translation;              ///< Maximum translational motion.
    itk::Vector<double,3>               m_Rotation;                 ///< Maximum rotational motion.
    NoiseModelType*                     m_NoiseModel;               ///< If != NULL, noise is added to the image.
    ItkDoubleImgType::Pointer           m_FrequencyMap;             ///< If != NULL, distortions are added to the image using this frequency map.
    ItkUcharImgType::Pointer            m_MaskImage;                ///< Signal is only genrated inside of the mask image.

    /** Output parameters (only relevant in GUI application) */
    mitk::DataNode::Pointer             m_ResultNode;               ///< Stores resulting image.
    mitk::DataNode::Pointer             m_ParentNode;               ///< Parent node of result node.
    string                              m_SignalModelString;        ///< Appendet to the name of the result node
    string                              m_ArtifactModelString;      ///< Appendet to the name of the result node
    string                              m_OutputPath;               ///< Image is automatically saved to the specified folder after simulation is finished.
    bool                                m_OutputVolumeFractions;
    bool                                m_AdvancedOptions;

    void PrintSelf();                           ///< Print parameters to stdout.
    void SaveParameters(string filename);       ///< Save image generation parameters to .ffp file.
    void LoadParameters(string filename);       ///< Load image generation parameters from .ffp file.
    void GenerateGradientHalfShell();           ///< Generates half shell of gradient directions (with m_NumGradients non-zero directions)

    std::vector< int > GetBaselineIndices();
    unsigned int GetFirstBaselineIndex();
    bool IsBaselineIndex(unsigned int idx);

    unsigned int GetNumWeightedVolumes();
    unsigned int GetNumBaselineVolumes();
    unsigned int GetNumVolumes();
    GradientListType GetGradientDirections();
    GradientType GetGradientDirection(unsigned int i);

    void SetNumWeightedGradients(int numGradients); ///< Automaticall calls GenerateGradientHalfShell() afterwards.
    void SetGradienDirections(GradientListType gradientList);
    void SetGradienDirections(mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer gradientList);

protected:

    unsigned int                        m_NumGradients;         ///< Number of diffusion-weighted image volumes.
    unsigned int                        m_NumBaseline;          ///< Number of non-diffusion-weighted image volumes.
    GradientListType                    m_GradientDirections;   ///< Total number of image volumes.

};
}

#include "mitkFiberfoxParameters.cpp"

#endif

