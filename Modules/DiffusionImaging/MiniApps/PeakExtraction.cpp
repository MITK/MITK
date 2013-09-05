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

#include <itkImageFileWriter.h>
#include <itkResampleImageFilter.h>
#include <itkFiniteDiffOdfMaximaExtractionFilter.h>

#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionImage.h>
#include <mitkQBallImage.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkTensorImage.h>

#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkCoreExtObjectFactory.h>
#include <mitkCoreObjectFactory.h>
#include "ctkCommandLineParser.h"
#include <mitkFiberBundleXWriter.h>
#include <itkShCoefficientImageImporter.h>
#include <mitkNrrdQBallImageWriter.h>
#include <itkFlipImageFilter.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

mitk::Image::Pointer LoadData(std::string filename)
{
    if( filename.empty() )
        return NULL;

    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( filename, s1, s2, false );
    if( infile.empty() )
    {
        MITK_INFO << "File " << filename << " could not be read!";
        return NULL;
    }

    mitk::BaseData::Pointer baseData = infile.at(0);
    return dynamic_cast<mitk::Image*>(baseData.GetPointer());
}


template<int shOrder>
int Start(int argc, char* argv[])
{
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("image", "i", ctkCommandLineParser::String, "sh coefficient image", us::Any(), false);
    parser.addArgument("outroot", "o", ctkCommandLineParser::String, "output root", us::Any(), false);
    parser.addArgument("mask", "m", ctkCommandLineParser::String, "mask image");
    parser.addArgument("normalization", "n", ctkCommandLineParser::Int, "0=no norm, 1=max norm, 2=single vec norm", 1, true);
    parser.addArgument("numpeaks", "p", ctkCommandLineParser::Int, "maximum number of extracted peaks", 2, true);
    parser.addArgument("peakthres", "r", ctkCommandLineParser::Float, "peak threshold relative to largest peak", 0.4, true);
    parser.addArgument("abspeakthres", "a", ctkCommandLineParser::Float, "absolute peak threshold weighted with local GFA value", 0.06, true);
    parser.addArgument("shConvention", "s", ctkCommandLineParser::String, "use specified SH-basis (MITK, FSL, MRtrix)", string("MITK"), true);
    parser.addArgument("noFlip", "f", ctkCommandLineParser::Bool, "do not flip input image to match MITK coordinate convention");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    // mandatory arguments
    string imageName = us::any_cast<string>(parsedArgs["image"]);
    string outRoot = us::any_cast<string>(parsedArgs["outroot"]);

    // optional arguments
    string maskImageName("");
    if (parsedArgs.count("mask"))
        maskImageName = us::any_cast<string>(parsedArgs["mask"]);

    int normalization = 1;
    if (parsedArgs.count("normalization"))
        normalization = us::any_cast<int>(parsedArgs["normalization"]);

    int numPeaks = 2;
    if (parsedArgs.count("numpeaks"))
        numPeaks = us::any_cast<int>(parsedArgs["numpeaks"]);

    float peakThres = 0.4;
    if (parsedArgs.count("peakthres"))
        peakThres = us::any_cast<float>(parsedArgs["peakthres"]);

    float absPeakThres = 0.06;
    if (parsedArgs.count("abspeakthres"))
        absPeakThres = us::any_cast<float>(parsedArgs["abspeakthres"]);

    bool noFlip = false;
    if (parsedArgs.count("noFlip"))
        noFlip = us::any_cast<bool>(parsedArgs["noFlip"]);

    MITK_INFO << "image: " << imageName;
    MITK_INFO << "outroot: " << outRoot;
    if (!maskImageName.empty())
        MITK_INFO << "mask: " << maskImageName;
    else
        MITK_INFO << "no mask image selected";
    MITK_INFO << "numpeaks: " << numPeaks;
    MITK_INFO << "peakthres: " << peakThres;
    MITK_INFO << "abspeakthres: " << absPeakThres;
    MITK_INFO << "shOrder: " << shOrder;

    try
    {
        mitk::CoreObjectFactory::GetInstance();

        RegisterDiffusionCoreObjectFactory();

        mitk::Image::Pointer image = LoadData(imageName);
        mitk::Image::Pointer mask = LoadData(maskImageName);

        typedef itk::Image<unsigned char, 3>  ItkUcharImgType;
        typedef itk::FiniteDiffOdfMaximaExtractionFilter< float, shOrder, 20242 > MaximaExtractionFilterType;
        typename MaximaExtractionFilterType::Pointer filter = MaximaExtractionFilterType::New();

        int toolkitConvention = 0;

        if (parsedArgs.count("shConvention"))
        {
            string convention = us::any_cast<string>(parsedArgs["shConvention"]).c_str();
            if ( boost::algorithm::equals(convention, "FSL") )
            {
                toolkitConvention = 1;
                MITK_INFO << "Using FSL SH-basis";
            }
            else if ( boost::algorithm::equals(convention, "MRtrix") )
            {
                toolkitConvention = 2;
                MITK_INFO << "Using MRtrix SH-basis";
            }
            else
                MITK_INFO << "Using MITK SH-basis";
        }
        else
            MITK_INFO << "Using MITK SH-basis";

        ItkUcharImgType::Pointer itkMaskImage = NULL;
        if (mask.IsNotNull())
        {
            try{
                itkMaskImage = ItkUcharImgType::New();
                mitk::CastToItkImage<ItkUcharImgType>(mask, itkMaskImage);
                filter->SetMaskImage(itkMaskImage);
            }
            catch(...)
            {

            }
        }

        if (toolkitConvention>0)
        {
            MITK_INFO << "Converting coefficient image to MITK format";
            typedef itk::ShCoefficientImageImporter< float, shOrder > ConverterType;
            typedef mitk::ImageToItk< itk::Image< float, 4 > > CasterType;
            CasterType::Pointer caster = CasterType::New();
            caster->SetInput(image);
            caster->Update();
            itk::Image< float, 4 >::Pointer itkImage = caster->GetOutput();

            typename ConverterType::Pointer converter = ConverterType::New();

            if (noFlip)
            {
                converter->SetInputImage(itkImage);
            }
            else
            {
                MITK_INFO << "Flipping image";
                itk::FixedArray<bool, 4> flipAxes;
                flipAxes[0] = true;
                flipAxes[1] = true;
                flipAxes[2] = false;
                flipAxes[3] = false;
                itk::FlipImageFilter< itk::Image< float, 4 > >::Pointer flipper = itk::FlipImageFilter< itk::Image< float, 4 > >::New();
                flipper->SetInput(itkImage);
                flipper->SetFlipAxes(flipAxes);
                flipper->Update();
                itk::Image< float, 4 >::Pointer flipped = flipper->GetOutput();
                itk::Matrix< double,4,4 > m = itkImage->GetDirection(); m[0][0] *= -1; m[1][1] *= -1;
                flipped->SetDirection(m);

                itk::Point< float, 4 > o = itkImage->GetOrigin();
                o[0] -= (flipped->GetLargestPossibleRegion().GetSize(0)-1);
                o[1] -= (flipped->GetLargestPossibleRegion().GetSize(1)-1);
                flipped->SetOrigin(o);
                converter->SetInputImage(flipped);
            }

            MITK_INFO << "Starting conversion";
            switch (toolkitConvention)
            {
            case 1:
                converter->SetToolkit(ConverterType::FSL);
                filter->SetToolkit(MaximaExtractionFilterType::FSL);
                break;
            case 2:
                converter->SetToolkit(ConverterType::MRTRIX);
                filter->SetToolkit(MaximaExtractionFilterType::MRTRIX);
                break;
            default:
                converter->SetToolkit(ConverterType::FSL);
                filter->SetToolkit(MaximaExtractionFilterType::FSL);
                break;
            }
            converter->GenerateData();
            filter->SetInput(converter->GetCoefficientImage());
        }
        else
        {
            try{
                typedef mitk::ImageToItk< typename MaximaExtractionFilterType::CoefficientImageType > CasterType;
                typename CasterType::Pointer caster = CasterType::New();
                caster->SetInput(image);
                caster->Update();
                filter->SetInput(caster->GetOutput());
            }
            catch(...)
            {
                MITK_INFO << "wrong image type";
                return EXIT_FAILURE;
            }
        }

        filter->SetMaxNumPeaks(numPeaks);
        filter->SetPeakThreshold(peakThres);
        filter->SetAbsolutePeakThreshold(absPeakThres);
        filter->SetAngularThreshold(1);

        switch (normalization)
        {
        case 0:
            filter->SetNormalizationMethod(MaximaExtractionFilterType::NO_NORM);
            break;
        case 1:
            filter->SetNormalizationMethod(MaximaExtractionFilterType::MAX_VEC_NORM);
            break;
        case 2:
            filter->SetNormalizationMethod(MaximaExtractionFilterType::SINGLE_VEC_NORM);
            break;
        }

        MITK_INFO << "Starting extraction";
        filter->Update();

        // write direction images
        {
            typedef typename MaximaExtractionFilterType::ItkDirectionImageContainer ItkDirectionImageContainer;
            typename ItkDirectionImageContainer::Pointer container = filter->GetDirectionImageContainer();
            for (int i=0; i<container->Size(); i++)
            {
                typename MaximaExtractionFilterType::ItkDirectionImage::Pointer itkImg = container->GetElement(i);

                if (itkMaskImage.IsNotNull())
                {
                    itkImg->SetDirection(itkMaskImage->GetDirection());
                    itkImg->SetOrigin(itkMaskImage->GetOrigin());
                }

                string outfilename = outRoot;
                outfilename.append("_DIRECTION_");
                outfilename.append(boost::lexical_cast<string>(i));
                outfilename.append(".nrrd");

                MITK_INFO << "writing " << outfilename;
                typedef itk::ImageFileWriter< typename MaximaExtractionFilterType::ItkDirectionImage > WriterType;
                typename WriterType::Pointer writer = WriterType::New();
                writer->SetFileName(outfilename);
                writer->SetInput(itkImg);
                writer->Update();
            }
        }

        // write num directions image
        {
            ItkUcharImgType::Pointer numDirImage = filter->GetNumDirectionsImage();

            if (itkMaskImage.IsNotNull())
            {
                numDirImage->SetDirection(itkMaskImage->GetDirection());
                numDirImage->SetOrigin(itkMaskImage->GetOrigin());
            }

            string outfilename = outRoot.c_str();
            outfilename.append("_NUM_DIRECTIONS.nrrd");
            MITK_INFO << "writing " << outfilename;
            typedef itk::ImageFileWriter< ItkUcharImgType > WriterType;
            WriterType::Pointer writer = WriterType::New();
            writer->SetFileName(outfilename);
            writer->SetInput(numDirImage);
            writer->Update();
        }

        // write vector field
        {
            mitk::FiberBundleX::Pointer directions = filter->GetOutputFiberBundle();

            string outfilename = outRoot.c_str();
            outfilename.append("_VECTOR_FIELD.fib");

            mitk::FiberBundleXWriter::Pointer fibWriter = mitk::FiberBundleXWriter::New();
            fibWriter->SetFileName(outfilename.c_str());
            fibWriter->DoWrite(directions.GetPointer());
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

int PeakExtraction(int argc, char* argv[])
{
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("image", "i", ctkCommandLineParser::String, "sh coefficient image", us::Any(), false);
    parser.addArgument("shOrder", "sh", ctkCommandLineParser::Int, "spherical harmonics order");
    parser.addArgument("outroot", "o", ctkCommandLineParser::String, "output root", us::Any(), false);
    parser.addArgument("mask", "m", ctkCommandLineParser::String, "mask image");
    parser.addArgument("normalization", "n", ctkCommandLineParser::Int, "0=no norm, 1=max norm, 2=single vec norm", 1, true);
    parser.addArgument("numpeaks", "p", ctkCommandLineParser::Int, "maximum number of extracted peaks", 2, true);
    parser.addArgument("peakthres", "r", ctkCommandLineParser::Float, "peak threshold relative to largest peak", 0.4, true);
    parser.addArgument("abspeakthres", "a", ctkCommandLineParser::Float, "absolute peak threshold weighted with local GFA value", 0.06, true);
    parser.addArgument("shConvention", "s", ctkCommandLineParser::String, "use specified SH-basis (MITK, FSL, MRtrix)", string("MITK"), true);
    parser.addArgument("noFlip", "f", ctkCommandLineParser::Bool, "do not flip input image to match MITK coordinate convention");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;


    int shOrder = -1;
    if (parsedArgs.count("shOrder"))
        shOrder = us::any_cast<int>(parsedArgs["shOrder"]);

    switch (shOrder)
    {
    case 4:
        return Start<4>(argc, argv);
    case 6:
        return Start<6>(argc, argv);
    case 8:
        return Start<8>(argc, argv);
    case 10:
        return Start<10>(argc, argv);
    case 12:
        return Start<12>(argc, argv);
    }
    return EXIT_FAILURE;
}
RegisterDiffusionMiniApp(PeakExtraction);
