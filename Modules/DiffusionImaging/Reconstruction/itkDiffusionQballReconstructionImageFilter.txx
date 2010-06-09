/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Language:  C++
Date:      $Date: 2006-07-19 15:11:41 $
Version:   $Revision: 1.11 $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDiffusionQballReconstructionImageFilter_txx
#define __itkDiffusionQballReconstructionImageFilter_txx

#include "itkDiffusionQballReconstructionImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkArray.h"
#include "vnl/vnl_vector.h"
#include "itkPointShell.h"

namespace itk {

#define QBALL_RECON_PI       3.14159265358979323846

  template< class TReferenceImagePixelType, 
  class TGradientImagePixelType,
  class TOdfPixelType,
    int NrOdfDirections,
    int NrBasisFunctionCenters>
    DiffusionQballReconstructionImageFilter< TReferenceImagePixelType, 
    TGradientImagePixelType, TOdfPixelType, NrOdfDirections,
    NrBasisFunctionCenters>
    ::DiffusionQballReconstructionImageFilter() :
    m_GradientDirectionContainer(NULL),
    m_NumberOfGradientDirections(0),
    m_NumberOfEquatorSamplingPoints(0),
    m_NumberOfBaselineImages(1),
    m_Threshold(NumericTraits< ReferencePixelType >::NonpositiveMin()),
    m_BValue(1.0),
    m_GradientImageTypeEnumeration(Else),
    m_DirectionsDuplicated(false)
  {
    // At least 1 inputs is necessary for a vector image.
    // For images added one at a time we need at least six
    this->SetNumberOfRequiredInputs( 1 ); 
  }

  template< class TReferenceImagePixelType, 
  class TGradientImagePixelType,
  class TOdfPixelType,
    int NrOdfDirections,
    int NrBasisFunctionCenters>
    void DiffusionQballReconstructionImageFilter< TReferenceImagePixelType, 
    TGradientImagePixelType, TOdfPixelType, NrOdfDirections,
    NrBasisFunctionCenters>
    ::BeforeThreadedGenerateData()
  {
    // If we have more than 2 inputs, then each input, except the first is a 
    // gradient image. The number of gradient images must match the number of
    // gradient directions.
    const unsigned int numberOfInputs = this->GetNumberOfInputs();

    // There need to be at least 6 gradient directions to be able to compute the 
    // tensor basis
    if( m_NumberOfGradientDirections < 6 )
    {
      itkExceptionMacro( << "At least 6 gradient directions are required" );
    }

    // If there is only 1 gradient image, it must be an itk::VectorImage. Otherwise 
    // we must have a container of (numberOfInputs-1) itk::Image. Check to make sure
    if ( numberOfInputs == 1
      && m_GradientImageTypeEnumeration != GradientIsInASingleImage )
    {
      std::string gradientImageClassName(
        this->ProcessObject::GetInput(0)->GetNameOfClass());
      if ( strcmp(gradientImageClassName.c_str(),"VectorImage") != 0 )
      {
        itkExceptionMacro( << 
          "There is only one Gradient image. I expect that to be a VectorImage. "
          << "But its of type: " << gradientImageClassName );
      }
    }

    // Compute reconstruction matrix that is multiplied to the data-vector
    // each voxel in order to reconstruct the ODFs
    this->ComputeReconstructionMatrix();

    // Allocate the b-zero image
    m_BZeroImage = BZeroImageType::New();
    if( m_GradientImageTypeEnumeration == GradientIsInManyImages )
    {
      typename ReferenceImageType::Pointer img = static_cast< ReferenceImageType * >(this->ProcessObject::GetInput(0));
      m_BZeroImage->SetSpacing( img->GetSpacing() );   // Set the image spacing
      m_BZeroImage->SetOrigin( img->GetOrigin() );     // Set the image origin
      m_BZeroImage->SetDirection( img->GetDirection() );  // Set the image direction
      m_BZeroImage->SetLargestPossibleRegion( img->GetLargestPossibleRegion());
      m_BZeroImage->SetBufferedRegion( img->GetLargestPossibleRegion() );
    }
    // The gradients are specified in a single multi-component image
    else if( m_GradientImageTypeEnumeration == GradientIsInASingleImage )
    {
      typename GradientImagesType::Pointer img = static_cast< GradientImagesType * >( 
        this->ProcessObject::GetInput(0) );
      m_BZeroImage->SetSpacing( img->GetSpacing() );   // Set the image spacing
      m_BZeroImage->SetOrigin( img->GetOrigin() );     // Set the image origin
      m_BZeroImage->SetDirection( img->GetDirection() );  // Set the image direction
      m_BZeroImage->SetLargestPossibleRegion( img->GetLargestPossibleRegion());
      m_BZeroImage->SetBufferedRegion( img->GetLargestPossibleRegion() );
    }
    
    m_BZeroImage->Allocate();

  }

