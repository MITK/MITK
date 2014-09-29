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
#include <mitkPlanarEllipse.h>

#include <mitkStickModel.h>
#include <mitkTensorModel.h>
#include <mitkAstroStickModel.h>
#include <mitkBallModel.h>
#include <mitkDotModel.h>
#include <mitkRawShModel.h>

using namespace std;

namespace mitk {

/** Signal generation */
class SignalGenerationParameters
{
public:
    typedef itk::Image<double, 3>                   ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>            ItkUcharImgType;
    typedef itk::Vector<double,3>                   GradientType;
    typedef std::vector<GradientType>               GradientListType;

    enum DiffusionDirectionMode {
        FIBER_TANGENT_DIRECTIONS,
        MAIN_FIBER_DIRECTIONS,
        RANDOM_DIRECTIONS
    };

    SignalGenerationParameters()
        : m_SignalScale(100)
        , m_tEcho(100)
        , m_tLine(1)
        , m_tInhom(50)
        , m_Bvalue(1000)
        , m_SimulateKspaceAcquisition(false)
        , m_AxonRadius(0)
        , m_DoDisablePartialVolume(false)
        , m_DiffusionDirectionMode(SignalGenerationParameters::FIBER_TANGENT_DIRECTIONS)
        , m_FiberSeparationThreshold(30)
        , m_Spikes(0)
        , m_SpikeAmplitude(1)
        , m_KspaceLineOffset(0)
        , m_EddyStrength(0)
        , m_Tau(70)
        , m_CroppingFactor(1)
        , m_DoAddGibbsRinging(false)
        , m_DoSimulateRelaxation(true)
        , m_DoAddMotion(false)
        , m_DoRandomizeMotion(true)
        , m_FrequencyMap(NULL)
        , m_MaskImage(NULL)
    {
        m_ImageRegion.SetSize(0, 12);
        m_ImageRegion.SetSize(1, 12);
        m_ImageRegion.SetSize(2, 3);
        m_ImageSpacing.Fill(2.0);
        m_ImageOrigin.Fill(0.0);
        m_ImageDirection.SetIdentity();
        m_Translation.Fill(0.0);
        m_Rotation.Fill(0.0);
        SetNumWeightedVolumes(6);
    }

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
    bool                                m_SimulateKspaceAcquisition;///< Flag to enable/disable k-space acquisition simulation
    double                              m_AxonRadius;               ///< Determines compartment volume fractions (0 == automatic axon radius estimation)
    bool                                m_DoDisablePartialVolume;   ///< Disable partial volume effects. Each voxel is either all fiber or all non-fiber.
    DiffusionDirectionMode              m_DiffusionDirectionMode;   ///< Determines how the main diffusion direction of the signal models is selected
    double                              m_FiberSeparationThreshold; ///< Used for random and and mein fiber deriction DiffusionDirectionMode

    /** Artifacts and other effects */
    unsigned int                        m_Spikes;                   ///< Number of spikes randomly appearing in the image
    double                              m_SpikeAmplitude;           ///< amplitude of spikes relative to the largest signal intensity (magnitude of complex)
    double                              m_KspaceLineOffset;         ///< Causes N/2 ghosts. Larger offset means stronger ghost.
    double                              m_EddyStrength;             ///< Strength of eddy current induced gradients in mT/m.
    double                              m_Tau;                      ///< Eddy current decay constant (in ms)
    double                              m_CroppingFactor;           ///< FOV size in y-direction is multiplied by this factor. Causes aliasing artifacts.
    bool                                m_DoAddGibbsRinging;        ///< Add Gibbs ringing artifact
    bool                                m_DoSimulateRelaxation;     ///< Add T2 relaxation effects
    bool                                m_DoAddMotion;              ///< Enable motion artifacts.
    bool                                m_DoRandomizeMotion;        ///< Toggles between random and linear motion.
    itk::Vector<double,3>               m_Translation;              ///< Maximum translational motion.
    itk::Vector<double,3>               m_Rotation;                 ///< Maximum rotational motion.
    ItkDoubleImgType::Pointer           m_FrequencyMap;             ///< If != NULL, distortions are added to the image using this frequency map.
    ItkUcharImgType::Pointer            m_MaskImage;                ///< Signal is only genrated inside of the mask image.

