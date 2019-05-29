#pragma once

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

#include <mitkImage.h>
#include <mitkDiffusionPropertyHelper.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <MitkFiberTrackingExports.h>

namespace mitk
{
  /**
  * \brief Datastructure to manage streamline tractography parameters.
  *
  */
  class MITKFIBERTRACKING_EXPORT StreamlineTractographyParameters
  {

  public:

    enum EndpointConstraints {
      NONE,                     ///< No constraints on endpoint locations
      EPS_IN_TARGET,            ///< Both EPs are required to be located in the target image
      EPS_IN_TARGET_LABELDIFF,  ///< Both EPs are required to be located in the target image and the image values at the respective position needs to be distinct
      EPS_IN_SEED_AND_TARGET,   ///< One EP is required to be located in the seed image and one in the target image
      MIN_ONE_EP_IN_TARGET,     ///< At least one EP is required to be located in the target image
      ONE_EP_IN_TARGET,         ///< Exactly one EP is required to be located in the target image
      NO_EP_IN_TARGET           ///< No EP is allowed to be located in the target image
    };

    enum MODE {
      DETERMINISTIC,
      PROBABILISTIC
    };

    typedef itk::Image<float, 3>                            ItkFloatImgType;
    typedef itk::Image<double, 3>                           ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>                    ItkUcharImgType;

    StreamlineTractographyParameters();
    StreamlineTractographyParameters(const StreamlineTractographyParameters &params) = default;
    ~StreamlineTractographyParameters();

    void SaveParameters(std::string filename);  ///< Save image generation parameters to .stp file.
    void LoadParameters(std::string filename);  ///< Load image generation parameters from .stp file.

    template< class ParameterType >
    ParameterType ReadVal(boost::property_tree::ptree::value_type const& v, std::string tag, ParameterType defaultValue, bool essential=false);

    // seeding
    unsigned int m_SeedsPerVoxel = 1;
    unsigned int m_TrialsPerSeed = 10;
    int m_MaxNumFibers = -1;
    // - seed image

    // interactive
    float m_InteractiveRadiusMm = 2;
    unsigned int m_NumInteractiveSeeds = 50;
    bool m_EnableInteractive = false;

    // ROI constraints
    EndpointConstraints m_EpConstraints;
    // - mask image
    // - stop image
    // - exclusion image
    // - target image

    // tractography
    MODE m_Mode;
    bool m_SharpenOdfs = false;
    float m_Cutoff = 0.1;
    // - fa/gfa image
    float m_OdfCutoff = 0.00025;
    float m_MinTractLengthMm = 20;
    float m_MaxTractLengthMm = 400;
    float m_F = 1;
    float m_G = 0;
    bool m_FixRandomSeed = false;
    unsigned int m_NumPreviousDirections = 1;
    float m_PeakJitter = 0.01; // actual jitter is drawn from a normal distribution with m_PeakJitter*fabs(direction_value) as standard deviation

    // prior
    // - peak image
    float m_Weight = 0.5;
    bool m_RestrictToPrior = true;
    bool m_NewDirectionsFromPrior = true;
    bool m_PriorFlipX = false;
    bool m_PriorFlipY = false;
    bool m_PriorFlipZ = false;

    // neighborhood sampling
    unsigned int m_NumSamples = 0;
    bool m_OnlyForwardSamples = false;
    bool m_StopVotes = false;
    bool m_AvoidStop = true;
    bool m_RandomSampling = false;
    float m_DeflectionMod = 1.0;

    // data handling
    bool m_FlipX = false;
    bool m_FlipY = false;
    bool m_FlipZ = false;
    bool m_InterpolateTractographyData = true;
    bool m_InterpolateRoiImages;
    bool m_ApplyDirectionMatrix = false;

    // output and postprocessing
    bool m_CompressFibers = true;
    float m_Compression = 0.1;
    bool m_OutputProbMap = false;

    float GetAngularThresholdDot() const;
    float GetAngularThresholdDeg() const;
    void SetAngularThresholdDeg(float angular_threshold_deg);

    float GetLoopCheckDeg() const;
    void SetLoopCheckDeg(float loop_check_deg);

    float GetStepSizeMm() const;
    float GetStepSizeVox() const;
    void SetStepSizeVox(float step_size_vox);

    float GetSamplingDistanceMm() const;
    float GetSamplingDistanceVox() const;
    void SetSamplingDistanceVox(float sampling_distance_vox);

    void SetMinVoxelSizeMm(float min_voxel_size_mm);
    float GetMinVoxelSizeMm() const;

  private:

    void AutoAdjust();

    float m_SamplingDistanceVox = -1;
    float m_SamplingDistanceMm;

    float m_AngularThresholdDeg = -1;
    float m_AngularThresholdDot;

    float m_LoopCheckDeg = -1;

    float m_StepSizeVox = -1;
    float m_StepSizeMm;

    float m_MinVoxelSizeMm = 1.0;
  };
}

