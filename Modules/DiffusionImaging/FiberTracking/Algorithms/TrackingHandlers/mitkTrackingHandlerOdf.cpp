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
#include <itkOrientationDistributionFunction.h>
#include <itkDiffusionQballGeneralizedFaImageFilter.h>

namespace mitk
{

TrackingHandlerOdf::TrackingHandlerOdf()
    : m_GfaThreshold(0.1)
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

    itk::OrientationDistributionFunction< float, QBALL_ODFSIZE > odf;
    typename ItkOdfImageType::PixelType odf_values = GetImageValue<float, QBALL_ODFSIZE>(pos, m_OdfImage, m_Interpolate);
    float max = 0;
    for (int i=0; i<QBALL_ODFSIZE; i++)
    {
        odf[i] = odf_values[i];
        if (odf[i]<0)
            odf[i] = 0;
        if (odf[i]>max)
            max = odf[i];
    }

    if (max>0.0001)
        odf = odf.MinMaxNormalize();

    if (olddirs.empty() || last_dir.magnitude()<=0.5)    // no previous direction, so return principal diffusion direction
    {
        int pd = odf.GetPrincipleDiffusionDirection();
        vnl_vector_fixed<double,3> odf_d = odf.GetDirection(pd);
        output_direction[0] = odf_d[0];
        output_direction[1] = odf_d[1];
        output_direction[2] = odf_d[2];
        return output_direction * odf[pd];
    }

    if (m_FlipX)
        last_dir[0] *= -1;
    if (m_FlipY)
        last_dir[1] *= -1;
    if (m_FlipZ)
        last_dir[2] *= -1;

    for (int i : m_OdfHemisphereIndices)
    {
        if (odf[i]>0)   // if probability of respective class is 0, do nothing
        {
            vnl_vector_fixed<float,3> d;
            vnl_vector_fixed<double,3> odf_d = odf.GetDirection(i);
            d[0] = odf_d[0];
            d[1] = odf_d[1];
            d[2] = odf_d[2];

            float dot = dot_product(d, last_dir);    // claculate angle between the candidate direction vector and the previous streamline direction
            if (dot<0)                          // make sure we don't walk backwards
                d *= -1;
            dot = fabs(dot);
            float w_i = odf[i]*dot*dot*dot;
            output_direction += w_i*d; // weight contribution to output direction with its probability and the angular deviation from the previous direction
        }
    }

    if (m_FlipX)
        output_direction[0] *= -1;
    if (m_FlipY)
        output_direction[1] *= -1;
    if (m_FlipZ)
        output_direction[2] *= -1;

    return output_direction;
}

}

