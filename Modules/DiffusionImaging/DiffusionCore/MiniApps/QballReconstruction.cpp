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
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

#include <itkImage.h>
#include <itkImageFileReader.h>

#include <itkExceptionObject.h>
#include <itkImageFileWriter.h>

#include <itkMetaDataObject.h>

#include "itkVectorContainer.h"
#include "vnl/vnl_vector_fixed.h"
#include "itkVectorImage.h"

#include "mitkBaseDataIOFactory.h"
#include "mitkDiffusionImage.h"
#include "mitkBaseData.h"
#include "mitkDiffusionCoreObjectFactory.h"
#include "mitkCoreObjectFactory.h"
#include "mitkCoreExtObjectFactory.h"
#include "mitkImageWriter.h"
#include "itkAnalyticalDiffusionQballReconstructionImageFilter.h"
#include <boost/lexical_cast.hpp>
#include <mitkNrrdQBallImageWriter.h>

using namespace mitk;
/**
 * Convert files from one ending to the other
 */
int QballReconstruction(int argc, char* argv[])
{

    if ( argc<6 )
    {
        std::cout << std::endl;
        std::cout << "Perform QBall reconstruction on an dwi file" << std::endl;
        std::cout << std::endl;
        std::cout << "usage: " << argv[0] << " " << argv[1] << " <in-file> <lambda> <baseline-threshold> <out-file> <sh order>" << std::endl;
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
        dwi->AverageRedundantGradients(0.001);

        mitk::QBallImage::Pointer image = mitk::QBallImage::New();


        int shOrder = 4;
        if ( argc>6 )
            shOrder = boost::lexical_cast<int>(argv[6]);
        MITK_INFO << "Using SH order of " << shOrder;
        switch ( shOrder )
        {
        case 4:
        {
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetB_Value());
            filter->SetThreshold( boost::lexical_cast<int>(argv[4]) );
            filter->SetLambda(boost::lexical_cast<float>(argv[3]));
            filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            break;
        }
        case 6:
        {
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,6,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetB_Value());
            filter->SetThreshold( boost::lexical_cast<int>(argv[4]) );
            filter->SetLambda(boost::lexical_cast<float>(argv[3]));
            filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            break;
        }
        case 8:
        {
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,8,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetB_Value());
            filter->SetThreshold( boost::lexical_cast<int>(argv[4]) );
            filter->SetLambda(boost::lexical_cast<float>(argv[3]));
            filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            break;
        }
        case 10:
        {
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,10,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetB_Value());
            filter->SetThreshold( boost::lexical_cast<int>(argv[4]) );
            filter->SetLambda(boost::lexical_cast<float>(argv[3]));
            filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
            break;
        }
        default:
        {
            MITK_INFO << "Supplied SH order not supported. Using default order of 4.";
            typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> FilterType;
            FilterType::Pointer filter = FilterType::New();
            filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
            filter->SetBValue(dwi->GetB_Value());
            filter->SetThreshold( boost::lexical_cast<int>(argv[4]) );
            filter->SetLambda(boost::lexical_cast<float>(argv[3]));
            filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
            filter->Update();
            image->InitializeByItk( filter->GetOutput() );
            image->SetVolume( filter->GetOutput()->GetBufferPointer() );
        }
        }

        std::string outfilename = argv[5];
        MITK_INFO << "writing image " << outfilename;
        mitk::NrrdQBallImageWriter::Pointer writer = mitk::NrrdQBallImageWriter::New();
        writer->SetInput(image.GetPointer());
        writer->SetFileName(outfilename.c_str());
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
RegisterDiffusionCoreMiniApp(QballReconstruction);
