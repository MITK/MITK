
#ifndef __itkDiffusionQballGeneralizedFaImageFilter_txx
#define __itkDiffusionQballGeneralizedFaImageFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkArray.h"
#include "vnl/vnl_vector.h"
#include "itkOrientationDistributionFunction.h"

namespace itk {

  //#define QBALL_RECON_PI       3.14159265358979323846

  template< class TOdfPixelType, 
  class TGfaPixelType,
    int NrOdfDirections>
    DiffusionQballGeneralizedFaImageFilter< TOdfPixelType, 
    TGfaPixelType, NrOdfDirections>
    ::DiffusionQballGeneralizedFaImageFilter() :
        m_ComputationMethod(GFA_STANDARD)
  {
    // At least 1 inputs is necessary for a vector image.
    // For images added one at a time we need at least six
    this->SetNumberOfRequiredInputs( 1 );
  }

  template< class TOdfPixelType, 
  class TGfaPixelType,
    int NrOdfDirections>
    void DiffusionQballGeneralizedFaImageFilter< TOdfPixelType, 
    TGfaPixelType, NrOdfDirections>
    ::BeforeThreadedGenerateData()
  {
  }

  template< class TOdfPixelType, 
  class TGfaPixelType,
    int NrOdfDirections>
    void DiffusionQballGeneralizedFaImageFilter< TOdfPixelType, 
    TGfaPixelType, NrOdfDirections>
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
    while( !git.IsAtEnd() )
    {
      OdfVectorType b = git.Get();
      TGfaPixelType outval = -1;

      switch( m_ComputationMethod )
      {
      case GFA_STANDARD:
        {
          OdfType odf = b.GetDataPointer();
          outval = odf.GetGeneralizedFractionalAnisotropy();
          break;
        }
      case GFA_QUANTILES_HIGH_LOW:
        {
          vnl_vector_fixed<TOdfPixelType,NrOdfDirections> sorted;
          for(int i=0; i<NrOdfDirections; i++)
          {
            sorted[i] = b[i];
          }
          std::sort( sorted.begin(), sorted.end() );
          double q60 = sorted[floor(0.6*(double)NrOdfDirections+0.5)];
          double q95 = sorted[floor(0.95*(double)NrOdfDirections+0.5)];
          outval = q95/q60 - 1.0;
          break;
        }
      case GFA_QUANTILE_HIGH:
        {
          vnl_vector_fixed<TOdfPixelType,NrOdfDirections> sorted;
          for(int i=0; i<NrOdfDirections; i++)
          {
            sorted[i] = b[i];
          }
          std::sort( sorted.begin(), sorted.end() );
          //double q40 = sorted[floor(0.4*(double)NrOdfDirections+0.5)];
          double q95 = sorted[floor(0.95*(double)NrOdfDirections+0.5)];
          outval = q95;
          break;
        }
      case GFA_MAX_ODF_VALUE:
        {
          outval = b.GetVnlVector().max_value();
          break;
        }
      case GFA_DECONVOLUTION_COEFFS:
        {
          break;
        }
      case GFA_MIN_MAX_NORMALIZED_STANDARD:
        {
          OdfType odf = b.GetDataPointer();
          odf = odf.MinMaxNormalize();
          outval = odf.GetGeneralizedFractionalAnisotropy();
          break;
        }  
      case GFA_NORMALIZED_ENTROPY:
        {
          OdfType odf = b.GetDataPointer();
          outval = odf.GetNormalizedEntropy();
          break;
        }  
      case GFA_NEMATIC_ORDER_PARAMETER:
        {
          OdfType odf = b.GetDataPointer();
          outval = odf.GetNematicOrderParameter();
          break;
        }  
      case GFA_QUANTILE_LOW:
        {
          vnl_vector_fixed<TOdfPixelType,NrOdfDirections> sorted;
          for(int i=0; i<NrOdfDirections; i++)
          {
            sorted[i] = b[i];
          }
          std::sort( sorted.begin(), sorted.end() );
          //double q40 = sorted[floor(0.4*(double)NrOdfDirections+0.5)];
          double q05 = sorted[floor(0.05*(double)NrOdfDirections+0.5)];
          outval = q05;
          break;
        }
      case GFA_MIN_ODF_VALUE:
        {
          outval = b.GetVnlVector().min_value();
          break;
        }
      case GFA_QUANTILES_LOW_HIGH:
        {
          vnl_vector_fixed<TOdfPixelType,NrOdfDirections> sorted;
          for(int i=0; i<NrOdfDirections; i++)
          {
            sorted[i] = b[i];
          }
          std::sort( sorted.begin(), sorted.end() );
          double q05 = sorted[floor(0.05*(double)NrOdfDirections+0.5)];
          double q40 = sorted[floor(0.4*(double)NrOdfDirections+0.5)];
          outval = q40/q05 - 1.0;
          break;
        }
      case GFA_STD_BY_MAX:
        {
          OdfType odf = b.GetDataPointer();
          outval = odf.GetStdDevByMaxValue();
          break;
        } 
      case GFA_PRINCIPLE_CURVATURE:
        {
          OdfType odf = b.GetDataPointer();
          outval = odf.GetPrincipleCurvature(m_Param1, m_Param2, 0);
          break;
        } 
      case GFA_GENERALIZED_GFA:
        {
          OdfType odf = b.GetDataPointer();
          outval = odf.GetGeneralizedGFA(m_Param1, m_Param2);
          break;
        } 
      }
      
      oit.Set( outval );
      
      ++oit;
      ++git; // Gradient  image iterator
    }

    std::cout << "One Thread finished calculation" << std::endl;
  }

  template< class TOdfPixelType, 
  class TGfaPixelType,
    int NrOdfDirections>
    void DiffusionQballGeneralizedFaImageFilter< TOdfPixelType, 
    TGfaPixelType, NrOdfDirections>
    ::PrintSelf(std::ostream& os, Indent indent) const
  {
    Superclass::PrintSelf(os,indent);
  }

}

#endif // __itkDiffusionQballGeneralizedFaImageFilter_txx
