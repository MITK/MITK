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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkMultiTensorImageFilter_h_
#define __itkMultiTensorImageFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionPropertyHelper.h>
#include <vnl/algo/vnl_levenberg_marquardt.h>
#include <mitkOdfImage.h>
#include <mitkPeakImage.h>
#include <mitkTensorImage.h>

namespace itk{
/** \class MultiTensorImageFilter
 */

template< class TInPixelType, class TOutPixelType >
class MultiTensorImageFilter :
    public ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >
{

public:

  typedef MultiTensorImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ImageToImageFilter< VectorImage< TInPixelType, 3 >, Image< TOutPixelType, 3 > >  Superclass;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionType GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType GradientContainerType;
  typedef itk::Image< unsigned char, 3>     ItkUcharImageType;
  typedef mitk::PeakImage::ItkPeakImageType PeakImageType;
  typedef mitk::TensorImage::PixelType TensorType;
  typedef mitk::TensorImage::ItkTensorImageType TensorImageType;

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(MultiTensorImageFilter, ImageToImageFilter)

  typedef typename Superclass::InputImageType InputImageType;
  typedef typename Superclass::OutputImageType OutputImageType;
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  itkSetMacro( MaskImage, ItkUcharImageType::Pointer )
  itkSetMacro( B_value, double )
  itkSetMacro( GradientDirections, GradientContainerType::Pointer )

  itkGetMacro( PeakImage, PeakImageType::Pointer )
  std::vector< TensorImageType::Pointer > GetTensorImages(){ return m_TensorImages; }

protected:
  MultiTensorImageFilter();
  ~MultiTensorImageFilter() {}
  void PrintSelf(std::ostream& os, Indent indent) const override;

  void BeforeThreadedGenerateData() override;
  void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType) override;

  double                            m_B_value;
  std::vector<double>               m_B_values;
  std::vector<int>                  m_WeightedIndices;
  std::vector<int>                  m_UnWeightedIndices;

  GradientContainerType::Pointer    m_GradientDirections;

  ItkUcharImageType::Pointer        m_MaskImage;
  PeakImageType::Pointer            m_PeakImage;

  std::vector< TensorImageType::Pointer >   m_TensorImages;
  int                                       m_NumTensors;

  vnl_vector<double> FitSingleVoxel( const typename InputImageType::PixelType &input);

//  struct multiTensorLeastSquaresFunction: public vnl_least_squares_function
//  {
//    GradientContainerType::Pointer gradientDirections;
//    typename InputImageType::PixelType measurements;
//    std::vector<double> bValues;
//    double S0;
//    int num_tensors;
//    std::vector<int>  weightedIndices;

//    void set_S0(double val)
//    {
//      S0 = val;
//    }

//    void set_measurements(const typename InputImageType::PixelType& m)
//    {
//      measurements = m;
//    }

//    void set_bvalues(std::vector<double>& x)
//    {
//      bValues = x;
//    }

//    void set_weightedIndices(std::vector<int>& x)
//    {
//      weightedIndices = x;
//    }

//    void set_gradient_directions(const GradientContainerType::Pointer& directions)
//    {
//      gradientDirections = directions;
//    }

//    int check(int n)
//    {
//      if (n==1)
//        return 6;
//      return n*7;
//    }

//    multiTensorLeastSquaresFunction(unsigned int number_of_tensors=2, unsigned int number_of_measurements=1) :
//      vnl_least_squares_function(check(number_of_tensors), number_of_measurements, no_gradient)
//    {
//      num_tensors = number_of_tensors;
//    }

//    void f(const vnl_vector<double>& x, vnl_vector<double>& fx) override
//    {
//      int elements = 7;
//      for(auto s : weightedIndices)
//      {
//        GradientDirectionType g = gradientDirections->GetElement(s);
//        g.normalize();

//        itk::DiffusionTensor3D< double > S;
//        S[0] = g[0]*g[0];
//        S[1] = g[1]*g[0];
//        S[2] = g[2]*g[0];
//        S[3] = g[1]*g[1];
//        S[4] = g[2]*g[1];
//        S[5] = g[2]*g[2];

//        double approx = 0;
//        double penalty = 0;
//        double f = 0;
//        if (num_tensors==1 && x.size()==6)
//          f = 1;

//        for (int i=0; i<num_tensors; i++)
//        {
//          if (num_tensors>1 && x.size()>6 && x[elements-1+i*elements]<0) // penalty if volume fractiuon is < 0
//            penalty += 10e6;

//          if (num_tensors>1 && x.size()>6)
//            f += fabs(x[elements-1+i*elements]);

//          double D = x[0+i*elements]*S[0] + x[1+i*elements]*S[1] + x[2+i*elements]*S[2] +
//                     x[1+i*elements]*S[1] + x[3+i*elements]*S[3] + x[4+i*elements]*S[4] +
//                     x[2+i*elements]*S[2] + x[4+i*elements]*S[4] + x[5+i*elements]*S[5];
//          double signal = S0 * std::exp ( -bValues[s] * D );
//          if (num_tensors>1 && x.size()>6)
//            signal *= x[elements-1+i*elements];
//          approx += signal;
//        }

//        const double factor = measurements[s] - approx;
//        penalty += fabs(f-1)*10e7;  // penalty if volume fractiuon sum is != 1

//        fx[s] = factor*factor + penalty;
//      }
//    }

//  };

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiTensorImageFilter.txx"
#endif

#endif //__itkMultiTensorImageFilter_h_