    inline void GenerateGradientHalfShell();                        ///< Generates half shell of gradient directions (with m_NumGradients non-zero directions)
    inline std::vector< int > GetBaselineIndices();                 ///< Returns list of nun-diffusion-weighted image volume indices
    inline unsigned int GetFirstBaselineIndex();                    ///< Returns index of first non-diffusion-weighted image volume
    inline bool IsBaselineIndex(unsigned int idx);                  ///< Checks if image volume with given index is non-diffusion-weighted volume or not.
    inline unsigned int GetNumWeightedVolumes();                    ///< Get number of diffusion-weighted image volumes
    inline unsigned int GetNumBaselineVolumes();                    ///< Get number of non-diffusion-weighted image volumes
    inline unsigned int GetNumVolumes();                            ///< Get number of baseline and diffusion-weighted image volumes
    inline GradientListType GetGradientDirections();                ///< Return gradient direction container
    inline GradientType GetGradientDirection(unsigned int i);

    inline void SetNumWeightedVolumes(int numGradients);            ///< Automaticall calls GenerateGradientHalfShell() afterwards.
    inline void SetGradienDirections(GradientListType gradientList);
    inline void SetGradienDirections(mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer gradientList);

protected:

    unsigned int                        m_NumGradients;             ///< Number of diffusion-weighted image volumes.
    unsigned int                        m_NumBaseline;              ///< Number of non-diffusion-weighted image volumes.
    GradientListType                    m_GradientDirections;       ///< Total number of image volumes.
};

/** Fiber generation */
class FiberGenerationParameters
{
public:

    enum FiberDistribution{
        DISTRIBUTE_UNIFORM, // distribute fibers uniformly in the ROIs
        DISTRIBUTE_GAUSSIAN // distribute fibers using a 2D gaussian
    };

    typedef vector< vector< mitk::PlanarEllipse::Pointer > >    FiducialListType;
    typedef vector< vector< unsigned int > >                    FlipListType;

    FiberGenerationParameters()
        : m_Distribution(DISTRIBUTE_UNIFORM)
        , m_Density(100)
        , m_Variance(100)
        , m_Sampling(1)
        , m_Tension(0)
        , m_Continuity(0)
        , m_Bias(0)
    {
        m_Rotation.Fill(0.0);
        m_Translation.Fill(0.0);
        m_Scale.Fill(1.0);
    }

    FiberDistribution   m_Distribution;
    unsigned int        m_Density;
    double              m_Variance;
    double              m_Sampling;
    double              m_Tension;
    double              m_Continuity;
    double              m_Bias;
    mitk::Vector3D      m_Rotation;
    mitk::Vector3D      m_Translation;
    mitk::Vector3D      m_Scale;
    FlipListType        m_FlipList;        ///< contains flags indicating a flip of the 2D fiber x-coordinates (needed to resolve some unwanted fiber twisting)
    FiducialListType    m_Fiducials;       ///< container of the planar ellipses used as fiducials for the fiber generation process
};

/** GUI persistence, input, output, ... */
class MiscFiberfoxParameters
{
public:
    MiscFiberfoxParameters()
        : m_ResultNode(DataNode::New())
        , m_ParentNode(NULL)
        , m_SignalModelString("")
        , m_ArtifactModelString("")
        , m_OutputPath("")
        , m_CheckOutputVolumeFractionsBox(false)
        , m_CheckAdvancedSignalOptionsBox(false)
        , m_CheckAddNoiseBox(false)
        , m_CheckAddGhostsBox(false)
        , m_CheckAddAliasingBox(false)
        , m_CheckAddSpikesBox(false)
        , m_CheckAddEddyCurrentsBox(false)
        , m_CheckAddDistortionsBox(false)
        , m_CheckRealTimeFibersBox(true)
        , m_CheckAdvancedFiberOptionsBox(false)
        , m_CheckConstantRadiusBox(false)
        , m_CheckIncludeFiducialsBox(true)
    {}

    DataNode::Pointer   m_ResultNode;                       ///< Stores resulting image.
    DataNode::Pointer   m_ParentNode;                       ///< Parent node of result node.
    string              m_SignalModelString;                ///< Appendet to the name of the result node
    string              m_ArtifactModelString;              ///< Appendet to the name of the result node
    string              m_OutputPath;                       ///< Image is automatically saved to the specified folder after simulation is finished.

    /** member variables that store the check-state of GUI checkboxes */
    // image generation
    bool                m_CheckOutputVolumeFractionsBox;
    bool                m_CheckAdvancedSignalOptionsBox;
    bool                m_CheckAddNoiseBox;
    bool                m_CheckAddGhostsBox;
    bool                m_CheckAddAliasingBox;
    bool                m_CheckAddSpikesBox;
    bool                m_CheckAddEddyCurrentsBox;
    bool                m_CheckAddDistortionsBox;
    // fiber generation
    bool                m_CheckRealTimeFibersBox;
    bool                m_CheckAdvancedFiberOptionsBox;
    bool                m_CheckConstantRadiusBox;
    bool                m_CheckIncludeFiducialsBox;
};

/**
  * \brief Datastructure to manage the Fiberfox signal generation parameters.
  *
  */
template< class ScalarType = double >
class FiberfoxParameters
{
public:

