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

#include <mitkBaseDataIOFactory.h>
#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <mitkIOUtil.h>
#include <boost/lexical_cast.hpp>
#include <itkShCoefficientImageExporter.h>
#include <itkFlipImageFilter.h>

#define _USE_MATH_DEFINES
#include <math.h>

template<int shOrder>
int StartShConversion(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Export SH Image");
    parser.setCategory("Preprocessing Tools");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "MITK SH image", us::Any(), false);
    parser.addArgument("output", "o", mitkCommandLineParser::InputFile, "Output", "MRtrix SH image", us::Any(), false);
    parser.addArgument("shOrder", "sh", mitkCommandLineParser::Int, "SH order:", "spherical harmonics order");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    string inFile = us::any_cast<string>(parsedArgs["input"]);
    string outFile = us::any_cast<string>(parsedArgs["output"]);

    try
    {

        typedef itk::Image< float, 4 > OutImageType;
        typedef itk::Image< itk::Vector< float, (shOrder*shOrder + shOrder + 2)/2 + shOrder >, 3 > InputImageType;

        typename InputImageType::Pointer itkInImage = InputImageType::New();
        typedef itk::ImageFileReader< InputImageType > ReaderType;
        typename ReaderType::Pointer reader = ReaderType::New();
        std::cout << "reading " << inFile;
        reader->SetFileName(inFile.c_str());
        reader->Update();
        itkInImage = reader->GetOutput();

        // extract directions from fiber bundle
        typename itk::ShCoefficientImageExporter<float, shOrder>::Pointer filter = itk::ShCoefficientImageExporter<float,shOrder>::New();
        filter->SetInputImage(itkInImage);
        filter->GenerateData();
        OutImageType::Pointer outImage = filter->GetOutputImage();

        typedef itk::ImageFileWriter< OutImageType > WriterType;
        WriterType::Pointer writer = WriterType::New();
        writer->SetFileName(outFile.c_str());
        writer->SetInput(outImage);
        writer->Update();
    }
    catch (itk::ExceptionObject e)
    {
        std::cout << e;
        return EXIT_FAILURE;
    }
    catch (std::exception e)
    {
        std::cout << e.what();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "ERROR!?!";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{

    mitkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input image", "MITK SH image", us::Any(), false);
    parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output image", "MRtrix SH image", us::Any(), false);
    parser.addArgument("shOrder", "sh", mitkCommandLineParser::Int, "Spherical harmonics order", "spherical harmonics order");

    parser.setCategory("Preprocessing Tools");
    parser.setTitle("Export SH Image");
    parser.setDescription("");
    parser.setContributor("MBI");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    int shOrder = -1;
    if (parsedArgs.count("shOrder"))
        shOrder = us::any_cast<int>(parsedArgs["shOrder"]);

    switch (shOrder)
    {
    case 4:
        return StartShConversion<4>(argc, argv);
    case 6:
        return StartShConversion<6>(argc, argv);
    case 8:
        return StartShConversion<8>(argc, argv);
    case 10:
        return StartShConversion<10>(argc, argv);
    case 12:
        return StartShConversion<12>(argc, argv);
    }
    return EXIT_FAILURE;
}
