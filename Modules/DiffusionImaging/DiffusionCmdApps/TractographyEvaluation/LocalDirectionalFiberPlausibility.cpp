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

#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <itkEvaluateDirectionImagesFilter.h>
#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <itkTractsToVectorImageFilter.h>
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <mitkIOUtil.h>
#include <mitkLexicalCast.h>
#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkCoreObjectFactory.h>

/*!
\brief Calculate angular error of a tractogram with respect to the input reference directions.
*/
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Local Directional Fiber Plausibility");
    parser.setCategory("Fiber Tracking Evaluation");
    parser.setDescription("Calculate angular error of a tractogram with respect to the input reference directions.");
    parser.setContributor("MIC");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input tractogram (.fib, vtk ascii file format)", us::Any(), false);
    parser.addArgument("reference", "r", mitkCommandLineParser::StringList, "Reference images:", "reference direction images", us::Any(), false);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);
    parser.addArgument("mask", "m", mitkCommandLineParser::StringList, "Masks:", "mask images");
    parser.addArgument("athresh", "a", mitkCommandLineParser::Float, "Angular threshold:", "angular threshold in degrees. closer fiber directions are regarded as one direction and clustered together.", 25, true);
    parser.addArgument("sthresh", "s", mitkCommandLineParser::Float, "Size threshold:", "Relative peak size threshold per voxel.", 0.0, true);
    parser.addArgument("maxdirs", "md", mitkCommandLineParser::Int, "Max. Clusters:", "Maximum number of fiber clusters.", 0, true);
    parser.addArgument("verbose", "v", mitkCommandLineParser::Bool, "Verbose:", "output optional and intermediate calculation results");
    parser.addArgument("ignore", "n", mitkCommandLineParser::Bool, "Ignore:", "don't increase error for missing or too many directions");
    parser.addArgument("empty", "e", mitkCommandLineParser::Bool, "Empty Voxels:", "don't increase error for empty voxels");
    parser.addArgument("fileID", "id", mitkCommandLineParser::String, "ID:", "optional ID field");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    mitkCommandLineParser::StringContainerType referenceImages = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["reference"]);
    mitkCommandLineParser::StringContainerType maskImages;
    if (parsedArgs.count("mask"))
        maskImages = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["mask"]);

    string fibFile = us::any_cast<string>(parsedArgs["input"]);

    float angularThreshold = 25;
    if (parsedArgs.count("athresh"))
        angularThreshold = us::any_cast<float>(parsedArgs["athresh"]);

    float sizeThreshold = 0;
    if (parsedArgs.count("sthresh"))
        sizeThreshold = us::any_cast<float>(parsedArgs["sthresh"]);

    int maxDirs = 0;
    if (parsedArgs.count("maxdirs"))
        maxDirs = us::any_cast<int>(parsedArgs["maxdirs"]);

    string outRoot = us::any_cast<string>(parsedArgs["out"]);

    bool verbose = false;
    if (parsedArgs.count("verbose"))
        verbose = us::any_cast<bool>(parsedArgs["verbose"]);

    bool ignoreMissing = false;
    if (parsedArgs.count("ignore"))
        ignoreMissing = us::any_cast<bool>(parsedArgs["ignore"]);

    bool ignoreEmpty = false;
    if (parsedArgs.count("empty"))
        ignoreEmpty = us::any_cast<bool>(parsedArgs["empty"]);

    string fileID = "";
    if (parsedArgs.count("fileID"))
        fileID = us::any_cast<string>(parsedArgs["fileID"]);


    try
    {
        typedef itk::Image<unsigned char, 3>                                    ItkUcharImgType;
        typedef itk::Image< itk::Vector< float, 3>, 3 >                         ItkDirectionImage3DType;
        typedef itk::VectorContainer< unsigned int, ItkDirectionImage3DType::Pointer >   ItkDirectionImageContainerType;
        typedef itk::EvaluateDirectionImagesFilter< float >                     EvaluationFilterType;

        // load fiber bundle
        mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::LoadData(fibFile)[0].GetPointer());

        // load reference directions
        ItkDirectionImageContainerType::Pointer referenceImageContainer = ItkDirectionImageContainerType::New();
        for (unsigned int i=0; i<referenceImages.size(); i++)
        {
            try
            {
                mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(referenceImages.at(i));
                typedef mitk::ImageToItk< ItkDirectionImage3DType > CasterType;
                CasterType::Pointer caster = CasterType::New();
                caster->SetInput(img);
                caster->Update();
                ItkDirectionImage3DType::Pointer itkImg = caster->GetOutput();
                referenceImageContainer->InsertElement(referenceImageContainer->Size(),itkImg);
            }
            catch(...){ std::cout << "could not load: " << referenceImages.at(i); }
        }

        ItkUcharImgType::Pointer itkMaskImage = ItkUcharImgType::New();
        ItkDirectionImage3DType::Pointer dirImg = referenceImageContainer->GetElement(0);
        itkMaskImage->SetSpacing( dirImg->GetSpacing() );
        itkMaskImage->SetOrigin( dirImg->GetOrigin() );
        itkMaskImage->SetDirection( dirImg->GetDirection() );
        itkMaskImage->SetLargestPossibleRegion( dirImg->GetLargestPossibleRegion() );
        itkMaskImage->SetBufferedRegion( dirImg->GetLargestPossibleRegion() );
        itkMaskImage->SetRequestedRegion( dirImg->GetLargestPossibleRegion() );
        itkMaskImage->Allocate();
        itkMaskImage->FillBuffer(1);

        // extract directions from fiber bundle
        itk::TractsToVectorImageFilter<float>::Pointer fOdfFilter = itk::TractsToVectorImageFilter<float>::New();
        fOdfFilter->SetFiberBundle(inputTractogram);
        fOdfFilter->SetMaskImage(itkMaskImage);
        fOdfFilter->SetAngularThreshold(cos(angularThreshold*itk::Math::pi/180));
        fOdfFilter->SetNormalizeVectors(true);
        fOdfFilter->SetUseWorkingCopy(false);
        fOdfFilter->SetSizeThreshold(sizeThreshold);
        fOdfFilter->SetMaxNumDirections(maxDirs);
        fOdfFilter->Update();

        if (verbose)
        {
            // write vector field
            mitk::FiberBundle::Pointer directions = fOdfFilter->GetOutputFiberBundle();

            string outfilename = outRoot;
            outfilename.append("_VECTOR_FIELD.fib");

            mitk::IOUtil::Save(directions.GetPointer(), outfilename );

            // write direction images
            {
                itk::TractsToVectorImageFilter<float>::ItkDirectionImageType::Pointer itkImg = fOdfFilter->GetDirectionImage();
                typedef itk::ImageFileWriter< itk::TractsToVectorImageFilter<float>::ItkDirectionImageType > WriterType;
                WriterType::Pointer writer = WriterType::New();

                string outfilename = outRoot;
                outfilename.append("_DIRECTIONS.nrrd");

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

                writer->SetFileName(outfilename.c_str());
                writer->SetInput(numDirImage);
                writer->Update();
            }
        }

        string logFile = outRoot;
        logFile.append("_ANGULAR_ERROR.csv");
        ofstream file;
        file.open (logFile.c_str());

        if (maskImages.size()>0)
        {
            for (unsigned int i=0; i<maskImages.size(); i++)
            {
                mitk::Image::Pointer mitkMaskImage = mitk::IOUtil::Load<mitk::Image>(maskImages.at(i));
                mitk::CastToItkImage(mitkMaskImage, itkMaskImage);

                // evaluate directions
                EvaluationFilterType::Pointer evaluationFilter = EvaluationFilterType::New();
                //evaluationFilter->SetImageSet(directionImageContainer);
                evaluationFilter->SetReferenceImageSet(referenceImageContainer);
                evaluationFilter->SetMaskImage(itkMaskImage);
                evaluationFilter->SetIgnoreMissingDirections(ignoreMissing);
                evaluationFilter->SetIgnoreEmptyVoxels(ignoreEmpty);
                evaluationFilter->Update();

                if (verbose)
                {
                    EvaluationFilterType::OutputImageType::Pointer angularErrorImage = evaluationFilter->GetOutput(0);
                    typedef itk::ImageFileWriter< EvaluationFilterType::OutputImageType > WriterType;
                    WriterType::Pointer writer = WriterType::New();

                    string outfilename = outRoot;
                    outfilename.append("_ERROR_IMAGE.nrrd");

                    writer->SetFileName(outfilename.c_str());
                    writer->SetInput(angularErrorImage);
                    writer->Update();
                }

                string maskFileName = itksys::SystemTools::GetFilenameWithoutExtension(maskImages.at(i));
                unsigned found = maskFileName.find_last_of("_");

                string sens = itksys::SystemTools::GetFilenameWithoutLastExtension(fibFile);
                if (!fileID.empty())
                    sens = fileID;
                sens.append(",");

                sens.append(maskFileName.substr(found+1));
                sens.append(",");

                sens.append(boost::lexical_cast<string>(evaluationFilter->GetMeanAngularError()));
                sens.append(",");

                sens.append(boost::lexical_cast<string>(evaluationFilter->GetMedianAngularError()));
                sens.append(",");

                sens.append(boost::lexical_cast<string>(evaluationFilter->GetMaxAngularError()));
                sens.append(",");

                sens.append(boost::lexical_cast<string>(evaluationFilter->GetMinAngularError()));
                sens.append(",");

                sens.append(boost::lexical_cast<string>(std::sqrt(evaluationFilter->GetVarAngularError())));
                sens.append(";\n");
                file << sens;
            }
        }
        else
        {
            // evaluate directions
            EvaluationFilterType::Pointer evaluationFilter = EvaluationFilterType::New();
            //evaluationFilter->SetImageSet(directionImageContainer);
            evaluationFilter->SetReferenceImageSet(referenceImageContainer);
            evaluationFilter->SetMaskImage(itkMaskImage);
            evaluationFilter->SetIgnoreMissingDirections(ignoreMissing);
            evaluationFilter->SetIgnoreEmptyVoxels(ignoreEmpty);
            evaluationFilter->Update();

            if (verbose)
            {
                EvaluationFilterType::OutputImageType::Pointer angularErrorImage = evaluationFilter->GetOutput(0);
                typedef itk::ImageFileWriter< EvaluationFilterType::OutputImageType > WriterType;
                WriterType::Pointer writer = WriterType::New();

                string outfilename = outRoot;
                outfilename.append("_ERROR_IMAGE.nrrd");

                writer->SetFileName(outfilename.c_str());
                writer->SetInput(angularErrorImage);
                writer->Update();
            }

            string sens = itksys::SystemTools::GetFilenameWithoutLastExtension(fibFile);
            if (!fileID.empty())
                sens = fileID;
            sens.append(",");

            sens.append(boost::lexical_cast<string>(evaluationFilter->GetMeanAngularError()));
            sens.append(",");

            sens.append(boost::lexical_cast<string>(evaluationFilter->GetMedianAngularError()));
            sens.append(",");

            sens.append(boost::lexical_cast<string>(evaluationFilter->GetMaxAngularError()));
            sens.append(",");

            sens.append(boost::lexical_cast<string>(evaluationFilter->GetMinAngularError()));
            sens.append(",");

            sens.append(boost::lexical_cast<string>(std::sqrt(evaluationFilter->GetVarAngularError())));
            sens.append(";\n");
            file << sens;
        }
        file.close();
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
