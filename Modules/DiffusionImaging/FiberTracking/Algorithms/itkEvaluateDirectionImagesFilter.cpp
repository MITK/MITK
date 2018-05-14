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

#ifndef __itkEvaluateDirectionImagesFilter_cpp
#define __itkEvaluateDirectionImagesFilter_cpp

#include "itkEvaluateDirectionImagesFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkImageDuplicator.h>
#include <boost/progress.hpp>

namespace itk {

template< class PixelType >
EvaluateDirectionImagesFilter< PixelType >
::EvaluateDirectionImagesFilter():
    m_ImageSet(nullptr),
    m_ReferenceImageSet(nullptr),
    m_IgnoreMissingDirections(false),
    m_IgnoreEmptyVoxels(false),
    m_Eps(0.0001)
{
    this->SetNumberOfIndexedOutputs(2);
}

template< class PixelType >
void EvaluateDirectionImagesFilter< PixelType >::GenerateData()
{
    if (m_ImageSet.IsNull() || m_ReferenceImageSet.IsNull())
        return;

    DirectionImageContainerType::Pointer set1 = DirectionImageContainerType::New();
    DirectionImageContainerType::Pointer set2 = DirectionImageContainerType::New();

    for (unsigned int i=0; i<m_ImageSet->Size(); i++)
    {
        typename itk::ImageDuplicator< DirectionImageType >::Pointer duplicator = itk::ImageDuplicator< DirectionImageType >::New();
        duplicator->SetInputImage( m_ImageSet->GetElement(i) );
        duplicator->Update();
        set1->InsertElement(i, dynamic_cast<DirectionImageType*>(duplicator->GetOutput()));
    }
    for (unsigned int i=0; i<m_ReferenceImageSet->Size(); i++)
    {
        typename itk::ImageDuplicator< DirectionImageType >::Pointer duplicator = itk::ImageDuplicator< DirectionImageType >::New();
        duplicator->SetInputImage( m_ReferenceImageSet->GetElement(i) );
        duplicator->Update();
        set2->InsertElement(i, dynamic_cast<DirectionImageType*>(duplicator->GetOutput()));
    }

    m_ImageSet = set1;
    m_ReferenceImageSet = set2;

    // angular error image
    typename OutputImageType::Pointer outputImage = OutputImageType::New();
    outputImage->SetOrigin( m_ReferenceImageSet->GetElement(0)->GetOrigin() );
    outputImage->SetRegions( m_ReferenceImageSet->GetElement(0)->GetLargestPossibleRegion() );
    outputImage->SetSpacing( m_ReferenceImageSet->GetElement(0)->GetSpacing() );
    outputImage->SetDirection( m_ReferenceImageSet->GetElement(0)->GetDirection() );
    outputImage->Allocate();
    outputImage->FillBuffer(0.0);
    this->SetNthOutput(0, outputImage);

    // length error image
    outputImage = OutputImageType::New();
    outputImage->SetOrigin( m_ReferenceImageSet->GetElement(0)->GetOrigin() );
    outputImage->SetRegions( m_ReferenceImageSet->GetElement(0)->GetLargestPossibleRegion() );
    outputImage->SetSpacing( m_ReferenceImageSet->GetElement(0)->GetSpacing() );
    outputImage->SetDirection( m_ReferenceImageSet->GetElement(0)->GetDirection() );
    outputImage->Allocate();
    outputImage->FillBuffer(0.0);
    this->SetNthOutput(1, outputImage);

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

    m_MeanLengthError = 0.0;
    m_MedianLengthError = 0;
    m_MaxLengthError = 0.0;
    m_MinLengthError = itk::NumericTraits<float>::max();
    m_VarLengthError = 0.0;
    m_LengthErrorVector.clear();

    if (m_ImageSet.IsNull() || m_ReferenceImageSet.IsNull())
        return;

    outputImage = static_cast< OutputImageType* >(this->ProcessObject::GetOutput(0));
    typename OutputImageType::Pointer outputImage2 = static_cast< OutputImageType* >(this->ProcessObject::GetOutput(1));

    ImageRegionIterator< OutputImageType > oit(outputImage, outputImage->GetLargestPossibleRegion());
    ImageRegionIterator< OutputImageType > oit2(outputImage2, outputImage2->GetLargestPossibleRegion());
    ImageRegionIterator< UCharImageType > mit(m_MaskImage, m_MaskImage->GetLargestPossibleRegion());

    int numTestImages = m_ImageSet->Size();
    int numReferenceImages = m_ReferenceImageSet->Size();
    int maxNumDirections = std::max(numReferenceImages, numTestImages);

    // matrix containing the angular error between the directions
    vnl_matrix< float > diffM; diffM.set_size(maxNumDirections, maxNumDirections);

    boost::progress_display disp(outputImage->GetLargestPossibleRegion().GetSize()[0]*outputImage->GetLargestPossibleRegion().GetSize()[1]*outputImage->GetLargestPossibleRegion().GetSize()[2]);
    while( !oit.IsAtEnd() )
    {
        ++disp;
        if( mit.Get()!=1 )
        {
            ++oit;
            ++oit2;
            ++mit;
            continue;
        }
        typename OutputImageType::IndexType index = oit.GetIndex();

        float maxAngularError = 1.0;

        diffM.fill(10); // initialize with invalid error value

        // get number of valid directions (length > 0)
        int numRefDirs = 0;
        int numTestDirs = 0;
        std::vector< vnl_vector_fixed< PixelType, 3 > > testDirs;
        std::vector< vnl_vector_fixed< PixelType, 3 > > refDirs;
        for (int i=0; i<numReferenceImages; i++)
        {
            vnl_vector_fixed< PixelType, 3 > refDir = m_ReferenceImageSet->GetElement(i)->GetPixel(index).GetVnlVector();
            if (refDir.magnitude() > m_Eps )
            {
                refDir.normalize();
                refDirs.push_back(refDir);
                numRefDirs++;
            }
        }
        for (int i=0; i<numTestImages; i++)
        {
            vnl_vector_fixed< PixelType, 3 > testDir = m_ImageSet->GetElement(i)->GetPixel(index).GetVnlVector();
            if (testDir.magnitude() > m_Eps )
            {
                testDir.normalize();
                testDirs.push_back(testDir);
                numTestDirs++;
            }
        }

        if (m_IgnoreEmptyVoxels && (numRefDirs==0 || numTestDirs==0) )
        {
            ++oit;
            ++oit2;
            ++mit;
            continue;
        }

        // i: index of reference direction
        // j: index of test direction
        for (int i=0; i<maxNumDirections; i++)     // for each reference direction
        {
            bool missingDir = false;
            vnl_vector_fixed< PixelType, 3 > refDir;

            if (i<numRefDirs)  // normalize if not null
                refDir = refDirs.at(i);
            else if (m_IgnoreMissingDirections)
                continue;
            else
                missingDir = true;

            for (int j=0; j<maxNumDirections; j++)     // and each test direction
            {
                vnl_vector_fixed< PixelType, 3 > testDir;
                if (j<numTestDirs)  // normalize if not null
                    testDir = testDirs.at(j);
                else if (m_IgnoreMissingDirections || missingDir)
                    continue;
                else
                    missingDir = true;

                // found missing direction
                if (missingDir)
                {
                    diffM[i][j] = -1;
                    continue;
                }

                // calculate angle between directions
                diffM[i][j] = fabs(dot_product(refDir, testDir));

                if (diffM[i][j] < maxAngularError)
                    maxAngularError = diffM[i][j];

                if (diffM[i][j]>1.0)
                    diffM[i][j] = 1.0;
            }
        }

        float angularError = 0.0;
        float lengthError = 0.0;
        int counter = 0;
        vnl_matrix< float > diffM_copy = diffM;
        for (int k=0; k<maxNumDirections; k++)
        {
            float error = -1;
            int a,b; a=-1; b=-1;
            bool missingDir = false;

            // i: index of reference direction
            // j: index of test direction
            // find smalles error between two directions (large value -> small error)
            for (int i=0; i<maxNumDirections; i++)
                for (int j=0; j<maxNumDirections; j++)
                {
                    if (diffM[i][j]>error && diffM[i][j]<2) // found valid error entry
                    {
                        error = diffM[i][j];
                        a = i;
                        b = j;
                        missingDir = false;
                    }
                    else if (diffM[i][j]<0 && error<0)    // found missing direction
                    {
                        a = i;
                        b = j;
                        missingDir = true;
                    }
                }

            if (a<0 || b<0 || (m_IgnoreMissingDirections && missingDir))
                continue; // no more directions found

            if (a>=numRefDirs && b>=numTestDirs)
            {
                MITK_INFO << "ERROR: missing test and reference direction. should not be possible. check code.";
                continue;
            }

            // remove processed directions from error matrix
            diffM.set_row(a, 10.0);
            diffM.set_column(b, 10.0);

            if (a>=numRefDirs) // missing reference direction (find next closest)
            {
                for (int i=0; i<numRefDirs; i++)
                    if (diffM_copy[i][b]>error)
                    {
                        error = diffM_copy[i][b];
                        a = i;
                    }
            }
            else if (b>=numTestDirs) // missing test direction (find next closest)
            {
                for (int i=0; i<numTestDirs; i++)
                    if (diffM_copy[a][i]>error)
                    {
                        error = diffM_copy[a][i];
                        b = i;
                    }
            }

            float refLength = 0;
            float testLength = 1;

            if (a>=numRefDirs || b>=numTestDirs || error<0)
                error = 0;
            else
            {
                refLength = m_ReferenceImageSet->GetElement(a)->GetPixel(index).GetVnlVector().magnitude();
                testLength = m_ImageSet->GetElement(b)->GetPixel(index).GetVnlVector().magnitude();
            }

            m_LengthErrorVector.push_back( fabs(refLength-testLength) );
            m_AngularErrorVector.push_back( acos(error)*180.0/itk::Math::pi );

            m_MeanAngularError += m_AngularErrorVector.back();
            m_MeanLengthError += m_LengthErrorVector.back();

            angularError += m_AngularErrorVector.back();
            lengthError += m_LengthErrorVector.back();
            counter++;
        }

        if (counter>0)
        {
            lengthError /= counter;
            angularError /= counter;
        }
        oit2.Set(lengthError);
        oit.Set(angularError);

        ++oit;
        ++oit2;
        ++mit;
    }

    std::sort( m_AngularErrorVector.begin(), m_AngularErrorVector.end() );
    m_MeanAngularError /= m_AngularErrorVector.size();      // mean
    for (unsigned int i=0; i<m_AngularErrorVector.size(); i++)
    {
        float temp = m_AngularErrorVector.at(i) - m_MeanAngularError;
        m_VarAngularError += temp*temp;

        if ( m_AngularErrorVector.at(i)>m_MaxAngularError )
            m_MaxAngularError = m_AngularErrorVector.at(i);
        if ( m_AngularErrorVector.at(i)<m_MinAngularError )
            m_MinAngularError = m_AngularErrorVector.at(i);
    }
    if (m_AngularErrorVector.size()>1)
    {
        m_VarAngularError /= (m_AngularErrorVector.size()-1);   // variance

        // median
        if (m_AngularErrorVector.size()%2 == 0)
            m_MedianAngularError = 0.5*( m_AngularErrorVector.at( m_AngularErrorVector.size()/2 ) +  m_AngularErrorVector.at( m_AngularErrorVector.size()/2+1 ) );
        else
            m_MedianAngularError = m_AngularErrorVector.at( (m_AngularErrorVector.size()+1)/2 ) ;
    }

    std::sort( m_LengthErrorVector.begin(), m_LengthErrorVector.end() );
    m_MeanLengthError /= m_LengthErrorVector.size();      // mean
    for (unsigned int i=0; i<m_LengthErrorVector.size(); i++)
    {
        float temp = m_LengthErrorVector.at(i) - m_MeanLengthError;
        m_VarLengthError += temp*temp;

        if ( m_LengthErrorVector.at(i)>m_MaxLengthError )
            m_MaxLengthError = m_LengthErrorVector.at(i);
        if ( m_LengthErrorVector.at(i)<m_MinLengthError )
            m_MinLengthError = m_LengthErrorVector.at(i);
    }
    if (m_LengthErrorVector.size()>1)
    {
        m_VarLengthError /= (m_LengthErrorVector.size()-1);   // variance

        // median
        if (m_LengthErrorVector.size()%2 == 0)
            m_MedianLengthError = 0.5*( m_LengthErrorVector.at( m_LengthErrorVector.size()/2 ) +  m_LengthErrorVector.at( m_LengthErrorVector.size()/2+1 ) );
        else
            m_MedianLengthError = m_LengthErrorVector.at( (m_LengthErrorVector.size()+1)/2 ) ;
    }
}

}

#endif // __itkEvaluateDirectionImagesFilter_cpp
