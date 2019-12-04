/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPAInSilicoTissueVolume.h>
#include <mitkProperties.h>
#include <mitkCoreServices.h>
#include <mitkIPropertyPersistence.h>
#include <itkImage.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <chrono>

mitk::pa::InSilicoTissueVolume::InSilicoTissueVolume(TissueGeneratorParameters::Pointer parameters, std::mt19937* rng)
{
  {
    unsigned int xDim = parameters->GetXDim();
    unsigned int yDim = parameters->GetYDim();
    unsigned int zDim = parameters->GetZDim();
    m_TDim = 4;
    unsigned int size = xDim * yDim * zDim;
    auto* absorptionArray = new double[size];
    auto* scatteringArray = new double[size];
    auto* anisotropyArray = new double[size];
    auto* segmentationArray = new double[size];

    m_InitialBackgroundAbsorption = (parameters->GetMinBackgroundAbsorption() + parameters->GetMaxBackgroundAbsorption()) / 2;
    m_Rng = rng;

    for (unsigned int index = 0; index < size; index++)
    {
      absorptionArray[index] = m_InitialBackgroundAbsorption;
      scatteringArray[index] = parameters->GetBackgroundScattering();
      anisotropyArray[index] = parameters->GetBackgroundAnisotropy();
      segmentationArray[index] = SegmentationType::BACKGROUND;
    }

    m_AbsorptionVolume = Volume::New(absorptionArray, xDim, yDim, zDim, parameters->GetVoxelSpacingInCentimeters());
    m_ScatteringVolume = Volume::New(scatteringArray, xDim, yDim, zDim, parameters->GetVoxelSpacingInCentimeters());
    m_AnisotropyVolume = Volume::New(anisotropyArray, xDim, yDim, zDim, parameters->GetVoxelSpacingInCentimeters());
    m_SegmentationVolume = Volume::New(segmentationArray, xDim, yDim, zDim, parameters->GetVoxelSpacingInCentimeters());
  }

  m_TissueParameters = parameters;
  m_PropertyList = mitk::PropertyList::New();
  UpdatePropertyList();
}

void mitk::pa::InSilicoTissueVolume::UpdatePropertyList()
{
  //Set properties
  AddIntProperty("mcflag", m_TissueParameters->GetMCflag());
  AddIntProperty("launchflag", m_TissueParameters->GetMCLaunchflag());
  AddIntProperty("boundaryflag", m_TissueParameters->GetMCBoundaryflag());
  AddDoubleProperty("launchPointX", m_TissueParameters->GetMCLaunchPointX());
  AddDoubleProperty("launchPointY", m_TissueParameters->GetMCLaunchPointY());
  AddDoubleProperty("launchPointZ", m_TissueParameters->GetMCLaunchPointZ());
  AddDoubleProperty("focusPointX", m_TissueParameters->GetMCFocusPointX());
  AddDoubleProperty("focusPointY", m_TissueParameters->GetMCFocusPointY());
  AddDoubleProperty("focusPointZ", m_TissueParameters->GetMCFocusPointZ());
  AddDoubleProperty("trajectoryVectorX", m_TissueParameters->GetMCTrajectoryVectorX());
  AddDoubleProperty("trajectoryVectorY", m_TissueParameters->GetMCTrajectoryVectorY());
  AddDoubleProperty("trajectoryVectorZ", m_TissueParameters->GetMCTrajectoryVectorZ());
  AddDoubleProperty("radius", m_TissueParameters->GetMCRadius());
  AddDoubleProperty("waist", m_TissueParameters->GetMCWaist());
  AddDoubleProperty("partialVolume", m_TissueParameters->GetDoPartialVolume());
  AddDoubleProperty("standardTissueAbsorptionMin", m_TissueParameters->GetMinBackgroundAbsorption());
  AddDoubleProperty("standardTissueAbsorptionMax", m_TissueParameters->GetMaxBackgroundAbsorption());
  AddDoubleProperty("standardTissueScattering", m_TissueParameters->GetBackgroundScattering());
  AddDoubleProperty("standardTissueAnisotropy", m_TissueParameters->GetBackgroundAnisotropy());
  AddDoubleProperty("airThickness", m_TissueParameters->GetAirThicknessInMillimeters());
  AddDoubleProperty("skinThickness", m_TissueParameters->GetSkinThicknessInMillimeters());
}

