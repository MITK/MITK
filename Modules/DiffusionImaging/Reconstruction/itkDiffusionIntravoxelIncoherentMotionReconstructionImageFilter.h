/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDiffusionTensor3DReconstructionImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006-03-27 17:01:06 $
  Version:   $Revision: 1.12 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter_h
#define __itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter_h

#include "itkImageToImageFilter.h"
//#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVectorContainer.h"
#include "itkVectorImage.h"
//#include "QuadProg.h"
#include "itkVectorImage.h"

#include "vnl/vnl_least_squares_function.h"
#include "vnl/algo/vnl_levenberg_marquardt.h"
#include "vnl/vnl_math.h"

#define IVIM_CEIL(val,u,o) (val) =       \
  ( (val) < (u) ) ? ( (u) ) : ( ( (val)>(o) ) ? ( (o) ) : ( (val) ) );

namespace itk{

  ///////////////////////////////////////////////////////////////////////
  // baseclass for IVIM fitting algorithms
  struct IVIM_base
  {

    void set_measurements(const vnl_vector<double>& x)
    {
      measurements.set_size(x.size());
      measurements.copy_in(x.data_block());
    }

    void set_bvalues(const vnl_vector<double>& x)
    {
      bvalues.set_size(x.size());
      bvalues.copy_in(x.data_block());
    }

    vnl_vector<double> measurements;
    vnl_vector<double> bvalues;

    int N;

  };

  ///////////////////////////////////////////////////////////////////////
  // fitt all three parameters
  struct IVIM_3param : public IVIM_base, vnl_least_squares_function
  {

    IVIM_3param(unsigned int number_of_measurements) :
        vnl_least_squares_function(3, number_of_measurements, no_gradient)
    {
      N = get_number_of_residuals();
    }

    void f(const vnl_vector<double>& x, vnl_vector<double>& fx) {

      double ef = x[0];
      double D = x[1];
      double Dstar = x[2];

      for(int s=0; s<N; s++)
      {
        double approx = (1-ef)*exp(-bvalues[s]*D)+ef*exp(-bvalues[s]*(D+Dstar));
        fx[s] = vnl_math_abs( measurements[s] - approx );
      }

    }
  };

  ///////////////////////////////////////////////////////////////////////
  // fit by setting DStar to a fix value
  struct IVIM_fixdstar : public IVIM_base, vnl_least_squares_function
  {

    IVIM_fixdstar(unsigned int number_of_measurements, double DStar) :
        vnl_least_squares_function(2, number_of_measurements, no_gradient)
    {
      N = get_number_of_residuals();
      fixDStar = DStar;
    }

    void f(const vnl_vector<double>& x, vnl_vector<double>& fx) {

      double ef = x[0];
      double D = x[1];

      for(int s=0; s<N; s++)
      {
        double approx = (1-ef)*exp(-bvalues[s]*D)+ef*exp(-bvalues[s]*(D+fixDStar));
        fx[s] = vnl_math_abs( measurements[s] - approx );
      }

    }

    double fixDStar;

  };

  ///////////////////////////////////////////////////////////////////////
  // fit a monoexponential curve only estimating D
  struct IVIM_d_and_f : public IVIM_base, vnl_least_squares_function
  {

    IVIM_d_and_f(unsigned int number_of_measurements) :
        vnl_least_squares_function(2, number_of_measurements, no_gradient)
    {
      N = get_number_of_residuals();
    }

    void f(const vnl_vector<double>& x, vnl_vector<double>& fx) {

      double D = x[0];
      double f = x[1];

      for(int s=0; s<N; s++)
      {
        double approx = (1-f) * exp(-bvalues[s]*D);
        fx[s] = vnl_math_abs( measurements[s] - approx );
      }

    }
  };

  ///////////////////////////////////////////////////////////////////////
  // fiting DStar and f with fix value of D
  struct IVIM_fixd : public IVIM_base, vnl_least_squares_function
  {

