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
#include <itkDiffusionQballGeneralizedFaImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkPointShell.h>
#include <omp.h>

namespace mitk
{

TrackingHandlerOdf::TrackingHandlerOdf()
  : m_GfaThreshold(0.1)
  , m_SecondOrder(false)
  , m_MinMaxNormalize(true)
  , m_NumProbSamples(10)
{

}

TrackingHandlerOdf::~TrackingHandlerOdf()
{
}

void TrackingHandlerOdf::InitForTracking()
{
  MITK_INFO << "Initializing ODF tracker.";

  if (m_NeedsDataInit)
  {
    m_OdfHemisphereIndices.clear();
    m_OdfReducedIndices.clear();
    itk::OrientationDistributionFunction< float, QBALL_ODFSIZE > odf;
    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;

    for (int i=0; i<QBALL_ODFSIZE; i++)
      if (dot_product(ref, odf.GetDirection(i))>0)
        m_OdfHemisphereIndices.push_back(i);
    m_OdfFloatDirs.set_size(m_OdfHemisphereIndices.size(), 3);

    for (unsigned int i=0; i<m_OdfHemisphereIndices.size(); i++)
    {
      m_OdfFloatDirs[i][0] = odf.GetDirection(m_OdfHemisphereIndices[i])[0];
      m_OdfFloatDirs[i][1] = odf.GetDirection(m_OdfHemisphereIndices[i])[1];
      m_OdfFloatDirs[i][2] = odf.GetDirection(m_OdfHemisphereIndices[i])[2];
    }

    if (m_GfaImage.IsNull())
    {
      MITK_INFO << "Calculating GFA image.";
      typedef itk::DiffusionQballGeneralizedFaImageFilter<float,float,QBALL_ODFSIZE> GfaFilterType;
      GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
      gfaFilter->SetInput(m_OdfImage);
      gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
      gfaFilter->Update();
      m_GfaImage = gfaFilter->GetOutput();
    }

//    m_WorkingOdfImage = ItkOdfImageType::New();
//    m_WorkingOdfImage->SetSpacing( m_OdfImage->GetSpacing() );
//    m_WorkingOdfImage->SetOrigin( m_OdfImage->GetOrigin() );
//    m_WorkingOdfImage->SetDirection( m_OdfImage->GetDirection() );
//    m_WorkingOdfImage->SetRegions( m_OdfImage->GetLargestPossibleRegion() );
//    m_WorkingOdfImage->Allocate();

//    MITK_INFO << "Rescaling ODFs.";
//    ItkOdfImageType::SizeType image_size = m_OdfImage->GetLargestPossibleRegion().GetSize();
//#ifdef WIN32
//#pragma omp parallel for
//#else
//#pragma omp parallel for collapse(3)
//#endif
//    for (unsigned int x=0; x<image_size[0]; x++)
//      for (unsigned int y=0; y<image_size[1]; y++)
//        for (unsigned int z=0; z<image_size[2]; z++)
//        {
//          ItkOdfImageType::IndexType idx; idx[0]=x; idx[1]=y; idx[2]=z;
//          ItkOdfImageType::PixelType odf_values = m_OdfImage->GetPixel(idx);

//          for (int i=0; i<QBALL_ODFSIZE; i++)
//            odf_values[i] = std::pow(odf_values[i], m_OdfPower);

//          float max = 0;
//          float min = 99999;
//          float sum = 0;
//          for (int i=0; i<QBALL_ODFSIZE; i++)
//          {
//            if (odf_values[i]<0)
//              odf_values[i] = 0;
//            if (odf_values[i]>=max)
//              max = odf_values[i];
//            else if (odf_values[i]<min)
//              min = odf_values[i];
//            sum += odf_values[i];
//          }

//          if (m_MinMaxNormalize)
//          {
//            max -= min;
//            if (max>0.0)
//            {
//              odf_values -= min;
//              odf_values /= max;
//            }
//          }
//          else if (sum>0)
//            odf_values /= sum;

//#pragma omp critical
//          m_WorkingOdfImage->SetPixel(idx, odf_values);
//        }

    m_NeedsDataInit = false;
  }
}

vnl_vector< float > TrackingHandlerOdf::GetSecondOrderProbabilities(const itk::Point<float, 3>& itkP, vnl_vector< float >& angles, vnl_vector< float >& probs)
{
  vnl_vector< float > out_probs;
  out_probs.set_size(m_OdfHemisphereIndices.size());
  out_probs.fill(0.0);
  float out_probs_sum = 0;

  int c = 0;
  for (unsigned int j=0; j<m_OdfHemisphereIndices.size(); j++)
  {
    if (fabs(angles[j])>=m_AngularThreshold)
      continue;

    vnl_vector_fixed<float,3> d = m_OdfFloatDirs.get_row(j);
    itk::Point<float, 3> pos;
    pos[0] = itkP[0] + d[0];
    pos[1] = itkP[1] + d[1];
    pos[2] = itkP[2] + d[2];
    ItkOdfImageType::PixelType odf_values = GetImageValue<float, QBALL_ODFSIZE>(pos, m_OdfImage, m_Interpolate);
    vnl_vector< float > new_angles = m_OdfFloatDirs*d;

    float probs_sum = 0;
    vnl_vector< float > new_probs; new_probs.set_size(m_OdfHemisphereIndices.size());
    for (unsigned int i=0; i<m_OdfHemisphereIndices.size(); i++)
    {
      if (fabs(new_angles[i])>=m_AngularThreshold)
      {
        new_probs[i] = odf_values[m_OdfHemisphereIndices[i]];
        probs_sum += new_probs[i];
      }
      else
        new_probs[i] = 0;
    }
    if (probs_sum>0.0001)
      new_probs /= probs_sum;

    for (unsigned int i=0; i<m_OdfHemisphereIndices.size(); i++)
    {
      float p = new_probs[i] * probs[i];
      out_probs_sum += p;
      out_probs[i] += p;
    }
    c += 1;
  }

  if (out_probs_sum>0.0001)
    out_probs /= out_probs_sum;

  return out_probs;
}

bool TrackingHandlerOdf::MinMaxNormalize() const
{
  return m_MinMaxNormalize;
}

void TrackingHandlerOdf::SetMinMaxNormalize(bool MinMaxNormalize)
{
  m_MinMaxNormalize = MinMaxNormalize;
}

void TrackingHandlerOdf::SetSecondOrder(bool SecondOrder)
{
  m_SecondOrder = SecondOrder;
}

vnl_vector_fixed<float,3> TrackingHandlerOdf::ProposeDirection(const itk::Point<float, 3>& pos, std::deque<vnl_vector_fixed<float, 3> >& olddirs, itk::Index<3>& oldIndex)
{

  vnl_vector_fixed<float,3> output_direction; output_direction.fill(0);

  itk::Index<3> idx;
  m_OdfImage->TransformPhysicalPointToIndex(pos, idx);

  if ( !m_OdfImage->GetLargestPossibleRegion().IsInside(idx) )
    return output_direction;

  float gfa = GetImageValue<float>(pos, m_GfaImage, m_Interpolate);
  if (gfa<m_GfaThreshold)
    return output_direction;

  vnl_vector_fixed<float,3> last_dir;
  if (!olddirs.empty())
    last_dir = olddirs.back();

  if (!m_Interpolate && oldIndex==idx)
    return last_dir;

  ItkOdfImageType::PixelType odf_values = GetImageValue<float, QBALL_ODFSIZE>(pos, m_OdfImage, m_Interpolate);
  vnl_vector< float > probs; probs.set_size(m_OdfHemisphereIndices.size());

  float max_odf_val = 0;
  float odf_sum = 0;
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

    odf_sum += odf_values[i];
    probs[c] = odf_values[i];
    c++;
  }

