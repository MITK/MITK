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

#include "mitkTrackingHandlerTensor.h"

namespace mitk
{

TrackingHandlerTensor::TrackingHandlerTensor()
    : m_InterpolateTensors(true)
    , m_FaThreshold(0.1)
{

}

TrackingHandlerTensor::~TrackingHandlerTensor()
{
}

void TrackingHandlerTensor::InitForTracking()
{
    MITK_INFO << "Initializing tensor tracker.";

    m_NumberOfInputs = m_TensorImages.size();
    for (int i=0; i<m_NumberOfInputs; i++)
    {
        ItkPDImgType::Pointer pdImage = ItkPDImgType::New();
        pdImage->SetSpacing( m_TensorImages.at(0)->GetSpacing() );
        pdImage->SetOrigin( m_TensorImages.at(0)->GetOrigin() );
        pdImage->SetDirection( m_TensorImages.at(0)->GetDirection() );
        pdImage->SetRegions( m_TensorImages.at(0)->GetLargestPossibleRegion() );
        pdImage->Allocate();
        m_PdImage.push_back(pdImage);

        ItkDoubleImgType::Pointer emaxImage = ItkDoubleImgType::New();
        emaxImage->SetSpacing( m_TensorImages.at(0)->GetSpacing() );
        emaxImage->SetOrigin( m_TensorImages.at(0)->GetOrigin() );
        emaxImage->SetDirection( m_TensorImages.at(0)->GetDirection() );
        emaxImage->SetRegions( m_TensorImages.at(0)->GetLargestPossibleRegion() );
        emaxImage->Allocate();
        emaxImage->FillBuffer(1.0);
        m_EmaxImage.push_back(emaxImage);
    }

    bool useUserFaImage = true;
    if (m_FaImage.IsNull())
    {
        m_FaImage = ItkFloatImgType::New();
        m_FaImage->SetSpacing( m_TensorImages.at(0)->GetSpacing() );
        m_FaImage->SetOrigin( m_TensorImages.at(0)->GetOrigin() );
        m_FaImage->SetDirection( m_TensorImages.at(0)->GetDirection() );
        m_FaImage->SetRegions( m_TensorImages.at(0)->GetLargestPossibleRegion() );
        m_FaImage->Allocate();
        m_FaImage->FillBuffer(0.0);
        useUserFaImage = false;
    }

    typedef itk::DiffusionTensor3D<float>    TensorType;
    typename TensorType::EigenValuesArrayType eigenvalues;
    typename TensorType::EigenVectorsMatrixType eigenvectors;
    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0] = 1;
    vnl_vector_fixed<double,3> ref2; ref2.fill(0); ref2[1] = 1;
    vnl_vector_fixed<double,3> ref3; ref3.fill(0); ref3[2] = 1;

    for (unsigned int x=0; x<m_TensorImages.at(0)->GetLargestPossibleRegion().GetSize()[0]; x++)
        for (unsigned int y=0; y<m_TensorImages.at(0)->GetLargestPossibleRegion().GetSize()[1]; y++)
            for (unsigned int z=0; z<m_TensorImages.at(0)->GetLargestPossibleRegion().GetSize()[2]; z++)
            {
                typename ItkTensorImageType::IndexType index;
                index[0] = x; index[1] = y; index[2] = z;
                for (int i=0; i<m_NumberOfInputs; i++)
                {
                    typename ItkTensorImageType::PixelType tensor = m_TensorImages.at(i)->GetPixel(index);
                    vnl_vector_fixed<double,3> dir;
                    tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);
                    dir[0] = eigenvectors(2, 0);
                    dir[1] = eigenvectors(2, 1);
                    dir[2] = eigenvectors(2, 2);
                    if (dir.magnitude()>mitk::eps)
                    {
                        dir.normalize();
                        if (dot_product(ref,dir)<0)
                            dir *= -1;
                        if (dot_product(ref2,dir)<0)
                            dir *= -1;
                        if (dot_product(ref3,dir)<0)
                            dir *= -1;
                    }
                    else
                        dir.fill(0.0);
                    m_PdImage.at(i)->SetPixel(index, dir);
                    if (!useUserFaImage)
                        m_FaImage->SetPixel(index, m_FaImage->GetPixel(index)+tensor.GetFractionalAnisotropy());
                    m_EmaxImage.at(i)->SetPixel(index, 2/eigenvalues[2]);
                }
                if (!useUserFaImage)
                    m_FaImage->SetPixel(index, m_FaImage->GetPixel(index)/m_NumberOfInputs);
            }
}

