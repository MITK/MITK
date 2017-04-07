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

namespace mitk
{

TrackingHandlerOdf::TrackingHandlerOdf()
    : m_GfaThreshold(0.1)
    , m_OdfPower(4)
{

}

TrackingHandlerOdf::~TrackingHandlerOdf()
{
}

void TrackingHandlerOdf::InitForTracking()
{
    MITK_INFO << "Initializing ODF tracker.";
    m_OdfHemisphereIndices.clear();
    itk::OrientationDistributionFunction< float, QBALL_ODFSIZE > odf;
    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;

    for (int i=0; i<QBALL_ODFSIZE; i++)
        if (dot_product(ref, odf.GetDirection(i))>0)
            m_OdfHemisphereIndices.push_back(i);
    m_OdfFloatDirs.set_size(m_OdfHemisphereIndices.size(), 3);
    for (int i=0; i<m_OdfHemisphereIndices.size(); i++)
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

    MITK_INFO << "Rescaling ODFs.";
    typedef itk::ImageRegionIterator< ItkOdfImageType > OdfIteratorType;
    OdfIteratorType it(m_OdfImage, m_OdfImage->GetLargestPossibleRegion() );
    it.GoToBegin();
    while( !it.IsAtEnd() )
    {
        typename ItkOdfImageType::PixelType odf_values = it.Get();

        float max = 0;
        float min = 99999;
        for (int i=0; i<QBALL_ODFSIZE; i++)
        {
            if (odf_values[i]<0)
                odf_values[i] = 0;
            if (odf_values[i]>=max)
                max = odf_values[i];
            else if (odf_values[i]<min)
                min = odf_values[i];
        }

        max -= min;
        if (max>0.0)
        {
            odf_values -= min;
            odf_values /= max;
        }

        for (int i=0; i<QBALL_ODFSIZE; i++)
            odf_values[i] = std::pow(odf_values[i], m_OdfPower);

        it.Set(odf_values);
        ++it;
    }
}

vnl_vector_fixed<float,3> TrackingHandlerOdf::ProposeDirection(itk::Point<float, 3>& pos, std::deque<vnl_vector_fixed<float, 3> >& olddirs, itk::Index<3>& oldIndex)
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

    typename ItkOdfImageType::PixelType odf_values = GetImageValue<float, QBALL_ODFSIZE>(pos, m_OdfImage, m_Interpolate);

    float max = 0;
    int max_idx_v = -1;
    int max_idx_d = -1;
    int c = 0;
    for (int i : m_OdfHemisphereIndices)
    {
        if (odf_values[i]<0)
            odf_values[i] = 0;
        if (odf_values[i]>=max)
        {
            max = odf_values[i];
            max_idx_v = i;
            max_idx_d = c;
        }
        c++;
    }

    if (max_idx_v>=0 && (olddirs.empty() || last_dir.magnitude()<=0.5) )    // no previous direction, so return principal diffusion direction
    {
        output_direction = m_OdfFloatDirs.get_row(max_idx_d);
        float odf_val = odf_values[max_idx_v];
        return output_direction * odf_val;
    }
    else if (max_idx_v<0.0001)
        return output_direction;

    if (m_FlipX)
        last_dir[0] *= -1;
    if (m_FlipY)
        last_dir[1] *= -1;
    if (m_FlipZ)
        last_dir[2] *= -1;

    vnl_vector< float > angles = m_OdfFloatDirs*last_dir;
    vnl_vector< float > probs; probs.set_size(m_OdfHemisphereIndices.size());
    float probs_sum = 0;
    for (unsigned int i=0; i<m_OdfHemisphereIndices.size(); i++)
    {
        float odf_val = odf_values[m_OdfHemisphereIndices[i]];
        float angle = angles[i];
        float abs_angle = fabs(angle);

        if (abs_angle<m_AngularThreshold)
            odf_val = 0;

        if (m_Mode==MODE::DETERMINISTIC)
        {
            vnl_vector_fixed<float,3> d = m_OdfFloatDirs.get_row(i);
            if (angle<0)                          // make sure we don't walk backwards
                d *= -1;
            output_direction += odf_val*d;
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
        boost::random::discrete_distribution<int, float> dist(probs.begin(), probs.end());
        boost::random::variate_generator<boost::random::mt19937&, boost::random::discrete_distribution<int,float>> sampler(m_Rng, dist);

        int sampled_idx = sampler();
        output_direction = m_OdfFloatDirs.get_row(sampled_idx);
        if (angles[sampled_idx]<0)                          // make sure we don't walk backwards
            output_direction *= -1;
        output_direction *= probs[sampled_idx];
    }

    if (m_FlipX)
        output_direction[0] *= -1;
    if (m_FlipY)
        output_direction[1] *= -1;
    if (m_FlipZ)
        output_direction[2] *= -1;

    return output_direction;
}

int TrackingHandlerOdf::OdfPower() const
{
    return m_OdfPower;
}

void TrackingHandlerOdf::SetOdfPower(int OdfPower)
{
    m_OdfPower = OdfPower;
}

}

