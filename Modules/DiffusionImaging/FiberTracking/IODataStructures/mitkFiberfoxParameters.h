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
};

/** Signal generation */
struct SignalGenerationParameters
{
    typedef itk::Image<double, 3>                   ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>            ItkUcharImgType;

    enum DiffusionDirectionMode {
        FIBER_TANGENT_DIRECTIONS,
        MAIN_FIBER_DIRECTIONS,
        RANDOM_DIRECTIONS
    };

    /** input/output image specifications */
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

    ItkDoubleImgType::Pointer           m_FrequencyMap;             ///< If != NULL, distortions are added to the image using this frequency map.
    ItkUcharImgType::Pointer            m_MaskImage;                ///< Signal is only genrated inside of the mask image.

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
};

/** GUI persistence, input, output, ... */
struct MiscFiberfoxParameters
{
    mitk::DataNode::Pointer     m_ResultNode;               ///< Stores resulting image.
    mitk::DataNode::Pointer     m_ParentNode;               ///< Parent node of result node.
    string                      m_SignalModelString;        ///< Appendet to the name of the result node
    string                      m_ArtifactModelString;      ///< Appendet to the name of the result node
    string                      m_OutputPath;               ///< Image is automatically saved to the specified folder after simulation is finished.
    bool                        m_OutputVolumeFractions;
    bool                        m_AdvancedOptions;
};

/**
  * \brief Datastructure to manage the Fiberfox signal generation parameters.
  *
  */
template< class ScalarType = double >
class FiberfoxParameters
{
public:

    typedef itk::Image<double, 3>                   ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>            ItkUcharImgType;
    typedef DiffusionSignalModel<ScalarType>        DiffusionModelType;
    typedef std::vector< DiffusionModelType* >      DiffusionModelListType;
    typedef typename DiffusionModelType::GradientListType    GradientListType;
    typedef typename DiffusionModelType::GradientType        GradientType;
    typedef DiffusionNoiseModel<ScalarType>         NoiseModelType;

    FiberfoxParameters();
    ~FiberfoxParameters();

    /** Get same parameter object with different template parameter */
    template< class OutType > FiberfoxParameters< OutType > CopyParameters()
    {
        FiberfoxParameters< OutType > out;

        out.m_FiberGen = m_FiberGen;
        out.m_SignalGen = m_SignalGen;
        out.m_Misc = m_Misc;

        out.SetNumWeightedGradients(m_NumGradients);

        if (m_NoiseModel!=NULL)
        {
            if (dynamic_cast<mitk::RicianNoiseModel<ScalarType>*>(m_NoiseModel))
                out.m_NoiseModel = new mitk::RicianNoiseModel<OutType>();
            else if (dynamic_cast<mitk::ChiSquareNoiseModel<ScalarType>*>(m_NoiseModel))
                out.m_NoiseModel = new mitk::ChiSquareNoiseModel<OutType>();
            out.m_NoiseModel->SetNoiseVariance(m_NoiseModel->GetNoiseVariance());
        }

        // TODO: copy constructor für singalmodelle, gradienten

        return out;
    }

    FiberGenerationParameters           m_FiberGen;
    SignalGenerationParameters          m_SignalGen;
    MiscFiberfoxParameters              m_Misc;

    /** Signal generation */
    DiffusionModelListType              m_FiberModelList;           ///< Intra- and inter-axonal compartments.
    DiffusionModelListType              m_NonFiberModelList;        ///< Extra-axonal compartments.
    NoiseModelType*                     m_NoiseModel;               ///< If != NULL, noise is added to the image.

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

