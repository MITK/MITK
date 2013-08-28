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
#include <mitkBaseDataIOFactory.h>
#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberTrackingObjectFactory.h>
#include <itkEvaluateDirectionImagesFilter.h>
#include <metaCommand.h>
#include "ctkCommandLineParser.h"
#include <itkTractsToVectorImageFilter.h>
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <mitkIOUtil.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>

#define _USE_MATH_DEFINES
#include <math.h>

int TractometerAngularErrorTool(int argc, char* argv[])
{
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", ctkCommandLineParser::String, "input tractogram (.fib, vtk ascii file format)", us::Any(), false);
    parser.addArgument("reference", "r", ctkCommandLineParser::StringList, "reference direction images", us::Any(), false);
    parser.addArgument("out", "o", ctkCommandLineParser::String, "output root", us::Any(), false);
    parser.addArgument("mask", "m", ctkCommandLineParser::String, "mask image");
    parser.addArgument("athresh", "a", ctkCommandLineParser::Float, "angular threshold in degrees. closer fiber directions are regarded as one direction and clustered together.", 25, true);
    parser.addArgument("verbose", "v", ctkCommandLineParser::Bool, "output optional and intermediate calculation results");
    parser.addArgument("ignore", "n", ctkCommandLineParser::Bool, "don't increase error for missing or too many directions");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    ctkCommandLineParser::StringContainerType referenceImages = us::any_cast<ctkCommandLineParser::StringContainerType>(parsedArgs["reference"]);

    string fibFile = us::any_cast<string>(parsedArgs["input"]);

    string maskImage("");
    if (parsedArgs.count("mask"))
        maskImage = us::any_cast<string>(parsedArgs["mask"]);

    float angularThreshold = 25;
    if (parsedArgs.count("athresh"))
        angularThreshold = us::any_cast<float>(parsedArgs["athresh"]);

    string outRoot = us::any_cast<string>(parsedArgs["out"]);

    bool verbose = false;
    if (parsedArgs.count("verbose"))
        verbose = us::any_cast<bool>(parsedArgs["verbose"]);

    bool ignore = false;
    if (parsedArgs.count("ignore"))
        ignore = us::any_cast<bool>(parsedArgs["ignore"]);

    try
    {
        RegisterDiffusionCoreObjectFactory();
        RegisterFiberTrackingObjectFactory();

        typedef itk::Image<unsigned char, 3>                                    ItkUcharImgType;
        typedef itk::Image< itk::Vector< float, 3>, 3 >                         ItkDirectionImage3DType;
        typedef itk::VectorContainer< int, ItkDirectionImage3DType::Pointer >   ItkDirectionImageContainerType;
        typedef itk::EvaluateDirectionImagesFilter< float >                     EvaluationFilterType;

        // load fiber bundle
        mitk::FiberBundleX::Pointer inputTractogram = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(fibFile)->GetData());

        // load reference directions
        ItkDirectionImageContainerType::Pointer referenceImageContainer = ItkDirectionImageContainerType::New();
        for (int i=0; i<referenceImages.size(); i++)
        {
            try
            {
                mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(referenceImages.at(i))->GetData());
                typedef mitk::ImageToItk< ItkDirectionImage3DType > CasterType;
                CasterType::Pointer caster = CasterType::New();
                caster->SetInput(img);
                caster->Update();
                ItkDirectionImage3DType::Pointer itkImg = caster->GetOutput();
                referenceImageContainer->InsertElement(referenceImageContainer->Size(),itkImg);
            }
            catch(...){ MITK_INFO << "could not load: " << referenceImages.at(i); }
        }

        // load/create mask image
        ItkUcharImgType::Pointer itkMaskImage = ItkUcharImgType::New();
        if (maskImage.compare("")==0)
        {
            ItkDirectionImage3DType::Pointer dirImg = referenceImageContainer->GetElement(0);
            itkMaskImage->SetSpacing( dirImg->GetSpacing() );
            itkMaskImage->SetOrigin( dirImg->GetOrigin() );
            itkMaskImage->SetDirection( dirImg->GetDirection() );
            itkMaskImage->SetLargestPossibleRegion( dirImg->GetLargestPossibleRegion() );
            itkMaskImage->SetBufferedRegion( dirImg->GetLargestPossibleRegion() );
            itkMaskImage->SetRequestedRegion( dirImg->GetLargestPossibleRegion() );
            itkMaskImage->Allocate();
            itkMaskImage->FillBuffer(1);
        }
        else
        {
            mitk::Image::Pointer mitkMaskImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(maskImage)->GetData());
            mitk::CastToItkImage<ItkUcharImgType>(mitkMaskImage, itkMaskImage);
        }


        // extract directions from fiber bundle
        itk::TractsToVectorImageFilter<float>::Pointer fOdfFilter = itk::TractsToVectorImageFilter<float>::New();
        fOdfFilter->SetFiberBundle(inputTractogram);
        fOdfFilter->SetMaskImage(itkMaskImage);
        fOdfFilter->SetAngularThreshold(cos(angularThreshold*M_PI/180));
        fOdfFilter->SetNormalizeVectors(true);
        fOdfFilter->SetUseWorkingCopy(false);
        fOdfFilter->Update();
        ItkDirectionImageContainerType::Pointer directionImageContainer = fOdfFilter->GetDirectionImageContainer();

        if (verbose)
        {
            // write vector field
            mitk::FiberBundleX::Pointer directions = fOdfFilter->GetOutputFiberBundle();
            mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
            for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
            {
                if ( (*it)->CanWriteBaseDataType(directions.GetPointer()) ) {
                    string outfilename = outRoot;
                    outfilename.append("_VECTOR_FIELD.fib");
                    (*it)->SetFileName( outfilename.c_str() );
                    (*it)->DoWrite( directions.GetPointer() );
                }
            }

            // write direction images
            for (int i=0; i<directionImageContainer->Size(); i++)
            {
                itk::TractsToVectorImageFilter<float>::ItkDirectionImageType::Pointer itkImg = directionImageContainer->GetElement(i);
                typedef itk::ImageFileWriter< itk::TractsToVectorImageFilter<float>::ItkDirectionImageType > WriterType;
                WriterType::Pointer writer = WriterType::New();

                string outfilename = outRoot;
                outfilename.append("_DIRECTION_");
                outfilename.append(boost::lexical_cast<string>(i));
                outfilename.append(".nrrd");

                MITK_INFO << "writing " << outfilename;
                writer->SetFileName(outfilename.c_str());
                writer->SetInput(itkImg);
                writer->Update();
            }

            // write num direction image
            {
                ItkUcharImgType::Pointer numDirImage = fOdfFilter->GetNumDirectionsImage();
                typedef itk::ImageFileWriter< ItkUcharImgType > WriterType;
                WriterType::Pointer writer = WriterType::New();

                string outfilename = outRoot;
                outfilename.append("_NUM_DIRECTIONS.nrrd");

                MITK_INFO << "writing " << outfilename;
                writer->SetFileName(outfilename.c_str());
                writer->SetInput(numDirImage);
                writer->Update();
            }
        }

        // evaluate directions
        EvaluationFilterType::Pointer evaluationFilter = EvaluationFilterType::New();
        evaluationFilter->SetImageSet(directionImageContainer);
        evaluationFilter->SetReferenceImageSet(referenceImageContainer);
        evaluationFilter->SetMaskImage(itkMaskImage);
        evaluationFilter->SetIgnoreMissingDirections(ignore);
        evaluationFilter->Update();

        if (verbose)
        {
            EvaluationFilterType::OutputImageType::Pointer angularErrorImage = evaluationFilter->GetOutput(0);
            typedef itk::ImageFileWriter< EvaluationFilterType::OutputImageType > WriterType;
            WriterType::Pointer writer = WriterType::New();

            string outfilename = outRoot;
            outfilename.append("_ERROR_IMAGE.nrrd");

            MITK_INFO << "writing " << outfilename;
            writer->SetFileName(outfilename.c_str());
            writer->SetInput(angularErrorImage);
            writer->Update();
        }

        string logFile = outRoot;
        logFile.append("_ANGULAR_ERROR.csv");

        ofstream file;
        file.open (logFile.c_str());

        string sens = "Mean:";
        sens.append(",");
        sens.append(boost::lexical_cast<string>(evaluationFilter->GetMeanAngularError()));
        sens.append(";\n");

        sens.append("Median:");
        sens.append(",");
        sens.append(boost::lexical_cast<string>(evaluationFilter->GetMedianAngularError()));
        sens.append(";\n");

        sens.append("Maximum:");
        sens.append(",");
        sens.append(boost::lexical_cast<string>(evaluationFilter->GetMaxAngularError()));
        sens.append(";\n");

        sens.append("Minimum:");
        sens.append(",");
        sens.append(boost::lexical_cast<string>(evaluationFilter->GetMinAngularError()));
        sens.append(";\n");

        sens.append("STDEV:");
        sens.append(",");
        sens.append(boost::lexical_cast<string>(std::sqrt(evaluationFilter->GetVarAngularError())));
        sens.append(";\n");

        file << sens;

        file.close();

        MITK_INFO << "DONE";
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
RegisterDiffusionMiniApp(TractometerAngularErrorTool);
