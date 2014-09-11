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
/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkElectrostaticRepulsionDiffusionGradientReductionFilter.txx $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_ElectrostaticRepulsionDiffusionGradientReductionFilter_txx_
#define _itk_ElectrostaticRepulsionDiffusionGradientReductionFilter_txx_
#endif

#define _USE_MATH_DEFINES

#include "itkElectrostaticRepulsionDiffusionGradientReductionFilter.h"
#include <math.h>
#include <time.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegion.h>

namespace itk
{

template <class TInputScalarType, class TOutputScalarType>
ElectrostaticRepulsionDiffusionGradientReductionFilter<TInputScalarType, TOutputScalarType>
::ElectrostaticRepulsionDiffusionGradientReductionFilter()
{
    this->SetNumberOfRequiredInputs( 1 );
}

template <class TInputScalarType, class TOutputScalarType>
double
ElectrostaticRepulsionDiffusionGradientReductionFilter<TInputScalarType, TOutputScalarType>
::Costs()
{
    double costs = 2*M_PI;
    for (IndicesVector::iterator it = m_UsedGradientIndices.begin(); it!=m_UsedGradientIndices.end(); ++it)
    {
        for (IndicesVector::iterator it2 = m_UsedGradientIndices.begin(); it2!=m_UsedGradientIndices.end(); ++it2)
            if (it != it2)
            {
                vnl_vector_fixed<double,3> v1 = m_OriginalGradientDirections->at(*it);
                vnl_vector_fixed<double,3> v2 = m_OriginalGradientDirections->at(*it2);

                v1.normalize(); v2.normalize();
                double temp = dot_product(v1,v2);
                if (temp>1)
                    temp = 1;
                else if (temp<-1)
                    temp = -1;

                double angle = acos(temp);

                if (angle<costs)
                    costs = angle;

                temp = dot_product(-v1,v2);
                if (temp>1)
                    temp = 1;
                else if (temp<-1)
                    temp = -1;

                angle = acos(temp);
                if (angle<costs)
                    costs = angle;
            }
    }

    return costs;
}

template <class TInputScalarType, class TOutputScalarType>
void
ElectrostaticRepulsionDiffusionGradientReductionFilter<TInputScalarType, TOutputScalarType>
::GenerateData()
{
    unsigned int randSeed = time(NULL);

    if(m_InputBValueMap.empty() || m_NumGradientDirections.size()!=m_InputBValueMap.size())
        return;

    BValueMap manipulatedMap = m_InputBValueMap;

    int shellCounter = 0;
    for(BValueMap::iterator it = m_InputBValueMap.begin(); it != m_InputBValueMap.end(); it++ )
    {
        srand(randSeed);

        // initialize index vectors
        m_UsedGradientIndices.clear();
        m_UnusedGradientIndices.clear();

        if ( it->second.size() <= m_NumGradientDirections[shellCounter] )
        {
            itkWarningMacro( << "current directions: " << it->second.size() << " wanted directions: " << m_NumGradientDirections[shellCounter]);
            m_NumGradientDirections[shellCounter] = it->second.size();
            shellCounter++;
            continue;
        }
        MITK_INFO << "Shell number: " << shellCounter;

        unsigned int c=0;

        for (unsigned int i=0; i<it->second.size(); i++)
        {
            if (c<m_NumGradientDirections[shellCounter])
                m_UsedGradientIndices.push_back(it->second.at(i));
            else
                m_UnusedGradientIndices.push_back(it->second.at(i));
            c++;
        }

        double minAngle = Costs();
        double newMinAngle = 0;

        MITK_INFO << "minimum angle: " << 180*minAngle/M_PI;
        int stagnationCount = 0;
        int rejectionCount = 0;
        int maxRejections = m_NumGradientDirections[shellCounter] * 1000;
        if (maxRejections<10000)
            maxRejections = 10000;
        int iUsed = 0;
        if (m_UsedGradientIndices.size()>0)
            while ( stagnationCount<1000 && rejectionCount<maxRejections )
            {
                // make proposal for new gradient configuration by randomly removing one of the currently used directions and instead adding one of the unused directions
                //int iUsed = rand() % m_UsedGradientIndices.size();
                int iUnUsed = rand() % m_UnusedGradientIndices.size();
                int vUsed = m_UsedGradientIndices.at(iUsed);
                int vUnUsed = m_UnusedGradientIndices.at(iUnUsed);
                m_UsedGradientIndices.at(iUsed) = vUnUsed;
                m_UnusedGradientIndices.at(iUnUsed) = vUsed;

                newMinAngle = Costs();          // calculate costs of proposed configuration
                if (newMinAngle > minAngle)     // accept or reject proposal
                {
                    MITK_INFO << "minimum angle: " << 180*newMinAngle/M_PI;

                    if ( (newMinAngle-minAngle)<0.01 )
                        stagnationCount++;
                    else
                        stagnationCount = 0;

                    minAngle = newMinAngle;
                    rejectionCount = 0;
                }
                else
                {
                    rejectionCount++;
                    m_UsedGradientIndices.at(iUsed) = vUsed;
                    m_UnusedGradientIndices.at(iUnUsed) = vUnUsed;
                }
                iUsed++;
                iUsed = iUsed % m_UsedGradientIndices.size();
            }
        manipulatedMap[it->first] = m_UsedGradientIndices;
        shellCounter++;
    }

    int vecLength = 0 ;
    for(BValueMap::iterator it = manipulatedMap.begin(); it != manipulatedMap.end(); it++)
        vecLength += it->second.size();

    // initialize output image
    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( this->GetInput()->GetSpacing() );   // Set the image spacing
    outImage->SetOrigin( this->GetInput()->GetOrigin() );     // Set the image origin
    outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction
    outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
    outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
    outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
    outImage->SetVectorLength( vecLength ); // Set the vector length
    outImage->Allocate();

    itk::ImageRegionIterator< OutputImageType > newIt(outImage, outImage->GetLargestPossibleRegion());
    newIt.GoToBegin();

    typename InputImageType::Pointer inImage = const_cast<InputImageType*>(this->GetInput(0));
    itk::ImageRegionIterator< InputImageType > oldIt(inImage, inImage->GetLargestPossibleRegion());
    oldIt.GoToBegin();

    // initial new value of voxel
    OutputPixelType newVec;
    newVec.SetSize( vecLength );
    newVec.AllocateElements( vecLength );

    // generate new pixel values
    while(!newIt.IsAtEnd())
    {
        // init new vector with zeros
        newVec.Fill(0.0);
        InputPixelType oldVec = oldIt.Get();

        int index = 0;
        for(BValueMap::iterator it=manipulatedMap.begin(); it!=manipulatedMap.end(); it++)
            for(unsigned int j=0; j<it->second.size(); j++)
            {
                newVec[index] = oldVec[it->second.at(j)];
                index++;
            }
        newIt.Set(newVec);

        ++newIt;
        ++oldIt;
    }

    // set new gradient directions
    m_GradientDirections = GradientDirectionContainerType::New();
    int index = 0;
    for(BValueMap::iterator it = manipulatedMap.begin(); it != manipulatedMap.end(); it++)
        for(unsigned int j = 0; j < it->second.size(); j++)
        {
            m_GradientDirections->InsertElement(index, m_OriginalGradientDirections->at(it->second.at(j)));
            index++;
        }

    this->SetNumberOfRequiredOutputs (1);
    this->SetNthOutput (0, outImage);
    MITK_INFO << "...done";
}



} // end of namespace
