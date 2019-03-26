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

#include "itkDiscreteGaussianImageFilter.h"
#include <itkLaplacianRecursiveGaussianImageFilter.h>
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "vnl/algo/vnl_symmetric_eigensystem.h"
#include <itkMultiHistogramFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkLocalStatisticFilter.h>

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
  for (unsigned int i = 0; i < VImageDimension; ++i)
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
LocalHistograms2(itk::Image<TPixel, VImageDimension>* itkImage, std::vector<mitk::Image::Pointer> &out, std::vector<double> params)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::MultiHistogramFilter <ImageType, ImageType> MultiHistogramType;

  double minimum = params[0];
  double maximum = params[1];
  int bins = std::round(params[2]);

  double offset = minimum;
  double delta = (maximum - minimum) / bins;

  typename MultiHistogramType::Pointer filter = MultiHistogramType::New();
  filter->SetInput(itkImage);
  filter->SetOffset(offset);
  filter->SetDelta(delta);
  filter->SetBins(bins);
  filter->Update();
  for (int i = 0; i < bins; ++i)
  {
    mitk::Image::Pointer img = mitk::Image::New();
    mitk::CastToMitkImage(filter->GetOutput(i), img);
    out.push_back(img);
  }
}

template<typename TPixel, unsigned int VImageDimension>
void
  LocalHistograms(itk::Image<TPixel, VImageDimension>* itkImage, std::vector<mitk::Image::Pointer> &out, double offset, double delta)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::MultiHistogramFilter <ImageType,ImageType> MultiHistogramType;

  typename MultiHistogramType::Pointer filter = MultiHistogramType::New();
  filter->SetInput(itkImage);
  filter->SetOffset(offset);
  filter->SetDelta(delta);
  filter->Update();
  for (int i = 0; i < 11; ++i)
  {
    mitk::Image::Pointer img = mitk::Image::New();
    mitk::CastToMitkImage(filter->GetOutput(i), img);
    out.push_back(img);
  }
}

