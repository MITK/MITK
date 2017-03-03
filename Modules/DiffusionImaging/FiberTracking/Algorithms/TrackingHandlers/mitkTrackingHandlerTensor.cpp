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
    : m_Interpolate(true)
{

}

TrackingHandlerTensor::~TrackingHandlerTensor()
{
}

void TrackingHandlerTensor::InitForTracking()
{
    MITK_INFO << "Initializing tensor tracker.";

    m_NumberOfInputs = m_TensorImages.size();
    for (unsigned int i=0; i<m_NumberOfInputs; i++)
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

    for (int x=0; x<m_TensorImages.at(0)->GetLargestPossibleRegion().GetSize()[0]; x++)
        for (int y=0; y<m_TensorImages.at(0)->GetLargestPossibleRegion().GetSize()[1]; y++)
            for (int z=0; z<m_TensorImages.at(0)->GetLargestPossibleRegion().GetSize()[2]; z++)
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
                    dir.normalize();
                    m_PdImage.at(i)->SetPixel(index, dir);
                    if (!useUserFaImage)
                        m_FaImage->SetPixel(index, m_FaImage->GetPixel(index)+tensor.GetFractionalAnisotropy());
                    m_EmaxImage.at(i)->SetPixel(index, 2/eigenvalues[2]);
                }
                if (!useUserFaImage)
                    m_FaImage->SetPixel(index, m_FaImage->GetPixel(index)/m_NumberOfInputs);
            }
}