  if (max_idx_d>=0 && (olddirs.empty() || last_dir.magnitude()<=0.5))    // no previous direction, so return principal diffusion direction
  {
    if (m_Mode==MODE::DETERMINISTIC)
    {
      output_direction = m_OdfFloatDirs.get_row(max_idx_d);
      return output_direction * max_odf_val;
    }
    else if (m_Mode==MODE::PROBABILISTIC)
    {
      probs /= odf_sum;
      boost::random::discrete_distribution<int, float> dist(probs.begin(), probs.end());
      int sampled_idx = 0;
      int max_sample_idx = -1;
      float max_prob = 0;
      int trials = 0;

      for (int i=0; i<m_NumProbSamples; i++)
      {
        trials++;
  #pragma omp critical
        {
          boost::random::variate_generator<boost::random::mt19937&, boost::random::discrete_distribution<int,float>> sampler(m_Rng, dist);
          sampled_idx = sampler();
        }
        if (probs[sampled_idx]>max_prob && probs[sampled_idx]>1.2/QBALL_ODFSIZE)
        {
          max_prob = probs[sampled_idx];
          max_sample_idx = sampled_idx;
        }
        else if (probs[sampled_idx]<=1.2/QBALL_ODFSIZE && trials<50)
          i--;
      }
      if (max_sample_idx>=0)
        output_direction = m_OdfFloatDirs.get_row(max_sample_idx);
      return output_direction;
    }
  }
  else if (max_idx_d<0)
  {
    return output_direction;
  }

