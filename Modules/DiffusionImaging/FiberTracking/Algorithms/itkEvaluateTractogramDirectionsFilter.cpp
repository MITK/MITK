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

#ifndef __itkEvaluateTractogramDirectionsFilter_cpp
#define __itkEvaluateTractogramDirectionsFilter_cpp

#include "itkEvaluateTractogramDirectionsFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkImageDuplicator.h>
#include <boost/progress.hpp>
#include <mitkDiffusionFunctionCollection.h>

namespace itk {

template< class PixelType >
EvaluateTractogramDirectionsFilter< PixelType >
::EvaluateTractogramDirectionsFilter():
    m_ReferenceImageSet(nullptr),
    m_IgnoreMissingDirections(false),
    m_Eps(0.0001),
    m_UseInterpolation(false)
{
    this->SetNumberOfOutputs(1);
}

template< class PixelType >
itk::Vector<PixelType, 3> EvaluateTractogramDirectionsFilter< PixelType >::GetItkVector(double point[3])
{
    itk::Vector<PixelType, 3> itkVector;
    itkVector[0] = point[0];
    itkVector[1] = point[1];
    itkVector[2] = point[2];
    return itkVector;
}

template< class PixelType >
vnl_vector_fixed<PixelType, 3> EvaluateTractogramDirectionsFilter< PixelType >::GetVnlVector(double point[3])
{
    vnl_vector_fixed<PixelType, 3> vnlVector;
    vnlVector[0] = point[0];
    vnlVector[1] = point[1];
    vnlVector[2] = point[2];
    return vnlVector;
}

template< class PixelType >
vnl_vector_fixed<PixelType, 3> EvaluateTractogramDirectionsFilter< PixelType >::GetVnlVector(Vector<PixelType,3>& vector)
{
    vnl_vector_fixed<PixelType, 3> vnlVector;
    vnlVector[0] = vector[0];
    vnlVector[1] = vector[1];
    vnlVector[2] = vector[2];
    return vnlVector;
}

template< class PixelType >
void EvaluateTractogramDirectionsFilter< PixelType >::GenerateData()
{
    if (m_Tractogram.IsNull() || m_ReferenceImageSet.IsNull())
        return;

    if (m_UseInterpolation)
        MITK_INFO << "Using trilinear interpolation";
    else
        MITK_INFO << "Using nearest neighbor interpolation";

    if (m_IgnoreMissingDirections)
        MITK_INFO << "Ignoring missing directions";
    else
        MITK_INFO << "Penalizing missing directions";

    // angular error image
    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetOrigin( m_ReferenceImageSet->GetElement(0)->GetOrigin() );
    outputImage->SetRegions( m_ReferenceImageSet->GetElement(0)->GetLargestPossibleRegion() );
    outputImage->SetSpacing( m_ReferenceImageSet->GetElement(0)->GetSpacing() );
    outputImage->SetDirection( m_ReferenceImageSet->GetElement(0)->GetDirection() );
    outputImage->Allocate();
    outputImage->FillBuffer(0.0);

    DoubleImageType::Pointer counterImage = DoubleImageType::New();
    counterImage->SetOrigin( m_ReferenceImageSet->GetElement(0)->GetOrigin() );
    counterImage->SetRegions( m_ReferenceImageSet->GetElement(0)->GetLargestPossibleRegion() );
    counterImage->SetSpacing( m_ReferenceImageSet->GetElement(0)->GetSpacing() );
    counterImage->SetDirection( m_ReferenceImageSet->GetElement(0)->GetDirection() );
    counterImage->Allocate();
    counterImage->FillBuffer(0.0);

    std::vector< DoubleImageType::Pointer > directionFound;
    for (int i=0; i<m_ReferenceImageSet->Size(); i++)
    {
        DoubleImageType::Pointer check = DoubleImageType::New();
        check->SetOrigin( m_ReferenceImageSet->GetElement(0)->GetOrigin() );
        check->SetRegions( m_ReferenceImageSet->GetElement(0)->GetLargestPossibleRegion() );
        check->SetSpacing( m_ReferenceImageSet->GetElement(0)->GetSpacing() );
        check->SetDirection( m_ReferenceImageSet->GetElement(0)->GetDirection() );
        check->Allocate();
        check->FillBuffer(90);
        directionFound.push_back(check);
    }

    if (m_MaskImage.IsNull())
    {
        m_MaskImage = UCharImageType::New();
        m_MaskImage->SetOrigin( outputImage->GetOrigin() );
        m_MaskImage->SetRegions( outputImage->GetLargestPossibleRegion() );
        m_MaskImage->SetSpacing( outputImage->GetSpacing() );
        m_MaskImage->SetDirection( outputImage->GetDirection() );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }

    m_MeanAngularError = 0.0;
    m_MedianAngularError = 0;
    m_MaxAngularError = 0.0;
    m_MinAngularError = itk::NumericTraits<float>::max();
    m_VarAngularError = 0.0;
    m_AngularErrorVector.clear();

    float minSpacing = 1;
    if(outputImage->GetSpacing()[0]<outputImage->GetSpacing()[1] && outputImage->GetSpacing()[0]<outputImage->GetSpacing()[2])
        minSpacing = outputImage->GetSpacing()[0];
    else if (outputImage->GetSpacing()[1] < outputImage->GetSpacing()[2])
        minSpacing = outputImage->GetSpacing()[1];
    else
        minSpacing = outputImage->GetSpacing()[2];
    FiberBundleType::Pointer fiberBundle = m_Tractogram->GetDeepCopy();
    fiberBundle->ResampleFibers(minSpacing/10);

    MITK_INFO << "Evaluating tractogram";
    vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
    boost::progress_display disp( fiberBundle->GetNumFibers() );
    for( int i=0; i<fiberBundle->GetNumFibers(); i++ )
    {
        vtkCell* cell = fiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        if (numPoints<2)
            continue;

        for( int j=0; j<numPoints; j++)
        {
            double* temp = points->GetPoint(j);
            itk::Point<PixelType, 3> vertex = mitk::imv::GetItkPoint<PixelType>(temp);
            itk::Vector<PixelType> v = GetItkVector(temp);

            itk::Vector<PixelType, 3> dir(3);
            if (j<numPoints-1)
                dir = GetItkVector(points->GetPoint(j+1))-v;
            else
                dir = v-GetItkVector(points->GetPoint(j-1));

            vnl_vector_fixed< PixelType, 3 > fiberDir = GetVnlVector(dir);
            fiberDir.normalize();

            itk::Index<3> idx;
            itk::ContinuousIndex<float, 3> contIndex;
            m_MaskImage->TransformPhysicalPointToIndex(vertex, idx);
            m_MaskImage->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

            if (!m_UseInterpolation)    // use nearest neighbour interpolation
            {
                if (!m_MaskImage->GetLargestPossibleRegion().IsInside(idx) || m_MaskImage->GetPixel(idx)<=0)
                    continue;

                double angle = 90;
                int usedIndex = -1;
                for (int k=0; k<m_ReferenceImageSet->Size(); k++)     // and each test direction
                {
                    vnl_vector_fixed< PixelType, 3 > refDir = m_ReferenceImageSet->GetElement(k)->GetPixel(idx).GetVnlVector();
                    if (refDir.magnitude()>m_Eps)  // normalize if not null
                        refDir.normalize();
                    else
                        continue;

                    // calculate angle between directions
                    double tempAngle = acos(fabs(dot_product(refDir, fiberDir)))*180.0/itk::Math::pi;
                    directionFound.at(k)->SetPixel(idx, tempAngle);

                    if (tempAngle < angle)
                    {
                        angle = tempAngle;
                        usedIndex = k;
                    }
                }
                if (usedIndex>=0)
                    directionFound.at(usedIndex)->SetPixel(idx, -1);
                else if (m_IgnoreMissingDirections)
                    angle = 0;

                counterImage->SetPixel(idx, counterImage->GetPixel(idx)+1);
                outputImage->SetPixel(idx, outputImage->GetPixel(idx)+angle);
                continue;
            }

            double frac_x = contIndex[0] - idx[0];
            double frac_y = contIndex[1] - idx[1];
            double frac_z = contIndex[2] - idx[2];
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

            // use trilinear interpolation
            itk::Index<3> newIdx;
            for (int x=0; x<2; x++)
            {
                frac_x = 1-frac_x;
                for (int y=0; y<2; y++)
                {
                    frac_y = 1-frac_y;
                    for (int z=0; z<2; z++)
                    {
                        frac_z = 1-frac_z;

                        newIdx[0] = idx[0]+x;
                        newIdx[1] = idx[1]+y;
                        newIdx[2] = idx[2]+z;

                        double frac = frac_x*frac_y*frac_z;

                        // is position valid?
                        if (!m_MaskImage->GetLargestPossibleRegion().IsInside(newIdx) || m_MaskImage->GetPixel(newIdx)<=0)
                            continue;

                        double angle = 90;
                        int usedIndex = -1;
                        for (int k=0; k<m_ReferenceImageSet->Size(); k++)     // and each test direction
                        {
                            vnl_vector_fixed< PixelType, 3 > refDir = m_ReferenceImageSet->GetElement(k)->GetPixel(newIdx).GetVnlVector();
                            if (refDir.magnitude()>m_Eps)  // normalize if not null
                                refDir.normalize();
                            else
                                continue;

                            // calculate angle between directions
                            double tempAngle = acos(fabs(dot_product(refDir, fiberDir)))*180.0/itk::Math::pi;
                            directionFound.at(k)->SetPixel(newIdx, tempAngle);

                            if (tempAngle < angle)
                            {
                                angle = tempAngle;
                                usedIndex = k;
                            }
                        }
                        if (usedIndex>=0)
                            directionFound.at(usedIndex)->SetPixel(newIdx, -1);
                        else if (m_IgnoreMissingDirections)
                            angle = 0;

                        counterImage->SetPixel(newIdx, counterImage->GetPixel(newIdx)+1);
                        outputImage->SetPixel(newIdx, outputImage->GetPixel(newIdx)+frac*angle);
                    }
                }
            }
        }
        ++disp;
    }

    ImageRegionIterator< OutputImageType > oit(outputImage, outputImage->GetLargestPossibleRegion());
    ImageRegionIterator< UCharImageType > mit(m_MaskImage, m_MaskImage->GetLargestPossibleRegion());
    ImageRegionIterator< DoubleImageType > cit(counterImage, counterImage->GetLargestPossibleRegion());


    if (!m_IgnoreMissingDirections)
        MITK_INFO << "Creatings statistics and accounting for missing directions";
    else
        MITK_INFO << "Creatings statistics";
    boost::progress_display disp2(outputImage->GetLargestPossibleRegion().GetNumberOfPixels());
    while( !oit.IsAtEnd() )
    {
        if ( mit.Get()>0)
        {
            if ( cit.Get()>m_Eps )
                oit.Set(oit.Get()/cit.Get());

            if (!m_IgnoreMissingDirections)
            {
                int missingCount = 0;

                if ( cit.Get()>m_Eps )
                    missingCount++;

                for (int i=0; i<directionFound.size(); i++)
                {
                    if ( directionFound.at(i)->GetPixel(oit.GetIndex())>0 && m_ReferenceImageSet->GetElement(i)->GetPixel(oit.GetIndex()).GetVnlVector().magnitude()>m_Eps )
                    {
                        oit.Set(oit.Get()+directionFound.at(i)->GetPixel(oit.GetIndex()));
                        missingCount++;
                    }
                }
                if (missingCount>1)
                    oit.Set(oit.Get()/missingCount);
            }

            if (oit.Get()>m_MaxAngularError)
                m_MaxAngularError = oit.Get();
            if (oit.Get()<m_MinAngularError)
                m_MinAngularError = oit.Get();
        }

        ++oit;
        ++cit;
        ++mit;
        ++disp2;
    }

    this->SetNthOutput(0, outputImage);
}

}

#endif // __itkEvaluateTractogramDirectionsFilter_cpp
