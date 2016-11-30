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
#include <mitkImageStatisticsCalculator.h>
#include <mitkExtendedLabelStatisticsImageFilter.h>
#include <mitkPlanarFigureMaskGenerator.h>
#include <mitkPlanarFigure.h>
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
#include <mitkHotspotMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>
#include <chrono>


struct statistics_res{
    double mean, variance, min, max, count, moment;
};

long getTimeInMs()
{
    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds > (std::chrono::system_clock::now().time_since_epoch());
    long time = ms.count();
    return time;
}

//std::string printstats(mitk::ImageStatisticsCalculator::Statistics statisticsStruct)
//{
//   std::string res = "";
//   res += std::string("Entropy ") + std::to_string(statisticsStruct.GetEntropy()) + std::string("\n");
//   res += std::string("Kurtosis ") + std::to_string(statisticsStruct.GetKurtosis()) + std::string("\n");
//   res += std::string("MPP ") + std::to_string(statisticsStruct.GetMPP()) + std::string("\n");
//   res += std::string("Max ") + std::to_string(statisticsStruct.GetMax()) + std::string("\n");
//   res += std::string("Mean ") + std::to_string(statisticsStruct.GetMean()) + std::string("\n");
//   res += std::string("Median ") + std::to_string(statisticsStruct.GetMedian()) + std::string("\n");
//   res += std::string("Min ") + std::to_string(statisticsStruct.GetMin()) + std::string("\n");
//   res += std::string("N ") + std::to_string(statisticsStruct.GetN()) + std::string("\n");
//   res += std::string("RMS ") + std::to_string(statisticsStruct.GetRMS()) + std::string("\n");
//   res += std::string("Skewness ") + std::to_string(statisticsStruct.GetSkewness()) + std::string("\n");
//   res += std::string("Std ") + std::to_string(statisticsStruct.GetSigma()) + std::string("\n");
//   res += std::string("UPP ") + std::to_string(statisticsStruct.GetUPP()) + std::string("\n");
//   res += std::string("Uniformity ") + std::to_string(statisticsStruct.GetUniformity()) + std::string("\n");
//   res += std::string("Variance ") + std::to_string(statisticsStruct.GetVariance()) + std::string("\n");
//   vnl_vector<int> minIndex = statisticsStruct.GetMinIndex();
//   vnl_vector<int> maxIndex = statisticsStruct.GetMaxIndex();

//   res += "Min Index:  ";
//   for (auto it = minIndex.begin(); it != minIndex.end(); it++)
//   {
//       res += std::to_string(*it) + " ";
//   }
//   res += std::string("\n");

//   res += "Max Index:  ";
//   for (auto it = maxIndex.begin(); it != maxIndex.end(); it++)
//   {
//       res += std::to_string(*it) + " ";
//   }
//   res += std::string("\n");
//   return res;
//}

template <class T, class P>
void printMap(std::map<T, P> input)
{
    for (auto it = input.begin(); it != input.end(); ++it)
    {
        std::cout << it->first<< ": " << it->second<< std::endl;
    }
    std::cout << std::endl;
}

