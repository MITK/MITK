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
#include <mitkIOUtil.h>
#include <boost/lexical_cast.hpp>
#include <itkEvaluateDirectionImagesFilter.h>
#include <itkTractsToVectorImageFilter.h>
#include <mitkCoreObjectFactory.h>

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Fiber Direction Extraction");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input tractogram (.fib/.trk)", us::Any(), false);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);
    parser.addArgument("mask", "m", mitkCommandLineParser::InputFile, "Mask:", "mask image");
    parser.addArgument("athresh", "a", mitkCommandLineParser::Float, "Angular threshold:", "angular threshold in degrees. closer fiber directions are regarded as one direction and clustered together.", 25, true);
    parser.addArgument("peakthresh", "t", mitkCommandLineParser::Float, "Peak size threshold:", "peak size threshold relative to largest peak in voxel", 0.2, true);
    parser.addArgument("verbose", "v", mitkCommandLineParser::Bool, "Verbose:", "output optional and intermediate calculation results");
    parser.addArgument("numdirs", "d", mitkCommandLineParser::Int, "Max. num. directions:", "maximum number of fibers per voxel", 3, true);
    parser.addArgument("normalize", "n", mitkCommandLineParser::Bool, "Normalize:", "normalize vectors");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    string fibFile = us::any_cast<string>(parsedArgs["input"]);

    string maskImage("");
    if (parsedArgs.count("mask"))
        maskImage = us::any_cast<string>(parsedArgs["mask"]);

    float peakThreshold = 0.2;
    if (parsedArgs.count("peakthresh"))
        peakThreshold = us::any_cast<float>(parsedArgs["peakthresh"]);

    float angularThreshold = 25;
    if (parsedArgs.count("athresh"))
        angularThreshold = us::any_cast<float>(parsedArgs["athresh"]);

    string outRoot = us::any_cast<string>(parsedArgs["out"]);

    bool verbose = false;
    if (parsedArgs.count("verbose"))
        verbose = us::any_cast<bool>(parsedArgs["verbose"]);

    int maxNumDirs = 3;
    if (parsedArgs.count("numdirs"))
        maxNumDirs = us::any_cast<int>(parsedArgs["numdirs"]);

    bool normalize = false;
    if (parsedArgs.count("normalize"))
        normalize = us::any_cast<bool>(parsedArgs["normalize"]);

    try
    {
        typedef itk::Image<unsigned char, 3>                                    ItkUcharImgType;
        typedef itk::Image< itk::Vector< float, 3>, 3 >                         ItkDirectionImage3DType;
        typedef itk::VectorContainer< unsigned int, ItkDirectionImage3DType::Pointer >   ItkDirectionImageContainerType;

        // load fiber bundle
        mitk::FiberBundleX::Pointer inputTractogram = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(fibFile)->GetData());

        // load/create mask image
        ItkUcharImgType::Pointer itkMaskImage = NULL;
        if (maskImage.compare("")!=0)
        {
            std::cout << "Using mask image";
            itkMaskImage = ItkUcharImgType::New();
            mitk::Image::Pointer mitkMaskImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(maskImage)->GetData());
            mitk::CastToItkImage<ItkUcharImgType>(mitkMaskImage, itkMaskImage);
        }

        // extract directions from fiber bundle
        itk::TractsToVectorImageFilter<float>::Pointer fOdfFilter = itk::TractsToVectorImageFilter<float>::New();
        fOdfFilter->SetFiberBundle(inputTractogram);
        fOdfFilter->SetMaskImage(itkMaskImage);
        fOdfFilter->SetAngularThreshold(cos(angularThreshold*M_PI/180));
        fOdfFilter->SetNormalizeVectors(normalize);
        fOdfFilter->SetUseWorkingCopy(false);
        fOdfFilter->SetSizeThreshold(peakThreshold);
        fOdfFilter->SetMaxNumDirections(maxNumDirs);
        fOdfFilter->Update();
        ItkDirectionImageContainerType::Pointer directionImageContainer = fOdfFilter->GetDirectionImageContainer();

        // write direction images
        for (unsigned int i=0; i<directionImageContainer->Size(); i++)
        {
            itk::TractsToVectorImageFilter<float>::ItkDirectionImageType::Pointer itkImg = directionImageContainer->GetElement(i);
            typedef itk::ImageFileWriter< itk::TractsToVectorImageFilter<float>::ItkDirectionImageType > WriterType;
            WriterType::Pointer writer = WriterType::New();

            string outfilename = outRoot;
            outfilename.append("_DIRECTION_");
            outfilename.append(boost::lexical_cast<string>(i));
            outfilename.append(".nrrd");

            writer->SetFileName(outfilename.c_str());
            writer->SetInput(itkImg);
            writer->Update();
        }

        if (verbose)
        {
            // write vector field
            mitk::FiberBundleX::Pointer directions = fOdfFilter->GetOutputFiberBundle();

            string outfilename = outRoot;
            outfilename.append("_VECTOR_FIELD.fib");

            mitk::IOUtil::SaveBaseData(directions.GetPointer(), outfilename );

            // write num direction image
            {
                ItkUcharImgType::Pointer numDirImage = fOdfFilter->GetNumDirectionsImage();
                typedef itk::ImageFileWriter< ItkUcharImgType > WriterType;
                WriterType::Pointer writer = WriterType::New();

                string outfilename = outRoot;
                outfilename.append("_NUM_DIRECTIONS.nrrd");

                writer->SetFileName(outfilename.c_str());
                writer->SetInput(numDirImage);
                writer->Update();
            }
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
