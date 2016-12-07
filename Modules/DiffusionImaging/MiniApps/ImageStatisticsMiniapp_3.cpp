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
#include <mitkImageGenerator.h>


int main( int argc, char* argv[] )
{
    unsigned int timeStep = 0;
    std::string inputImageFile, maskImageFile;
    inputImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D.nrrd";
    maskImageFile = "/home/fabian/MITK/MITK_platform_project/bin/MITK-superbuild/MITK-Data/Pic3D_someSegmentation.nrrd";
    // Load image
    mitk::Image::Pointer inputImage = mitk::IOUtil::LoadImage(inputImageFile);
    mitk::Image::Pointer maskImage = mitk::IOUtil::LoadImage(maskImageFile);


    // Calculate statistics
    mitk::ImageStatisticsCalculator::Pointer calculator = mitk::ImageStatisticsCalculator::New();

    std::cout << "calculating statistics (unmasked) itk: " << std::endl;
    mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer result;

    mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
    imgMaskGen->SetImageMask(maskImage);

    calculator->SetMask(imgMaskGen.GetPointer());
    calculator->SetInputImage(inputImage);
    calculator->SetNBinsForHistogramStatistics(100);

    for (unsigned int i=0; i < inputImage->GetTimeSteps(); i++)
    {
        std::cout << "Results for time step " << i << ":" << std::endl;
        result = calculator->GetStatistics(i, 1);
        result->Print();
        std::cout << std::endl;
    }
    MITK_INFO << "-------------";
    mitk::Image::Pointer test = imgMaskGen->GetMask();
    MITK_INFO << "-------------";

    mitk::Image::Pointer test2 = imgMaskGen->GetMask();
    MITK_INFO << "-------------";
    imgMaskGen->SetTimeStep(2);
    mitk::Image::Pointer test3 = imgMaskGen->GetMask();


    return EXIT_SUCCESS;
}