template < typename TPixel, unsigned int VImageDimension >
void get_statistics_boost(itk::Image<TPixel, VImageDimension>* itkImage, statistics_res& res){
    typedef itk::Image<TPixel, VImageDimension> ImageType;

    itk::ImageRegionConstIterator<ImageType> it(itkImage, itkImage->GetLargestPossibleRegion());

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

void compute_statistics(std::string inputImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D.nrrd",
                        std::string outfname = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_statistics_new.txt",
                        std::string outfname2 = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_statistics_old.txt",
                        std::string maskImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_someSegmentation.nrrd",
                        std::string pFfile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_rectangle.pf"                        )
{
    ofstream outFile;
    outFile.open(outfname);

    unsigned int timeStep = 0;

    mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage(inputImageFile);

    mitk::Image::Pointer maskImage;
    maskImage = mitk::IOUtil::LoadImage(maskImageFile);

    std::vector<mitk::BaseData::Pointer> loadedObjects;
    loadedObjects = mitk::IOUtil::Load(pFfile);
    mitk::BaseData::Pointer pf = loadedObjects[0];
    mitk::PlanarFigure::Pointer planarFigure = dynamic_cast<mitk::PlanarFigure*>(pf.GetPointer());

    mitk::ImageStatisticsCalculator::Pointer calculator = mitk::ImageStatisticsCalculator::New();
    mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result;
    calculator->SetInputImage(inputImage);
    calculator->SetNBinsForHistogramStatistics(100);

    long start_time;

    outFile << "Calculating Statistics on Pic3D" << std::endl << std::endl;

    outFile << "New Image Statistics Calculator" << std::endl;

    start_time = getTimeInMs();
    outFile << "1) unmasked: " << std::endl;
    calculator->SetMask(nullptr);
    result = calculator->GetStatistics(timeStep);
    outFile << result->GetAsString() << std::endl;
    outFile << "new image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;

    start_time = getTimeInMs();
    outFile << "2) planarfigure: " << std::endl;
    mitk::PlanarFigureMaskGenerator::Pointer planarFigMaskExtr = mitk::PlanarFigureMaskGenerator::New();
    planarFigMaskExtr->SetPlanarFigure(planarFigure);
    planarFigMaskExtr->SetInputImage(inputImage);
    calculator->SetMask(planarFigMaskExtr.GetPointer());
    result = calculator->GetStatistics(timeStep, 1);
    outFile << result->GetAsString() << std::endl;
    outFile << "new image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;

    start_time = getTimeInMs();
    outFile << "3) ignore pixel value mask: " << std::endl;
    mitk::IgnorePixelMaskGenerator::Pointer ignPixVal = mitk::IgnorePixelMaskGenerator::New();
    ignPixVal->SetInputImage(inputImage);
    ignPixVal->SetIgnoredPixelValue(0);
    calculator->SetMask(ignPixVal.GetPointer());
    result = calculator->GetStatistics(timeStep, 1);
    outFile << result->GetAsString() << std::endl;
    outFile << "new image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;

    start_time = getTimeInMs();
    outFile << "4) image mask: " << std::endl;
    mitk::ImageMaskGenerator::Pointer binaryImageMaskGen = mitk::ImageMaskGenerator::New();
    binaryImageMaskGen->SetImageMask(maskImage);
    calculator->SetMask(binaryImageMaskGen.GetPointer());
    result = calculator->GetStatistics(timeStep, 1);
    outFile << result->GetAsString() << std::endl;
    outFile << "new image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;

    start_time = getTimeInMs();
    outFile << "5) hotspot mask: " << std::endl;
    mitk::HotspotMaskGenerator::Pointer hotSpotMaskGen = mitk::HotspotMaskGenerator::New();
    hotSpotMaskGen->SetInputImage(inputImage);
    hotSpotMaskGen->SetHotspotRadiusInMM(10);
    hotSpotMaskGen->SetTimeStep(0);
    calculator->SetMask(hotSpotMaskGen.GetPointer());
    result = calculator->GetStatistics(timeStep, 1);
    outFile << result->GetAsString() << std::endl;
    outFile << "new image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;
    //mitk::IOUtil::SaveImage(hotSpotMaskGen->GetMask(), "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic2DplusTHotspot.nrrd");

    outFile << "6) all time steps (image mask): " << std::endl;
    if (inputImage->GetTimeSteps() > 1)
    {
        start_time = getTimeInMs();
        for (unsigned int i=0; i < inputImage->GetTimeSteps(); i++)
        {
            outFile << "timestep: " << i << std::endl;
            calculator->SetMask(binaryImageMaskGen.GetPointer());
            result = calculator->GetStatistics(i, 1);
            outFile << result->GetAsString() << std::endl;
            outFile << "new image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;
        }
    }
    else
    {
        outFile << "Input image has only 1 time step!" << std::endl;
    }

    outFile.flush();
    outFile.close();

    // -----------------------------------------------------------------------------------------------------------------------------------------------------------
//    outFile.open(outfname2);

//    std::cout << std::endl << std::endl << "calculating statistics old imgstatcalc: " << std::endl;
//    mitk::ImageStatisticsCalculator::Statistics statisticsStruct;
//    mitk::ImageStatisticsCalculator::Pointer calculator_old;



//    start_time = getTimeInMs();
//    outFile << "1) unmasked: " << std::endl;
//    calculator_old = mitk::ImageStatisticsCalculator::New();
//    calculator_old->SetHistogramBinSize(100);
//    calculator_old->SetImage(inputImage);
//    calculator_old->SetMaskingModeToNone();
//    calculator_old->ComputeStatistics(timeStep);
//    statisticsStruct = calculator_old->GetStatistics(timeStep);
//    outFile << printstats(statisticsStruct) << std::endl;
//    outFile << "old image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;

//    start_time = getTimeInMs();
//    outFile << "2) planarFigure: " << std::endl;
//    calculator_old = mitk::ImageStatisticsCalculator::New();
//    calculator_old->SetHistogramBinSize(100);
//    calculator_old->SetImage(inputImage);
//    calculator_old->SetPlanarFigure(planarFigure);
//    calculator_old->SetMaskingModeToPlanarFigure();
//    calculator_old->ComputeStatistics(timeStep);
//    statisticsStruct = calculator_old->GetStatistics(timeStep);
//    outFile << printstats(statisticsStruct) << std::endl;
//    outFile << "old image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;

//    start_time = getTimeInMs();
//    outFile << "3) IgnorePixelValue: " << std::endl;
//    calculator_old = mitk::ImageStatisticsCalculator::New();
//    calculator_old->SetHistogramBinSize(100);
//    calculator_old->SetImage(inputImage);
//    calculator_old->SetMaskingModeToNone();
//    calculator_old->SetDoIgnorePixelValue(true);
//    calculator_old->SetIgnorePixelValue(0);
//    calculator_old->ComputeStatistics(timeStep);
//    statisticsStruct = calculator_old->GetStatistics(timeStep);
//    outFile << printstats(statisticsStruct) << std::endl;
//    outFile << "old image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;
//    calculator_old->SetDoIgnorePixelValue(false);

//    start_time = getTimeInMs();
//    outFile << "4) Image Mask: " << std::endl;
//    calculator_old = mitk::ImageStatisticsCalculator::New();
//    calculator_old->SetHistogramBinSize(100);
//    calculator_old->SetImage(inputImage);
//    calculator_old->SetImageMask(maskImage);
//    calculator_old->SetMaskingModeToImage();
//    calculator_old->ComputeStatistics(timeStep);
//    statisticsStruct = calculator_old->GetStatistics(timeStep);
//    outFile << printstats(statisticsStruct) << std::endl;
//    outFile << "old image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;

//    start_time = getTimeInMs();
//    outFile << "5) Hotspot Mask: " << std::endl;
//    calculator_old = mitk::ImageStatisticsCalculator::New();
//    calculator_old->SetHistogramBinSize(100);
//    calculator_old->SetImage(inputImage);
//    calculator_old->SetMaskingModeToNone();
//    calculator_old->SetCalculateHotspot(true);
//    calculator_old->SetHotspotRadiusInMM(10);
//    calculator_old->ComputeStatistics(timeStep);
//    statisticsStruct = calculator_old->GetStatistics(timeStep).GetHotspotStatistics();
//    outFile << printstats(statisticsStruct) << std::endl;
//    outFile << "old image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;
//    calculator_old->SetCalculateHotspot(false);

//    outFile << "6) all time steps (image mask): " << std::endl;
//    if (inputImage->GetTimeSteps() > 1)
//    {
//        start_time = getTimeInMs();
//        calculator_old = mitk::ImageStatisticsCalculator::New();
//        calculator_old->SetHistogramBinSize(100);
//        calculator_old->SetImage(inputImage);
//        calculator_old->SetImageMask(maskImage);
//        calculator_old->SetMaskingModeToImage();
//        for (unsigned int i=0; i < inputImage->GetTimeSteps(); i++)
//        {
//            calculator_old->ComputeStatistics(i);
//            statisticsStruct = calculator_old->GetStatistics(i);
//            outFile << printstats(statisticsStruct) << std::endl;
//            outFile << "old image statistics calculator took: " << getTimeInMs()-start_time << " ms." << std::endl << std::endl;
//        }
//     }
//    else
//    {
//        outFile << "Input image has only 1 time step!" << std::endl;
//    }

//    outFile.flush();
//    outFile.close();
}

int main( int argc, char* argv[] )
{
    std::string inputImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D.nrrd";
    std::string outfname = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_statistics_new.txt";
    std::string outfname2 = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_statistics_old.txt";
    std::string maskImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_someSegmentation.nrrd";
    std::string pFfile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_rectangle.pf";
    compute_statistics(inputImageFile, outfname, outfname2, maskImageFile, pFfile);

    inputImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic2DplusT.nrrd";
    outfname = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic2DplusT_statistics_new.txt";
    outfname2 = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic2DplusT_statistics_old.txt";
    maskImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic2DplusT_someSegmentation.nrrd";
    pFfile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic2DplusT_ellipse.pf";
    compute_statistics(inputImageFile, outfname, outfname2, maskImageFile, pFfile);

    inputImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/leber_ct.pic";
    outfname = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/leber_ct_statistics_new.txt";
    outfname2 = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/leber_ct_statistics_old.txt";
    maskImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/leber_ct_segmentation.nrrd";
    pFfile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/leber_ct_PF.pf";
    compute_statistics(inputImageFile, outfname, outfname2, maskImageFile, pFfile);


    return EXIT_SUCCESS;
}