vnl_vector_fixed<double,3> TrackingHandlerTensor::GetDirection(itk::Point<float, 3> itkP, itk::Image<vnl_vector_fixed<double,3>, 3>* image, bool interpolate){
    // transform physical point to index coordinates
    itk::Index<3> idx;
    itk::ContinuousIndex< double, 3> cIdx;
    image->TransformPhysicalPointToIndex(itkP, idx);
    image->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

    vnl_vector_fixed<double,3> dir; dir.fill(0.0);
    if ( image->GetLargestPossibleRegion().IsInside(idx) )
    {
        dir = image->GetPixel(idx);
        if (!interpolate)
        {
            if (m_FlipX)
              dir[0] *= -1;
            if (m_FlipY)
              dir[1] *= -1;
            if (m_FlipZ)
              dir[2] *= -1;
            return dir;
        }
    }
    else
        return dir;

    double frac_x = cIdx[0] - idx[0];
    double frac_y = cIdx[1] - idx[1];
    double frac_z = cIdx[2] - idx[2];
    if (frac_x<0)
    {
        idx[0] -= 1;
        frac_x += 1;
    }
    if (frac_y<0)
    {
        idx[1] -= 1;
        frac_y += 1;
    }
    if (frac_z<0)
    {
        idx[2] -= 1;
        frac_z += 1;
    }
    frac_x = 1-frac_x;
    frac_y = 1-frac_y;
    frac_z = 1-frac_z;

    // int coordinates inside image?
    if (idx[0] >= 0 && idx[0] < image->GetLargestPossibleRegion().GetSize(0)-1 &&
            idx[1] >= 0 && idx[1] < image->GetLargestPossibleRegion().GetSize(1)-1 &&
            idx[2] >= 0 && idx[2] < image->GetLargestPossibleRegion().GetSize(2)-1)
    {
        // trilinear interpolation
        vnl_vector_fixed<double, 8> interpWeights;
        interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
        interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
        interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
        interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);

        dir = image->GetPixel(idx) * interpWeights[0];

        typename itk::Image<vnl_vector_fixed<double,3>, 3>::IndexType tmpIdx = idx; tmpIdx[0]++;
        dir +=  image->GetPixel(tmpIdx) * interpWeights[1];

        tmpIdx = idx; tmpIdx[1]++;
        dir +=  image->GetPixel(tmpIdx) * interpWeights[2];

        tmpIdx = idx; tmpIdx[2]++;
        dir +=  image->GetPixel(tmpIdx) * interpWeights[3];

        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++;
        dir +=  image->GetPixel(tmpIdx) * interpWeights[4];

        tmpIdx = idx; tmpIdx[1]++; tmpIdx[2]++;
        dir +=  image->GetPixel(tmpIdx) * interpWeights[5];

        tmpIdx = idx; tmpIdx[2]++; tmpIdx[0]++;
        dir +=  image->GetPixel(tmpIdx) * interpWeights[6];

        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
        dir +=  image->GetPixel(tmpIdx) * interpWeights[7];
    }

    if (m_FlipX)
      dir[0] *= -1;
    if (m_FlipY)
      dir[1] *= -1;
    if (m_FlipZ)
      dir[2] *= -1;

    return dir;
}

vnl_vector_fixed<double,3> TrackingHandlerTensor::GetLargestEigenvector(TensorType& tensor)
{
    vnl_vector_fixed<double,3> dir;
    typename TensorType::EigenValuesArrayType eigenvalues;
    typename TensorType::EigenVectorsMatrixType eigenvectors;
    tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);
    dir[0] = eigenvectors(2, 0);
    dir[1] = eigenvectors(2, 1);
    dir[2] = eigenvectors(2, 2);
    if (dir.magnitude()<mitk::eps)
        dir.fill(0.0);
    else
    {
        if (m_FlipX)
          dir[0] *= -1;
        if (m_FlipY)
          dir[1] *= -1;
        if (m_FlipZ)
          dir[2] *= -1;
    }
    return dir;
}