  if (m_FlipX)
    last_dir[0] *= -1;
  if (m_FlipY)
    last_dir[1] *= -1;
  if (m_FlipZ)
    last_dir[2] *= -1;

  vnl_vector< float > angles = m_OdfFloatDirs*last_dir;
  float probs_sum = 0;
  float max_prob = 0;
  for (unsigned int i=0; i<m_OdfHemisphereIndices.size(); i++)
  {
    float odf_val = probs[i];
    float angle = angles[i];
    float abs_angle = fabs(angle);

    odf_val *= abs_angle; // weight probabilities according to deviation from last direction

    if (m_Mode==MODE::DETERMINISTIC && odf_val>max_prob)
    {
      max_prob = odf_val;
      vnl_vector_fixed<float,3> d = m_OdfFloatDirs.get_row(i);
      if (angle<0)                          // make sure we don't walk backwards
        d *= -1;
      output_direction = odf_val*d;
    }
    else if (m_Mode==MODE::PROBABILISTIC)
    {
      probs[i] = odf_val;
      probs_sum += probs[i];
    }
  }
  if (m_Mode==MODE::PROBABILISTIC && probs_sum>0.0001)
  {
    probs /= probs_sum;

    if (m_SecondOrder)
      probs = GetSecondOrderProbabilities(pos, angles, probs);

    boost::random::discrete_distribution<int, float> dist(probs.begin(), probs.end());

    int sampled_idx = 0;
    int max_sample_idx = -1;
    float max_prob = 0;
    int trials = 0;

    for (int i=0; i<m_NumProbSamples; i++)
    {
      trials++;
#pragma omp critical
      {
        boost::random::variate_generator<boost::random::mt19937&, boost::random::discrete_distribution<int,float>> sampler(m_Rng, dist);
        sampled_idx = sampler();
      }
      if (probs[sampled_idx]>max_prob && probs[sampled_idx]>1.2/QBALL_ODFSIZE)
      {
        max_prob = probs[sampled_idx];
        max_sample_idx = sampled_idx;
      }
      else if (probs[sampled_idx]<=1.2/QBALL_ODFSIZE && trials<50)
        i--;
    }
    if (max_sample_idx>=0)
    {
      output_direction = m_OdfFloatDirs.get_row(max_sample_idx);
      if (angles[max_sample_idx]<0)                          // make sure we don't walk backwards
        output_direction *= -1;
      output_direction *= max_prob;
    }
  }

  float mag = output_direction.magnitude();
  if (mag>=0.0001)
  {
    output_direction.normalize();
    float a = dot_product(output_direction, last_dir);
    if (a<m_AngularThreshold)
      output_direction.fill(0);
  }
  else
    output_direction.fill(0);

  if (m_FlipX)
    output_direction[0] *= -1;
  if (m_FlipY)
    output_direction[1] *= -1;
  if (m_FlipZ)
    output_direction[2] *= -1;

  return output_direction;
}

void TrackingHandlerOdf::SetNumProbSamples(int NumProbSamples)
{
  m_NumProbSamples = NumProbSamples;
}

}

