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
    for (std::vector<int>::iterator it = m_UsedGradientIndices.begin(); it!=m_UsedGradientIndices.end(); ++it)
    {
      for (std::vector<int>::iterator it2 = m_UsedGradientIndices.begin()+c; it2!=m_UsedGradientIndices.end(); ++it2)
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

    // initialize index vectors
    m_UsedGradientIndices.clear();
    m_UnUsedGradientIndices.clear();

    if ( m_OriginalGradientDirections->Size()<= m_NumGradientDirections )
      m_NumGradientDirections = m_OriginalGradientDirections->Size();

    int c=0;
    int numB0 = 0;
    for (int i=0; i<m_OriginalGradientDirections->Size(); i++)
    {
      vnl_vector_fixed< double, 3 > v = m_OriginalGradientDirections->at(i);
      v.normalize();
      if (fabs(v[0])>0.001 || fabs(v[1])>0.001 || fabs(v[2])>0.001)
      {
        if (c<m_NumGradientDirections)
          m_UsedGradientIndices.push_back(i);
        else
          m_UnUsedGradientIndices.push_back(i);
        c++;
      }
      else
        m_BaselineImageIndices.push_back(i);
    }
    numB0 = m_BaselineImageIndices.size();

    double costs = Costs();
    MITK_INFO << "starting costs: " << costs;
    m_Iterations = m_NumGradientDirections*m_OriginalGradientDirections->Size();
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

    // initialize output image
    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( this->GetInput()->GetSpacing() );   // Set the image spacing
    outImage->SetOrigin( this->GetInput()->GetOrigin() );     // Set the image origin
    outImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction
    outImage->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion());
    outImage->SetBufferedRegion( this->GetInput()->GetLargestPossibleRegion() );
    outImage->SetRequestedRegion( this->GetInput()->GetLargestPossibleRegion() );
    outImage->SetVectorLength(m_NumGradientDirections+numB0);
    outImage->Allocate();

    itk::ImageRegionIterator< OutputImageType > newIt(outImage, outImage->GetLargestPossibleRegion());
    newIt.GoToBegin();

    typename InputImageType::Pointer inImage = const_cast<InputImageType*>(this->GetInput(0));
    itk::ImageRegionIterator< InputImageType > oldIt(inImage, inImage->GetLargestPossibleRegion());
    oldIt.GoToBegin();

    // initial new value of voxel
    OutputPixelType newVec;
    newVec.SetSize(m_NumGradientDirections+numB0);
    newVec.AllocateElements(m_NumGradientDirections+numB0);

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

      for(int i=0; i<numB0; i++)
        newVec[i] = oldVec[m_BaselineImageIndices.at(i)];

      for(int i=0; i<m_NumGradientDirections; i++)
        newVec[i+numB0] = oldVec[m_UsedGradientIndices.at(i)];

      newIt.Set(newVec);

      ++newIt;
      ++oldIt;
    }

    m_GradientDirections = GradientDirectionContainerType::New();
    for(int i=0; i<numB0; i++)
      m_GradientDirections->InsertElement(i, m_OriginalGradientDirections->at(m_BaselineImageIndices.at(i)));

    for(int i=0; i<m_NumGradientDirections; i++)
      m_GradientDirections->InsertElement(i+numB0, m_OriginalGradientDirections->at(m_UsedGradientIndices.at(i)));

    this->SetNumberOfRequiredOutputs (1);
    this->SetNthOutput (0, outImage);
  }

  template <class TInputScalarType, class TOutputScalarType>
  std::vector< int >
    ReduceDirectionGradientsFilter<TInputScalarType, TOutputScalarType>
  ::GetUsedGradientIndices()
  {
    return m_UsedGradientIndices;
  }
} // end of namespace
