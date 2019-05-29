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

#include "mitkTrackingHandlerOdf.h"
#include <itkDiffusionOdfGeneralizedFaImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkPointShell.h>
#include <omp.h>
#include <cmath>

namespace mitk
{

TrackingHandlerOdf::TrackingHandlerOdf()
  : m_NumProbSamples(1)
  , m_OdfFromTensor(false)
{
  m_GfaInterpolator = itk::LinearInterpolateImageFunction< itk::Image< float, 3 >, float >::New();
  m_OdfInterpolator = itk::LinearInterpolateImageFunction< itk::Image< ItkOdfImageType::PixelType, 3 >, float >::New();
}

TrackingHandlerOdf::~TrackingHandlerOdf()
{
}

bool TrackingHandlerOdf::WorldToIndex(itk::Point<float, 3>& pos, itk::Index<3>& index)
{
  m_OdfImage->TransformPhysicalPointToIndex(pos, index);
  return m_OdfImage->GetLargestPossibleRegion().IsInside(index);
}

void TrackingHandlerOdf::InitForTracking()
{
  MITK_INFO << "Initializing ODF tracker.";

  if (m_NeedsDataInit)
  {
    m_OdfHemisphereIndices.clear();
    itk::OrientationDistributionFunction< float, ODF_SAMPLING_SIZE > odf;
    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;

    for (int i=0; i<ODF_SAMPLING_SIZE; i++)
      if (dot_product(ref, odf.GetDirection(i))>0)
        m_OdfHemisphereIndices.push_back(i);
    m_OdfFloatDirs.set_size(m_OdfHemisphereIndices.size(), 3);

    auto double_dir = m_OdfImage->GetDirection().GetVnlMatrix();
    for (int r=0; r<3; r++)
      for (int c=0; c<3; c++)
      {
        m_FloatImageRotation[r][c] = double_dir[r][c];
      }

    for (unsigned int i=0; i<m_OdfHemisphereIndices.size(); i++)
    {
      m_OdfFloatDirs[i][0] = odf.GetDirection(m_OdfHemisphereIndices[i])[0];
      m_OdfFloatDirs[i][1] = odf.GetDirection(m_OdfHemisphereIndices[i])[1];
      m_OdfFloatDirs[i][2] = odf.GetDirection(m_OdfHemisphereIndices[i])[2];
    }

    if (m_GfaImage.IsNull())
    {
      MITK_INFO << "Calculating GFA image.";
      typedef itk::DiffusionOdfGeneralizedFaImageFilter<float,float,ODF_SAMPLING_SIZE> GfaFilterType;
      GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
      gfaFilter->SetInput(m_OdfImage);
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
      gfaFilter->Update();
      m_GfaImage = gfaFilter->GetOutput();
    }

    this->CalculateMinVoxelSize();
    m_NeedsDataInit = false;
  }

  if (m_OdfFromTensor)
  {
    m_Parameters->m_OdfCutoff = 0;
    m_Parameters->m_SharpenOdfs = false;
  }

  m_GfaInterpolator->SetInputImage(m_GfaImage);
  m_OdfInterpolator->SetInputImage(m_OdfImage);

  std::cout << "TrackingHandlerOdf - GFA threshold: " << m_Parameters->m_Cutoff << std::endl;
  std::cout << "TrackingHandlerOdf - ODF threshold: " << m_Parameters->m_OdfCutoff << std::endl;
  if (m_Parameters->m_SharpenOdfs)
    std::cout << "TrackingHandlerOdf - Sharpening ODfs" << std::endl;
}

int TrackingHandlerOdf::SampleOdf(vnl_vector< float >& probs, vnl_vector< float >& angles)
{
  boost::random::discrete_distribution<int, float> dist(probs.begin(), probs.end());
  int sampled_idx = 0;
  int max_sample_idx = -1;
  float max_prob = 0;
  int trials = 0;

  for (int i=0; i<m_NumProbSamples; i++)  // we sample m_NumProbSamples times and retain the sample with maximum probabilty
  {
    trials++;
#pragma omp critical
    {
      boost::random::variate_generator<boost::random::mt19937&, boost::random::discrete_distribution<int,float>> sampler(m_Rng, dist);
      sampled_idx = sampler();
    }
    if (probs[sampled_idx]>max_prob && probs[sampled_idx]>m_Parameters->m_OdfCutoff && fabs(angles[sampled_idx])>=m_Parameters->GetAngularThresholdDot())
    {
      max_prob = probs[sampled_idx];
      max_sample_idx = sampled_idx;
    }
    else if ( (probs[sampled_idx]<=m_Parameters->m_OdfCutoff || fabs(angles[sampled_idx])<m_Parameters->GetAngularThresholdDot()) && trials<50) // we allow 50 trials to exceed the ODF threshold
      i--;
  }

  return max_sample_idx;
}

void TrackingHandlerOdf::SetIsOdfFromTensor(bool OdfFromTensor)
{
  m_OdfFromTensor = OdfFromTensor;
}

bool TrackingHandlerOdf::GetIsOdfFromTensor() const
{
  return m_OdfFromTensor;
}

vnl_vector_fixed<float,3> TrackingHandlerOdf::ProposeDirection(const itk::Point<float, 3>& pos, std::deque<vnl_vector_fixed<float, 3> >& olddirs, itk::Index<3>& oldIndex)
{

  vnl_vector_fixed<float,3> output_direction; output_direction.fill(0);

  itk::Index<3> idx;
  m_OdfImage->TransformPhysicalPointToIndex(pos, idx);

  if ( !m_OdfImage->GetLargestPossibleRegion().IsInside(idx) )
    return output_direction;

  // check GFA threshold for termination
  float gfa = mitk::imv::GetImageValue<float>(pos, m_Parameters->m_InterpolateTractographyData, m_GfaInterpolator);
  if (gfa<m_Parameters->m_Cutoff)
    return output_direction;

  vnl_vector_fixed<float,3> last_dir;
  if (!olddirs.empty())
    last_dir = olddirs.back();

  if (!m_Parameters->m_InterpolateTractographyData && oldIndex==idx)
    return last_dir;

  ItkOdfImageType::PixelType odf_values = mitk::imv::GetImageValue<ItkOdfImageType::PixelType>(pos, m_Parameters->m_InterpolateTractographyData, m_OdfInterpolator);
  vnl_vector< float > probs; probs.set_size(m_OdfHemisphereIndices.size());
  vnl_vector< float > angles; angles.set_size(m_OdfHemisphereIndices.size()); angles.fill(1.0);

  // Find ODF maximum and remove <0 values
  float max_odf_val = 0;
  float min_odf_val = 999;
  int max_idx_d = -1;
  int c = 0;
  for (int i : m_OdfHemisphereIndices)
  {
    if (odf_values[i]<0)
      odf_values[i] = 0;

    if (odf_values[i]>max_odf_val)
    {
      max_odf_val = odf_values[i];
      max_idx_d = c;
    }
    if (odf_values[i]<min_odf_val)
      min_odf_val = odf_values[i];

    probs[c] = odf_values[i];
    c++;
  }

  if (m_Parameters->m_SharpenOdfs)
  {
    // sharpen ODF
    probs -= min_odf_val;
    probs /= (max_odf_val-min_odf_val);
    for (unsigned int i=0; i<probs.size(); i++)
      probs[i] = pow(probs[i], 4);
    float odf_sum = probs.sum();
    if (odf_sum>0)
    {
      probs /= odf_sum;
      max_odf_val /= odf_sum;
    }
  }

  // no previous direction
  if (max_odf_val>m_Parameters->m_OdfCutoff && (olddirs.empty() || last_dir.magnitude()<=0.5))
  {
    if (m_Parameters->m_Mode==MODE::DETERMINISTIC)  // return maximum peak
    {
      output_direction = m_OdfFloatDirs.get_row(max_idx_d);
      return output_direction * max_odf_val;
    }
    else if (m_Parameters->m_Mode==MODE::PROBABILISTIC) // sample from complete ODF
    {
      int max_sample_idx = SampleOdf(probs, angles);
      if (max_sample_idx>=0)
        output_direction = m_OdfFloatDirs.get_row(max_sample_idx) * probs[max_sample_idx];
      return output_direction;
    }
  }
  else if (max_odf_val<=m_Parameters->m_OdfCutoff) // return (0,0,0)
  {
    return output_direction;
  }

  // correct previous direction
  if (m_Parameters->m_FlipX)
    last_dir[0] *= -1;
  if (m_Parameters->m_FlipY)
    last_dir[1] *= -1;
  if (m_Parameters->m_FlipZ)
    last_dir[2] *= -1;

  // calculate angles between previous direction and ODF directions
  angles = m_OdfFloatDirs*last_dir;

  float probs_sum = 0;
  float max_prob = 0;
  for (unsigned int i=0; i<m_OdfHemisphereIndices.size(); i++)
  {
    float odf_val = probs[i];
    float angle = angles[i];
    float abs_angle = fabs(angle);

    odf_val *= abs_angle; // weight probabilities according to deviation from last direction
    if (m_Parameters->m_Mode==MODE::DETERMINISTIC && odf_val>max_prob && odf_val>m_Parameters->m_OdfCutoff)
    {
      // use maximum peak of the ODF weighted with the directional prior
      max_prob = odf_val;
      vnl_vector_fixed<float,3> d = m_OdfFloatDirs.get_row(i);
      if (angle<0)
        d *= -1;
      output_direction = odf_val*d;
    }
    else if (m_Parameters->m_Mode==MODE::PROBABILISTIC)
    {
      // update ODF probabilties with the ODF values pow(abs_angle, m_DirPriorPower)
      probs[i] = odf_val;
      probs_sum += probs[i];
    }
  }

  // do probabilistic sampling
  if (m_Parameters->m_Mode==MODE::PROBABILISTIC && probs_sum>0.0001)
  {
    int max_sample_idx = SampleOdf(probs, angles);
    if (max_sample_idx>=0)
    {
      output_direction = m_OdfFloatDirs.get_row(max_sample_idx);
      if (angles[max_sample_idx]<0)
        output_direction *= -1;
      output_direction *= probs[max_sample_idx];
    }
  }

  // check hard angular threshold
  float mag = output_direction.magnitude();
  if (mag>=0.0001)
  {
    output_direction.normalize();
    float a = dot_product(output_direction, last_dir);
    if (a<m_Parameters->GetAngularThresholdDot())
      output_direction.fill(0);
  }
  else
    output_direction.fill(0);

  if (m_Parameters->m_FlipX)
    output_direction[0] *= -1;
  if (m_Parameters->m_FlipY)
    output_direction[1] *= -1;
  if (m_Parameters->m_FlipZ)
    output_direction[2] *= -1;
  if (m_Parameters->m_ApplyDirectionMatrix)
    output_direction = m_FloatImageRotation*output_direction;

  return output_direction;
}

void TrackingHandlerOdf::SetNumProbSamples(int NumProbSamples)
{
  m_NumProbSamples = NumProbSamples;
}

}

