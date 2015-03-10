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

#include "mitkImage.h"
#include <mitkImageCast.h>
#include "mitkBaseData.h"
#include <mitkDiffusionPropertyHelper.h>

#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkDiffusionTensor3D.h>
#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include "mitkCommandLineParser.h"
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>

using namespace mitk;
/**
 * Convert files from one ending to the other
 */
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input file", "input raw dwi (.dwi or .fsl/.fslgz)", us::Any(), false);
    parser.addArgument("outFile", "o", mitkCommandLineParser::OutputFile, "Output file", "output file", us::Any(), false);
    parser.addArgument("b0Threshold", "t", mitkCommandLineParser::Int, "b0 threshold", "baseline image intensity threshold", 0, true);

    parser.setCategory("Preprocessing Tools");
    parser.setTitle("Tensor Reconstruction");
    parser.setDescription("");
    parser.setContributor("MBI");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    std::string inFileName = us::any_cast<string>(parsedArgs["input"]);
    std::string outfilename = us::any_cast<string>(parsedArgs["outFile"]);
    outfilename = itksys::SystemTools::GetFilenamePath(outfilename)+"/"+itksys::SystemTools::GetFilenameWithoutExtension(outfilename);
    outfilename += ".dti";

    int threshold = 0;
    if (parsedArgs.count("b0Threshold"))
        threshold = us::any_cast<int>(parsedArgs["b0Threshold"]);

    try
    {
        Image::Pointer dwi = IOUtil::LoadImage(inFileName);

        mitk::DiffusionPropertyHelper::ImageType::Pointer itkVectorImagePointer = mitk::DiffusionPropertyHelper::ImageType::New();
        mitk::CastToItkImage(dwi, itkVectorImagePointer);

        typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, float > TensorReconstructionImageFilterType;
        TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
        filter->SetGradientImage( mitk::DiffusionPropertyHelper::GetGradientContainer(dwi), itkVectorImagePointer );
        filter->SetBValue( mitk::DiffusionPropertyHelper::GetReferenceBValue( dwi ));
        filter->SetThreshold(threshold);
        filter->Update();

        // Save tensor image
        itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
        io->SetFileType( itk::ImageIOBase::Binary );
        io->UseCompressionOn();

        itk::ImageFileWriter< itk::Image< itk::DiffusionTensor3D< float >, 3 > >::Pointer writer = itk::ImageFileWriter< itk::Image< itk::DiffusionTensor3D< float >, 3 > >::New();
        writer->SetInput(filter->GetOutput());
        writer->SetFileName(outfilename);
        writer->SetImageIO(io);
        writer->UseCompressionOn();
        writer->Update();
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