  template< class TReferenceImagePixelType, 
  class TGradientImagePixelType,
  class TOdfPixelType,
    int NrOdfDirections,
    int NrBasisFunctionCenters>
    typename itk::DiffusionQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType, NrOdfDirections, NrBasisFunctionCenters>::OdfPixelType 
    itk::DiffusionQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType, NrOdfDirections, NrBasisFunctionCenters>::Normalize( OdfPixelType odf, typename NumericTraits<ReferencePixelType>::AccumulateType b0 )
  {
    switch( m_NormalizationMethod )
    {
    
      // divide by sum to retreive a PDF
    case QBR_STANDARD:
      {
        odf.Normalize();
        return odf;
        break;
      }
      // ADC style
    case QBR_B_ZERO_B_VALUE:
      {
        for(int i=0; i<NrOdfDirections; i++)
        {
          odf[i] = ((TOdfPixelType)log((TOdfPixelType)b0)-odf[i])/m_BValue;
        }
        return odf;
        break;
      }
      // divide by b-zero value of voxel
    case QBR_B_ZERO:
      {
        odf *= 1.0/b0;
        return odf;
        break;
      }
      // no normalization of ODF
    case QBR_NONE:
      {
        return odf;
        break;
      }
    }

    return odf;
  }


  template< class TReferenceImagePixelType, 
  class TGradientImagePixelType,
  class TOdfPixelType,
    int NrOdfDirections,
    int NrBasisFunctionCenters >
    vnl_vector<TOdfPixelType> itk::DiffusionQballReconstructionImageFilter<TReferenceImagePixelType, TGradientImagePixelType, TOdfPixelType, NrOdfDirections, NrBasisFunctionCenters>::PreNormalize( vnl_vector<TOdfPixelType> vec )
  {
    switch( m_NormalizationMethod )
    {
      // standard: no normalization before reconstruction
    case QBR_STANDARD:
      {
        return vec;
        break;
      }
      // log of signal
    case QBR_B_ZERO_B_VALUE:
      {
        int n = vec.size();
        for(int i=0; i<n; i++)
        {
          vec[i] = log(vec[i]);
        }
        return vec;
        break;
      }
      // no normalization before reconstruction here
    case QBR_B_ZERO:
      {
        return vec;
        break;
      }
      // no normalization before reconstruction here
    case QBR_NONE:
      {
        return vec;
        break;
      }
    }

    return vec;
  }

