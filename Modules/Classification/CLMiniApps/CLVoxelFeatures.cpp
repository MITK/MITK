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
#ifndef mitkCLVoxeFeatures_cpp
#define mitkCLVoxeFeatures_cpp

#include "time.h"
#include <sstream>
#include <fstream>

#include <mitkIOUtil.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include "mitkCommandLineParser.h"

#include <mitkGIFCooccurenceMatrix.h>
#include <mitkGIFGrayLevelRunLength.h>

#include "itkDiscreteGaussianImageFilter.h"
#include <itkLaplacianRecursiveGaussianImageFilter.h>
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"
#include <itkMultiHistogramFilter.h>
#include "vnl/algo/vnl_symmetric_eigensystem.h"
#include <itkSubtractImageFilter.h>

static std::vector<double> splitDouble(std::string str, char delimiter) {
  std::vector<double> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  double val;
  while (std::getline(ss, tok, delimiter)) {
    std::stringstream s2(tok);
    s2 >> val;
    internal.push_back(val);
  }

  return internal;
}

namespace Functor
{
  template <class TInput, class TOutput>
  class MatrixFirstEigenvalue
  {
  public:
    MatrixFirstEigenvalue() {}
    virtual ~MatrixFirstEigenvalue() {}

    int order;

    inline TOutput operator ()(const TInput& input)
    {
      double a,b,c;
      if (input[0] < 0.01 && input[1] < 0.01 &&input[2] < 0.01 &&input[3] < 0.01 &&input[4] < 0.01 &&input[5] < 0.01)
        return 0;
      vnl_symmetric_eigensystem_compute_eigenvals(input[0], input[1],input[2],input[3],input[4],input[5],a,b,c);
      switch (order)
      {
      case 0: return a;
      case 1: return b;
      case 2: return c;
      default: return a;
      }
    }
    bool operator !=(const MatrixFirstEigenvalue) const
    {
      return false;
    }
    bool operator ==(const MatrixFirstEigenvalue& other) const
    {
      return !(*this != other);
    }
  };
}

template<typename TPixel, unsigned int VImageDimension>
void
  GaussianFilter(itk::Image<TPixel, VImageDimension>* itkImage, double variance, mitk::Image::Pointer &output)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::DiscreteGaussianImageFilter< ImageType, ImageType >  GaussFilterType;

  typename GaussFilterType::Pointer gaussianFilter = GaussFilterType::New();
  gaussianFilter->SetInput( itkImage );
  gaussianFilter->SetVariance(variance);
  gaussianFilter->Update();
  mitk::CastToMitkImage(gaussianFilter->GetOutput(), output);
}

template<typename TPixel, unsigned int VImageDimension>
void
  DifferenceOfGaussFilter(itk::Image<TPixel, VImageDimension>* itkImage, double variance, mitk::Image::Pointer &output)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::DiscreteGaussianImageFilter< ImageType, ImageType >  GaussFilterType;
  typedef itk::SubtractImageFilter<ImageType, ImageType, ImageType> SubFilterType;

  typename GaussFilterType::Pointer gaussianFilter1 = GaussFilterType::New();
  gaussianFilter1->SetInput( itkImage );
  gaussianFilter1->SetVariance(variance);
  gaussianFilter1->Update();
  typename GaussFilterType::Pointer gaussianFilter2 = GaussFilterType::New();
  gaussianFilter2->SetInput( itkImage );
  gaussianFilter2->SetVariance(variance*0.66*0.66);
  gaussianFilter2->Update();
  typename SubFilterType::Pointer subFilter = SubFilterType::New();
  subFilter->SetInput1(gaussianFilter1->GetOutput());
  subFilter->SetInput2(gaussianFilter2->GetOutput());
  subFilter->Update();
  mitk::CastToMitkImage(subFilter->GetOutput(), output);
}

template<typename TPixel, unsigned int VImageDimension>
void
  LaplacianOfGaussianFilter(itk::Image<TPixel, VImageDimension>* itkImage, double variance, mitk::Image::Pointer &output)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::DiscreteGaussianImageFilter< ImageType, ImageType >  GaussFilterType;
  typedef itk::LaplacianRecursiveGaussianImageFilter<ImageType, ImageType>    LaplacianFilter;

  typename GaussFilterType::Pointer gaussianFilter = GaussFilterType::New();
  gaussianFilter->SetInput( itkImage );
  gaussianFilter->SetVariance(variance);
  gaussianFilter->Update();
  typename LaplacianFilter::Pointer laplaceFilter = LaplacianFilter::New();
  laplaceFilter->SetInput(gaussianFilter->GetOutput());
  laplaceFilter->Update();
  mitk::CastToMitkImage(laplaceFilter->GetOutput(), output);
}

