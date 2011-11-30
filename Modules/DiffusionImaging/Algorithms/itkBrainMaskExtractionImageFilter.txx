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
#ifndef __itkBrainMaskExtractionImageFilter_txx
#define __itkBrainMaskExtractionImageFilter_txx

#include "itkBrainMaskExtractionImageFilter.h"

#include <itkBinaryThresholdImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkVotingBinaryHoleFillingImageFilter.h>
#include <itkVotingBinaryIterativeHoleFillingImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkAndImageFilter.h>
#include <itkRecursiveGaussianImageFilter.h>
#include <itkMaskImageFilter.h>

namespace itk {

  template< class TOutputImagePixelType >
    BrainMaskExtractionImageFilter< TOutputImagePixelType >
    ::BrainMaskExtractionImageFilter()
  {
    // At least 1 inputs is necessary for a vector image.
    // For images added one at a time we need at least six
    this->SetNumberOfRequiredInputs( 1 );
  }

  template< class TOutputImagePixelType >
    void BrainMaskExtractionImageFilter< TOutputImagePixelType >
    ::GenerateData()
  {

    // gaussian filter
    typename itk::RecursiveGaussianImageFilter<InputImageType,InputImageType>::Pointer gaussian
      = itk::RecursiveGaussianImageFilter<InputImageType,InputImageType>::New();
    gaussian->SetInput( this->GetInput(0) );
    gaussian->SetSigma( 1.0 );

    try
    {
      gaussian->Update();
    }
    catch( itk::ExceptionObject &e)
    {
      std::cerr << e;
      return;
    }

    // threshold the image
    typename itk::BinaryThresholdImageFilter<InputImageType,OutputImageType>::Pointer threshold =
      itk::BinaryThresholdImageFilter<InputImageType,OutputImageType>::New();

    threshold->SetInput( gaussian->GetOutput() );

    int seuil = static_cast<int>( ComputeHistogram( gaussian->GetOutput() ) );
    threshold->SetLowerThreshold( seuil );

    std::cout << "Thresholding..." << std::flush;
    try
    {
      threshold->Update();
    }
    catch( itk::ExceptionObject &e)
    {
      std::cerr << e;
      return;
    }
    std::cout << "Done." << std::endl;

#ifdef DEBUG_ME
    {
      WriterType::Pointer writer = WriterType::New();
      writer->SetInput( threshold->GetOutput() );
      writer->SetFileName( "AfterThreshold.hdr" );
      writer->Update();
    }
#endif

    // erode to remove background noise
    typedef itk::BinaryBallStructuringElement<int, 3> StructuralElementType;
    StructuralElementType ball;

    typename itk::BinaryErodeImageFilter<OutputImageType,OutputImageType,StructuralElementType>::Pointer erode =
      itk::BinaryErodeImageFilter<OutputImageType,OutputImageType,StructuralElementType>::New();

    ball.SetRadius( 3 );

    erode->SetInput( threshold->GetOutput() );
    erode->SetKernel( ball );

    std::cout << "Eroding..." << std::flush;
    try
    {
      erode->Update();
    }
    catch( itk::ExceptionObject &e)
    {
      std::cerr << e;
      return;
    }
    std::cout << "Done." << std::endl;

#ifdef DEBUG_ME
    {
      WriterType::Pointer writer = WriterType::New();
      writer->SetInput( erode->GetOutput() );
      writer->SetFileName( "AfterErode.hdr" );
      writer->Update();
    }
#endif


    typedef BinaryCrossStructuringElement<int, 3> CrossType;

    typedef BinaryDilateImageFilter<OutputImageType,OutputImageType,CrossType> DilateFilterType;
    typedef AndImageFilter<OutputImageType,OutputImageType,OutputImageType> AndFilterType;

    typename OutputImageType::Pointer M0 = threshold->GetOutput();
    typename OutputImageType::Pointer Mn = OutputImageType::New();
    Mn->SetRegions( M0->GetLargestPossibleRegion() );
    Mn->SetSpacing( M0->GetSpacing() );
    Mn->SetOrigin( M0->GetOrigin() );
    Mn->SetDirection( M0->GetDirection() );
    Mn->Allocate();

    typename OutputImageType::Pointer Mnplus1 = erode->GetOutput();


    CrossType cross;
    cross.SetRadius( 1 );
    //unsigned long rad[3]={3,3,3};

    //ball2.SetRadius( rad );

    std::cout << "Conditional reconstruction..." << std::flush;
    int iter = 0;
    do
    {
      std::cout << "Iteration: " << iter++ << std::endl;
      CopyImage( Mn, Mnplus1);

      typename DilateFilterType::Pointer dilater = DilateFilterType::New();
      dilater->SetInput( Mn );
      dilater->SetKernel( cross );

      try
      {
        dilater->Update();
      }
      catch( itk::ExceptionObject &e)
      {
        std::cerr << e;
        return;
      }

      typename AndFilterType::Pointer andfilter = AndFilterType::New();
      andfilter->SetInput(0, M0);
      andfilter->SetInput(1, dilater->GetOutput() );

      try
      {
        andfilter->Update();
      }
      catch( itk::ExceptionObject &e)
      {
        std::cerr << e;
        return;
      }

      Mnplus1 = andfilter->GetOutput();

      /*
      #ifdef DEBUG_ME
      {
      WriterType::Pointer writer = WriterType::New();
      writer->SetInput( andfilter->GetOutput() );
      char filename[512];
      sprintf( filename, "CondReconstruction_iter_%d.hdr", iter);
      writer->SetFileName( filename );
      writer->Update();
      }
      #endif*/

    } while( !CompareImages( Mn, Mnplus1) );

    std::cout << "Done." << std::endl;

#ifdef DEBUG_ME
    {
      WriterType::Pointer writer = WriterType::New();
      writer->SetInput( Mn );
      writer->SetFileName( "AfterCondReconstruction.hdr" );
      writer->Update();
    }
#endif
    // now fill the holes

    typename itk::VotingBinaryIterativeHoleFillingImageFilter< OutputImageType >::Pointer filler =
      itk::VotingBinaryIterativeHoleFillingImageFilter< OutputImageType >::New();
    filler->SetInput( Mn );
    filler->SetMaximumNumberOfIterations (1000);

    std::cout << "Filling the holes..." << std::flush;
    try
    {
      filler->Update();
    }
    catch( itk::ExceptionObject &e)
    {
      std::cerr << e;
      return;
    }
    std::cout << "Done." << std::endl;

    typename OutputImageType::Pointer outputImage =
      static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    outputImage->SetSpacing( filler->GetOutput()->GetSpacing() );   // Set the image spacing
    outputImage->SetOrigin( filler->GetOutput()->GetOrigin() );     // Set the image origin
    outputImage->SetLargestPossibleRegion( filler->GetOutput()->GetLargestPossibleRegion());
    outputImage->SetBufferedRegion( filler->GetOutput()->GetLargestPossibleRegion() );
    outputImage->SetDirection( filler->GetOutput()->GetDirection() );
    outputImage->Allocate();

    itk::ImageRegionIterator<OutputImageType> itIn( filler->GetOutput(), filler->GetOutput()->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<OutputImageType> itOut( outputImage, outputImage->GetLargestPossibleRegion() );

    while( !itIn.IsAtEnd() )
    {
        itOut.Set(itIn.Get());
        ++itIn;
        ++itOut;
    }

  }


  template< class TOutputImagePixelType >
    void BrainMaskExtractionImageFilter< TOutputImagePixelType >
    ::CopyImage( typename OutputImageType::Pointer target, typename OutputImageType::Pointer source)
  {
    itk::ImageRegionConstIterator<OutputImageType> itIn( source, source->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<OutputImageType> itOut( target, target->GetLargestPossibleRegion() );

    while( !itOut.IsAtEnd() )
    {
      itOut.Set( itIn.Get() );
      ++itIn;
      ++itOut;
    }
  }


  template< class TOutputImagePixelType >
    bool BrainMaskExtractionImageFilter< TOutputImagePixelType >
    ::CompareImages( typename OutputImageType::Pointer im1, typename OutputImageType::Pointer im2)
  {
    itk::ImageRegionConstIterator<OutputImageType> itIn( im1, im1->GetLargestPossibleRegion() );
    itk::ImageRegionConstIterator<OutputImageType> itOut( im2, im2->GetLargestPossibleRegion() );

    while( !itOut.IsAtEnd() )
    {
      if( itOut.Value() != itIn.Value() )
      {
        return false;
      }
      ++itOut;
      ++itIn;
    }

    return true;
  }


  template< class TOutputImagePixelType >
    int BrainMaskExtractionImageFilter< TOutputImagePixelType >
    ::ComputeHistogram( typename InputImageType::Pointer image)
  {
    // IMPORTANT: IMAGE MUST BE UNSIGNED SHORT
    int N=65535;
    int* histogram = new int[N];
    for( int i=0; i<N; i++)
    {
      histogram[i] = 0;
    }

    itk::ImageRegionConstIterator<InputImageType> itIn( image, image->GetLargestPossibleRegion() );

    long totVoxels = 0;

    int max = -1;
    int min = 9999999;
    while( !itIn.IsAtEnd() )
    {
      histogram[ (int)(itIn.Value()) ]++;

      if( itIn.Value()>max )
      {
        max = itIn.Value();
      }

      if( itIn.Value()<min )
      {
        min = itIn.Value();
      }
      ++itIn;
      ++totVoxels;
    }


    //int EPS = 1;
    int seuil = 0;
    //int newseuil = (max + min)/2;

    N = max;

    double V = 0.0;
    int S = 0;

    double mean = 0.0;
    for( int i=min; i<=max; i++)
    {
      mean += (double)(i) * (double)histogram[i];
    }
    mean /= (double)totVoxels;

    //std::cout << "Min/Max/Mean: " << min << "/" << max << "/" << mean << std::endl;

    //while( abs(newseuil - seuil)>EPS )
    for( seuil = min; seuil<=max; seuil++)
    {
      //seuil = newseuil;

      // compute the classes:
      double mean0 = 0.0;
      double mean1 = 0.0;
      //double std0 = 0.0;
      //double std1 = 0.0;

      double num0 = 0.0;
      double num1 = 0.0;
      for( int i=min; i<seuil; i++)
      {
        //mean0 += (double)(histogram[i])/(double)(totVoxels) * (double)(i);
        mean0 += (double)histogram[i] * (double)i;
        num0  += (double)histogram[i];
      }

      for( int i=seuil; i<max; i++)
      {
        //mean1 += (double)(histogram[i])/(double)(totVoxels) * (double)(i);
        mean1 += (double)histogram[i] * (double)i;
        num1  += (double)histogram[i];
      }

      if( num0 )
      {
        mean0/=num0;
      }
      if( num1 )
      {
        mean1/=num1;
      }

      /*
      for( int i=0; i<seuil; i++)
      {
      std0 += (double)histogram[i]/(double)totVoxels* ((double)i - mean0)*((double)i - mean0);
      //std0 += (double)histogram[i]/num0* ((double)i - mean0)*((double)i - mean0);
      }

      for( int i=seuil; i<65536; i++)
      {
      std1 += (double)histogram[i]/(double)totVoxels* ((double)i - mean1)* ((double)i - mean1);
      //std1 += (double)histogram[i]/num1* ((double)i - mean1)* ((double)i - mean1);
      }
      */


      //std0 = sqrt(std0);
      //std1 = sqrt(std1);

      //std::cout << "Classe 0: " << mean0 << " " << std0 << std::endl;
      //std::cout << "Classe 1: " << mean1 << " " << std1 << std::endl;

      //std::cout << "Classe 0: " << mean0 << std::endl;
      //std::cout << "Classe 1: " << mean1 << std::endl;

      //double alpha = std0*std1/(std0+std1);

      //std::cout << "alpha: " << alpha << " " << std0 << " " << std1 << std::endl;
      //newseuil = (int) (alpha*(mean0/std0 + mean1/std1));

      //std::cout << "New threshold: " << newseuil << std::endl;

      /*
      if( newseuil > 65535 || newseuil<0)
      {
      std::cerr << "Error: threshold is too low or high, exiting" << std::endl;
      return -1;
      }*/

      double Vm = num0 * (mean0 - mean)*(mean0 - mean) + num1*(mean1 - mean)*(mean1 - mean);
      if( Vm > V )
      {
        V = Vm;
        S = seuil;
        //std::cout << "New seuil: " << S << std::endl;
        //getchar();
      }

    }

    delete [] histogram;

    std::cout << "Seuil: " << S << std::endl;

    return S;
  }
}

#endif // __itkBrainMaskExtractionImageFilter_txx