mitk::pa::InSilicoTissueVolume::InSilicoTissueVolume(
  Volume::Pointer absorptionVolume,
  Volume::Pointer scatteringVolume,
  Volume::Pointer anisotropyVolume,
  Volume::Pointer segmentationVolume,
  TissueGeneratorParameters::Pointer tissueParameters,
  mitk::PropertyList::Pointer propertyList)
{
  m_AbsorptionVolume = absorptionVolume;
  m_ScatteringVolume = scatteringVolume;
  m_AnisotropyVolume = anisotropyVolume;
  m_SegmentationVolume = segmentationVolume;
  m_TissueParameters = tissueParameters;
  m_PropertyList = propertyList;
  if (m_SegmentationVolume.IsNotNull())
    m_TDim = 4;
  else
    m_TDim = 3;
}

double mitk::pa::InSilicoTissueVolume::GetSpacing()
{
  return m_AbsorptionVolume->GetSpacing();
}

void mitk::pa::InSilicoTissueVolume::SetSpacing(double spacing)
{
  m_AbsorptionVolume->SetSpacing(spacing);
  m_ScatteringVolume->SetSpacing(spacing);
  m_AnisotropyVolume->SetSpacing(spacing);
  if (m_SegmentationVolume.IsNotNull())
  {
    m_SegmentationVolume->SetSpacing(spacing);
  }
}

void mitk::pa::InSilicoTissueVolume::AddDoubleProperty(std::string label, double value)
{
  m_PropertyList->SetDoubleProperty(label.c_str(), value);
  mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::PropertyPersistenceInfo::New(label));
}

void mitk::pa::InSilicoTissueVolume::AddIntProperty(std::string label, int value)
{
  m_PropertyList->SetIntProperty(label.c_str(), value);
  mitk::CoreServices::GetPropertyPersistence()->AddInfo(mitk::PropertyPersistenceInfo::New(label));
}

mitk::Image::Pointer mitk::pa::InSilicoTissueVolume::ConvertToMitkImage()
{
  mitk::Image::Pointer resultImage = mitk::Image::New();
  mitk::PixelType TPixel = mitk::MakeScalarPixelType<double>();
  auto* dimensionsOfImage = new unsigned int[4];

  // Copy dimensions
  dimensionsOfImage[0] = m_TissueParameters->GetYDim();
  dimensionsOfImage[1] = m_TissueParameters->GetXDim();
  dimensionsOfImage[2] = m_TissueParameters->GetZDim();
  dimensionsOfImage[3] = m_TDim;

  resultImage->Initialize(TPixel, 4, dimensionsOfImage, 1);

  mitk::Vector3D spacing;
  spacing.Fill(m_TissueParameters->GetVoxelSpacingInCentimeters());
  resultImage->SetSpacing(spacing);

  resultImage->SetImportVolume(m_AbsorptionVolume->GetData(), 0, 0, mitk::Image::CopyMemory);
  resultImage->SetImportVolume(m_ScatteringVolume->GetData(), 1, 0, mitk::Image::CopyMemory);
  resultImage->SetImportVolume(m_AnisotropyVolume->GetData(), 2, 0, mitk::Image::CopyMemory);
  if (m_TDim > 3)
  {
      resultImage->SetImportVolume(m_SegmentationVolume->GetData(), 3, 0, mitk::Image::CopyMemory);
  }

  resultImage->SetPropertyList(m_PropertyList);

  return resultImage;
}

mitk::pa::InSilicoTissueVolume::Pointer mitk::pa::InSilicoTissueVolume::New(
  Volume::Pointer absorptionVolume,
  Volume::Pointer scatteringVolume,
  Volume::Pointer anisotropyVolume,
  Volume::Pointer segmentationVolume,
  TissueGeneratorParameters::Pointer tissueParameters,
  mitk::PropertyList::Pointer propertyList)
{
  InSilicoTissueVolume::Pointer smartPtr = new InSilicoTissueVolume(
    absorptionVolume, scatteringVolume, anisotropyVolume, segmentationVolume,
    tissueParameters, propertyList);
  smartPtr->UnRegister();
  return smartPtr;
}

