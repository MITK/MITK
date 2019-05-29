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
#include <mitkImage.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkPlanarEllipse.h>
#include <mitkStickModel.h>
#include <mitkTensorModel.h>
#include <mitkAstroStickModel.h>
#include <mitkBallModel.h>
#include <mitkDotModel.h>
#include <mitkRawShModel.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <limits>
#include <MitkFiberTrackingExports.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

namespace mitk
{
  class MITKFIBERTRACKING_EXPORT FiberfoxParameters;

  /** Signal generation */
  class MITKFIBERTRACKING_EXPORT SignalGenerationParameters
  {
    friend FiberfoxParameters;
  public:
    typedef itk::Image<float, 3>                    ItkFloatImgType;
    typedef itk::Image<unsigned char, 3>            ItkUcharImgType;
    typedef itk::Vector<double,3>                   GradientType;
    typedef std::vector<GradientType>               GradientListType;

    enum CoilSensitivityProfile : int {
      COIL_CONSTANT,
      COIL_LINEAR,
      COIL_EXPONENTIAL
    };

    enum AcquisitionType : int
    {
      SingleShotEpi,
      ConventionalSpinEcho,
      FastSpinEcho,
    };

    SignalGenerationParameters()
      : m_AcquisitionType(SignalGenerationParameters::SingleShotEpi)
      , m_SignalScale(100)
      , m_tEcho(100)
      , m_tRep(4000)
      , m_tInv(0)
      , m_tLine(1)
      , m_tInhom(50)
      , m_EchoTrainLength(8)
      , m_ReversePhase(false)
      , m_PartialFourier(1.0)
      , m_NoiseVariance(0.001f)
      , m_NumberOfCoils(1)
      , m_CoilSensitivityProfile(SignalGenerationParameters::COIL_CONSTANT)
      , m_CoilSensitivity(0.3f)
      , m_SimulateKspaceAcquisition(false)
      , m_AxonRadius(0)
      , m_DoDisablePartialVolume(false)
      , m_Spikes(0)
      , m_SpikeAmplitude(1)
      , m_KspaceLineOffset(0)
      , m_EddyStrength(0.002f)
      , m_Tau(70)
      , m_CroppingFactor(1)
      , m_Drift(0.06)
      , m_DoAddGibbsRinging(false)
      , m_ZeroRinging(0)
      , m_DoSimulateRelaxation(true)
      , m_DoAddMotion(false)
      , m_DoRandomizeMotion(true)
      , m_DoAddDrift(false)
      , m_FrequencyMap(nullptr)
      , m_MaskImage(nullptr)
      , m_Bvalue(1000)
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
    itk::ImageRegion<3>                 m_CroppedRegion;            ///< Image size with reduced FOV.
    itk::ImageRegion<3>                 m_ImageRegion;              ///< Image size.
    itk::Vector<float,3>                m_ImageSpacing;             ///< Image voxel size.
    itk::Point<float,3>                 m_ImageOrigin;              ///< Image origin.
    itk::Matrix<double, 3, 3>           m_ImageDirection;           ///< Image rotation matrix.

    /** Other acquisitions parameters */
    AcquisitionType                     m_AcquisitionType;          ///< determines k-space trajectory and maximum echo position(s)
    float                               m_SignalScale;              ///< Scaling factor for output signal (before noise is added).
    float                               m_tEcho;                    ///< Echo time TE.
    float                               m_tRep;                     ///< Echo time TR.
    float                               m_tInv;                       ///< Inversion time
    float                               m_tLine;                    ///< k-space line readout time (dwell time).
    float                               m_tInhom;                   ///< T2'
    unsigned int                        m_EchoTrainLength;          ///< Only relevant for Fast Spin Echo sequence (number of k-space lines acquired with one RF pulse)
    bool                                m_ReversePhase;             ///< If true, the phase readout direction will be inverted (-y instead of y)
    float                               m_PartialFourier;           ///< Partial fourier factor (0.5-1)
    float                               m_NoiseVariance;            ///< Variance of complex gaussian noise
    unsigned int                        m_NumberOfCoils;            ///< Number of coils in multi-coil acquisition
    CoilSensitivityProfile              m_CoilSensitivityProfile;   ///< Choose between constant, linear or exponential sensitivity profile of the used coils
    float                               m_CoilSensitivity;          ///< signal remaining in slice center
    bool                                m_SimulateKspaceAcquisition;///< Flag to enable/disable k-space acquisition simulation
    double                              m_AxonRadius;               ///< Determines compartment volume fractions (0 == automatic axon radius estimation)
    bool                                m_DoDisablePartialVolume;   ///< Disable partial volume effects. Each voxel is either all fiber or all non-fiber.