    IVIM_fixd(unsigned int number_of_measurements, double D) :
        vnl_least_squares_function(2, number_of_measurements, no_gradient)
    {
      N = get_number_of_residuals();
      fixD = D;
    }

    void f(const vnl_vector<double>& x, vnl_vector<double>& fx) {

      double ef = x[0];
      double Dstar = x[1];

      for(int s=0; s<N; s++)
      {
        double approx = (1-ef)*exp(-bvalues[s]*fixD)+ef*exp(-bvalues[s]*(fixD+Dstar));
        fx[s] = vnl_math_abs( measurements[s] - approx );
      }

    }

    double fixD;
  };

  ///////////////////////////////////////////////////////////////////////
  // fiting DStar with given f and D
  struct IVIM_dstar_only : public IVIM_base, vnl_least_squares_function
  {

    IVIM_dstar_only(unsigned int number_of_measurements, double D, double f) :
        vnl_least_squares_function(1, number_of_measurements, no_gradient)
    {
      N = get_number_of_residuals();
      fixD = D;
      fixF = f;
    }

    void f(const vnl_vector<double>& x, vnl_vector<double>& fx) {

      double Dstar = x[0];

      for(int s=0; s<N; s++)
      {
        double approx = (1-fixF)*exp(-bvalues[s]*fixD)+fixF*exp(-bvalues[s]*(fixD+Dstar));
        fx[s] = vnl_math_abs( measurements[s] - approx );
      }

    }

    double fixD;
    double fixF;
  };

  struct MeasAndBvals
  {
    vnl_vector<double> meas;
    vnl_vector<double> bvals;
    int N;
  };

  /** \class DiffusionIntravoxelIncoherentMotionReconstructionImageFilter
 * 
 */

