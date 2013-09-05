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
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkFiberBundleX.h>
#include <itkStreamlineTrackingFilter.h>
#include <itkDiffusionTensor3D.h>
#include "ctkCommandLineParser.h"

int StreamlineTracking(int argc, char* argv[])
{
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", ctkCommandLineParser::String, "input tensor image (.dti)", us::Any(), false);
    parser.addArgument("seed", "si", ctkCommandLineParser::String, "binary seed image");
    parser.addArgument("mask", "mi", ctkCommandLineParser::String, "binary mask image");
    parser.addArgument("minFA", "fa", ctkCommandLineParser::Float, "minimum fractional anisotropy threshold", 0.15, true);
    parser.addArgument("minCurv", "c", ctkCommandLineParser::Float, "minimum curvature radius in mm (default = 0.5*minimum-spacing)");
    parser.addArgument("stepSize", "s", ctkCommandLineParser::Float, "stepsize in mm (default = 0.1*minimum-spacing)");
    parser.addArgument("tendf", "f", ctkCommandLineParser::Float, "Weighting factor between first eigenvector (f=1 equals FACT tracking) and input vector dependent direction (f=0).", 1.0, true);
    parser.addArgument("tendg", "g", ctkCommandLineParser::Float, "Weighting factor between input vector (g=0) and tensor deflection (g=1 equals TEND tracking)", 0.0, true);
    parser.addArgument("numSeeds", "n", ctkCommandLineParser::Int, "Number of seeds per voxel.", 1, true);
    parser.addArgument("minLength", "l", ctkCommandLineParser::Float, "minimum fiber length in mm", 20, true);

    parser.addArgument("interpolate", "ip", ctkCommandLineParser::Bool, "Use linear interpolation", false, true);
    parser.addArgument("outFile", "o", ctkCommandLineParser::String, "output fiber bundle (.fib)", us::Any(), false);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    string dtiFileName = us::any_cast<string>(parsedArgs["input"]);
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
        RegisterDiffusionCoreObjectFactory();
        RegisterFiberTrackingObjectFactory();

        // load input image
        const std::string s1="", s2="";
        std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( dtiFileName, s1, s2, false );

        MITK_INFO << "Loading tensor image ...";
        typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
        mitk::TensorImage::Pointer mitkTensorImage = dynamic_cast<mitk::TensorImage*>(infile.at(0).GetPointer());
        ItkTensorImage::Pointer itk_dti = ItkTensorImage::New();
        mitk::CastToItkImage<ItkTensorImage>(mitkTensorImage, itk_dti);

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
        typedef itk::StreamlineTrackingFilter< float > FilterType;
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(itk_dti);
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
            mitk::CastToItkImage<ItkUCharImageType>(mitkSeedImage, mask);
            filter->SetSeedImage(mask);
        }

        if (mitkMaskImage.IsNotNull())
        {
            ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
            mitk::CastToItkImage<ItkUCharImageType>(mitkMaskImage, mask);
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
    MITK_INFO << "DONE";
    return EXIT_SUCCESS;
}
RegisterDiffusionMiniApp(StreamlineTracking);
