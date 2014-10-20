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

#include "MiniAppManager.h"
#include <mitkImageCast.h>
#include <mitkTensorImage.h>
#include <mitkIOUtil.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkFiberBundleX.h>
#include <itkStreamlineTrackingFilter.h>
#include <itkDiffusionTensor3D.h>
#include "ctkCommandLineParser.h"
#include <mitkCoreObjectFactory.h>
#include <mitkFiberBundleXWriter.h>

int StreamlineTracking(int argc, char* argv[])
{
    MITK_INFO << "StreamlineTracking";
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", ctkCommandLineParser::StringList, "Input image", "input tensor image (.dti)", us::Any(), false);
    parser.addArgument("seed", "si", ctkCommandLineParser::InputFile, "Seed image", "binary seed image", us::Any(), true);
    parser.addArgument("mask", "mi", ctkCommandLineParser::InputFile, "Mask", "binary mask image", us::Any(), true);
    parser.addArgument("faImage", "fai", ctkCommandLineParser::InputFile, "FA image", "FA image", us::Any(), true);
    parser.addArgument("minFA", "fa", ctkCommandLineParser::Float, "Min. FA threshold", "minimum fractional anisotropy threshold", 0.15, true);
    parser.addArgument("minCurv", "c", ctkCommandLineParser::Float, "Min. curvature radius", "minimum curvature radius in mm (default = 0.5*minimum-spacing)");
    parser.addArgument("stepSize", "s", ctkCommandLineParser::Float, "Step size", "step size in mm (default = 0.1*minimum-spacing)");
    parser.addArgument("tendf", "f", ctkCommandLineParser::Float, "Weight f", "Weighting factor between first eigenvector (f=1 equals FACT tracking) and input vector dependent direction (f=0).", 1.0, true);
    parser.addArgument("tendg", "g", ctkCommandLineParser::Float, "Weight g", "Weighting factor between input vector (g=0) and tensor deflection (g=1 equals TEND tracking)", 0.0, true);
    parser.addArgument("numSeeds", "n", ctkCommandLineParser::Int, "Seeds per voxel", "Number of seeds per voxel.", 1, true);
    parser.addArgument("minLength", "l", ctkCommandLineParser::Float, "Min. fiber length", "minimum fiber length in mm", 20, true);

    parser.addArgument("interpolate", "ip", ctkCommandLineParser::Bool, "Interpolate", "Use linear interpolation", false, true);
    parser.addArgument("outFile", "o", ctkCommandLineParser::String, "Output file", "output fiber bundle (.fib)", us::Any(), false);

    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setTitle("Streamline Tracking");
    parser.setDescription("");
    parser.setContributor("MBI");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    ctkCommandLineParser::StringContainerType inputImages = us::any_cast<ctkCommandLineParser::StringContainerType>(parsedArgs["input"]);
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

        MITK_INFO << "Loading tensor images ...";
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
            catch(...){ MITK_INFO << "could not load: " << inputImages.at(i); }
        }

        MITK_INFO << "Loading seed image ...";
        typedef itk::Image< unsigned char, 3 >    ItkUCharImageType;
        mitk::Image::Pointer mitkSeedImage = NULL;
        if (parsedArgs.count("seed"))
            mitkSeedImage = mitk::IOUtil::LoadImage(us::any_cast<string>(parsedArgs["seed"]));

        MITK_INFO << "Loading mask image ...";
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
            MITK_INFO << "No fibers reconstructed. Check parametrization.";
            return EXIT_FAILURE;
        }
        mitk::FiberBundleX::Pointer fib = mitk::FiberBundleX::New(fiberBundle);
        fib->SetReferenceGeometry(mitkImage->GetGeometry());

        mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
        for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
        {
            if ( (*it)->CanWriteBaseDataType(fib.GetPointer()) ) {
                (*it)->SetFileName( outFileName.c_str() );
                (*it)->DoWrite( fib.GetPointer() );
            }
        }
    }
    catch (itk::ExceptionObject e)
    {
        MITK_INFO << e;
        return EXIT_FAILURE;
    }
    catch (std::exception e)
    {
        MITK_INFO << e.what();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        MITK_INFO << "ERROR!?!";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
RegisterDiffusionMiniApp(StreamlineTracking);
