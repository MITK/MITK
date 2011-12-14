
#ifndef __itkDiffusionQballPrepareVisualizationImageFilter_cpp
#define __itkDiffusionQballPrepareVisualizationImageFilter_cpp

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkDiffusionQballPrepareVisualizationImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkArray.h"
#include "vnl/vnl_vector.h"
#include "itkOrientationDistributionFunction.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

  //#define QBALL_RECON_PI       M_PI

  template< class TOdfPixelType, int NrOdfDirections>
    DiffusionQballPrepareVisualizationImageFilter< TOdfPixelType, NrOdfDirections>
    ::DiffusionQballPrepareVisualizationImageFilter() :
  m_Threshold(0),
    m_ScaleByGfaType(GfaFilterType::GFA_STANDARD),
    m_DoScaleGfa(false),
    m_GfaParam1(2),
    m_GfaParam2(1)
  {
    // At least 1 inputs is necessary for a vector image.
    // For images added one at a time we need at least six
    this->SetNumberOfRequiredInputs( 1 );
  }

  template< class TOdfPixelType, 
    int NrOdfDirections>
    void DiffusionQballPrepareVisualizationImageFilter< TOdfPixelType, 
    NrOdfDirections>
    ::BeforeThreadedGenerateData()
  {
    if( m_NormalizationMethod == PV_GLOBAL_MAX )
    {
      typename InputImageType::Pointer inputImagePointer = NULL;
      inputImagePointer = static_cast< InputImageType * >( 
        this->ProcessObject::GetInput(0) );

      typename GfaFilterType::Pointer filter = GfaFilterType::New();
      filter->SetInput(inputImagePointer);
      filter->SetNumberOfThreads(4);
      filter->SetComputationMethod(GfaFilterType::GFA_MAX_ODF_VALUE);
      filter->Update();

      typedef typename itk::MinimumMaximumImageCalculator< typename GfaFilterType::OutputImageType > 
        MaxFilterType;
      typename MaxFilterType::Pointer maxFilter = MaxFilterType::New();
      maxFilter->SetImage(filter->GetOutput());
      maxFilter->ComputeMaximum();

      m_GlobalInputMaximum = maxFilter->GetMaximum();
    }

    //if(m_DoScaleGfa)
    {
      typename InputImageType::Pointer inputImagePointer = NULL;
      inputImagePointer = static_cast< InputImageType * >( 
        this->ProcessObject::GetInput(0) );

      typename GfaFilterType::Pointer filter = GfaFilterType::New();
      filter->SetInput(inputImagePointer);
      filter->SetNumberOfThreads(4);
      filter->SetComputationMethod(m_ScaleByGfaType);
      filter->SetParam1(m_GfaParam1);
      filter->SetParam2(m_GfaParam2);
      filter->Update();
      m_GfaImage = filter->GetOutput();
    }
  }

  template< class TOdfPixelType, 
    int NrOdfDirections>
    void DiffusionQballPrepareVisualizationImageFilter< TOdfPixelType, 
    NrOdfDirections>
    ::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
    int ) 
  {
    typename OutputImageType::Pointer outputImage = 
      static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    typedef itk::OrientationDistributionFunction<TOdfPixelType,NrOdfDirections> OdfType;
    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typedef typename InputImageType::PixelType         OdfVectorType;
    typename InputImageType::Pointer inputImagePointer = NULL;
    inputImagePointer = static_cast< InputImageType * >( 
      this->ProcessObject::GetInput(0) );
    InputIteratorType git(inputImagePointer, outputRegionForThread );
    git.GoToBegin();

    typedef ImageRegionConstIterator< GfaImageType > GfaIteratorType;
    GfaIteratorType gfaIt(m_GfaImage, outputRegionForThread);
    
    while( !git.IsAtEnd() )
    {
      OdfVectorType b = git.Get();
      OdfType odf = b.GetDataPointer();
 
      switch( m_NormalizationMethod )
      {
      case PV_NONE:
        {
          break;
        }
      case PV_MAX:
        {
          odf = odf.MaxNormalize();
          break;
        }
      case PV_MIN_MAX:
        {
          odf = odf.MinMaxNormalize();
          break;
        }
      case PV_GLOBAL_MAX:
        {
          odf *= 1.0/m_GlobalInputMaximum;
          break;
        }
      case PV_MIN_MAX_INVERT:
        {
          odf = odf.MinMaxNormalize();
          for(int i=0; i<NrOdfDirections; i++)
          {
            odf[i] = 1.0 - odf[i];
          }
          break;
        }
      }
        
      if(m_DoScaleGfa)
      {
        odf *= gfaIt.Get();
        ++gfaIt;
      }

      odf *= 0.5;
      oit.Set( odf.GetDataPointer() );
      ++oit;
      ++git; // Gradient  image iterator
    }

    std::cout << "One Thread finished extraction" << std::endl;
  }

  template< class TOdfPixelType, 
    int NrOdfDirections>
    void DiffusionQballPrepareVisualizationImageFilter< TOdfPixelType, 
    NrOdfDirections>
    ::PrintSelf(std::ostream& os, Indent indent) const
  {
    Superclass::PrintSelf(os,indent);
    os << indent << "m_Threshold: " << 
      m_Threshold << std::endl;
  }

}

#endif // __itkDiffusionQballPrepareVisualizationImageFilter_cpp
