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

#ifndef _itk_MultiShellRadialAdcKurtosisImageFilter_h_
#define _itk_MultiShellRadialAdcKurtosisImageFilter_h_

#include <itkImageToImageFilter.h>
#include <itkVectorImage.h>
#include <itkPointShell.h>
#include "vnl/vnl_least_squares_function.h"
#include "vnl/algo/vnl_levenberg_marquardt.h"

namespace itk
{

/**
* \brief Select subset of the input vectors equally distributed over the sphere using an iterative electrostatic repulsion strategy.   */

  template <class TInputScalarType, class TOutputScalarType>
  class MultiShellRadialAdcKurtosisImageFilter
    : public ImageToImageFilter<itk::VectorImage<TInputScalarType,3>, itk::VectorImage<TOutputScalarType,3> >
  {

  private:
    struct lestSquaresFunction: public vnl_least_squares_function
    {

      void set_measurements(const vnl_vector<double>& x)
      {
        measurements.set_size(x.size());
        measurements.copy_in(x.data_block());
      }

      void set_bvalues(const vnl_vector<double>& x)
      {
        bValueVector.set_size(x.size());
        bValueVector.copy_in(x.data_block());
      }

      void set_reference_measurement(const double & x)
      {
        S0 = x;
      }

      vnl_vector<double> measurements;
      vnl_vector<double> bValueVector;
      double S0;
      int N;

      lestSquaresFunction(unsigned int number_of_measurements) :
        vnl_least_squares_function(2 /*number of unknowns [ADC AKC]*/, number_of_measurements, no_gradient)
      {
        N = get_number_of_residuals();
      }

      void f(const vnl_vector<double>& x, vnl_vector<double>& fx) {

        const double & D = x[0];
        const double & K = x[1];
        const vnl_vector<double> & b = bValueVector;

        for(int s=0; s<N; s++)
        {
          double approx = std::log(S0) - b[s] * D + 1./6. *b[s] * b[s] *D * D * K;
          fx[s] = vnl_math_abs( std::log(measurements[s]) - approx );
        }

      }
    };

  public:

    typedef MultiShellRadialAdcKurtosisImageFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< itk::VectorImage<TInputScalarType,3>, itk::VectorImage<TOutputScalarType,3> > Superclass;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(MultiShellAdcAverageReconstructionImageFilter, ImageToImageFilter)

    typedef TInputScalarType                         InputScalarType;
    typedef itk::VectorImage<InputScalarType,3>      InputImageType;
    typedef typename InputImageType::PixelType       InputPixelType;

    typedef TOutputScalarType                         OutputScalarType;
    typedef itk::VectorImage<OutputScalarType,3>      OutputImageType;
    typedef typename OutputImageType::PixelType       OutputPixelType;

    typedef OutputScalarType                          BaselineScalarType;
    typedef BaselineScalarType                        BaselinePixelType;
    typedef typename itk::Image<BaselinePixelType,3>  BaselineImageType;

    typedef vnl_vector_fixed< double, 3 >                               GradientDirectionType;
    typedef itk::VectorContainer< unsigned int, GradientDirectionType > GradientDirectionContainerType;

    typedef std::vector<unsigned int>           IndicesVector;
    typedef std::map<unsigned int, IndicesVector>     BValueMap;

    GradientDirectionContainerType::Pointer GetOriginalGradientDirections(){return m_OriginalGradientDirections;}
    void SetOriginalGradientDirections(GradientDirectionContainerType::Pointer ptr){m_OriginalGradientDirections = ptr;}

    GradientDirectionContainerType::Pointer GetTargetGradientDirections(){return m_TargetGradientDirections;}

    double GetTargetB_Value(){return m_TargetB_Value;}

    double GetB_Value(){return m_B_Value;}
    void SetB_Value(double val){m_B_Value = val;}

    void SetOriginalBValueMap(BValueMap inp){m_B_ValueMap = inp;}

  protected:
    MultiShellRadialAdcKurtosisImageFilter();
    ~MultiShellRadialAdcKurtosisImageFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType NumberOfThreads );
    void logarithm( vnl_vector<double> & vec);
    void calculateCoeffs( vnl_matrix<double> & lsfCoeffs, const vnl_matrix<double> & SignalMatrix, const double & S0);
    void calculateSignalFromLsfCoeffs( vnl_vector<double> & vec, const vnl_matrix<double> & lsfCoeffs, const double & S0, const double & b);

    GradientDirectionContainerType::Pointer m_TargetGradientDirections;   ///< container for the subsampled output gradient directions
    GradientDirectionContainerType::Pointer m_OriginalGradientDirections;   ///< input gradient directions

    BValueMap m_B_ValueMap;
    double m_B_Value;

    double m_TargetB_Value;

    std::vector<double> m_WeightsVector;

    std::vector<vnl_matrix< double > > m_ShellInterpolationMatrixVector;
    vnl_vector<double> m_bValueVector;
    std::vector<IndicesVector> m_bZeroIndicesSplitVectors;

    IndicesVector m_allDirectionsIndicies;

    unsigned int m_allDirectionsSize;

   };


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiShellRadialAdcKurtosisImageFilter.cpp"
#endif


#endif
