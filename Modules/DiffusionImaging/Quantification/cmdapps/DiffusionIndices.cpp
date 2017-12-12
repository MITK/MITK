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

#include <mitkImageCast.h>
#include <itkExceptionObject.h>
#include <itkImageFileWriter.h>
#include <mitkOdfImage.h>
#include <itkTensorDerivedMeasurementsFilter.h>
#include <itkDiffusionOdfGeneralizedFaImageFilter.h>
#include <mitkTensorImage.h>
#include "mitkCommandLineParser.h"
#include <boost/algorithm/string.hpp>
#include <itksys/SystemTools.hxx>
#include <itkMultiThreader.h>
#include <mitkIOUtil.h>

/**
 * Calculate indices derived from Odf or tensor images
 */
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Diffusion Indices");
    parser.setCategory("Diffusion Related Measures");
    parser.setDescription("Computes requested diffusion related measures");
    parser.setContributor("MIC");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input image (tensor, ODF or FSL/MRTrix SH-coefficient image)", us::Any(), false);
    parser.addArgument("index", "idx", mitkCommandLineParser::String, "Index:", "index (fa, gfa, ra, ad, rd, ca, l2, l3, md)", us::Any(), false);
    parser.addArgument("outFile", "o", mitkCommandLineParser::OutputFile, "Output:", "output file", us::Any(), false);

    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    std::string inFileName = us::any_cast<std::string>(parsedArgs["input"]);
    std::string index = us::any_cast<std::string>(parsedArgs["index"]);
    std::string outFileName = us::any_cast<std::string>(parsedArgs["outFile"]);

    std::string ext = itksys::SystemTools::GetFilenameLastExtension(outFileName);
    if (ext.empty())
        outFileName += ".nrrd";

    try
    {
        // load input image
        std::vector<mitk::BaseData::Pointer> infile = mitk::IOUtil::Load( inFileName );

        if( (boost::algorithm::ends_with(inFileName, ".odf") || boost::algorithm::ends_with(inFileName, ".qbi")) && index=="gfa" )
        {
            typedef itk::Vector<float, ODF_SAMPLING_SIZE>   OdfVectorType;
            typedef itk::Image<OdfVectorType,3>         ItkOdfImageType;
            mitk::OdfImage::Pointer mitkOdfImage = dynamic_cast<mitk::OdfImage*>(infile[0].GetPointer());
            ItkOdfImageType::Pointer itk_odf = ItkOdfImageType::New();
            mitk::CastToItkImage(mitkOdfImage, itk_odf);


            typedef itk::DiffusionOdfGeneralizedFaImageFilter<float,float,ODF_SAMPLING_SIZE> GfaFilterType;
            GfaFilterType::Pointer gfaFilter = GfaFilterType::New();
            gfaFilter->SetInput(itk_odf);
            gfaFilter->SetComputationMethod(GfaFilterType::GFA_STANDARD);
            gfaFilter->Update();

            itk::ImageFileWriter< itk::Image<float,3> >::Pointer fileWriter = itk::ImageFileWriter< itk::Image<float,3> >::New();
            fileWriter->SetInput(gfaFilter->GetOutput());
            fileWriter->SetFileName(outFileName);
            fileWriter->Update();
        }
        else if( boost::algorithm::ends_with(inFileName, ".dti") )
        {
            typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
            mitk::TensorImage::Pointer mitkTensorImage = dynamic_cast<mitk::TensorImage*>(infile[0].GetPointer());
            ItkTensorImage::Pointer itk_dti = ItkTensorImage::New();
            mitk::CastToItkImage(mitkTensorImage, itk_dti);

            typedef itk::TensorDerivedMeasurementsFilter<float> MeasurementsType;
            MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
            measurementsCalculator->SetInput(itk_dti.GetPointer() );

            if(index=="fa")
                measurementsCalculator->SetMeasure(MeasurementsType::FA);
            else if(index=="ra")
                measurementsCalculator->SetMeasure(MeasurementsType::RA);
            else if(index=="ad")
                measurementsCalculator->SetMeasure(MeasurementsType::AD);
            else if(index=="rd")
                measurementsCalculator->SetMeasure(MeasurementsType::RD);
            else if(index=="ca")
                measurementsCalculator->SetMeasure(MeasurementsType::CA);
            else if(index=="l2")
                measurementsCalculator->SetMeasure(MeasurementsType::L2);
            else if(index=="l3")
                measurementsCalculator->SetMeasure(MeasurementsType::L3);
            else if(index=="md")
                measurementsCalculator->SetMeasure(MeasurementsType::MD);
            else
            {
                MITK_WARN << "No valid diffusion index for input image (tensor image) defined";
                return EXIT_FAILURE;
            }

            measurementsCalculator->Update();

            itk::ImageFileWriter< itk::Image<float,3> >::Pointer fileWriter = itk::ImageFileWriter< itk::Image<float,3> >::New();
            fileWriter->SetInput(measurementsCalculator->GetOutput());
            fileWriter->SetFileName(outFileName);
            fileWriter->Update();
        }
        else
            std::cout << "Diffusion index " << index << " not supported for supplied file type.";
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