  template< class TInputPixelType,
  class TOutputPixelType>
  class DiffusionIntravoxelIncoherentMotionReconstructionImageFilter :
  public ImageToImageFilter< VectorImage< TInputPixelType,  3 >,
  Image< TOutputPixelType, 3 > >
  {

  public:

    struct IVIMSnapshot
    {
      double currentF;
      double currentFunceiled;
      double currentD;
      double currentDStar;

      bool iterated_sequence; // wether each measurement has its own b0-acqu.
      std::vector<unsigned int> baselineind; // baseline image indicies

      int N;                                 // total number of measurements
      std::vector<unsigned int> gradientind; // gradient image indicies
      std::vector<double> bvals;             // b-values != 0
      vnl_vector<double> bvalues;            // copy of bvalues != 0
      vnl_vector<double> meas;               // all measurements, thresholded blanked out
      vnl_vector<double> allmeas;            // all measurements

      int Nhigh;                       // number of used measurements
      std::vector<int> high_indices;   // indices of used measurements
      vnl_vector<double> high_bvalues; // bvals of used measurements
      vnl_vector<double> high_meas;    // used measurements

      vnl_vector<double> meas1;
      vnl_vector<double> bvals1;

      vnl_vector<double> meas2;
      vnl_vector<double> bvals2;

//      double currentADC;
//      int Nadc;                       // number used measurements for ADC calculation
//      std::vector<int> adc_indices;   // indices used for ADC calculation
//      vnl_vector<double> adc_bvalues; // bvals used for ADC calculation
//      vnl_vector<double> adc_meas;    // for ADC calculation

    };

    enum IVIM_Method
    {
      IVIM_FIT_ALL,
      IVIM_DSTAR_FIX,
      IVIM_D_THEN_DSTAR,
      IVIM_LINEAR_D_THEN_F,
      IVIM_REGULARIZED
    };

    typedef DiffusionIntravoxelIncoherentMotionReconstructionImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage< TInputPixelType, 3>,
      Image< TOutputPixelType,3 > >                 Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Runtime information support. */
    itkTypeMacro(DiffusionIntravoxelIncoherentMotionReconstructionImageFilter,
                 ImageToImageFilter);

    typedef TOutputPixelType                         OutputPixelType;

    typedef TInputPixelType                          InputPixelType;

    /** Reference image data,  This image is aquired in the absence
   * of a diffusion sensitizing field gradient */
    typedef typename Superclass::InputImageType      InputImageType;

    typedef Image< OutputPixelType, 3 >              OutputImageType;

    typedef typename Superclass::OutputImageRegionType
        OutputImageRegionType;

    /** Holds each magnetic field gradient used to acquire one DWImage */
    typedef vnl_vector_fixed< double, 3 >            GradientDirectionType;

    /** Container to hold gradient directions of the 'n' DW measurements */
    typedef VectorContainer< unsigned int,
    GradientDirectionType >                  GradientDirectionContainerType;

    // vector image typedefs for regularized fit
    typedef itk::VectorImage<float,3> VectorImageType;
    typedef itk::Image<itk::Vector<double, 3>, 3> InitialFitImageType;

    /** set method to add gradient directions and its corresponding
   * image. The image here is a VectorImage. The user is expected to pass the 
   * gradient directions in a container. The ith element of the container 
   * corresponds to the gradient direction of the ith component image the 
   * VectorImage.  For the baseline image, a vector of all zeros
   * should be set.*/
    void SetGradientDirections( GradientDirectionContainerType * );

    void SetBValue(double bval){m_BValue = bval;}
    void SetBThres(double bval){m_BThres = bval;}
    void SetS0Thres(double val){m_S0Thres = val;}
    void SetDStar(double dstar){m_DStar  = dstar;}
    void SetFitDStar(bool fit){m_FitDStar  = fit;}
    void SetVerbose(bool verbose){m_Verbose = verbose;}
    void SetNumberIterations(int num){m_NumberIterations = num;}
    void SetLambda(double lambda){m_Lambda = lambda;}
    void SetCrossPosition(typename InputImageType::IndexType crosspos){this->m_CrossPosition = crosspos;}
    void SetMethod(IVIM_Method method){m_Method = method;}
//    void SetGradientIndicesForADCCalculation(std::vector<int> inds){this->m_ADCInds = inds;}

    IVIMSnapshot GetSnapshot(){return m_Snap;}

    /** Return the gradient direction. idx is 0 based */
    virtual GradientDirectionType GetGradientDirection( unsigned int idx) const
    {
      if( idx >= m_NumberOfGradientDirections )
      {
        itkExceptionMacro( << "Gradient direction " << idx << "does not exist" );
      }
      return m_GradientDirectionContainer->ElementAt( idx+1 );
    }

  protected:
    DiffusionIntravoxelIncoherentMotionReconstructionImageFilter();
    ~DiffusionIntravoxelIncoherentMotionReconstructionImageFilter() {};
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const
                               OutputImageRegionType &outputRegionForThread, int);
    void AfterThreadedGenerateData();

    MeasAndBvals ApplyS0Threshold(vnl_vector<double> &meas, vnl_vector<double> &bvals);

  private:

    double myround(double number);

    /** container to hold gradient directions */
    GradientDirectionContainerType::Pointer           m_GradientDirectionContainer;

    /** Number of gradient measurements */
    unsigned int                                      m_NumberOfGradientDirections;

    double m_BValue;

    double m_BThres;

    double m_S0Thres;

    double m_DStar;

    IVIM_Method m_Method;

    typename OutputImageType::Pointer m_DMap;

    typename OutputImageType::Pointer m_DStarMap;

//    typename OutputImageType::Pointer m_ADCMap;

    bool m_FitDStar;

    IVIMSnapshot m_Snap;

    bool m_Verbose;

    typename VectorImageType::Pointer m_InternalVectorImage;

    typename InitialFitImageType::Pointer m_InitialFitImage;

    int m_NumberIterations; // for total variation

    vnl_vector<double> m_tmp_allmeas;

    double m_Lambda;

    typename InputImageType::IndexType m_CrossPosition;

//    std::vector<int> m_ADCInds;

  };

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter.cpp"
#endif

#endif //__itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter_h

