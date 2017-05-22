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

#include "mitkCommandLineParser.h"
#include "mitkImage.h"
#include "mitkImageStatisticsCalculator.h"
#include "mitkIOUtil.h"
#include <iostream>
#include <usAny.h>
#include <fstream>
#include <itkImageRegionConstIterator.h>
#include "mitkImageAccessByItk.h"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <mitkImageMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>

struct statistics_res{
    double mean, variance, min, max, count, moment;
};

void printstats(statistics_res s)
{
    std::cout << "mean: " << s.mean << std::endl
              << "variance: " << s.variance << std::endl
              << "min: " << s.min << std::endl
              << "max: " << s.max << std::endl
              << "count: " << s.count << std::endl
              << "moment: " << s.moment << std::endl;
}

template < typename TPixel, unsigned int VImageDimension >
void get_statistics_boost(itk::Image<TPixel, VImageDimension>* itkImage, statistics_res& res){
    typedef itk::Image<TPixel, VImageDimension> ImageType;

    itk::ImageRegionConstIterator<ImageType> it(itkImage, itkImage->GetLargestPossibleRegion());

    TPixel currentPixel;
    int ctr=0;
    double sum=0;

    boost::accumulators::accumulator_set<double, boost::accumulators::stats<
            boost::accumulators::tag::mean,
            boost::accumulators::tag::variance,
            boost::accumulators::tag::min,
            boost::accumulators::tag::max,
            boost::accumulators::tag::count,
            boost::accumulators::tag::moment<2>> > acc;

    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
        acc(it.Get());
//        currentPixel = it.Get();
//        sum+=currentPixel;
//        ctr+=1;
    }

//    res.mean=(double)sum/(double)ctr;
    res.mean = boost::accumulators::mean(acc);
    res.variance = boost::accumulators::variance(acc);
    res.min = boost::accumulators::min(acc);
    res.max = boost::accumulators::max(acc);
    res.count = boost::accumulators::count(acc);
    res.moment = boost::accumulators::moment<2>(acc);

    std::cout << "sum: " << sum << " N: " <<  ctr << " mean: " << res.mean << std::endl;
}

int main( int argc, char* argv[] )
{
    mitkCommandLineParser parser;

    parser.setTitle("Extract Image Statistics");
    parser.setCategory("Preprocessing Tools");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("help", "h", mitkCommandLineParser::String, "Help:", "Show this help text");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input image", us::Any(),false);
    parser.addArgument("mask", "m", mitkCommandLineParser::InputFile, "Mask:", "mask image / roi image denotin area on which statistics are calculated", us::Any(),true);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output", "output file (default: filenameOfRoi.nrrd_statistics.txt)", us::Any());

    std::cout << "test...." << std::endl;

    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    std::cout << "parsedArgs.size()= " << parsedArgs.size() << std::endl;
    if (parsedArgs.size()==0 || parsedArgs.count("help") || parsedArgs.count("h"))
    {
      std::cout << "\n\n MiniApp Description: \nCalculates statistics on the supplied image using given mask." << endl;
      std::cout << "Output is written to the designated output file in this order:" << endl;
      std::cout << "Mean, Standard Deviation, RMS, Max, Min, Number of Voxels, Volume [mm3]" << endl;
      std::cout << "\n\n Parameters:"<< endl;
      std::cout << parser.helpText();
      return EXIT_SUCCESS;
    }


    // Parameters:
    bool ignoreZeroValues = false;
    unsigned int timeStep = 0;

    std::string inputImageFile = us::any_cast<std::string>(parsedArgs["input"]);
    mitk::Image::Pointer maskImage;
    if (parsedArgs.count("mask") || parsedArgs.count("m"))
    {
        std::string maskImageFile = us::any_cast<std::string>(parsedArgs["mask"]);
        maskImage = mitk::IOUtil::LoadImage(maskImageFile);
    }

    std::string outFile;
    if (parsedArgs.count("out") || parsedArgs.count("o") )
      outFile = us::any_cast<std::string>(parsedArgs["out"]);
    else
      outFile = inputImageFile + "_statistics.txt";

    // Load image and mask
    mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage(inputImageFile);

    // Calculate statistics
    mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer statisticsStruct;
    mitk::ImageStatisticsCalculator::Pointer calculator = mitk::ImageStatisticsCalculator::New();
    try
    {
      calculator->SetInputImage(inputImage);
      if (parsedArgs.count("mask") || parsedArgs.count("m"))
      {
          mitk::ImageMaskGenerator::Pointer imgMask = mitk::ImageMaskGenerator::New();
          imgMask->SetImageMask(maskImage);
          imgMask->SetTimeStep(timeStep);
          calculator->SetMask(imgMask.GetPointer());
      }
      else
      {
          calculator->SetMask(nullptr);
      }

    }
    catch( const itk::ExceptionObject& e)
    {
      MITK_ERROR << "Statistic Calculation Failed - ITK Exception:" << e.what();
      return -1;
    }


    if (ignoreZeroValues)
    {
        // TODO, cannot have more than one mask, using ignore pixel value will override the image mask :-c
        mitk::IgnorePixelMaskGenerator::Pointer ignorePixelMask = mitk::IgnorePixelMaskGenerator::New();
        ignorePixelMask->SetInputImage(inputImage);
        ignorePixelMask->SetTimeStep(timeStep);
        ignorePixelMask->SetIgnoredPixelValue(0);
        calculator->SetMask(ignorePixelMask.GetPointer());
    }

    std::cout << "calculating statistics itk: " << std::endl;
    try
    {
        statisticsStruct = calculator->GetStatistics(timeStep);
    }
    catch ( mitk::Exception& e)
    {
      MITK_ERROR<< "MITK Exception: " << e.what();
      return -1;
    }


    // Calculate Volume
    double volume = 0;
    const mitk::BaseGeometry *geometry = inputImage->GetGeometry();
    if ( geometry != NULL )
    {
      const mitk::Vector3D &spacing = inputImage->GetGeometry()->GetSpacing();
      volume = spacing[0] * spacing[1] * spacing[2] * (double) statisticsStruct->GetN();
    }

    // Write Results to file
    std::ofstream output;
    output.open(outFile.c_str());
    output << statisticsStruct->GetMean() << " , ";
    output << statisticsStruct->GetStd() << " , ";
    output << statisticsStruct->GetRMS() << " , ";
    output << statisticsStruct->GetMax() << " , ";
    output << statisticsStruct->GetMin() << " , ";
    output << statisticsStruct->GetN() << " , ";
    output << volume << "\n";

    output.flush();
    output.close();

    std::cout << "calculating statistics boost: " << std::endl;

    statistics_res res;
    AccessByItk_n(inputImage, get_statistics_boost, (res));

    printstats(res);

    return EXIT_SUCCESS;
}
