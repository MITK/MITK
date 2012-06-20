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
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkReduceDirectionGradientsFilter.txx $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_ReduceDirectionGradientsFilter_txx_
#define _itk_ReduceDirectionGradientsFilter_txx_
#endif

#define _USE_MATH_DEFINES

#include "itkReduceDirectionGradientsFilter.h"
#include <math.h>
#include <time.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegion.h>

namespace itk
{

template <class TInputScalarType, class TOutputScalarType>
ReduceDirectionGradientsFilter<TInputScalarType, TOutputScalarType>
::ReduceDirectionGradientsFilter():
  m_Iterations(100000)
{
  this->SetNumberOfRequiredInputs( 1 );
}

template <class TInputScalarType, class TOutputScalarType>
double
ReduceDirectionGradientsFilter<TInputScalarType, TOutputScalarType>
::Costs()
{
  double costs = 0;
  int c=0;
  for (IndicesVector::iterator it = m_UsedGradientIndices.begin(); it!=m_UsedGradientIndices.end(); ++it)
  {
    for (IndicesVector::iterator it2 = m_UsedGradientIndices.begin()+c; it2!=m_UsedGradientIndices.end(); ++it2)
      if (*it!=*it2)
      {
        vnl_vector_fixed<double,3> v1 = m_OriginalGradientDirections->at(*it);
        vnl_vector_fixed<double,3> v2 = m_OriginalGradientDirections->at(*it2);
        v1.normalize(); v2.normalize();
        double angle = acos(dot_product(v1,v2));
        if (angle>0)
          costs += 1/angle;
      }
    c++;
  }

  return costs;
}

template <class TInputScalarType, class TOutputScalarType>
void
ReduceDirectionGradientsFilter<TInputScalarType, TOutputScalarType>
::GenerateData()
{
  srand(time(NULL));

  if(m_InputBValueMap.empty())
  {
   // if no InputMap is set, do for each shell the same
    m_InputBValueMap = m_OriginalBValueMap;
  }

  if(m_InputBValueMap.find(0) != m_InputBValueMap.end())
  {
    //delete b0 from input BValueMap
    m_InputBValueMap.erase(0);
  }

  BValueMap manipulatedMap = m_OriginalBValueMap;

  for(BValueMap::iterator it = m_InputBValueMap.begin(); it != m_InputBValueMap.end(); it++ ){

    // initialize index vectors
    m_UsedGradientIndices.clear();
    m_UnUsedGradientIndices.clear();

    if ( it->second.size() <= m_NumGradientDirections ){
       itkWarningMacro( << "current directions: " << it->second.size() << " wanted directions: " << m_NumGradientDirections);
       m_NumGradientDirections = it->second.size();
       continue;
    }


    int c=0;

    for (int i=0; i<it->second.size(); i++)
    {
      if (c<m_NumGradientDirections){
        m_UsedGradientIndices.push_back(it->second.at(i));
      }else{
        m_UnUsedGradientIndices.push_back(it->second.at(i));
      }
      c++;
    }

    double costs = Costs();
    MITK_INFO << "starting costs: " << costs;
    m_Iterations = m_NumGradientDirections * (it->second.size());
    for (unsigned long i=0; i<m_Iterations; i++)
    {
      int iUsed = rand() % m_UsedGradientIndices.size();
      int iUnUsed = rand() % m_UnUsedGradientIndices.size();
      int vUsed = m_UsedGradientIndices.at(iUsed);
      int vUnUsed = m_UnUsedGradientIndices.at(iUnUsed);

      m_UsedGradientIndices.at(iUsed) = vUnUsed;
      m_UnUsedGradientIndices.at(iUnUsed) = vUsed;

      double newCosts = Costs();
      if (newCosts < costs)
      {
        MITK_INFO << "costs: " << newCosts;
        costs = newCosts;
      }
      else
      {
        m_UsedGradientIndices.at(iUsed) = vUsed;
        m_UnUsedGradientIndices.at(iUnUsed) = vUnUsed;
      }
    }
    MITK_INFO << "...done";

    manipulatedMap[it->first] = m_UsedGradientIndices;

  }

  int vecLength = 0 ;
  for(BValueMap::iterator it = manipulatedMap.begin(); it != manipulatedMap.end(); it++)
  {
    vecLength += it->second.size();
  }


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

  int ind1 = -1;
  while(!newIt.IsAtEnd())
  {

    // progress
    typename OutputImageType::IndexType ind = newIt.GetIndex();
    ind1 = ind.m_Index[2];

    // init new vector with zeros
    newVec.Fill(0.0);

    // the old voxel value with duplicates
    InputPixelType oldVec = oldIt.Get();

    int index = 0;
    for(BValueMap::iterator it = manipulatedMap.begin(); it != manipulatedMap.end(); it++)
    {
      for(int j = 0; j < it->second.size(); j++)
      {
        newVec[index] = oldVec[it->second.at(j)];
        index++;
      }
    }

    newIt.Set(newVec);

    ++newIt;
    ++oldIt;
  }

  m_GradientDirections = GradientDirectionContainerType::New();
  int index = 0;
  for(BValueMap::iterator it = manipulatedMap.begin(); it != manipulatedMap.end(); it++)
  {
    for(int j = 0; j < it->second.size(); j++)
    {
      m_GradientDirections->InsertElement(index, m_OriginalGradientDirections->at(it->second.at(j)));
      index++;
    }
  }

  this->SetNumberOfRequiredOutputs (1);
  this->SetNthOutput (0, outImage);
}



} // end of namespace
