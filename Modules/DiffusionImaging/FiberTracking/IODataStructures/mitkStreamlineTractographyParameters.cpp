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

#include <boost/foreach.hpp>
#include <mitkLexicalCast.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <mitkLog.h>
#include <algorithm>
#include <string>
#include <mitkStreamlineTractographyParameters.h>
#include <mitkDiffusionFunctionCollection.h>

mitk::StreamlineTractographyParameters::StreamlineTractographyParameters()
{
  AutoAdjust();
}

mitk::StreamlineTractographyParameters::~StreamlineTractographyParameters()
{

}

void mitk::StreamlineTractographyParameters::SaveParameters(std::string filename)
{
  if(filename.empty())
    return;
  if(".stp"!=filename.substr(filename.size()-4, 4))
    filename += ".stp";

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, nullptr );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not set locale " << locale;
    }
  }

  boost::property_tree::ptree parameters;

  parameters.put("seeding.seeds_per_voxel", m_SeedsPerVoxel);
  parameters.put("seeding.trials_per_seed", m_TrialsPerSeed);
  parameters.put("seeding.max_num_fibers", m_MaxNumFibers);
  parameters.put("seeding.interactive_radius_mm", m_InteractiveRadiusMm);
  parameters.put("seeding.num_interactive_seeds", m_NumInteractiveSeeds);
  parameters.put("seeding.enable_interactive", m_EnableInteractive);

  parameters.put("roi_constraints.ep_constraints", m_EpConstraints);

  parameters.put("tractography.mode", m_Mode);
  parameters.put("tractography.sharpen_odfs", m_SharpenOdfs);
  parameters.put("tractography.cutoff", m_Cutoff);
  parameters.put("tractography.odf_cutoff", m_OdfCutoff);
  parameters.put("tractography.step_size_vox", m_StepSizeVox);
  parameters.put("tractography.min_tract_length_mm", m_MinTractLengthMm);
  parameters.put("tractography.max_tract_length_mm", m_MaxTractLengthMm);
  parameters.put("tractography.angluar_threshold_deg", m_AngularThresholdDeg);
  parameters.put("tractography.loop_check_deg", m_LoopCheckDeg);
  parameters.put("tractography.f", m_F);
  parameters.put("tractography.g", m_G);
  parameters.put("tractography.fix_seed", m_FixRandomSeed);
  parameters.put("tractography.peak_jitter", m_PeakJitter);

  parameters.put("prior.weight", m_Weight);
  parameters.put("prior.restrict_to_prior", m_RestrictToPrior);
  parameters.put("prior.new_directions", m_NewDirectionsFromPrior);
  parameters.put("prior.flip_x", m_PriorFlipX);
  parameters.put("prior.flip_y", m_PriorFlipY);
  parameters.put("prior.flip_z", m_PriorFlipZ);

  parameters.put("nsampling.num_samples", m_NumSamples);
  parameters.put("nsampling.sampling_distance_vox", m_SamplingDistanceVox);
  parameters.put("nsampling.only_frontal", m_OnlyForwardSamples);
  parameters.put("nsampling.stop_votes", m_StopVotes);

  parameters.put("data_handling.flip_x", m_FlipX);
  parameters.put("data_handling.flip_y", m_FlipY);
  parameters.put("data_handling.flip_z", m_FlipZ);
  parameters.put("data_handling.interpolate_tracto_data", m_InterpolateTractographyData);
  parameters.put("data_handling.interpolate_roi_images", m_InterpolateRoiImages);

  parameters.put("output.compress", m_CompressFibers);
  parameters.put("output.compression", m_Compression);
  parameters.put("output.prob_map", m_OutputProbMap);

  boost::property_tree::json_parser::write_json(filename, parameters, std::locale(), true);

//  try{
//    itk::ImageFileWriter<ItkFloatImgType>::Pointer writer = itk::ImageFileWriter<ItkFloatImgType>::New();
//    writer->SetFileName(filename+"_FMAP.nii.gz");
//    writer->SetInput(m_SignalGen.m_FrequencyMap);
//    writer->Update();
//  }
//  catch(...)
//  {
//    MITK_INFO << "No frequency map saved.";
//  }

  setlocale(LC_ALL, currLocale.c_str());
}



template< class ParameterType >
ParameterType mitk::StreamlineTractographyParameters::ReadVal(boost::property_tree::ptree::value_type const& v, std::string tag, ParameterType defaultValue, bool essential)
{
  try
  {
    return v.second.get<ParameterType>(tag);
  }
  catch (...)
  {
    if (essential)
    {
      mitkThrow() << "Parameter file corrupted. Essential tag is missing: '" << tag << "'";
    }
    MITK_INFO << "Tag '" << tag << "' not found. Using default value '" << defaultValue << "'.";
    return defaultValue;
  }
}