template<typename TPixel, unsigned int VImageDimension>
void
  HessianOfGaussianFilter(itk::Image<TPixel, VImageDimension>* itkImage, double variance, std::vector<mitk::Image::Pointer> &out)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<double, VImageDimension> FloatImageType;
  typedef itk::HessianRecursiveGaussianImageFilter <ImageType> HessianFilterType;
  typedef typename HessianFilterType::OutputImageType VectorImageType;
  typedef Functor::MatrixFirstEigenvalue<typename VectorImageType::PixelType, double> DeterminantFunctorType;
  typedef itk::UnaryFunctorImageFilter<VectorImageType, FloatImageType, DeterminantFunctorType> DetFilterType;

  typename HessianFilterType::Pointer hessianFilter = HessianFilterType::New();
  hessianFilter->SetInput(itkImage);
  hessianFilter->SetSigma(std::sqrt(variance));
  for (int i = 0; i < VImageDimension; ++i)
  {
    typename DetFilterType::Pointer detFilter = DetFilterType::New();
    detFilter->SetInput(hessianFilter->GetOutput());
    detFilter->GetFunctor().order = i;
    detFilter->Update();
    mitk::CastToMitkImage(detFilter->GetOutput(), out[i]);
  }
}

template<typename TPixel, unsigned int VImageDimension>
void
  LocalHistograms(itk::Image<TPixel, VImageDimension>* itkImage, std::vector<mitk::Image::Pointer> &out, double offset, double delta)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<double, VImageDimension> FloatImageType;
  typedef itk::MultiHistogramFilter <ImageType,ImageType> MultiHistogramType;

  typename MultiHistogramType::Pointer filter = MultiHistogramType::New();
  filter->SetInput(itkImage);
  filter->SetOffset(offset);
  filter->SetDelta(delta);
  filter->Update();
  for (int i = 0; i < VImageDimension; ++i)
  {
    mitk::Image::Pointer img = mitk::Image::New();
    mitk::CastToMitkImage(filter->GetOutput(), img);
    out.push_back(img);
  }
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to the input VTK polydata", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output text file", "Target file. The output statistic is appended to this file.", us::Any(), false);

  parser.addArgument("gaussian","g",mitkCommandLineParser::String, "Gaussian Filtering of the input images", "Gaussian Filter. Followed by the used variances seperated by ';' ",us::Any());
  parser.addArgument("difference-of-gaussian","dog",mitkCommandLineParser::String, "Difference of Gaussian Filtering of the input images", "Difference of Gaussian Filter. Followed by the used variances seperated by ';' ",us::Any());
  parser.addArgument("laplace-of-gauss","log",mitkCommandLineParser::String, "Laplacian of Gaussian Filtering", "Laplacian of Gaussian Filter. Followed by the used variances seperated by ';' ",us::Any());
  parser.addArgument("hessian-of-gauss","hog",mitkCommandLineParser::String, "Hessian of Gaussian Filtering", "Hessian of Gaussian Filter. Followed by the used variances seperated by ';' ",us::Any());
  parser.addArgument("local-histogram", "lh", mitkCommandLineParser::String, "Local Histograms", "Calculate the local histogram based feature. Specify Offset and Delta, for exampel -3;0.6 ", us::Any());
  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Global Image Feature calculator");
  parser.setDescription("Calculates different global statistics for a given segmentation / image combination");
  parser.setContributor("MBI");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }
  bool useCooc = parsedArgs.count("cooccurence");

  mitk::Image::Pointer image = mitk::IOUtil::LoadImage(parsedArgs["image"].ToString());
  std::string filename=parsedArgs["output"].ToString();

  ////////////////////////////////////////////////////////////////
  // CAlculate Gaussian Features
  ////////////////////////////////////////////////////////////////
  MITK_INFO << "Check for Local Histogram...";
  if (parsedArgs.count("local-histogram"))
  {
    std::vector<mitk::Image::Pointer> outs;
    auto ranges = splitDouble(parsedArgs["local-histogram"].ToString(), ';');
    if (ranges.size() < 2)
    {
      MITK_INFO << "Missing Delta and Offset for Local Histogram";
    }
    else
    {
      AccessByItk_3(image, LocalHistograms, outs, ranges[0], ranges[1]);
      for (int i = 0; i < outs.size(); ++i)
      {
        std::string name = filename + "-lh" + us::any_value_to_string<int>(i)+".nii.gz";
        mitk::IOUtil::SaveImage(outs[i], name);
      }
    }
  }

  ////////////////////////////////////////////////////////////////
  // CAlculate Gaussian Features
  ////////////////////////////////////////////////////////////////
  MITK_INFO << "Check for Gaussian...";
  if (parsedArgs.count("gaussian"))
  {
    MITK_INFO << "Calculate Gaussian... " << parsedArgs["gaussian"].ToString();
    auto ranges = splitDouble(parsedArgs["gaussian"].ToString(),';');

    for (int i = 0; i < ranges.size(); ++i)
    {
      mitk::Image::Pointer output;
      AccessByItk_2(image, GaussianFilter, ranges[i], output);
      std::string name = filename + "-gaussian-" + us::any_value_to_string(ranges[i])+".nii.gz";
      mitk::IOUtil::SaveImage(output, name);
    }
  }

  ////////////////////////////////////////////////////////////////
  // CAlculate Difference of Gaussian Features
  ////////////////////////////////////////////////////////////////
  MITK_INFO << "Check for DoG...";
  if (parsedArgs.count("difference-of-gaussian"))
  {
    MITK_INFO << "Calculate Difference of Gaussian... " << parsedArgs["difference-of-gaussian"].ToString();
    auto ranges = splitDouble(parsedArgs["difference-of-gaussian"].ToString(),';');

    for (int i = 0; i < ranges.size(); ++i)
    {
      mitk::Image::Pointer output;
      AccessByItk_2(image, DifferenceOfGaussFilter, ranges[i], output);
      std::string name = filename + "-dog-" + us::any_value_to_string(ranges[i])+".nii.gz";
      mitk::IOUtil::SaveImage(output, name);
    }
  }

  MITK_INFO << "Check for LoG...";
  ////////////////////////////////////////////////////////////////
  // CAlculate Laplacian Of Gauss Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("laplace-of-gauss"))
  {
    MITK_INFO << "Calculate LoG... " << parsedArgs["laplace-of-gauss"].ToString();
    auto ranges = splitDouble(parsedArgs["laplace-of-gauss"].ToString(),';');

    for (int i = 0; i < ranges.size(); ++i)
    {
      mitk::Image::Pointer output;
      AccessByItk_2(image, LaplacianOfGaussianFilter, ranges[i], output);
      std::string name = filename + "-log-" + us::any_value_to_string(ranges[i])+".nii.gz";
      mitk::IOUtil::SaveImage(output, name);
    }
  }

  MITK_INFO << "Check for HoG...";
  ////////////////////////////////////////////////////////////////
  // CAlculate Hessian Of Gauss Features
  ////////////////////////////////////////////////////////////////
  if (parsedArgs.count("hessian-of-gauss"))
  {
    MITK_INFO << "Calculate HoG... " << parsedArgs["hessian-of-gauss"].ToString();
    auto ranges = splitDouble(parsedArgs["hessian-of-gauss"].ToString(),';');

    for (int i = 0; i < ranges.size(); ++i)
    {
      std::vector<mitk::Image::Pointer> outs;
      outs.push_back(mitk::Image::New());
      outs.push_back(mitk::Image::New());
      outs.push_back(mitk::Image::New());
      AccessByItk_2(image, HessianOfGaussianFilter, ranges[i], outs);
      std::string name = filename + "-hog0-" + us::any_value_to_string(ranges[i])+".nii.gz";
      mitk::IOUtil::SaveImage(outs[0], name);
      name = filename + "-hog1-" + us::any_value_to_string(ranges[i])+".nii.gz";
      mitk::IOUtil::SaveImage(outs[1], name);
      name = filename + "-hog2-" + us::any_value_to_string(ranges[i])+".nii.gz";
      mitk::IOUtil::SaveImage(outs[2], name);
    }
  }

  return 0;
}

#endif