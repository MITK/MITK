/*=======================================================================
Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter_cpp
#define __itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter_cpp

#include "itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"

#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_symmetric_eigensystem.h"

#include "itkRegularizedIVIMReconstructionFilter.h"

#include <mitkLogMacros.h>

#define IVIM_FOO -100000

namespace itk {


  template< class TIn, class TOut>
  DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<TIn, TOut>
      ::DiffusionIntravoxelIncoherentMotionReconstructionImageFilter() :
      m_GradientDirectionContainer(NULL),
      m_Method(IVIM_DSTAR_FIX),
      m_FitDStar(true),
      m_Verbose(true)
  {
    this->SetNumberOfRequiredInputs( 1 );

    this->SetNumberOfRequiredOutputs( 3 );
    typename OutputImageType::Pointer outputPtr1 = OutputImageType::New();
    this->SetNthOutput(0, outputPtr1.GetPointer());
    typename OutputImageType::Pointer outputPtr2 = OutputImageType::New();
    this->SetNthOutput(1, outputPtr2.GetPointer());
    typename OutputImageType::Pointer outputPtr3 = OutputImageType::New();
    this->SetNthOutput(2, outputPtr3.GetPointer());
  }



  template< class TIn, class TOut>
  void DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<TIn, TOut>
      ::BeforeThreadedGenerateData()
  {

    // Input must be an itk::VectorImage.
    std::string gradientImageClassName(
        this->ProcessObject::GetInput(0)->GetNameOfClass());
    if ( strcmp(gradientImageClassName.c_str(),"VectorImage") != 0 )
    {
      itkExceptionMacro( <<
                         "There is only one Gradient image. I expect that to be a VectorImage. "
                         << "But its of type: " << gradientImageClassName );
    }

    typename InputImageType::Pointer img = static_cast< InputImageType * >(
        this->ProcessObject::GetInput(0) );

    // Compute the indicies of the baseline images and gradient images
    GradientDirectionContainerType::ConstIterator gdcit =
        this->m_GradientDirectionContainer->Begin();
    while( gdcit != this->m_GradientDirectionContainer->End() )
    {
      if(gdcit.Value().one_norm() <= 0.0)
      {
        m_Snap.baselineind.push_back(gdcit.Index());
      }
      else
      {
        m_Snap.gradientind.push_back(gdcit.Index());
        double twonorm = gdcit.Value().two_norm();
        m_Snap.bvals.push_back( m_BValue*twonorm*twonorm );
      }

      ++gdcit;
    }


    // check sind die grad und base gleichlang? alle grad gerade und base ungerade? dann iterierende aufnahme!!
    m_Snap.iterated_sequence = false;
    if(m_Snap.baselineind.size() == m_Snap.gradientind.size())
    {
      int size = m_Snap.baselineind.size();
      int sum_b = 0, sum_g = 0;
      for(int i=0; i<size; i++)
      {
        sum_b += m_Snap.baselineind.at(i) % 2;
        sum_g += m_Snap.gradientind.at(i) % 2;
      }
      if(  (sum_b == size || sum_b == 0)
        && (sum_g == size || sum_g == 0) )
        {
        m_Snap.iterated_sequence = true;
        if(m_Verbose)
        {
          MITK_INFO << "Iterating b0 and diffusion weighted aquisition detected. Weighting each weighted measurement with its own b0.";
        }
      }
    }

    // number of measurements
    m_Snap.N = m_Snap.gradientind.size();

    // bvalue array
    m_Snap.bvalues.set_size(m_Snap.N);
    for(int i=0; i<m_Snap.N; i++)
    {
      m_Snap.bvalues[i] = m_Snap.bvals.at(i);
    }

    if(m_Verbose)
    {
      std::cout << "ref bval: " << m_BValue << "; b-values: ";
      for(int i=0; i<m_Snap.N; i++)
      {
        std::cout << m_Snap.bvalues[i] << "; ";
      }
      std::cout << std::endl;
    }

    // extract bvals higher than threshold
    if(m_Method == IVIM_D_THEN_DSTAR || m_Method == IVIM_LINEAR_D_THEN_F || m_Method == IVIM_REGULARIZED)
    {
      for(int i=0; i<m_Snap.N; i++)
      {
        if(m_Snap.bvalues[i]>m_BThres)
        {
          m_Snap.high_indices.push_back(i);
        }
      }
    }
    m_Snap.Nhigh = m_Snap.high_indices.size();
    m_Snap.high_bvalues.set_size(m_Snap.Nhigh);
    m_Snap.high_meas.set_size(m_Snap.Nhigh);
    for(int i=0; i<m_Snap.Nhigh; i++)
    {
      m_Snap.high_bvalues[i] = m_Snap.bvalues[m_Snap.high_indices.at(i)];
    }

  }

  template< class TIn, class TOut>
  MeasAndBvals DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<TIn, TOut>
      ::ApplyS0Threshold(vnl_vector<double> &meas, vnl_vector<double> &bvals)
  {
    std::vector<double> newmeas;
    std::vector<double> newbvals;

    int N = meas.size();
    for(int i=0; i<N; i++)
    {
      if( meas[i] != IVIM_FOO )
      {
        newmeas.push_back(meas[i]);
        newbvals.push_back(bvals[i]);
      }
    }

    MeasAndBvals retval;

    retval.N = newmeas.size();

    retval.meas.set_size(retval.N);
    for(int i=0; i<newmeas.size(); i++)
    {
      retval.meas[i] = newmeas[i];
    }

    retval.bvals.set_size(retval.N);
    for(int i=0; i<newbvals.size(); i++)
    {
      retval.bvals[i] = newbvals[i];
    }

    return retval;
  }

  template< class TIn, class TOut>
  void DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<TIn, TOut>
      ::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                             int )
  {

    typename OutputImageType::Pointer outputImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
    ImageRegionIterator< OutputImageType > oit(outputImage, outputRegionForThread);
    oit.GoToBegin();

    typename OutputImageType::Pointer dImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetOutput(1));
    ImageRegionIterator< OutputImageType > oit1(dImage, outputRegionForThread);
    oit1.GoToBegin();

    typename OutputImageType::Pointer dstarImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetOutput(2));
    ImageRegionIterator< OutputImageType > oit2(dstarImage, outputRegionForThread);
    oit2.GoToBegin();

    typedef ImageRegionConstIterator< InputImageType > InputIteratorType;
    typedef typename InputImageType::PixelType         InputVectorType;
    typename InputImageType::Pointer inputImagePointer = NULL;

    // Would have liked a dynamic_cast here, but seems SGI doesn't like it
    // The enum will DiffusionIntravoxelIncoherentMotionReconstructionImageFilterensure that an inappropriate cast is not done
    inputImagePointer = static_cast< InputImageType * >(
        this->ProcessObject::GetInput(0) );

    InputIteratorType iit(inputImagePointer, outputRegionForThread );
    iit.GoToBegin();

    // init internal vector image for regularized fit
    m_InternalVectorImage = VectorImageType::New();
    m_InternalVectorImage->SetSpacing( inputImagePointer->GetSpacing() );   // Set the image spacing
    m_InternalVectorImage->SetOrigin( inputImagePointer->GetOrigin() );     // Set the image origin
    m_InternalVectorImage->SetDirection( inputImagePointer->GetDirection() );  // Set the image direction
    m_InternalVectorImage->SetRegions( inputImagePointer->GetLargestPossibleRegion() );

    m_InitialFitImage = InitialFitImageType::New();
    m_InitialFitImage->SetSpacing( inputImagePointer->GetSpacing() );   // Set the image spacing
    m_InitialFitImage->SetOrigin( inputImagePointer->GetOrigin() );     // Set the image origin
    m_InitialFitImage->SetDirection( inputImagePointer->GetDirection() );  // Set the image direction
    m_InitialFitImage->SetRegions( inputImagePointer->GetLargestPossibleRegion() );

    if(m_Method == IVIM_REGULARIZED)
    {
      m_InternalVectorImage->SetVectorLength(m_Snap.Nhigh);
      m_InternalVectorImage->Allocate();
      VectorImageType::PixelType varvec(m_Snap.Nhigh);
      for(int i=0; i<m_Snap.Nhigh; i++) varvec[i] = IVIM_FOO;
      m_InternalVectorImage->FillBuffer(varvec);

      m_InitialFitImage->Allocate();
      InitialFitImageType::PixelType vec;
      vec[0] = 0.5; vec[1] = 0.01; vec[2]=0.001;
      m_InitialFitImage->FillBuffer(vec);
    }

    typedef itk::ImageRegionIterator<VectorImageType> VectorIteratorType;
    VectorIteratorType vecit(m_InternalVectorImage, outputRegionForThread );
    vecit.GoToBegin();

    typedef itk::ImageRegionIterator<InitialFitImageType> InitIteratorType;
    InitIteratorType initit(m_InitialFitImage, outputRegionForThread );
    initit.GoToBegin();

    while( !iit.IsAtEnd() )
    {
      InputVectorType measvec = iit.Get();

      typename NumericTraits<InputPixelType>::AccumulateType b0 = NumericTraits<InputPixelType>::Zero;

      m_Snap.meas.set_size(m_Snap.N);
      m_Snap.allmeas.set_size(m_Snap.N);
      if(!m_Snap.iterated_sequence)
      {
        // Average the baseline image pixels
        for(unsigned int i = 0; i < m_Snap.baselineind.size(); ++i)
        {
          b0 += measvec[m_Snap.baselineind[i]];
        }
        if(m_Snap.baselineind.size())
          b0 /= m_Snap.baselineind.size();

        // measurement vector
        for(int i = 0; i < m_Snap.N; ++i)
        {
          m_Snap.allmeas[i] = measvec[m_Snap.gradientind[i]] / (b0+.0001);

          if(measvec[m_Snap.gradientind[i]] > m_S0Thres)
          {
            m_Snap.meas[i] = measvec[m_Snap.gradientind[i]] / (b0+.0001);
          }
          else
          {
            m_Snap.meas[i] = IVIM_FOO;
          }
        }
      }
      else
      {
        // measurement vector
        for(int i = 0; i < m_Snap.N; ++i)
        {
          b0 = measvec[m_Snap.baselineind[i]];

          m_Snap.allmeas[i] = measvec[m_Snap.gradientind[i]] / (b0+.0001);

          if(measvec[m_Snap.gradientind[i]] > m_S0Thres)
          {
            m_Snap.meas[i] = measvec[m_Snap.gradientind[i]] / (b0+.0001);
          }
          else
          {
            m_Snap.meas[i] = IVIM_FOO;
          }
        }
      }

      m_Snap.currentF = 0;
      m_Snap.currentD = 0;
      m_Snap.currentDStar = 0;

      switch(m_Method)
      {

      case IVIM_D_THEN_DSTAR:
        {

          for(int i=0; i<m_Snap.Nhigh; i++)
          {
            m_Snap.high_meas[i] = m_Snap.meas[m_Snap.high_indices.at(i)];
          }

          MeasAndBvals input = ApplyS0Threshold(m_Snap.high_meas, m_Snap.high_bvalues);
          m_Snap.bvals1 = input.bvals;
          m_Snap.meas1 = input.meas;

          if (input.N < 2)
          {
            if (input.N == 1)
            {
              m_Snap.currentD = - log(input.meas[0]) / input.bvals[0];
              m_Snap.currentF = 0;
              m_Snap.currentDStar = 0;
            }
            break;
          }

          IVIM_d_and_f f_donly(input.N);
          f_donly.set_bvalues(input.bvals);
          f_donly.set_measurements(input.meas);

          vnl_vector< double > x_donly(2);
          x_donly[0] = 0.001;
          x_donly[1] = 0.1;
          // f 0.1 Dstar 0.01 D 0.001

          vnl_levenberg_marquardt lm_donly(f_donly);
          lm_donly.set_f_tolerance(0.0001);
          lm_donly.minimize(x_donly);
          m_Snap.currentD = x_donly[0];
          m_Snap.currentF = x_donly[1];


          if(m_FitDStar)
          {
            MeasAndBvals input2 = ApplyS0Threshold(m_Snap.meas, m_Snap.bvalues);
            m_Snap.bvals2 = input2.bvals;
            m_Snap.meas2 = input2.meas;
            if (input2.N < 2) break;

            IVIM_dstar_only f_dstar_only(input2.N,m_Snap.currentD,m_Snap.currentF);
            f_dstar_only.set_bvalues(input2.bvals);
            f_dstar_only.set_measurements(input2.meas);

            vnl_vector< double > x_dstar_only(1);
            vnl_vector< double > fx_dstar_only(input2.N);

            double opt = 1111111111111111.0;
            int opt_idx = -1;
            int num_its = 100;
            double min_val = .001;
            double max_val = .15;
            for(int i=0; i<num_its; i++)
            {
              x_dstar_only[0] = min_val + i * ((max_val-min_val) / num_its);
              f_dstar_only.f(x_dstar_only, fx_dstar_only);
              double err = fx_dstar_only.two_norm();
              if(err<opt)
              {
                opt = err;
                opt_idx = i;
              }
            }

            m_Snap.currentDStar = min_val + opt_idx * ((max_val-min_val) / num_its);
            //          IVIM_fixd f_fixd(input2.N,m_Snap.currentD);
            //          f_fixd.set_bvalues(input2.bvals);
            //          f_fixd.set_measurements(input2.meas);

            //          vnl_vector< double > x_fixd(2);
            //          x_fixd[0] = 0.1;
            //          x_fixd[1] = 0.01;
            //          // f 0.1 Dstar 0.01 D 0.001

            //          vnl_levenberg_marquardt lm_fixd(f_fixd);
            //          lm_fixd.set_f_tolerance(0.0001);
            //          lm_fixd.minimize(x_fixd);

            //          m_Snap.currentF = x_fixd[0];
            //          m_Snap.currentDStar = x_fixd[1];
          }

          break;
        }

      case IVIM_DSTAR_FIX:
        {
          MeasAndBvals input = ApplyS0Threshold(m_Snap.meas, m_Snap.bvalues);
          m_Snap.bvals1 = input.bvals;
          m_Snap.meas1 = input.meas;
          if (input.N < 2) break;

          IVIM_fixdstar f_fixdstar(input.N,m_DStar);
          f_fixdstar.set_bvalues(input.bvals);
          f_fixdstar.set_measurements(input.meas);

          vnl_vector< double > x(2);
          x[0] = 0.1;
          x[1] = 0.001;
          // f 0.1 Dstar 0.01 D 0.001

          vnl_levenberg_marquardt lm(f_fixdstar);
          lm.set_f_tolerance(0.0001);
          lm.minimize(x);

          m_Snap.currentF = x[0];
          m_Snap.currentD = x[1];
          m_Snap.currentDStar = m_DStar;

          break;
        }

      case IVIM_FIT_ALL:
        {

          MeasAndBvals input = ApplyS0Threshold(m_Snap.meas, m_Snap.bvalues);
          m_Snap.bvals1 = input.bvals;
          m_Snap.meas1 = input.meas;
          if (input.N < 3) break;

          IVIM_3param f_3param(input.N);
          f_3param.set_bvalues(input.bvals);
          f_3param.set_measurements(input.meas);

          vnl_vector< double > x(3);
          x[0] = 0.1;
          x[1] = 0.001;
          x[2] = 0.01;
          // f 0.1 Dstar 0.01 D 0.001

          vnl_levenberg_marquardt lm(f_3param);
          lm.set_f_tolerance(0.0001);
          lm.minimize(x);

          m_Snap.currentF = x[0];
          m_Snap.currentD = x[1];
          m_Snap.currentDStar = x[2];

          break;
        }

      case IVIM_LINEAR_D_THEN_F:
        {

          //          // neglect zero-measurements
          //          bool zero = false;
          //          for(int i=0; i<Nhigh; i++)
          //          {
          //            if( meas[high_indices.at(i)] == 0 )
          //            {
          //              f=0;
          //              zero = true;
          //              break;
          //            }
          //          }
          //          if(zero) break;

          for(int i=0; i<m_Snap.Nhigh; i++)
          {
            m_Snap.high_meas[i] = m_Snap.meas[m_Snap.high_indices.at(i)];
          }

          MeasAndBvals input = ApplyS0Threshold(m_Snap.high_meas, m_Snap.high_bvalues);
          m_Snap.bvals1 = input.bvals;
          m_Snap.meas1 = input.meas;

          if (input.N < 2)
          {
            if (input.N == 1)
            {
              m_Snap.currentD = - log(input.meas[0]) / input.bvals[0];
              m_Snap.currentF = 0;
              m_Snap.currentDStar = 0;
            }
            break;
          }

          for(int i=0; i<input.N; i++)
          {
            input.meas[i] = log(input.meas[i]);
          }

          double bval_m = 0;
          double meas_m = 0;
          for(int i=0; i<input.N; i++)
          {
            bval_m += input.bvals[i];
            meas_m += input.meas[i];
          }
          bval_m /= input.N;
          meas_m /= input.N;

          vnl_matrix<double> X(input.N,2);
          for(int i=0; i<input.N; i++)
          {
            X(i,0) = input.bvals[i] - bval_m;
            X(i,1) = input.meas[i] - meas_m;
          }

          vnl_matrix<double> XX = X.transpose() * X;
          vnl_symmetric_eigensystem<double> eigs(XX);

          vnl_vector<double> eig;
          if(eigs.get_eigenvalue(0) > eigs.get_eigenvalue(1))
            eig = eigs.get_eigenvector(0);
          else
            eig = eigs.get_eigenvector(1);

          m_Snap.currentF = 1 - exp( meas_m - bval_m*(eig(1)/eig(0)) );
          m_Snap.currentD = -eig(1)/eig(0);

          if(m_FitDStar)
          {
            MeasAndBvals input2 = ApplyS0Threshold(m_Snap.meas, m_Snap.bvalues);
            m_Snap.bvals2 = input2.bvals;
            m_Snap.meas2 = input2.meas;
            if (input2.N < 2) break;

            IVIM_dstar_only f_dstar_only(input2.N,m_Snap.currentD,m_Snap.currentF);
            f_dstar_only.set_bvalues(input2.bvals);
            f_dstar_only.set_measurements(input2.meas);

            vnl_vector< double > x_dstar_only(1);
            vnl_vector< double > fx_dstar_only(input2.N);

            double opt = 1111111111111111.0;
            int opt_idx = -1;
            int num_its = 100;
            double min_val = .001;
            double max_val = .15;
            for(int i=0; i<num_its; i++)
            {
              x_dstar_only[0] = min_val + i * ((max_val-min_val) / num_its);
              f_dstar_only.f(x_dstar_only, fx_dstar_only);
              double err = fx_dstar_only.two_norm();
              if(err<opt)
              {
                opt = err;
                opt_idx = i;
              }
            }

            m_Snap.currentDStar = min_val + opt_idx * ((max_val-min_val) / num_its);
          }
          // MITK_INFO << "choosing " << opt_idx << " => " << DStar;
          //          x_dstar_only[0] = 0.01;
          //          // f 0.1 Dstar 0.01 D 0.001

          //          vnl_levenberg_marquardt lm_dstar_only(f_dstar_only);
          //          lm_dstar_only.set_f_tolerance(0.0001);
          //          lm_dstar_only.minimize(x_dstar_only);

          //          DStar = x_dstar_only[0];

          break;
        }

      case IVIM_REGULARIZED:
        {
          //m_Snap.high_meas, m_Snap.high_bvalues;
          for(int i=0; i<m_Snap.Nhigh; i++)
          {
            m_Snap.high_meas[i] = m_Snap.meas[m_Snap.high_indices.at(i)];
          }

          MeasAndBvals input = ApplyS0Threshold(m_Snap.high_meas, m_Snap.high_bvalues);

          vnl_vector< double > x_donly(2);
          x_donly[0] = 0.001;
          x_donly[1] = 0.1;

          if(input.N >= 2)
          {
            IVIM_d_and_f f_donly(input.N);
            f_donly.set_bvalues(input.bvals);
            f_donly.set_measurements(input.meas);
            //MITK_INFO << "initial fit N=" << input.N << ", min-b = " << input.bvals[0] << ", max-b = " << input.bvals[input.N-1];
            vnl_levenberg_marquardt lm_donly(f_donly);
            lm_donly.set_f_tolerance(0.0001);
            lm_donly.minimize(x_donly);
          }

          typename InitialFitImageType::PixelType initvec;
          initvec[0] = x_donly[1];
          initvec[1] = x_donly[0];
          initit.Set(initvec);
          //MITK_INFO << "Init vox " << initit.GetIndex() << " with " << initvec[0] << "; " << initvec[1];
          ++initit;

          int N = m_Snap.high_meas.size();
          typename VectorImageType::PixelType vec(N);
          for(int i=0; i<N; i++)
          {
            vec[i] = m_Snap.high_meas[i];
            //MITK_INFO << "vec" << i << " = " << m_Snap.high_meas[i];
          }

          vecit.Set(vec);
          ++vecit;

          if(!m_Verbose)
          {
            // report the middle voxel
            if(  vecit.GetIndex()[0] == m_CrossPosition[0]
                 && vecit.GetIndex()[0] == m_CrossPosition[1]
                 && vecit.GetIndex()[0] == m_CrossPosition[2] )
            {
              MeasAndBvals input = ApplyS0Threshold(m_Snap.high_meas, m_Snap.high_bvalues);
              m_Snap.bvals1 = input.bvals;
              m_Snap.meas1 = input.meas;

              MeasAndBvals input2 = ApplyS0Threshold(m_Snap.meas, m_Snap.bvalues);
              m_Snap.bvals2 = input2.bvals;
              m_Snap.meas2 = input2.meas;

              m_tmp_allmeas = m_Snap.allmeas;
            }
          }

          break;
        }
      }
      m_Snap.currentFunceiled = m_Snap.currentF;
      IVIM_CEIL( m_Snap.currentF, 0.0, 1.0 );

      oit.Set( myround(m_Snap.currentF * 10000.0) );
      oit1.Set( myround(m_Snap.currentD * 1000000.0) );
      oit2.Set( myround(m_Snap.currentDStar * 100000.0) );

      // std::cout << "\tf=" << x[0] << "\tD=" << x[1] << " ; "<<std::endl;

      ++oit;
      ++oit1;
      ++oit2;
      ++iit;
    }
    std::cout << std::endl << std::endl;

    if(m_Verbose)
    {
      std::cout << "One Thread finished reconstruction" << std::endl;
    }
  }

  template< class TIn, class TOut>
  void DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<TIn, TOut>
      ::AfterThreadedGenerateData()
  {
    if(m_Method == IVIM_REGULARIZED)
    {
      typename OutputImageType::Pointer outputImage =
          static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));
      ImageRegionIterator< OutputImageType > oit0(outputImage, outputImage->GetLargestPossibleRegion());
      oit0.GoToBegin();

      typename OutputImageType::Pointer dImage =
          static_cast< OutputImageType * >(this->ProcessObject::GetOutput(1));
      ImageRegionIterator< OutputImageType > oit1(dImage, dImage->GetLargestPossibleRegion());
      oit1.GoToBegin();

      typename OutputImageType::Pointer dstarImage =
          static_cast< OutputImageType * >(this->ProcessObject::GetOutput(2));
      ImageRegionIterator< OutputImageType > oit2(dstarImage, dstarImage->GetLargestPossibleRegion());
      oit2.GoToBegin();

      typedef itk::RegularizedIVIMReconstructionFilter
          <double,double,float> RegFitType;
      RegFitType::Pointer filter = RegFitType::New();
      filter->SetInput(m_InitialFitImage);
      filter->SetReferenceImage(m_InternalVectorImage);
      filter->SetBValues(m_Snap.high_bvalues);
      filter->SetNumberIterations(m_NumberIterations);
      filter->SetNumberOfThreads(1);
      filter->SetLambda(m_Lambda);
      filter->Update();
      typename RegFitType::OutputImageType::Pointer outimg = filter->GetOutput();

      ImageRegionConstIterator< RegFitType::OutputImageType > iit(outimg, outimg->GetLargestPossibleRegion());
      iit.GoToBegin();

      while( !iit.IsAtEnd() )
      {
        double f = iit.Get()[0];
        IVIM_CEIL( f, 0.0, 1.0 );

        oit0.Set( myround(f * 100.0) );
        oit1.Set( myround(iit.Get()[1] * 10000.0) );
        oit2.Set( myround(iit.Get()[2] * 1000.0) );

        if(!m_Verbose)
        {
          // report the middle voxel
          if(  iit.GetIndex()[0] == m_CrossPosition[0]
               && iit.GetIndex()[1] == m_CrossPosition[1]
               && iit.GetIndex()[2] == m_CrossPosition[2] )
          {
            m_Snap.currentF = f;
            m_Snap.currentD = iit.Get()[1];
            m_Snap.currentDStar = iit.Get()[2];
            m_Snap.allmeas = m_tmp_allmeas;
            MITK_INFO << "setting " << f << ";" << iit.Get()[1] << ";" << iit.Get()[2];
          }
        }

        ++oit0;
        ++oit1;
        ++oit2;
        ++iit;
      }
    }
  }

  template< class TIn, class TOut>
  double DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<TIn, TOut>
      ::myround(double number)
  {
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
  }

  template< class TIn, class TOut>
  void DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<TIn, TOut>
      ::SetGradientDirections( GradientDirectionContainerType *gradientDirection )
  {
    this->m_GradientDirectionContainer = gradientDirection;
    this->m_NumberOfGradientDirections = gradientDirection->Size();
  }

  template< class TIn, class TOut>
  void DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<TIn, TOut>
      ::PrintSelf(std::ostream& os, Indent indent) const
  {
    Superclass::PrintSelf(os,indent);

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
  }

}

#endif // __itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter_cpp