    /** Artifacts and other effects */
    unsigned int                        m_Spikes;                   ///< Number of spikes randomly appearing in the image
    float                               m_SpikeAmplitude;           ///< amplitude of spikes relative to the largest signal intensity (magnitude of complex)
    float                               m_KspaceLineOffset;         ///< Causes N/2 ghosts. Larger offset means stronger ghost.
    float                               m_EddyStrength;             ///< Strength of eddy current induced gradients in mT/m.
    float                               m_Tau;                      ///< Eddy current decay constant (in ms)
    float                               m_CroppingFactor;           ///< FOV size in y-direction is multiplied by this factor. Causes aliasing artifacts.
    float                               m_Drift;                    ///< Global signal decrease by the end of the acquisition.
    bool                                m_DoAddGibbsRinging;        ///< Add Gibbs ringing artifact
    int                                 m_ZeroRinging;              ///< If > 0, ringing is simulated by by setting the defined percentage of higher frequencies to 0 in k-space. Otherwise, the input to the k-space simulation is generated with twice the resolution and cropped during k-space simulation (much slower).
    bool                                m_DoSimulateRelaxation;     ///< Add T2 relaxation effects
    bool                                m_DoAddMotion;              ///< Enable motion artifacts.
    bool                                m_DoRandomizeMotion;        ///< Toggles between random and linear motion.
    bool                                m_DoAddDrift;               ///< Add quadratic signal drift.
    std::vector< bool >                 m_MotionVolumes;            ///< Indicates the image volumes that are affected by motion
    ///< with positive numbers, inverted logic with negative numbers.
    itk::Vector<float,3>                m_Translation;              ///< Maximum translational motion.
    itk::Vector<float,3>                m_Rotation;                 ///< Maximum rotational motion.
    ItkFloatImgType::Pointer            m_FrequencyMap;             ///< If != nullptr, distortions are added to the image using this frequency map.
    ItkUcharImgType::Pointer            m_MaskImage;                ///< Signal is only genrated inside of the mask image.

    std::vector< int > GetBaselineIndices();                 ///< Returns list of nun-diffusion-weighted image volume indices
    unsigned int GetFirstBaselineIndex();                    ///< Returns index of first non-diffusion-weighted image volume
    bool IsBaselineIndex(unsigned int idx);                  ///< Checks if image volume with given index is non-diffusion-weighted volume or not.
    unsigned int GetNumWeightedVolumes();                    ///< Get number of diffusion-weighted image volumes
    unsigned int GetNumBaselineVolumes();                    ///< Get number of non-diffusion-weighted image volumes
    unsigned int GetNumVolumes();                            ///< Get number of baseline and diffusion-weighted image volumes
    GradientListType GetGradientDirections();                ///< Return gradient direction container
    mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer GetItkGradientContainer();
    GradientType GetGradientDirection(unsigned int i);
    std::vector< int > GetBvalues();                         ///< Returns a vector with all unique b-values (determined by the gradient magnitudes)
    double GetBvalue();
    void ApplyDirectionMatrix();

  protected:

    unsigned int                        m_NumGradients;             ///< Number of diffusion-weighted image volumes.
    unsigned int                        m_NumBaseline;              ///< Number of non-diffusion-weighted image volumes.
    GradientListType                    m_GradientDirections;       ///< Total number of image volumes.
    double                              m_Bvalue;                   ///< Acquisition b-value

    void SetNumWeightedVolumes(int numGradients);            ///< Automaticall calls GenerateGradientHalfShell() afterwards.
    void SetGradienDirections(GradientListType gradientList);
    void SetGradienDirections(mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradientList);
    void GenerateGradientHalfShell();                        ///< Generates half shell of gradient directions (with m_NumGradients non-zero directions)
  };

  /** Fiber generation */
  class MITKFIBERTRACKING_EXPORT FiberGenerationParameters
  {
  public:

    enum FiberDistribution
    {
      DISTRIBUTE_UNIFORM, // distribute fibers uniformly in the ROIs
      DISTRIBUTE_GAUSSIAN // distribute fibers using a 2D gaussian
    };

