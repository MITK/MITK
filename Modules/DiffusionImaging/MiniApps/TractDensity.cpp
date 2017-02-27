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

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

#include <itkImageFileWriter.h>
#include <itkMetaDataObject.h>
#include <itkVectorImage.h>
#include <mitkImageCast.h>

#include <mitkBaseData.h>
#include <mitkFiberBundle.h>
#include "mitkCommandLineParser.h"
#include <boost/lexical_cast.hpp>
#include <mitkCoreObjectFactory.h>
#include <mitkIOUtil.h>
#include <itkTractDensityImageFilter.h>


mitk::FiberBundle::Pointer LoadFib(std::string filename)
{
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(filename);
    if( fibInfile.empty() )
        std::cout << "File " << filename << " could not be read!";
    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    return dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());
}

/*!
\brief Modify input tractogram: fiber resampling, compression, pruning and transformation.
*/
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Tract Density");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("Generate tract density image or fiber envelope.");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input fiber bundle (.fib)", us::Any(), false);
    parser.addArgument("outFile", "o", mitkCommandLineParser::OutputFile, "Output:", "output image", us::Any(), false);
    parser.addArgument("binary", "b", mitkCommandLineParser::Int, "Binary output:", "calculate binary tract envelope", us::Any());
    parser.addArgument("ref_image", "r", mitkCommandLineParser::StringList, "Reference image:", "output image will have geometry of this reference image", us::Any());


    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    bool binary = false;
    if (parsedArgs.count("binary"))
        binary = us::any_cast<int>(parsedArgs["binary"]);

    std::string ref_image = "";
    if (parsedArgs.count("ref_image"))
        ref_image = us::any_cast<std::string>(parsedArgs["ref_image"]);

    std::string inFileName = us::any_cast<std::string>(parsedArgs["input"]);
    std::string outFileName = us::any_cast<std::string>(parsedArgs["outFile"]);

    try
    {
        mitk::FiberBundle::Pointer fib = LoadFib(inFileName);

        mitk::Image::Pointer ref_img;
        if (!ref_image.empty())
            ref_img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(ref_image).GetPointer());

        if (binary)
        {
            typedef unsigned char OutPixType;
            typedef itk::Image<OutPixType, 3> OutImageType;

            itk::TractDensityImageFilter< OutImageType >::Pointer generator = itk::TractDensityImageFilter< OutImageType >::New();
            generator->SetFiberBundle(fib);
            generator->SetBinaryOutput(binary);
            generator->SetOutputAbsoluteValues(false);
            generator->SetWorkOnFiberCopy(false);

            if (ref_img.IsNotNull())
            {
                OutImageType::Pointer itkImage = OutImageType::New();
                CastToItkImage(ref_img, itkImage);
                generator->SetInputImage(itkImage);
                generator->SetUseImageGeometry(true);

            }
            generator->Update();

            // get output image
            typedef itk::Image<OutPixType,3> OutType;
            OutType::Pointer outImg = generator->GetOutput();
            mitk::Image::Pointer img = mitk::Image::New();
            img->InitializeByItk(outImg.GetPointer());
            img->SetVolume(outImg->GetBufferPointer());

            mitk::IOUtil::SaveBaseData(img, outFileName );
        }
        else
        {
            typedef float OutPixType;
            typedef itk::Image<OutPixType, 3> OutImageType;

            itk::TractDensityImageFilter< OutImageType >::Pointer generator = itk::TractDensityImageFilter< OutImageType >::New();
            generator->SetFiberBundle(fib);
            generator->SetBinaryOutput(binary);
            generator->SetOutputAbsoluteValues(false);
            generator->SetWorkOnFiberCopy(false);

            if (ref_img.IsNotNull())
            {
                OutImageType::Pointer itkImage = OutImageType::New();
                CastToItkImage(ref_img, itkImage);
                generator->SetInputImage(itkImage);
                generator->SetUseImageGeometry(true);

            }
            generator->Update();

            // get output image
            typedef itk::Image<OutPixType,3> OutType;
            OutType::Pointer outImg = generator->GetOutput();
            mitk::Image::Pointer img = mitk::Image::New();
            img->InitializeByItk(outImg.GetPointer());
            img->SetVolume(outImg->GetBufferPointer());

            mitk::IOUtil::SaveBaseData(img, outFileName );
        }

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