vnl_vector_fixed<double,3> TrackingHandlerTensor::ProposeDirection(itk::Point<double, 3>& pos, int& candidates, std::deque<vnl_vector_fixed<double, 3> >& olddirs, double angularThreshold, double& w, itk::Index<3>& oldIndex, ItkUcharImgType::Pointer mask)
{
    w = 1;
    itk::Index<3> index;
    m_TensorImages.at(0)->TransformPhysicalPointToIndex(pos, index);

    vnl_vector_fixed<double,3> output_direction; output_direction.fill(0);
    vnl_vector_fixed<double,3> dirOld;
    if (olddirs.size()>0 && olddirs.at(0).magnitude()>0.5)
        dirOld = olddirs.at(0);
    else
    {
        float fa = m_FaImage->GetPixel(index);
        if (fa<m_FaThreshold)
            return output_direction;

        for (int img=0; img<m_NumberOfInputs; img++)
        {
            vnl_vector_fixed<double,3> newDir = m_PdImage.at(img)->GetPixel(index);   // get principal direction
            if (newDir.magnitude()>output_direction.magnitude())
                output_direction = newDir;
        }

        if (output_direction.magnitude()>mitk::eps)
        {
            candidates = 1;
            output_direction.normalize();
        }
        return output_direction;
    }

    typename TensorType::EigenValuesArrayType eigenvalues;
    typename TensorType::EigenVectorsMatrixType eigenvectors;
    vnl_vector_fixed< double, 8 > interpWeights;

    if (!m_Interpolate)                         // use nearest neighbour interpolation
    {
        float fa = m_FaImage->GetPixel(index);
        if (fa<m_FaThreshold)
            return output_direction;

        if (oldIndex!=index)                    // did we enter a new voxel? if yes, calculate new direction
        {
            double minAngle = 0;
            for (int img=0; img<m_NumberOfInputs; img++)
            {
                vnl_vector_fixed<double,3> newDir = m_PdImage.at(img)->GetPixel(index);   // get principal direction
                if (newDir.magnitude()<mitk::eps)
                    continue;

                typename ItkTensorImageType::PixelType tensor = m_TensorImages.at(img)->GetPixel(index);
                double scale = m_EmaxImage.at(img)->GetPixel(index);

                newDir[0] = m_F*newDir[0] + (1-m_F)*( (1-m_G)*dirOld[0] + scale*m_G*(tensor[0]*dirOld[0] + tensor[1]*dirOld[1] + tensor[2]*dirOld[2]));
                newDir[1] = m_F*newDir[1] + (1-m_F)*( (1-m_G)*dirOld[1] + scale*m_G*(tensor[1]*dirOld[0] + tensor[3]*dirOld[1] + tensor[4]*dirOld[2]));
                newDir[2] = m_F*newDir[2] + (1-m_F)*( (1-m_G)*dirOld[2] + scale*m_G*(tensor[2]*dirOld[0] + tensor[4]*dirOld[1] + tensor[5]*dirOld[2]));
                newDir.normalize();

                double angle = dot_product(dirOld, newDir);
                if (angle<0)
                {
                    newDir *= -1;
                    angle *= -1;
                }

                if (angle>minAngle)
                {
                    minAngle = angle;
                    output_direction = newDir;
                }
            }

            if (minAngle<angularThreshold)
            {
                output_direction.fill(0);
                return output_direction;
            }
        }
        else
            output_direction = dirOld;
    }
    else // use trilinear interpolation (weights calculated in IsValidPosition())
    {
        float fa = GetImageValue<float>(pos, m_FaImage, interpWeights);
        if (fa<m_FaThreshold)
            return output_direction;

        typename ItkTensorImageType::PixelType tensor;
        typename ItkTensorImageType::IndexType tmpIdx = index;
        typename ItkTensorImageType::PixelType tmpTensor;

        if (m_NumberOfInputs>1)
        {
            double minAngle = 0;
            for (int img=0; img<m_NumberOfInputs; img++)
            {
                double angle = dot_product(dirOld, m_PdImage.at(img)->GetPixel(tmpIdx));
                if (std::fabs(angle)>minAngle)
                {
                    minAngle = angle;
                    tmpTensor = m_TensorImages.at(img)->GetPixel(tmpIdx);
                }
            }
            tensor = tmpTensor * interpWeights[0];

            minAngle = 0;
            tmpIdx = index; tmpIdx[0]++;
            for (int img=0; img<m_NumberOfInputs; img++)
            {
                double angle = dot_product(dirOld, m_PdImage.at(img)->GetPixel(tmpIdx));
                if (fabs(angle)>minAngle)
                {
                    minAngle = angle;
                    tmpTensor = m_TensorImages.at(img)->GetPixel(tmpIdx);
                }
            }
            tensor += tmpTensor * interpWeights[1];

            minAngle = 0;
            tmpIdx = index; tmpIdx[1]++;
            for (int img=0; img<m_NumberOfInputs; img++)
            {
                double angle = dot_product(dirOld, m_PdImage.at(img)->GetPixel(tmpIdx));
                if (fabs(angle)>minAngle)
                {
                    minAngle = angle;
                    tmpTensor = m_TensorImages.at(img)->GetPixel(tmpIdx);
                }
            }
            tensor += tmpTensor * interpWeights[2];

            minAngle = 0;
            tmpIdx = index; tmpIdx[2]++;
            for (int img=0; img<m_NumberOfInputs; img++)
            {
                double angle = dot_product(dirOld, m_PdImage.at(img)->GetPixel(tmpIdx));
                if (fabs(angle)>minAngle)
                {
                    minAngle = angle;
                    tmpTensor = m_TensorImages.at(img)->GetPixel(tmpIdx);
                }
            }
            tensor += tmpTensor * interpWeights[3];

            minAngle = 0;
            tmpIdx = index; tmpIdx[0]++; tmpIdx[1]++;
            for (int img=0; img<m_NumberOfInputs; img++)
            {
                double angle = dot_product(dirOld, m_PdImage.at(img)->GetPixel(tmpIdx));
                if (fabs(angle)>minAngle)
                {
                    minAngle = angle;
                    tmpTensor = m_TensorImages.at(img)->GetPixel(tmpIdx);
                }
            }
            tensor += tmpTensor * interpWeights[4];

            minAngle = 0;
            tmpIdx = index; tmpIdx[1]++; tmpIdx[2]++;
            for (int img=0; img<m_NumberOfInputs; img++)
            {
                double angle = dot_product(dirOld, m_PdImage.at(img)->GetPixel(tmpIdx));
                if (fabs(angle)>minAngle)
                {
                    minAngle = angle;
                    tmpTensor = m_TensorImages.at(img)->GetPixel(tmpIdx);
                }
            }
            tensor += tmpTensor * interpWeights[5];

            minAngle = 0;
            tmpIdx = index; tmpIdx[2]++; tmpIdx[0]++;
            for (int img=0; img<m_NumberOfInputs; img++)
            {
                double angle = dot_product(dirOld, m_PdImage.at(img)->GetPixel(tmpIdx));
                if (fabs(angle)>minAngle)
                {
                    minAngle = angle;
                    tmpTensor = m_TensorImages.at(img)->GetPixel(tmpIdx);
                }
            }
            tensor += tmpTensor * interpWeights[6];

            minAngle = 0;
            tmpIdx = index; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
            for (int img=0; img<m_NumberOfInputs; img++)
            {
                double angle = dot_product(dirOld, m_PdImage.at(img)->GetPixel(tmpIdx));
                if (fabs(angle)>minAngle)
                {
                    minAngle = angle;
                    tmpTensor = m_TensorImages.at(img)->GetPixel(tmpIdx);
                }
            }
            tensor += tmpTensor * interpWeights[7];
        }
        else
        {
            tensor = m_TensorImages.at(0)->GetPixel(index) * interpWeights[0];
            typename ItkTensorImageType::IndexType tmpIdx = index; tmpIdx[0]++;
            tensor +=  m_TensorImages.at(0)->GetPixel(tmpIdx) * interpWeights[1];
            tmpIdx = index; tmpIdx[1]++;
            tensor +=  m_TensorImages.at(0)->GetPixel(tmpIdx) * interpWeights[2];
            tmpIdx = index; tmpIdx[2]++;
            tensor +=  m_TensorImages.at(0)->GetPixel(tmpIdx) * interpWeights[3];
            tmpIdx = index; tmpIdx[0]++; tmpIdx[1]++;
            tensor +=  m_TensorImages.at(0)->GetPixel(tmpIdx) * interpWeights[4];
            tmpIdx = index; tmpIdx[1]++; tmpIdx[2]++;
            tensor +=  m_TensorImages.at(0)->GetPixel(tmpIdx) * interpWeights[5];
            tmpIdx = index; tmpIdx[2]++; tmpIdx[0]++;
            tensor +=  m_TensorImages.at(0)->GetPixel(tmpIdx) * interpWeights[6];
            tmpIdx = index; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
            tensor +=  m_TensorImages.at(0)->GetPixel(tmpIdx) * interpWeights[7];
        }

        tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);
        output_direction[0] = eigenvectors(2, 0);
        output_direction[1] = eigenvectors(2, 1);
        output_direction[2] = eigenvectors(2, 2);
        if (output_direction.magnitude()<mitk::eps)
            return output_direction;
        output_direction.normalize();

        double scale = 2/eigenvalues[2];
        output_direction[0] = m_F*output_direction[0] + (1-m_F)*( (1-m_G)*dirOld[0] + scale*m_G*(tensor[0]*dirOld[0] + tensor[1]*dirOld[1] + tensor[2]*dirOld[2]));
        output_direction[1] = m_F*output_direction[1] + (1-m_F)*( (1-m_G)*dirOld[1] + scale*m_G*(tensor[1]*dirOld[0] + tensor[3]*dirOld[1] + tensor[4]*dirOld[2]));
        output_direction[2] = m_F*output_direction[2] + (1-m_F)*( (1-m_G)*dirOld[2] + scale*m_G*(tensor[2]*dirOld[0] + tensor[4]*dirOld[1] + tensor[5]*dirOld[2]));
        output_direction.normalize();

        double angle = dot_product(dirOld, output_direction);
        if (angle<0)
        {
            output_direction *= -1;
            angle *= -1;
        }

        if (angle<angularThreshold)
        {
            output_direction.fill(0);
            return output_direction;
        }
    }

    if (output_direction.magnitude()>mitk::eps)
    {
        candidates = 1;
        output_direction.normalize();
    }

    return output_direction;
}

}