    typedef std::vector< std::vector< mitk::PlanarEllipse::Pointer > >    FiducialListType;
    typedef std::vector< std::vector< unsigned int > >                    FlipListType;

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
  class MITKFIBERTRACKING_EXPORT MiscFiberfoxParameters
  {
  public:
    MiscFiberfoxParameters()
      : m_ResultNode(DataNode::New())
      , m_ParentNode(nullptr)
      , m_SignalModelString("")
      , m_ArtifactModelString("")
      , m_OutputPath("/tmp/")
      , m_OutputPrefix("fiberfox")
      , m_AfterSimulationMessage("")
      , m_BvalsFile("")
      , m_BvecsFile("")
      , m_OutputAdditionalImages(false)
      , m_CheckAdvancedSignalOptionsBox(false)
      , m_DoAddNoise(false)
      , m_DoAddGhosts(false)
      , m_DoAddAliasing(false)
      , m_DoAddSpikes(false)
      , m_DoAddEddyCurrents(false)
      , m_DoAddDistortions(false)
      , m_MotionVolumesBox("random")
      , m_CheckRealTimeFibersBox(true)
      , m_CheckAdvancedFiberOptionsBox(false)
      , m_CheckConstantRadiusBox(false)
      , m_CheckIncludeFiducialsBox(true)
    {}

    DataNode::Pointer   m_ResultNode;                       ///< Stores resulting image.
    DataNode::Pointer   m_ParentNode;                       ///< Parent node of result node.
    std::string         m_SignalModelString;                ///< Appendet to the name of the result node
    std::string         m_ArtifactModelString;              ///< Appendet to the name of the result node
    std::string         m_OutputPath;                       ///< Image is automatically saved to the specified folder after simulation is finished.
    std::string         m_OutputPrefix;  /** Prefix for filename of output files and logfile. */
    std::string         m_AfterSimulationMessage;           ///< Store messages that are displayed after the simulation has finished (e.g. warnings, automatic parameter adjustments etc.)
    std::string         m_BvalsFile;
    std::string         m_BvecsFile;

    /** member variables that store the check-state of GUI checkboxes */
    // image generation
    bool                m_OutputAdditionalImages;
    bool                m_CheckAdvancedSignalOptionsBox;
    bool                m_DoAddNoise;
    bool                m_DoAddGhosts;
    bool                m_DoAddAliasing;
    bool                m_DoAddSpikes;
    bool                m_DoAddEddyCurrents;
    bool                m_DoAddDistortions;
    std::string         m_MotionVolumesBox;
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
  class MITKFIBERTRACKING_EXPORT FiberfoxParameters
  {

  public:


    typedef itk::Image<float, 3>                            ItkFloatImgType;
    typedef itk::Image<double, 3>                           ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>                    ItkUcharImgType;
    typedef DiffusionSignalModel<double>                    DiffusionModelType;
    typedef std::vector< DiffusionModelType* >              DiffusionModelListType;
    typedef DiffusionNoiseModel<double>                     NoiseModelType;

    FiberfoxParameters();
    FiberfoxParameters(const FiberfoxParameters &params);
    ~FiberfoxParameters();

    /** Not templated parameters */
    FiberGenerationParameters           m_FiberGen;             ///< Fiber generation parameters
    SignalGenerationParameters          m_SignalGen;            ///< Signal generation parameters
    MiscFiberfoxParameters              m_Misc;                 ///< GUI realted and I/O parameters

    /** Templated parameters */
    DiffusionModelListType              m_FiberModelList;       ///< Intra- and inter-axonal compartments.
    DiffusionModelListType              m_NonFiberModelList;    ///< Extra-axonal compartments.
    std::shared_ptr< NoiseModelType >   m_NoiseModel;           ///< If != nullptr, noise is added to the image.

    void GenerateGradientHalfShell();
    void SetNumWeightedVolumes(int numGradients);            ///< Automaticall calls GenerateGradientHalfShell() afterwards.
    void SetGradienDirections(mitk::SignalGenerationParameters::GradientListType gradientList);
    void SetGradienDirections(mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradientList);
    void SetBvalue(double Bvalue);
    void UpdateSignalModels();
    void ClearFiberParameters();
    void ClearSignalParameters();
    void ApplyDirectionMatrix();

    void PrintSelf();                           ///< Print parameters to stdout.
    void SaveParameters(std::string filename);  ///< Save image generation parameters to .ffp file.
    void LoadParameters(std::string filename, bool fix_seed=false);  ///< Load image generation parameters from .ffp file.
    template< class ParameterType >
    ParameterType ReadVal(boost::property_tree::ptree::value_type const& v, std::string tag, ParameterType defaultValue, bool essential=false);
    std::string                         m_MissingTags;
  };
}

#endif