mitk::pa::InSilicoTissueVolume::~InSilicoTissueVolume()
{
  m_AbsorptionVolume = nullptr;
  m_ScatteringVolume = nullptr;
  m_AnisotropyVolume = nullptr;
  m_SegmentationVolume = nullptr;
  m_TissueParameters = nullptr;
  m_PropertyList = nullptr;
}

void mitk::pa::InSilicoTissueVolume::SetVolumeValues(int x, int y, int z, double absorption, double scattering, double anisotropy)
{
  if (IsInsideVolume(x, y, z))
  {
    m_AbsorptionVolume->SetData(absorption, x, y, z);
    m_ScatteringVolume->SetData(scattering, x, y, z);
    m_AnisotropyVolume->SetData(anisotropy, x, y, z);
  }
}

void mitk::pa::InSilicoTissueVolume::SetVolumeValues(int x, int y, int z, double absorption, double scattering, double anisotropy, SegmentationType segmentType)
{
  if (IsInsideVolume(x, y, z))
  {
    m_AbsorptionVolume->SetData(absorption, x, y, z);
    m_ScatteringVolume->SetData(scattering, x, y, z);
    m_AnisotropyVolume->SetData(anisotropy, x, y, z);
    if (m_SegmentationVolume.IsNotNull())
    {
      m_SegmentationVolume->SetData(segmentType, x, y, z);
    }
  }
}

bool mitk::pa::InSilicoTissueVolume::IsInsideVolume(int x, int y, int z)
{
  return x >= 0 && x < m_TissueParameters->GetXDim() &&
    y >= 0 && y < m_TissueParameters->GetYDim() &&
    z >= 0 && z < m_TissueParameters->GetZDim();
}

mitk::pa::Volume::Pointer mitk::pa::InSilicoTissueVolume::GetAbsorptionVolume()
{
  return m_AbsorptionVolume;
}

mitk::pa::Volume::Pointer mitk::pa::InSilicoTissueVolume::GetSegmentationVolume()
{
  return m_SegmentationVolume;
}

void mitk::pa::InSilicoTissueVolume::FinalizeVolume()
{
  AddSkinAndAirLayers();

  // If specified, randomize all tissue parameters
  if (m_TissueParameters->GetRandomizePhysicalProperties())
  {
    RandomizeTissueCoefficients(m_TissueParameters->GetUseRngSeed(),
      m_TissueParameters->GetRngSeed(),
      m_TissueParameters->GetRandomizePhysicalPropertiesPercentage());
  }

  unsigned int xDim = m_TissueParameters->GetXDim();
  unsigned int yDim = m_TissueParameters->GetYDim();
  unsigned int zDim = m_TissueParameters->GetZDim();

  std::uniform_real_distribution<double> randomBackgroundAbsorptionDistribution(
    m_TissueParameters->GetMinBackgroundAbsorption(), m_TissueParameters->GetMaxBackgroundAbsorption());

  for (unsigned int z = 0; z < zDim; z++)
  {
    for (unsigned int y = 0; y < yDim; y++)
    {
      for (unsigned int x = 0; x < xDim; x++)
      {
        if (fabs(m_AbsorptionVolume->GetData(x, y, z) - m_InitialBackgroundAbsorption) < mitk::eps)
        {
          m_AbsorptionVolume->SetData(randomBackgroundAbsorptionDistribution(*m_Rng), x, y, z);
        }
      }
    }
  }
}

void mitk::pa::InSilicoTissueVolume::AddSkinAndAirLayers()
{
  //Calculate the index location according to thickness in cm
  double airvoxel = (m_TissueParameters->GetAirThicknessInMillimeters() / m_TissueParameters->GetVoxelSpacingInCentimeters()) / 10;
  double skinvoxel = airvoxel + (m_TissueParameters->GetSkinThicknessInMillimeters() / m_TissueParameters->GetVoxelSpacingInCentimeters()) / 10;

  for (int y = 0; y < m_TissueParameters->GetYDim(); y++)
  {
    for (int x = 0; x < m_TissueParameters->GetXDim(); x++)
    {
      // Add air from index 0 to airvoxel
      if (m_TissueParameters->GetAirThicknessInMillimeters() > mitk::eps)
      {
        FillZLayer(x, y, 0, airvoxel,
          m_TissueParameters->GetAirAbsorption(),
          m_TissueParameters->GetAirScattering(),
          m_TissueParameters->GetAirAnisotropy(),
          SegmentationType::AIR);
      }

      //Add skin from index airvoxel to skinvoxel
      if (m_TissueParameters->GetSkinThicknessInMillimeters() > mitk::eps)
      {
        FillZLayer(x, y, airvoxel, skinvoxel,
          m_TissueParameters->GetSkinAbsorption(),
          m_TissueParameters->GetSkinScattering(),
          m_TissueParameters->GetSkinAnisotropy(),
          SegmentationType::SKIN);
      }
    }
  }
}