  template< class TReferenceImagePixelType, 
  class TGradientImagePixelType,
  class TOdfPixelType,
    int NrOdfDirections,
    int NrBasisFunctionCenters>
    void DiffusionQballReconstructionImageFilter< TReferenceImagePixelType, 
    TGradientImagePixelType, TOdfPixelType, NrOdfDirections,
    NrBasisFunctionCenters>
    ::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
    int ) 
  {
    // init output and b-zero iterators
    typename OutputImageType::Pointer outputImage = 
      static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();
    ImageRegionIterator< BZeroImageType > oit2(m_BZeroImage, outputRegionForThread);
    oit2.GoToBegin();
    vnl_vector<TOdfPixelType> B(m_NumberOfGradientDirections);

    // Two cases here:
    // 1. Gradients specified in multiple images
    // 'n' iterators for each of the gradient images
    // 2. Gradients specified in a single multi-component image
    // one iterator for all gradient directions

    if( m_GradientImageTypeEnumeration == GradientIsInManyImages )
    {
      // b-zero reference image iterator
      ImageRegionConstIterator< ReferenceImageType > 
        it(static_cast< ReferenceImageType * >(this->ProcessObject::GetInput(0)), 
        outputRegionForThread);
      it.GoToBegin();

      // fill vector with gradient iterators
      typedef ImageRegionConstIterator< GradientImageType > GradientIteratorType;
      std::vector< GradientIteratorType * > gradientItContainer;
      for( unsigned int i = 1; i<= m_NumberOfGradientDirections; i++ )
      {
        // adapt index in case we have a duplicated shell
        int index = i;
        if(m_DirectionsDuplicated)
          index = i % (m_NumberOfGradientDirections/2);

        // init and pushback current input image iterator
        typename GradientImageType::Pointer gradientImagePointer = NULL;
        // dynamic_cast would be nice, static because of SGI
        gradientImagePointer = static_cast< GradientImageType * >( 
          this->ProcessObject::GetInput(index) );
        GradientIteratorType *git = new GradientIteratorType( 
          gradientImagePointer, outputRegionForThread );
        git->GoToBegin();
        gradientItContainer.push_back(git);
      }

      // Following loop does the actual reconstruction work in each voxel
      // (Tuch, Q-Ball Reconstruction [1])
      while( !it.IsAtEnd() )
      {

        // b-zero reference value
        ReferencePixelType b0 = it.Get();

        // init ODF
        OdfPixelType odf(0.0);

        // threshold on reference value to suppress noisy regions
        if( (b0 != 0) && (b0 >= m_Threshold) )
        {

          // fill array of diffusion measurements
          for( unsigned int i = 0; i< m_NumberOfGradientDirections; i++ )
          {
            GradientPixelType b = gradientItContainer[i]->Get();
            B[i] = static_cast<TOdfPixelType>(b);
            ++(*gradientItContainer[i]);
          }

          // pre-normalization according to m_NormalizationMethod
          B = PreNormalize(B);

          // actual reconstruction
          odf = ( (*m_ReconstructionMatrix) * B ).data_block();

          // post-normalization according to m_NormalizationMethod
          odf.Normalize();

        }
        else
        {
          // in case we fall below threshold, we just increment to next voxel
          for( unsigned int i = 0; i< m_NumberOfGradientDirections; i++ )
          {
            ++(*gradientItContainer[i]);  
          }
        }

        // set and increment output iterators
        oit.Set( odf );
        ++oit;
        oit2.Set( b0 );
        ++oit2;
        ++it;
      }

      // clean up
      for( unsigned int i = 0; i< gradientItContainer.size(); i++ )
      {
        delete gradientItContainer[i];
      }
    }
    // The gradients are specified in a single multi-component image
    else if( m_GradientImageTypeEnumeration == GradientIsInASingleImage )
    {

      // init input iterator
      typedef ImageRegionConstIterator< GradientImagesType > GradientIteratorType;
      typedef typename GradientImagesType::PixelType         GradientVectorType;
      typename GradientImagesType::Pointer gradientImagePointer = NULL;
      // dynamic_cast would be nice, static because of SGI
      gradientImagePointer = static_cast< GradientImagesType * >( 
        this->ProcessObject::GetInput(0) );
      GradientIteratorType git(gradientImagePointer, outputRegionForThread );
      git.GoToBegin();

      // set of indicies each for the baseline images and gradient images
      std::vector<unsigned int> baselineind; // contains baseline indicies
      std::vector<unsigned int> gradientind; // contains gradient indicies
      for(GradientDirectionContainerType::ConstIterator gdcit = this->m_GradientDirectionContainer->Begin();
        gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
      {
        if(gdcit.Value().one_norm() <= 0.0)
        {
          baselineind.push_back(gdcit.Index());
        }
        else
        {
          gradientind.push_back(gdcit.Index());
        }
      }

      // in case we have a duplicated shell, we also duplicate or indices
      if( m_DirectionsDuplicated )
      {
        int gradIndSize = gradientind.size();
        for(int i=0; i<gradIndSize; i++)
          gradientind.push_back(gradientind[i]);
      }

      // Following loop does the actual reconstruction work in each voxel
      // (Tuch, Q-Ball Reconstruction [1])
      while( !git.IsAtEnd() )
      {
        // current vector of diffusion measurements
        GradientVectorType b = git.Get();

        // average of current b-zero reference values
        typename NumericTraits<ReferencePixelType>::AccumulateType b0 = NumericTraits<ReferencePixelType>::Zero;
        for(unsigned int i = 0; i < baselineind.size(); ++i)
        {
          b0 += b[baselineind[i]];
        }
        b0 /= this->m_NumberOfBaselineImages;

        // init resulting ODF
        OdfPixelType odf(0.0);

        // threshold on reference value to suppress noisy regions
        if( (b0 != 0) && (b0 >= m_Threshold) )
        {
          for( unsigned int i = 0; i< m_NumberOfGradientDirections; i++ )
          {
            B[i] = static_cast<TOdfPixelType>(b[gradientind[i]]);
          }

          // pre-normalization according to m_NormalizationMethod
          B = PreNormalize(B);

          // actual reconstruction
          odf = ( (*m_ReconstructionMatrix) * B ).data_block();

          // post-normalization according to m_NormalizationMethod
          odf = Normalize(odf, b0);

        }

        // set and increment output iterators
        oit.Set( odf );
        ++oit;
        oit2.Set( b0 );
        ++oit2;
        ++git; // Gradient  image iterator
      }
    }

    std::cout << "One Thread finished reconstruction" << std::endl;
  }

  //void tofile(vnl_matrix<double> A, std::string fname)
  //{
  //  ofstream myfile;
  //  myfile.open (fname.c_str());
  //  myfile << "A1=[";
  //  for(int i=0; i<A.rows(); i++)
  //  {
  //    for(int j=0; j<A.columns(); j++)
  //    {
  //      myfile << A(i,j) << " ";
  //      if(j==A.columns()-1 && i!=A.rows()-1)
  //        myfile << ";" << std::endl;
  //    }
  //  }
  //  myfile << "];";
  //  myfile.close();

  //}

  template< class TReferenceImagePixelType, 
  class TGradientImagePixelType,
  class TOdfPixelType,
    int NrOdfDirections,
    int NrBasisFunctionCenters>
    void DiffusionQballReconstructionImageFilter< TReferenceImagePixelType, 
    TGradientImagePixelType, TOdfPixelType, NrOdfDirections,
    NrBasisFunctionCenters>
    ::ComputeReconstructionMatrix()
  {

    if( m_NumberOfGradientDirections < 6 )
    {
      itkExceptionMacro( << "Not enough gradient directions supplied. Need to supply at least 6" );
    }

    {
      // check for duplicate diffusion gradients
      bool warning = false;
      for(GradientDirectionContainerType::ConstIterator gdcit1 = this->m_GradientDirectionContainer->Begin();
        gdcit1 != this->m_GradientDirectionContainer->End(); ++gdcit1)
      {
        for(GradientDirectionContainerType::ConstIterator gdcit2 = this->m_GradientDirectionContainer->Begin();
          gdcit2 != this->m_GradientDirectionContainer->End(); ++gdcit2)
        {
          if(gdcit1.Value() == gdcit2.Value() && gdcit1.Index() != gdcit2.Index())
          {
            itkWarningMacro( << "Some of the Diffusion Gradients equal each other. Corresponding image data should be averaged before calling this filter." );
            warning = true;
            break;
          }
        }
        if (warning) break;
      }

      // handle acquisition schemes where only half of the spherical
      // shell is sampled by the gradient directions. In this case,
      // each gradient direction is duplicated in negative direction.
      vnl_vector<double> centerMass(3);
      centerMass.fill(0.0);
      int count = 0;
      for(GradientDirectionContainerType::ConstIterator gdcit1 = this->m_GradientDirectionContainer->Begin();
        gdcit1 != this->m_GradientDirectionContainer->End(); ++gdcit1)
      {
        if(gdcit1.Value().one_norm() > 0.0)
        {
          centerMass += gdcit1.Value();
          count ++;
        }
      }
      centerMass /= count;
      if(centerMass.two_norm() > 0.1)
      {
        m_DirectionsDuplicated = true;
        m_NumberOfGradientDirections *= 2;
      }
    }

    // set default number of equator sampling points if needed
    if(!this->m_NumberOfEquatorSamplingPoints)
      this->m_NumberOfEquatorSamplingPoints
        = (int) ceil((double)sqrt(8*QBALL_RECON_PI*this->m_NumberOfGradientDirections));

    vnl_matrix<double>* Q =
      new vnl_matrix<double>(3, m_NumberOfGradientDirections);

    {
      // Fill matrix Q with gradient directions
      int i = 0;
      for(GradientDirectionContainerType::ConstIterator gdcit = this->m_GradientDirectionContainer->Begin();
        gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
      {
        if(gdcit.Value().one_norm() > 0.0)
        {
          (*Q)(0,i) = gdcit.Value().get(0);
          (*Q)(1,i) = gdcit.Value().get(1);
          (*Q)(2,i++) = gdcit.Value().get(2);
        }
      }
      if(m_DirectionsDuplicated)
      {
        for(GradientDirectionContainerType::ConstIterator gdcit = this->m_GradientDirectionContainer->Begin();
          gdcit != this->m_GradientDirectionContainer->End(); ++gdcit)
        {
          if(gdcit.Value().one_norm() > 0.0)
          {
            (*Q)(0,i) = -gdcit.Value().get(0);
            (*Q)(1,i) = -gdcit.Value().get(1);
            (*Q)(2,i++) = -gdcit.Value().get(2);
          }
        }
      }

    }

    vnl_matrix_fixed<double, 3, NOdfDirections>* U =
      itk::PointShell<NOdfDirections, vnl_matrix_fixed<double, 3, NOdfDirections> >::DistributePointShell();

    vnl_matrix_fixed<double, 3, NBasisFunctionCenters>* V =
      itk::PointShell<NBasisFunctionCenters, vnl_matrix_fixed<double, 3, NBasisFunctionCenters> >::DistributePointShell();

    // calculate sampling points on the equator perpendicular to z-axis
    vnl_matrix<double> *C
      = new vnl_matrix<double>(3, m_NumberOfEquatorSamplingPoints);
    for(int i=0; i<m_NumberOfEquatorSamplingPoints; i++)
    {
      double theta = i * (2*QBALL_RECON_PI / m_NumberOfEquatorSamplingPoints);
      (*C)(0,i) = cos(theta);
      (*C)(1,i) = sin(theta);
      (*C)(2,i) = NumericTraits<double>::Zero;
    }

    // rotate the sampling points to each directions of interest
    vnl_matrix<double> *S
      = new vnl_matrix<double>(3,m_NumberOfEquatorSamplingPoints*NOdfDirections);

    {
      vnl_vector_fixed<double,3> z(NumericTraits<double>::Zero);
      z.put(2,NumericTraits<double>::One);
      vnl_matrix_fixed<double,3,3> eye(NumericTraits<double>::Zero);
      eye.fill_diagonal(NumericTraits<double>::One);
      for(int i=0; i<NOdfDirections; i++)
      {
        vnl_vector_fixed<double,3> ui   = (*U).get_column(i);
        vnl_matrix<double> *RC 
          = new vnl_matrix<double>(3,m_NumberOfEquatorSamplingPoints);

        if( (z(0)*ui(0)+z(1)*ui(1)+z(2)*ui(2)+1) != 0 )
        {
          vnl_matrix_fixed<double,3,3> R;
          R.set_column(0, (z+ui)*(z(0)+ui(0)));
          R.set_column(1, (z+ui)*(z(1)+ui(1)));
          R.set_column(2, (z+ui)*(z(2)+ui(2)));
          R /= (z(0)*ui(0)+z(1)*ui(1)+z(2)*ui(2)+1);
          R -= eye;
          (*RC) = R*(*C);
        }
        else
        {
          RC = C;
        }
        (*S).set_columns(i*m_NumberOfEquatorSamplingPoints, *RC);
      }
    }

    // determine interpolation kernel width first
    // use to determine diffusion measurement contribution to each of the kernels
    vnl_matrix<double> *H_plus
      = new vnl_matrix<double>(NBasisFunctionCenters,m_NumberOfGradientDirections);

    double maxSigma = QBALL_RECON_PI/6;
    double bestSigma = maxSigma;

    {
      double stepsize = 0.01;
      double start = 0.01;
      double minCondition = NumericTraits<double>::max();

      vnl_matrix<double> *H 
        = new vnl_matrix<double>(m_NumberOfGradientDirections,NBasisFunctionCenters,(double)0);

      {

        int increasing = 0;
        for( double sigma=start; sigma<maxSigma; sigma+=stepsize)
        {
          vnl_matrix<double> *tmpH
            = new vnl_matrix<double>(m_NumberOfGradientDirections,NBasisFunctionCenters);

          for(int r=0; r<m_NumberOfGradientDirections; r++)
          {
            for(int c=0; c<NBasisFunctionCenters; c++)
            {
              double qtv =  (*Q)(0,r)*(*V)(0,c)
                + (*Q)(1,r)*(*V)(1,c)
                + (*Q)(2,r)*(*V)(2,c);
              qtv = (qtv<-1.0) ? -1.0 : ( (qtv>1.0) ? 1.0 : qtv);
              double x = acos(qtv);
              (*tmpH)(r,c) = (1.0/(sigma*sqrt(2.0*QBALL_RECON_PI)))
                *exp((-x*x)/(2*sigma*sigma));
            }
          }

          vnl_svd<double> *solver;
          if(m_NumberOfGradientDirections>NBasisFunctionCenters)
          {
            solver = new vnl_svd<double>(*tmpH);
          }
          else
          {
            solver = new vnl_svd<double>(tmpH->transpose());
          }
          double condition = solver->sigma_max() / solver->sigma_min();

          std::cout << sigma << ": " << condition << std::endl;

          if( condition < minCondition )
          {
            minCondition = condition;
            bestSigma = sigma;
            H->update(*tmpH);
          }
          else
          {
            // optimum assumed to be hit after condition increased 3 times
            if (++increasing>3) break;
          }
        }
      }


      vnl_matrix_inverse<double> *pseudoInverse
        = new vnl_matrix_inverse<double>(*H);
      (*H_plus) = pseudoInverse->pinverse();

      std::cout << "choosing sigma = " << bestSigma << std::endl;

    }

    // this is the contribution of each kernel to each sampling point on the
    // equator
    vnl_matrix<double> *G
      = new vnl_matrix<double>(m_NumberOfEquatorSamplingPoints*NOdfDirections,NBasisFunctionCenters);

    {
      for(int r=0; r<m_NumberOfEquatorSamplingPoints*NOdfDirections; r++)
      {
        for(int c=0; c<NBasisFunctionCenters; c++)
        {
          double stv =  (*S)(0,r)*(*V)(0,c)
            + (*S)(1,r)*(*V)(1,c)
            + (*S)(2,r)*(*V)(2,c);
          stv = (stv<-1.0) ? -1.0 : ( (stv>1.0) ? 1.0 : stv);
          double x = acos(stv);
          (*G)(r,c) = (1.0/(bestSigma*sqrt(2.0*QBALL_RECON_PI)))
            *exp((-x*x)/(2*bestSigma*bestSigma));
        }
      }
    }

    vnl_matrix<double> *GH_plus = 
      new vnl_matrix<double>(m_NumberOfEquatorSamplingPoints*NOdfDirections,m_NumberOfGradientDirections);

    // simple matrix multiplication, manual cause of stack overflow using operator
    for (unsigned i = 0; i < m_NumberOfEquatorSamplingPoints*NOdfDirections; ++i)
      for (unsigned j = 0; j < m_NumberOfGradientDirections; ++j)
      {
        double accum = (*G)(i,0) * (*H_plus)(0,j);
        for (unsigned k = 1; k < NOdfDirections; ++k)
          accum += (*G)(i,k) * (*H_plus)(k,j);
        (*GH_plus)(i,j) = accum;
      }

      typename vnl_matrix<double>::iterator it3;
      for( it3 = (*GH_plus).begin(); it3 != (*GH_plus).end(); it3++)
      {
        if(*it3<0.0)
          *it3 = 0;
      }

      // this is an addition to the original tuch algorithm
      for(int i=0; i<NOdfDirections*m_NumberOfEquatorSamplingPoints; i++)
      {
        vnl_vector< double > r = GH_plus->get_row(i);
        r /= r.sum();
        GH_plus->set_row(i,r);
      }

      //tofile((*G),"G.m");
      //tofile((*H_plus),"H_plus.m");
      //tofile((*GH_plus),"GH_plus.m");

      m_ReconstructionMatrix
        = new vnl_matrix<TOdfPixelType>(NOdfDirections,m_NumberOfGradientDirections,0.0);
      for(int i=0; i<NOdfDirections; i++)
      {
        for(int j=0; j<m_NumberOfGradientDirections; j++)
        {
          for(int k=0; k<m_NumberOfEquatorSamplingPoints; k++)
          {
            (*m_ReconstructionMatrix)(i,j) += (TOdfPixelType)(*GH_plus)(m_NumberOfEquatorSamplingPoints*i+k,j);
          }
        }
      }

      // this is also an addition to the original tuch algorithm
      for(int i=0; i<NOdfDirections; i++)
      {
        vnl_vector< TOdfPixelType > r = m_ReconstructionMatrix->get_row(i);
        r /= r.sum();
        m_ReconstructionMatrix->set_row(i,r);
      }
      std::cout << "Reconstruction Matrix computed." << std::endl;

  }

  template< class TReferenceImagePixelType, 
  class TGradientImagePixelType,
  class TOdfPixelType,
    int NrOdfDirections,
    int NrBasisFunctionCenters>
    void DiffusionQballReconstructionImageFilter< TReferenceImagePixelType, 
    TGradientImagePixelType, TOdfPixelType, NrOdfDirections,
    NrBasisFunctionCenters>
    ::AddGradientImage( const GradientDirectionType &gradientDirection, 
    const GradientImageType *gradientImage )
  {
    // Make sure crazy users did not call both AddGradientImage and 
    // SetGradientImage
    if( m_GradientImageTypeEnumeration == GradientIsInASingleImage)
    {
      itkExceptionMacro( << "Cannot call both methods:" 
        << "AddGradientImage and SetGradientImage. Please call only one of them.");
    }

    // If the container to hold the gradient directions hasn't been allocated
    // yet, allocate it.
    if( !this->m_GradientDirectionContainer )
    {
      this->m_GradientDirectionContainer = GradientDirectionContainerType::New();
    }

    this->m_NumberOfGradientDirections = m_GradientDirectionContainer->Size();

    m_GradientDirectionContainer->InsertElement( this->m_NumberOfGradientDirections, 
      gradientDirection / gradientDirection.two_norm() );

    this->ProcessObject::SetNthInput( this->m_NumberOfGradientDirections, 
      const_cast< GradientImageType* >(gradientImage) );

    m_GradientImageTypeEnumeration = GradientIsInManyImages;
  }

  template< class TReferenceImagePixelType, 
  class TGradientImagePixelType,
  class TOdfPixelType,
    int NrOdfDirections,
    int NrBasisFunctionCenters>
    void DiffusionQballReconstructionImageFilter< TReferenceImagePixelType, 
    TGradientImagePixelType, TOdfPixelType, NrOdfDirections,
    NrBasisFunctionCenters>
    ::SetGradientImage( GradientDirectionContainerType *gradientDirection, 
    const GradientImagesType *gradientImage )
  {
    // Make sure crazy users did not call both AddGradientImage and 
    // SetGradientImage
    if( m_GradientImageTypeEnumeration == GradientIsInManyImages )
    {
      itkExceptionMacro( << "Cannot call both methods:" 
        << "AddGradientImage and SetGradientImage. Please call only one of them.");
    }

    this->m_GradientDirectionContainer = gradientDirection;

    unsigned int numImages = gradientDirection->Size();
    this->m_NumberOfBaselineImages = 0;
    for(GradientDirectionContainerType::Iterator it = this->m_GradientDirectionContainer->Begin();
      it != this->m_GradientDirectionContainer->End(); it++)
    {
      if(it.Value().one_norm() <= 0.0)
      {
        this->m_NumberOfBaselineImages++;
      }
      else // Normalize non-zero gradient directions
      {
        it.Value() = it.Value() / it.Value().two_norm();
      }
    }

    this->m_NumberOfGradientDirections = numImages - this->m_NumberOfBaselineImages;

    // ensure that the gradient image we received has as many components as 
    // the number of gradient directions
    if( gradientImage->GetVectorLength() != this->m_NumberOfBaselineImages + m_NumberOfGradientDirections )
    {
      itkExceptionMacro( << m_NumberOfGradientDirections << " gradients + " << this->m_NumberOfBaselineImages
        << "baselines = " << m_NumberOfGradientDirections + this->m_NumberOfBaselineImages
        << " directions specified but image has " << gradientImage->GetVectorLength()
        << " components.");
    }

    this->ProcessObject::SetNthInput( 0, 
      const_cast< GradientImagesType* >(gradientImage) );
    m_GradientImageTypeEnumeration = GradientIsInASingleImage;
  }


  template< class TReferenceImagePixelType, 
  class TGradientImagePixelType,
  class TOdfPixelType,
    int NrOdfDirections,
    int NrBasisFunctionCenters>
    void DiffusionQballReconstructionImageFilter< TReferenceImagePixelType, 
    TGradientImagePixelType, TOdfPixelType, NrOdfDirections,
    NrBasisFunctionCenters>
    ::PrintSelf(std::ostream& os, Indent indent) const
  {
    Superclass::PrintSelf(os,indent);

    os << indent << "OdfReconstructionMatrix: " << m_ReconstructionMatrix << std::endl;
    if ( m_GradientDirectionContainer )
    {
      os << indent << "GradientDirectionContainer: "
        << m_GradientDirectionContainer << std::endl;
    }
    else
    {
      os << indent << 
        "GradientDirectionContainer: (Gradient directions not set)" << std::endl;
    }
    os << indent << "NumberOfGradientDirections: " << 
      m_NumberOfGradientDirections << std::endl;
    os << indent << "NumberOfBaselineImages: " << 
      m_NumberOfBaselineImages << std::endl;
    os << indent << "Threshold for reference B0 image: " << m_Threshold << std::endl;
    os << indent << "BValue: " << m_BValue << std::endl;
    if ( this->m_GradientImageTypeEnumeration == GradientIsInManyImages )
    {
      os << indent << "Gradient images haven been supplied " << std::endl;
    }
    else if ( this->m_GradientImageTypeEnumeration == GradientIsInManyImages )
    {
      os << indent << "A multicomponent gradient image has been supplied" << std::endl;
    }
  }

}

#endif // __itkDiffusionQballReconstructionImageFilter_txx
