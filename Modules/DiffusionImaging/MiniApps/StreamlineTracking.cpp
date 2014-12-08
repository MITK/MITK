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
#include <mitkTensorImage.h>
#include <mitkIOUtil.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkFiberBundleX.h>
#include <itkStreamlineTrackingFilter.h>
#include <itkDiffusionTensor3D.h>
#include "mitkCommandLineParser.h"
#include <mitkCoreObjectFactory.h>

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::StringList, "Input image", "input tensor image (.dti)", us::Any(), false);
    parser.addArgument("seed", "si", mitkCommandLineParser::InputFile, "Seed image", "binary seed image", us::Any(), true);
    parser.addArgument("mask", "mi", mitkCommandLineParser::InputFile, "Mask", "binary mask image", us::Any(), true);
    parser.addArgument("faImage", "fai", mitkCommandLineParser::InputFile, "FA image", "FA image", us::Any(), true);
    parser.addArgument("minFA", "fa", mitkCommandLineParser::Float, "Min. FA threshold", "minimum fractional anisotropy threshold", 0.15, true);
    parser.addArgument("minCurv", "c", mitkCommandLineParser::Float, "Min. curvature radius", "minimum curvature radius in mm (default = 0.5*minimum-spacing)");
    parser.addArgument("stepSize", "s", mitkCommandLineParser::Float, "Step size", "step size in mm (default = 0.1*minimum-spacing)");
    parser.addArgument("tendf", "f", mitkCommandLineParser::Float, "Weight f", "Weighting factor between first eigenvector (f=1 equals FACT tracking) and input vector dependent direction (f=0).", 1.0, true);
    parser.addArgument("tendg", "g", mitkCommandLineParser::Float, "Weight g", "Weighting factor between input vector (g=0) and tensor deflection (g=1 equals TEND tracking)", 0.0, true);
    parser.addArgument("numSeeds", "n", mitkCommandLineParser::Int, "Seeds per voxel", "Number of seeds per voxel.", 1, true);
    parser.addArgument("minLength", "l", mitkCommandLineParser::Float, "Min. fiber length", "minimum fiber length in mm", 20, true);

    parser.addArgument("interpolate", "ip", mitkCommandLineParser::Bool, "Interpolate", "Use linear interpolation", false, true);
    parser.addArgument("outFile", "o", mitkCommandLineParser::String, "Output file", "output fiber bundle (.fib)", us::Any(), false);

    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setTitle("Streamline Tracking");
    parser.setDescription("");
    parser.setContributor("MBI");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    mitkCommandLineParser::StringContainerType inputImages = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["input"]);
    string dtiFileName;
    string outFileName = us::any_cast<string>(parsedArgs["outFile"]);

    float minFA = 0.15;
    float minCurv = -1;
    float stepSize = -1;
    float tendf = 1;
    float tendg = 0;
    float minLength = 20;
    int numSeeds = 1;
    bool interpolate = false;

    if (parsedArgs.count("minCurv"))
        minCurv = us::any_cast<float>(parsedArgs["minCurv"]);
    if (parsedArgs.count("minFA"))
        minFA = us::any_cast<float>(parsedArgs["minFA"]);
    if (parsedArgs.count("stepSize"))
        stepSize = us::any_cast<float>(parsedArgs["stepSize"]);
    if (parsedArgs.count("tendf"))
        tendf = us::any_cast<float>(parsedArgs["tendf"]);
    if (parsedArgs.count("tendg"))
        tendg = us::any_cast<float>(parsedArgs["tendg"]);
    if (parsedArgs.count("minLength"))
        minLength = us::any_cast<float>(parsedArgs["minLength"]);
    if (parsedArgs.count("numSeeds"))
        numSeeds = us::any_cast<int>(parsedArgs["numSeeds"]);


    if (parsedArgs.count("interpolate"))
        interpolate = us::any_cast<bool>(parsedArgs["interpolate"]);



    try
    {
        typedef itk::StreamlineTrackingFilter< float > FilterType;
        FilterType::Pointer filter = FilterType::New();

        mitk::Image::Pointer mitkImage = NULL;

        std::cout << "Loading tensor images ...";
        typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
        dtiFileName = inputImages.at(0);
        for (unsigned int i=0; i<inputImages.size(); i++)
        {
            try
            {
                mitkImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(inputImages.at(i))->GetData());
                mitk::TensorImage::Pointer img = dynamic_cast<mitk::TensorImage*>(mitk::IOUtil::LoadDataNode(inputImages.at(i))->GetData());
                ItkTensorImage::Pointer itk_dti = ItkTensorImage::New();
                mitk::CastToItkImage(img, itk_dti);
                filter->SetInput(i, itk_dti);
            }
            catch(...){ std::cout << "could not load: " << inputImages.at(i); }
        }

        std::cout << "Loading seed image ...";
        typedef itk::Image< unsigned char, 3 >    ItkUCharImageType;
        mitk::Image::Pointer mitkSeedImage = NULL;
        if (parsedArgs.count("seed"))
            mitkSeedImage = mitk::IOUtil::LoadImage(us::any_cast<string>(parsedArgs["seed"]));

        std::cout << "Loading mask image ...";
        mitk::Image::Pointer mitkMaskImage = NULL;
        if (parsedArgs.count("mask"))
            mitkMaskImage = mitk::IOUtil::LoadImage(us::any_cast<string>(parsedArgs["mask"]));

        // instantiate tracker
        filter->SetSeedsPerVoxel(numSeeds);
        filter->SetFaThreshold(minFA);
        filter->SetMinCurvatureRadius(minCurv);
        filter->SetStepSize(stepSize);
        filter->SetF(tendf);
        filter->SetG(tendg);
        filter->SetInterpolate(interpolate);
        filter->SetMinTractLength(minLength);

        if (mitkSeedImage.IsNotNull())
        {
            ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
            mitk::CastToItkImage(mitkSeedImage, mask);
            filter->SetSeedImage(mask);
        }

        if (mitkMaskImage.IsNotNull())
        {
            ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
            mitk::CastToItkImage(mitkMaskImage, mask);
            filter->SetMaskImage(mask);
        }

        filter->Update();

        vtkSmartPointer<vtkPolyData> fiberBundle = filter->GetFiberPolyData();
        if ( fiberBundle->GetNumberOfLines()==0 )
        {
            std::cout << "No fibers reconstructed. Check parametrization.";
            return EXIT_FAILURE;
        }
        mitk::FiberBundleX::Pointer fib = mitk::FiberBundleX::New(fiberBundle);
        fib->SetReferenceGeometry(mitkImage->GetGeometry());

        mitk::IOUtil::SaveBaseData(fib.GetPointer(), outFileName );

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