void mitk::pa::InSilicoTissueVolume::FillZLayer(int x, int y, double startIdx, double endIdx,
  double absorption, double scattering, double anisotropy,
  SegmentationType segmentationType)
{
  for (int z = startIdx; z < endIdx; z++)
  {
    if (IsInsideVolume(x, y, z))
    {
      if (endIdx - z < 1)
      {
        //Simulate partial volume effects
        m_AbsorptionVolume->SetData((1 - (endIdx - z)) *
          m_AbsorptionVolume->GetData(x, y, z) + (endIdx - z) * absorption, x, y, z);
        m_ScatteringVolume->SetData((1 - (endIdx - z)) *
          m_ScatteringVolume->GetData(x, y, z) + (endIdx - z) * scattering, x, y, z);
        m_AnisotropyVolume->SetData((1 - (endIdx - z)) *
          m_AnisotropyVolume->GetData(x, y, z) + (endIdx - z) * anisotropy, x, y, z);
        if (endIdx - z > 0.5)
        {
          //Only put the segmentation label if more than half of the partial volume is the wanted tissue type
          if (m_SegmentationVolume.IsNotNull())
          {
            m_SegmentationVolume->SetData(segmentationType, x, y, z);
          }
        }
      }
      else
      {
        m_AbsorptionVolume->SetData(absorption, x, y, z);
        m_ScatteringVolume->SetData(scattering, x, y, z);
        m_AnisotropyVolume->SetData(anisotropy, x, y, z);
        if (m_SegmentationVolume.IsNotNull())
        {
          m_SegmentationVolume->SetData(segmentationType, x, y, z);
        }
      }
    }
  }
}

void mitk::pa::InSilicoTissueVolume::RandomizeTissueCoefficients(long rngSeed, bool useRngSeed, double percentage)
{
  std::mt19937 rng;
  std::random_device randomDevice;
  if (useRngSeed)
  {
    rng.seed(rngSeed);
  }
  else
  {
    if (randomDevice.entropy() > 0.1)
    {
      rng.seed(randomDevice());
    }
    else
    {
      rng.seed(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    }
  }
  std::normal_distribution<> percentageDistribution(1, percentage / 100);

  for (int y = 0; y < m_TissueParameters->GetYDim(); y++)
  {
    for (int x = 0; x < m_TissueParameters->GetXDim(); x++)
    {
      for (int z = 0; z < m_TissueParameters->GetZDim(); z++)
      {
        m_AbsorptionVolume->SetData(m_AbsorptionVolume->GetData(x, y, z) * percentageDistribution(rng), x, y, z);
        m_ScatteringVolume->SetData(m_ScatteringVolume->GetData(x, y, z) * percentageDistribution(rng), x, y, z);
      }
    }
  }
}

mitk::pa::Volume::Pointer mitk::pa::InSilicoTissueVolume::GetScatteringVolume()
{
  return m_ScatteringVolume;
}

mitk::pa::Volume::Pointer mitk::pa::InSilicoTissueVolume::GetAnisotropyVolume()
{
  return m_AnisotropyVolume;
}

void mitk::pa::InSilicoTissueVolume::SetAbsorptionVolume(Volume::Pointer volume)
{
  m_AbsorptionVolume = volume;
}

void mitk::pa::InSilicoTissueVolume::SetScatteringVolume(Volume::Pointer volume)
{
  m_ScatteringVolume = volume;
}

void mitk::pa::InSilicoTissueVolume::SetAnisotropyVolume(Volume::Pointer volume)
{
  m_AnisotropyVolume = volume;
}

void mitk::pa::InSilicoTissueVolume::SetSegmentationVolume(Volume::Pointer volume)
{
  m_SegmentationVolume = volume;
}