void mitk::StreamlineTractographyParameters::LoadParameters(std::string filename)
{ 
  if(filename.empty()) { return; }

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, nullptr );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not set locale " << locale;
    }
  }


  boost::property_tree::ptree parameterTree;
  boost::property_tree::json_parser::read_json( filename, parameterTree );

  BOOST_FOREACH( boost::property_tree::ptree::value_type const& v1, parameterTree )
  {
    if( v1.first == "seeding" )
    {
      m_SeedsPerVoxel = ReadVal<unsigned int>(v1,"seeds_per_voxel", m_SeedsPerVoxel);
      m_TrialsPerSeed = ReadVal<unsigned int>(v1,"trials_per_seed", m_TrialsPerSeed);
      m_MaxNumFibers = ReadVal<int>(v1,"max_num_fibers", m_MaxNumFibers);
      m_InteractiveRadiusMm = ReadVal<float>(v1,"interactive_radius_mm", m_InteractiveRadiusMm);
      m_NumInteractiveSeeds = ReadVal<unsigned int>(v1,"num_interactive_seeds", m_NumInteractiveSeeds);
      m_EnableInteractive = ReadVal<bool>(v1,"enable_interactive", m_EnableInteractive);
    }
    else if( v1.first == "roi_constraints" )
    {
      switch( ReadVal<int>(v1,"ep_constraints", 0) )
      {
      default:
        m_EpConstraints = EndpointConstraints::NONE;
        break;
      case 1:
        m_EpConstraints = EndpointConstraints::EPS_IN_TARGET;
        break;
      case 2:
        m_EpConstraints = EndpointConstraints::EPS_IN_TARGET_LABELDIFF;
        break;
      case 3:
        m_EpConstraints = EndpointConstraints::EPS_IN_SEED_AND_TARGET;
        break;
      case 4:
        m_EpConstraints = EndpointConstraints::MIN_ONE_EP_IN_TARGET;
        break;
      case 5:
        m_EpConstraints = EndpointConstraints::ONE_EP_IN_TARGET;
        break;
      case 6:
        m_EpConstraints = EndpointConstraints::NO_EP_IN_TARGET;
        break;
      }
    }
    else if( v1.first == "tractography" )
    {
      if(ReadVal<int>(v1,"mode", 0) == 0)
        m_Mode = MODE::DETERMINISTIC;
      else
        m_Mode = MODE::PROBABILISTIC;

      m_SharpenOdfs = ReadVal<bool>(v1,"sharpen_odfs", m_SharpenOdfs);
      m_Cutoff = ReadVal<float>(v1,"cutoff", m_Cutoff);
      m_OdfCutoff = ReadVal<float>(v1,"odf_cutoff", m_OdfCutoff);
      SetStepSizeVox(ReadVal<float>(v1,"step_size_vox", m_StepSizeVox));
      m_MinTractLengthMm = ReadVal<float>(v1,"min_tract_length_mm", m_MinTractLengthMm);
      m_MaxTractLengthMm = ReadVal<float>(v1,"max_tract_length_mm", m_MaxTractLengthMm);
      SetAngularThresholdDeg(ReadVal<float>(v1,"angluar_threshold_deg", m_AngularThresholdDeg));
      SetLoopCheckDeg(ReadVal<float>(v1,"loop_check_deg", m_LoopCheckDeg));
      m_F = ReadVal<float>(v1,"f", m_F);
      m_G = ReadVal<float>(v1,"g", m_G);
      m_FixRandomSeed = ReadVal<bool>(v1,"fix_seed", m_FixRandomSeed);
      m_PeakJitter = ReadVal<float>(v1,"peak_jitter", m_PeakJitter);
    }
    else if( v1.first == "prior" )
    {
      m_Weight = ReadVal<float>(v1,"weight", m_Weight);
      m_RestrictToPrior = ReadVal<bool>(v1,"restrict_to_prior", m_RestrictToPrior);
      m_NewDirectionsFromPrior = ReadVal<bool>(v1,"new_directions", m_NewDirectionsFromPrior);
      m_PriorFlipX = ReadVal<bool>(v1,"flip_x", m_PriorFlipX);
      m_PriorFlipY = ReadVal<bool>(v1,"flip_y", m_PriorFlipY);
      m_PriorFlipZ = ReadVal<bool>(v1,"flip_z", m_PriorFlipZ);
    }
    else if( v1.first == "nsampling" )
    {
      m_NumSamples = ReadVal<unsigned int>(v1,"num_samples", m_NumSamples);
      m_SamplingDistanceVox = ReadVal<float>(v1,"sampling_distance_vox", m_SamplingDistanceVox);
      m_OnlyForwardSamples = ReadVal<bool>(v1,"only_frontal", m_OnlyForwardSamples);
      m_StopVotes = ReadVal<bool>(v1,"stop_votes", m_StopVotes);
    }
    else if( v1.first == "data_handling" )
    {
      m_FlipX = ReadVal<bool>(v1,"flip_x", m_FlipX);
      m_FlipY = ReadVal<bool>(v1,"flip_y", m_FlipY);
      m_FlipZ = ReadVal<bool>(v1,"flip_z", m_FlipZ);
      m_InterpolateTractographyData = ReadVal<bool>(v1,"interpolate_tracto_data", m_InterpolateTractographyData);
      m_InterpolateRoiImages = ReadVal<bool>(v1,"interpolate_roi_images", m_InterpolateRoiImages);
    }
    else if( v1.first == "output" )
    {
      m_CompressFibers = ReadVal<bool>(v1,"compress", m_CompressFibers);
      m_Compression = ReadVal<float>(v1,"compression", m_Compression);
      m_OutputProbMap = ReadVal<bool>(v1,"prob_map", m_OutputProbMap);
    }
  }

