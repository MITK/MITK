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

#include "mitkBaseDataIOFactory.h"
#include <mitkCoreObjectFactory.h>
#include "mitkDiffusionImage.h"
#include "itkAnalyticalDiffusionQballReconstructionImageFilter.h"
#include <boost/lexical_cast.hpp>
#include "mitkCommandLineParser.h"
#include <mitkIOUtil.h>
#include <itksys/SystemTools.hxx>

using namespace mitk;

/**
 * Perform Q-ball reconstruction using a spherical harmonics basis
 */
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input file", "input raw dwi (.dwi or .fsl/.fslgz)", us::Any(), false);
    parser.addArgument("outFile", "o", mitkCommandLineParser::OutputFile, "Output file", "output file", us::Any(), false);
    parser.addArgument("shOrder", "sh", mitkCommandLineParser::Int, "Spherical harmonics order", "spherical harmonics order", 4, true);
    parser.addArgument("b0Threshold", "t", mitkCommandLineParser::Int, "b0 threshold", "baseline image intensity threshold", 0, true);
    parser.addArgument("lambda", "r", mitkCommandLineParser::Float, "Lambda", "ragularization factor lambda", 0.006, true);
    parser.addArgument("csa", "csa", mitkCommandLineParser::Bool, "Constant solid angle consideration", "use constant solid angle consideration");
    parser.addArgument("outputCoeffs", "shc", mitkCommandLineParser::Bool, "Output coefficients", "output file containing the SH coefficients");
    parser.addArgument("mrtrix", "mb", mitkCommandLineParser::Bool, "MRtrix", "use MRtrix compatible spherical harmonics definition");

    parser.setCategory("Preprocessing Tools");
    parser.setTitle("Qball Reconstruction");
    parser.setDescription("");
    parser.setContributor("MBI");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    std::string inFileName = us::any_cast<string>(parsedArgs["input"]);
    std::string outfilename = us::any_cast<string>(parsedArgs["outFile"]);
    outfilename = itksys::SystemTools::GetFilenamePath(outfilename)+"/"+itksys::SystemTools::GetFilenameWithoutExtension(outfilename);

    int threshold = 0;
    if (parsedArgs.count("b0Threshold"))
        threshold = us::any_cast<int>(parsedArgs["b0Threshold"]);

    int shOrder = 4;
    if (parsedArgs.count("shOrder"))
        shOrder = us::any_cast<int>(parsedArgs["shOrder"]);

    float lambda = 0.006;
    if (parsedArgs.count("lambda"))
        lambda = us::any_cast<float>(parsedArgs["lambda"]);

    int normalization = 0;
    if (parsedArgs.count("csa") && us::any_cast<bool>(parsedArgs["csa"]))
        normalization = 6;

    bool outCoeffs = false;
    if (parsedArgs.count("outputCoeffs"))
        outCoeffs = us::any_cast<bool>(parsedArgs["outputCoeffs"]);

    bool mrTrix = false;
    if (parsedArgs.count("mrtrix"))
        mrTrix = us::any_cast<bool>(parsedArgs["mrtrix"]);

    try
    {
        const std::string s1="", s2="";
        std::vector<BaseData::Pointer> infile = BaseDataIO::LoadBaseDataFromFile( inFileName, s1, s2, false );
        DiffusionImage<short>::Pointer dwi = dynamic_cast<DiffusionImage<short>*>(infile.at(0).GetPointer());
        dwi->AverageRedundantGradients(0.001);

        mitk::QBallImage::Pointer image = mitk::QBallImage::New();
        mitk::Image::Pointer coeffsImage = mitk::Image::New();

        std::cout << "SH order: " << shOrder;
        std::cout << "lambda: " << lambda;
        std::cout << "B0 threshold: " << threshold;
        switch ( shOrder )
        {
        case 4:
        {
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetReferenceBValue());
            filter->SetThreshold( threshold );
            filter->SetLambda(lambda);
            filter->SetUseMrtrixBasis(mrTrix);
            if (normalization==0)
                filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
            else
                filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
            coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );
            break;
        }
        case 6:
        {
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,6,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetReferenceBValue());
            filter->SetThreshold( threshold );
            filter->SetLambda(lambda);
            filter->SetUseMrtrixBasis(mrTrix);
            if (normalization==0)
                filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
            else
                filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
            coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );
            break;
        }
        case 8:
        {
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,8,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetReferenceBValue());
            filter->SetThreshold( threshold );
            filter->SetLambda(lambda);
            filter->SetUseMrtrixBasis(mrTrix);
            if (normalization==0)
                filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
            else
                filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
            coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );
            break;
        }
        case 10:
        {
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,10,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetReferenceBValue());
            filter->SetThreshold( threshold );
            filter->SetLambda(lambda);
            filter->SetUseMrtrixBasis(mrTrix);
            if (normalization==0)
                filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
            else
                filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
            coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );
            break;
        }
        case 12:
        {
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,12,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetReferenceBValue());
            filter->SetThreshold( threshold );
            filter->SetLambda(lambda);
            if (normalization==0)
                filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
            else
                filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
            coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );
            break;
        }
        default:
        {
            std::cout << "Supplied SH order not supported. Using default order of 4.";
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetReferenceBValue());
            filter->SetThreshold( threshold );
            filter->SetLambda(lambda);
            filter->SetUseMrtrixBasis(mrTrix);
            if (normalization==0)
                filter->SetNormalizationMethod(FilterType::QBAR_STANDARD);
            else
                filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            coeffsImage->InitializeByItk( filter->GetCoefficientImage().GetPointer() );
            coeffsImage->SetVolume( filter->GetCoefficientImage()->GetBufferPointer() );
        }
        }

        std::string coeffout = outfilename;
        coeffout += "_shcoeffs.nrrd";

        outfilename += ".qbi";
        mitk::IOUtil::SaveBaseData(image, outfilename);

        if (outCoeffs)
            mitk::IOUtil::SaveImage(coeffsImage, coeffout);
    }
    catch ( itk::ExceptionObject &err)
    {
        std::cout << "Exception: " << err;
    }
    catch ( std::exception err)
    {
        std::cout << "Exception: " << err.what();
    }
    catch ( ... )
    {
        std::cout << "Exception!";
    }
    return EXIT_SUCCESS;
}
