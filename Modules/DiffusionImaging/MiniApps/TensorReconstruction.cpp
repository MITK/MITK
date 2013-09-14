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
#include "mitkBaseDataIOFactory.h"
#include "mitkDiffusionImage.h"
#include "mitkBaseData.h"
#include "mitkDiffusionCoreObjectFactory.h"

#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkDiffusionTensor3D.h>
#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include "ctkCommandLineParser.h"
#include <itksys/SystemTools.hxx>

using namespace mitk;
/**
 * Convert files from one ending to the other
 */
int TensorReconstruction(int argc, char* argv[])
{
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", ctkCommandLineParser::String, "input raw dwi (.dwi or .fsl/.fslgz)", us::Any(), false);
    parser.addArgument("outFile", "o", ctkCommandLineParser::String, "output file", us::Any(), false);
    parser.addArgument("b0Threshold", "t", ctkCommandLineParser::Int, "baseline image intensity threshold", 0, true);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    std::string inFileName = us::any_cast<string>(parsedArgs["input"]);
    std::string outfilename = us::any_cast<string>(parsedArgs["outFile"]);
    outfilename = itksys::SystemTools::GetFilenameWithoutExtension(outfilename);
    outfilename += ".dti";

    int threshold = 0;
    if (parsedArgs.count("b0Threshold"))
        threshold = us::any_cast<int>(parsedArgs["b0Threshold"]);

    try
    {
        RegisterDiffusionCoreObjectFactory();

        MITK_INFO << "Loading image ...";
        const std::string s1="", s2="";
        std::vector<BaseData::Pointer> infile = BaseDataIO::LoadBaseDataFromFile( inFileName, s1, s2, false );
        DiffusionImage<short>::Pointer dwi = dynamic_cast<DiffusionImage<short>*>(infile.at(0).GetPointer());

        MITK_INFO << "B0 threshold: " << threshold;
        typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, float > TensorReconstructionImageFilterType;
        TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
        filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
        filter->SetBValue(dwi->GetB_Value());
        filter->SetThreshold(threshold);
        filter->Update();

        // Save tensor image
        MITK_INFO << "writing image " << outfilename;
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
        MITK_INFO << "Exception: " << err;
    }
    catch ( std::exception err)
    {
        MITK_INFO << "Exception: " << err.what();
    }
    catch ( ... )
    {
        MITK_INFO << "Exception!";
    }
    return EXIT_SUCCESS;

}
RegisterDiffusionMiniApp(TensorReconstruction);