vnl_vector_fixed<double,3> TrackingHandlerTensor::ProposeDirection(itk::Point<double, 3>& pos, int& candidates, std::deque<vnl_vector_fixed<double, 3> >& olddirs, double angularThreshold, double& w, itk::Index<3>& oldIndex, ItkUcharImgType::Pointer mask)
{
    vnl_vector_fixed<double,3> output_direction; output_direction.fill(0);

    try
    {
        w = 1;
        itk::Index<3> index;
        m_TensorImages.at(0)->TransformPhysicalPointToIndex(pos, index);

        float fa = GetImageValue<float>(pos, m_FaImage, m_Interpolate);

        vnl_vector_fixed<double,3> oldDir;
        if (olddirs.size()>0 && olddirs.back().magnitude()>mitk::eps)
            oldDir = olddirs.at(0);
        else
        {
            if (fa<m_FaThreshold)
                return output_direction;

            float e_max = 0;
            float mag = 0;
            for (int i=0; i<m_NumberOfInputs; i++)
            {
                double e = GetImageValue<double>(pos, m_EmaxImage[i], m_Interpolate);
                if (e>e_max)
                {
                    if (m_InterpolateTensors && m_Interpolate)
                    {
                        TensorType tensor = GetImageValue<TensorType>(pos, m_TensorImages[i], m_Interpolate);
                        output_direction = GetLargestEigenvector(tensor);
                    }
                    else
                        output_direction = GetDirection(pos, m_PdImage[i], m_Interpolate);

                    mag = output_direction.magnitude();
                    e_max = e;
                }
            }

            if (mag>mitk::eps)
            {
                candidates = 1;
                output_direction.normalize();
            }
            else
                output_direction.fill(0);

            return output_direction;
        }

        if (fa<m_FaThreshold)
            return output_direction;

        if (!m_Interpolate && oldIndex==index)
        {
            candidates = 1;
            return oldDir;
        }


        float max_angle = 0;
        TensorType tensor; tensor.Fill(0.0);

        for (int i=0; i<m_NumberOfInputs; i++)
        {
            vnl_vector_fixed<double,3> newDir;
            if (m_InterpolateTensors && m_Interpolate)
            {
                tensor = GetImageValue<TensorType>(pos, m_TensorImages[i], m_Interpolate);
                newDir = GetLargestEigenvector(tensor);
            }
            else
                newDir = GetDirection(pos, m_PdImage[i], m_Interpolate);

            float mag = newDir.magnitude();

            if (mag<mitk::eps)
                continue;
            else
                newDir.normalize();

            double angle = dot_product(oldDir, newDir);
            if (angle<0)
            {
                newDir *= -1;
                angle *= -1;
            }

            if (m_G>mitk::eps)  // TEND tracking
            {
                if (!m_InterpolateTensors || !m_Interpolate)
                    tensor = GetImageValue<TensorType>(pos, m_TensorImages[i], m_Interpolate);
                double scale = GetImageValue<double>(pos, m_EmaxImage[i], m_Interpolate);

                newDir[0] = m_F*newDir[0] + (1-m_F)*( (1-m_G)*oldDir[0] + scale*m_G*(tensor[0]*oldDir[0] + tensor[1]*oldDir[1] + tensor[2]*oldDir[2]));
                newDir[1] = m_F*newDir[1] + (1-m_F)*( (1-m_G)*oldDir[1] + scale*m_G*(tensor[1]*oldDir[0] + tensor[3]*oldDir[1] + tensor[4]*oldDir[2]));
                newDir[2] = m_F*newDir[2] + (1-m_F)*( (1-m_G)*oldDir[2] + scale*m_G*(tensor[2]*oldDir[0] + tensor[4]*oldDir[1] + tensor[5]*oldDir[2]));

                newDir.normalize();

                angle = dot_product(oldDir, newDir);
                if (angle<0)
                {
                    newDir *= -1;
                    angle *= -1;
                }
            }

            if (angle>max_angle)
            {
                max_angle = angle;
                output_direction = newDir;
            }
        }

        if (max_angle<angularThreshold)
        {
            output_direction.fill(0);
            return output_direction;
        }

        if (output_direction.magnitude()>mitk::eps)
            candidates = 1;
    }
    catch(...)
    {

    }

    return output_direction;
}

}