template<typename TPixel, unsigned int VImageDimension>
void
localStatistic(itk::Image<TPixel, VImageDimension>* itkImage, std::vector<mitk::Image::Pointer> &out, int size)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::LocalStatisticFilter <ImageType, ImageType> MultiHistogramType;

  typename MultiHistogramType::Pointer filter = MultiHistogramType::New();
  filter->SetInput(itkImage);
  filter->SetSize(size);
  filter->Update();
  for (int i = 0; i < 5; ++i)
  {
    mitk::Image::Pointer img = mitk::Image::New();
    mitk::CastToMitkImage(filter->GetOutput(i), img);
    out.push_back(img);
  }
}


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::Image, "Input Image", "Path to the input VTK polydata", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output text file", "Target file. The output statistic is appended to this file.", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("extension", "e", mitkCommandLineParser::File, "Extension", "File extension. Default is .nii.gz", us::Any(), true, false, false, mitkCommandLineParser::Output);

  parser.addArgument("gaussian","g",mitkCommandLineParser::String, "Gaussian Filtering of the input images", "Gaussian Filter. Followed by the used variances seperated by ';' ",us::Any());
  parser.addArgument("difference-of-gaussian","dog",mitkCommandLineParser::String, "Difference of Gaussian Filtering of the input images", "Difference of Gaussian Filter. Followed by the used variances seperated by ';' ",us::Any());
  parser.addArgument("laplace-of-gauss","log",mitkCommandLineParser::String, "Laplacian of Gaussian Filtering", "Laplacian of Gaussian Filter. Followed by the used variances seperated by ';' ",us::Any());
  parser.addArgument("hessian-of-gauss","hog",mitkCommandLineParser::String, "Hessian of Gaussian Filtering", "Hessian of Gaussian Filter. Followed by the used variances seperated by ';' ",us::Any());
  parser.addArgument("local-histogram", "lh", mitkCommandLineParser::String, "Local Histograms", "Calculate the local histogram based feature. Specify Offset and Delta, for exampel -3;0.6 ", us::Any());
  parser.addArgument("local-histogram2", "lh2", mitkCommandLineParser::String, "Local Histograms", "Calculate the local histogram based feature. Specify Minimum;Maximum;Bins, for exampel -3;3;6 ", us::Any());
  parser.addArgument("local-statistic", "ls", mitkCommandLineParser::String, "Local Histograms", "Calculate the local histogram based feature. Specify Offset and Delta, for exampel -3;0.6 ", us::Any());
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

  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(parsedArgs["image"].ToString());
  std::string filename=parsedArgs["output"].ToString();

  std::string extension = ".nii.gz";
  if (parsedArgs.count("extension"))
  {
    extension = parsedArgs["extension"].ToString();
  }

  ////////////////////////////////////////////////////////////////
  // CAlculate Local Histogram
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
      for (std::size_t i = 0; i < outs.size(); ++i)
      {
        std::string name = filename + "-lh" + us::any_value_to_string<int>(i)+extension;
        mitk::IOUtil::Save(outs[i], name);
      }
    }
  }

  ////////////////////////////////////////////////////////////////
  // CAlculate Local Histogram 2
  ////////////////////////////////////////////////////////////////
  MITK_INFO << "Check for Local Histogram...";
  if (parsedArgs.count("local-histogram2"))
  {
    std::vector<mitk::Image::Pointer> outs;
    auto ranges = splitDouble(parsedArgs["local-histogram2"].ToString(), ';');
    if (ranges.size() < 3)
    {
      MITK_INFO << "Missing Delta and Offset for Local Histogram";
    }
    else
    {
      AccessByItk_2(image, LocalHistograms2, outs, ranges);
      for (std::size_t i = 0; i < outs.size(); ++i)
      {
        std::string name = filename + "-lh2" + us::any_value_to_string<int>(i)+extension;
        mitk::IOUtil::Save(outs[i], name);
      }
    }
  }

  ////////////////////////////////////////////////////////////////
  // CAlculate Local Statistic
  ////////////////////////////////////////////////////////////////
  MITK_INFO << "Check for Local Histogram...";
  if (parsedArgs.count("local-statistic"))
  {
    std::vector<mitk::Image::Pointer> outs;
    auto ranges = splitDouble(parsedArgs["local-statistic"].ToString(), ';');
    if (ranges.size() < 1)
    {
      MITK_INFO << "Missing Rage";
    }
    else
    {
      for (std::size_t j = 0; j < ranges.size(); ++j)
      {
        AccessByItk_2(image, localStatistic, outs, ranges[j]);
        for (std::size_t i = 0; i < outs.size(); ++i)
        {
          std::string name = filename + "-lstat" + us::any_value_to_string<int>(ranges[j])+ "_" +us::any_value_to_string<int>(i)+extension;
          mitk::IOUtil::Save(outs[i], name);
        }
        outs.clear();
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

    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
      MITK_INFO << "Gaussian with sigma: " << ranges[i];
      mitk::Image::Pointer output;
      AccessByItk_2(image, GaussianFilter, ranges[i], output);
      MITK_INFO << "Write output:";
      std::string name = filename + "-gaussian-" + us::any_value_to_string(ranges[i]) + extension;
      mitk::IOUtil::Save(output, name);
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

    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
      mitk::Image::Pointer output;
      AccessByItk_2(image, DifferenceOfGaussFilter, ranges[i], output);
      std::string name = filename + "-dog-" + us::any_value_to_string(ranges[i]) + extension;
      mitk::IOUtil::Save(output, name);
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

    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
      mitk::Image::Pointer output;
      AccessByItk_2(image, LaplacianOfGaussianFilter, ranges[i], output);
      std::string name = filename + "-log-" + us::any_value_to_string(ranges[i]) + extension;
      mitk::IOUtil::Save(output, name);
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

    for (std::size_t i = 0; i < ranges.size(); ++i)
    {
      std::vector<mitk::Image::Pointer> outs;
      outs.push_back(mitk::Image::New());
      outs.push_back(mitk::Image::New());
      outs.push_back(mitk::Image::New());
      AccessByItk_2(image, HessianOfGaussianFilter, ranges[i], outs);
      std::string name = filename + "-hog0-" + us::any_value_to_string(ranges[i]) + extension;
      mitk::IOUtil::Save(outs[0], name);
      name = filename + "-hog1-" + us::any_value_to_string(ranges[i]) + extension;
      mitk::IOUtil::Save(outs[1], name);
      name = filename + "-hog2-" + us::any_value_to_string(ranges[i]) + extension;
      mitk::IOUtil::Save(outs[2], name);
    }
  }

  return 0;
}

#endif