    typedef itk::Image<double, 3>                           ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>                    ItkUcharImgType;
    typedef DiffusionSignalModel<ScalarType>                DiffusionModelType;
    typedef std::vector< DiffusionModelType* >              DiffusionModelListType;
    typedef DiffusionNoiseModel<ScalarType>                 NoiseModelType;

    FiberfoxParameters();
    ~FiberfoxParameters();

    /** Get same parameter object with different template parameter */
    template< class OutType >
    FiberfoxParameters< OutType > CopyParameters()
    {
        FiberfoxParameters< OutType > out;

        out.m_FiberGen = m_FiberGen;
        out.m_SignalGen = m_SignalGen;
        out.m_Misc = m_Misc;

        if (m_NoiseModel!=NULL)
        {
            if (dynamic_cast<mitk::RicianNoiseModel<ScalarType>*>(m_NoiseModel))
                out.m_NoiseModel = new mitk::RicianNoiseModel<OutType>();
            else if (dynamic_cast<mitk::ChiSquareNoiseModel<ScalarType>*>(m_NoiseModel))
                out.m_NoiseModel = new mitk::ChiSquareNoiseModel<OutType>();
            out.m_NoiseModel->SetNoiseVariance(m_NoiseModel->GetNoiseVariance());
        }

        for (unsigned int i=0; i<m_FiberModelList.size()+m_NonFiberModelList.size(); i++)
        {
            mitk::DiffusionSignalModel<OutType>* outModel = NULL;
            mitk::DiffusionSignalModel<ScalarType>* signalModel = NULL;
            if (i<m_FiberModelList.size())
                signalModel = m_FiberModelList.at(i);
            else
                signalModel = m_NonFiberModelList.at(i-m_FiberModelList.size());

            if (dynamic_cast<mitk::StickModel<ScalarType>*>(signalModel))
                outModel = new mitk::StickModel<OutType>(dynamic_cast<mitk::StickModel<ScalarType>*>(signalModel));
            else  if (dynamic_cast<mitk::TensorModel<ScalarType>*>(signalModel))
                outModel = new mitk::TensorModel<OutType>(dynamic_cast<mitk::TensorModel<ScalarType>*>(signalModel));
            else  if (dynamic_cast<mitk::RawShModel<ScalarType>*>(signalModel))
                outModel = new mitk::RawShModel<OutType>(dynamic_cast<mitk::RawShModel<ScalarType>*>(signalModel));
            else  if (dynamic_cast<mitk::BallModel<ScalarType>*>(signalModel))
                outModel = new mitk::BallModel<OutType>(dynamic_cast<mitk::BallModel<ScalarType>*>(signalModel));
            else if (dynamic_cast<mitk::AstroStickModel<ScalarType>*>(signalModel))
                outModel = new mitk::AstroStickModel<OutType>(dynamic_cast<mitk::AstroStickModel<ScalarType>*>(signalModel));
            else  if (dynamic_cast<mitk::DotModel<ScalarType>*>(signalModel))
                outModel = new mitk::DotModel<OutType>(dynamic_cast<mitk::DotModel<ScalarType>*>(signalModel));

            if (i<m_FiberModelList.size())
                out.m_FiberModelList.push_back(outModel);
            else
                out.m_NonFiberModelList.push_back(outModel);
        }

        return out;
    }

    /** Not templated parameters */
    FiberGenerationParameters           m_FiberGen;             ///< Fiber generation parameters
    SignalGenerationParameters          m_SignalGen;            ///< Signal generation parameters
    MiscFiberfoxParameters              m_Misc;                 ///< GUI realted and I/O parameters

    /** Templated parameters */
    DiffusionModelListType              m_FiberModelList;       ///< Intra- and inter-axonal compartments.
    DiffusionModelListType              m_NonFiberModelList;    ///< Extra-axonal compartments.
    NoiseModelType*                     m_NoiseModel;           ///< If != NULL, noise is added to the image.

    void PrintSelf();                           ///< Print parameters to stdout.
    void SaveParameters(string filename);       ///< Save image generation parameters to .ffp file.
    void LoadParameters(string filename);       ///< Load image generation parameters from .ffp file.
};
}

#include "mitkFiberfoxParameters.cpp"

#endif