//  try
//  {
//    itk::ImageFileReader<ItkFloatImgType>::Pointer reader = itk::ImageFileReader<ItkFloatImgType>::New();
//    reader->SetFileName(filename+"_FMAP.nrrd");
//    if ( itksys::SystemTools::FileExists(filename+"_FMAP.nii.gz") )
//      reader->SetFileName(filename+"_FMAP.nii.gz");
//    else if ( itksys::SystemTools::FileExists(filename+"_FMAP.nii") )
//      reader->SetFileName(filename+"_FMAP.nii");
//    else
//      reader->SetFileName(filename+"_FMAP.nrrd");
//    reader->Update();
//    m_SignalGen.m_FrequencyMap = reader->GetOutput();
//    MITK_INFO << "Frequency map loaded.";
//  }
//  catch(...)
//  {
//    MITK_INFO << "No frequency map found.";
//  }

  setlocale(LC_ALL, currLocale.c_str());
}

float mitk::StreamlineTractographyParameters::GetSamplingDistanceMm() const
{
  return m_SamplingDistanceMm;
}

void mitk::StreamlineTractographyParameters::SetSamplingDistanceVox(float sampling_distance_vox)
{
  m_SamplingDistanceVox = sampling_distance_vox;
  AutoAdjust();
}

void mitk::StreamlineTractographyParameters::AutoAdjust()
{
  if (m_StepSizeVox<static_cast<float>(mitk::eps))
    m_StepSizeVox = 0.5;
  m_StepSizeMm = m_StepSizeVox*m_MinVoxelSizeMm;

  if (m_AngularThresholdDeg<0)
  {
    if  (m_StepSizeMm/m_MinVoxelSizeMm<=0.966f)  // minimum 15° for automatic estimation
      m_AngularThresholdDot = static_cast<float>(std::cos( 0.5 * itk::Math::pi * static_cast<double>(m_StepSizeMm/m_MinVoxelSizeMm) ));
    else
      m_AngularThresholdDot = static_cast<float>(std::cos( 0.5 * itk::Math::pi * 0.966 ));

    m_AngularThresholdDeg = std::acos(m_AngularThresholdDot)*180.0/itk::Math::pi;
  }
  else
    m_AngularThresholdDot = static_cast<float>(std::cos( static_cast<double>(m_AngularThresholdDeg)*itk::Math::pi/180.0 ));

  if (m_SamplingDistanceVox<static_cast<float>(mitk::eps))
    m_SamplingDistanceVox = m_MinVoxelSizeMm*0.25f;
  m_SamplingDistanceMm = m_SamplingDistanceVox*m_MinVoxelSizeMm;
}

float mitk::StreamlineTractographyParameters::GetStepSizeVox() const
{
    return m_StepSizeVox;
}

float mitk::StreamlineTractographyParameters::GetAngularThresholdDeg() const
{
    return m_AngularThresholdDeg;
}

float mitk::StreamlineTractographyParameters::GetSamplingDistanceVox() const
{
    return m_SamplingDistanceVox;
}

float mitk::StreamlineTractographyParameters::GetMinVoxelSizeMm() const
{
    return m_MinVoxelSizeMm;
}

float mitk::StreamlineTractographyParameters::GetStepSizeMm() const
{
  return m_StepSizeMm;
}

void mitk::StreamlineTractographyParameters::SetMinVoxelSizeMm(float min_voxel_size_mm)
{
  m_MinVoxelSizeMm = min_voxel_size_mm;
  AutoAdjust();
}

float mitk::StreamlineTractographyParameters::GetAngularThresholdDot() const
{
  return m_AngularThresholdDot;
}

void mitk::StreamlineTractographyParameters::SetStepSizeVox(float step_size_vox)
{
  m_StepSizeVox = step_size_vox;
  AutoAdjust();
}

float mitk::StreamlineTractographyParameters::GetLoopCheckDeg() const
{
  return m_LoopCheckDeg;
}

void mitk::StreamlineTractographyParameters::SetLoopCheckDeg(float loop_check_deg)
{
  m_LoopCheckDeg = loop_check_deg;
}

void mitk::StreamlineTractographyParameters::SetAngularThresholdDeg(float angular_threshold_deg)
{
  m_AngularThresholdDeg = angular_threshold_deg;
  AutoAdjust();
}
