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

using namespace mitk;
/**
 * Convert files from one ending to the other
 */
int TensorReconstruction(int argc, char* argv[])
{
    if ( argc!=4 )
    {
        std::cout << std::endl;
        std::cout << "Perform Tensor estimation on an dwi file" << std::endl;
        std::cout << std::endl;
        std::cout << "usage: " << argv[0] << " " << argv[1] << "<in-filename> <out-filename> " << std::endl;
        std::cout << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        RegisterDiffusionCoreObjectFactory();

        MITK_INFO << "Loading image ...";
        const std::string s1="", s2="";
        std::vector<BaseData::Pointer> infile = BaseDataIO::LoadBaseDataFromFile( argv[2], s1, s2, false );
        DiffusionImage<short>::Pointer dwi = dynamic_cast<DiffusionImage<short>*>(infile.at(0).GetPointer());

        std::string outfilename = argv[3];

        typedef itk::DiffusionTensor3DReconstructionImageFilter< short, short, float > TensorReconstructionImageFilterType;
        TensorReconstructionImageFilterType::Pointer filter = TensorReconstructionImageFilterType::New();
        filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
        filter->SetBValue(dwi->GetB_Value());
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
RegisterDiffusionCoreMiniApp(TensorReconstruction);
