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

namespace mitk
{

TrackingHandlerOdf::TrackingHandlerOdf()
    : m_GfaThreshold(0.1)
    , m_OdfPower(4)
    , m_SecondOrder(false)
    , m_MinMaxNormalize(true)
{

}

TrackingHandlerOdf::~TrackingHandlerOdf()
{
}

void TrackingHandlerOdf::InitForTracking()
{
    MITK_INFO << "Initializing ODF tracker.";
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

//    itk::OrientationDistributionFunction< float, 42 > small_set;
//    for (int j=0; j<12; j++)
//    {
//        vnl_vector_fixed<double,3> v = small_set.GetDirection(j);
//        vnl_vector_fixed<float,3> float_v;
//        float_v[0]=v[0];
//        float_v[1]=v[1];
//        float_v[2]=v[2];

//        if (dot_product(ref, v)<=0)
//            continue;

//        int min_idx = 0;
//        float min_angle = -1;
//        for (unsigned int i=0; i<m_OdfHemisphereIndices.size(); i++)
//        {
//            float angle = dot_product(float_v, m_OdfFloatDirs.get_row(i));
//            if (angle>min_angle)
//            {
//                min_idx = i;
//                min_angle = angle;
//            }
//        }
//        m_OdfReducedIndices.push_back(min_idx);
//    }

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

    m_WorkingOdfImage = ItkOdfImageType::New();
    m_WorkingOdfImage->SetSpacing( m_OdfImage->GetSpacing() );
    m_WorkingOdfImage->SetOrigin( m_OdfImage->GetOrigin() );
    m_WorkingOdfImage->SetDirection( m_OdfImage->GetDirection() );
    m_WorkingOdfImage->SetRegions( m_OdfImage->GetLargestPossibleRegion() );
    m_WorkingOdfImage->Allocate();

    MITK_INFO << "Rescaling ODFs.";
    typedef itk::ImageRegionIterator< ItkOdfImageType > OdfIteratorType;
    OdfIteratorType it(m_OdfImage, m_OdfImage->GetLargestPossibleRegion() );
    OdfIteratorType wit(m_WorkingOdfImage, m_WorkingOdfImage->GetLargestPossibleRegion() );
    it.GoToBegin();
    wit.GoToBegin();
    while( !it.IsAtEnd() )
    {
        ItkOdfImageType::PixelType odf_values = it.Get();
        ItkOdfImageType::PixelType wodf_values = wit.Get();

        if (m_MinMaxNormalize)
        {
            float max = 0;
            float min = 99999;
            for (int i=0; i<QBALL_ODFSIZE; i++)
            {
                wodf_values[i] = odf_values[i];
                if (wodf_values[i]<0)
                    wodf_values[i] = 0;
                if (wodf_values[i]>=max)
                    max = wodf_values[i];
                else if (wodf_values[i]<min)
                    min = wodf_values[i];
            }

            max -= min;
            if (max>0.0)
            {
                wodf_values -= min;
                wodf_values /= max;
            }
        }

        for (int i=0; i<QBALL_ODFSIZE; i++)
            wodf_values[i] = std::pow(wodf_values[i], m_OdfPower);

        wit.Set(wodf_values);
        ++it;
        ++wit;
    }
}

vnl_vector< float > TrackingHandlerOdf::GetSecondOrderProbabilities(itk::Point<float, 3>& itkP, vnl_vector< float >& angles, vnl_vector< float >& probs)
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
        ItkOdfImageType::PixelType odf_values = GetImageValue<float, QBALL_ODFSIZE>(pos, m_WorkingOdfImage, m_Interpolate);
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

void TrackingHandlerOdf::setMinMaxNormalize(bool MinMaxNormalize)
{
    m_MinMaxNormalize = MinMaxNormalize;
}

void TrackingHandlerOdf::SetSecondOrder(bool SecondOrder)
{
    m_SecondOrder = SecondOrder;
}

vnl_vector_fixed<float,3> TrackingHandlerOdf::ProposeDirection(itk::Point<float, 3>& pos, std::deque<vnl_vector_fixed<float, 3> >& olddirs, itk::Index<3>& oldIndex)
{

    vnl_vector_fixed<float,3> output_direction; output_direction.fill(0);

    itk::Index<3> idx;
    m_WorkingOdfImage->TransformPhysicalPointToIndex(pos, idx);

    if ( !m_WorkingOdfImage->GetLargestPossibleRegion().IsInside(idx) )
        return output_direction;

    float gfa = GetImageValue<float>(pos, m_GfaImage, m_Interpolate);
    if (gfa<m_GfaThreshold)
        return output_direction;

    vnl_vector_fixed<float,3> last_dir;
    if (!olddirs.empty())
        last_dir = olddirs.back();

    if (!m_Interpolate && oldIndex==idx)
        return last_dir;

    ItkOdfImageType::PixelType odf_values = GetImageValue<float, QBALL_ODFSIZE>(pos, m_WorkingOdfImage, m_Interpolate);

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

        if (m_SecondOrder)
            probs = GetSecondOrderProbabilities(pos, angles, probs);

        boost::random::discrete_distribution<int, float> dist(probs.begin(), probs.end());

        int sampled_idx = 0;
#pragma omp critical
        {
            boost::random::variate_generator<boost::random::mt19937&, boost::random::discrete_distribution<int,float>> sampler(m_Rng, dist);
            sampled_idx = sampler();
        }
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

