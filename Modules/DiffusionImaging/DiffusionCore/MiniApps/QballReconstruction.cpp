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

using namespace mitk;
/**
 * Convert files from one ending to the other
 */
int QballReconstruction(int argc, char* argv[])
{

    if ( argc!=5
         )
    {
        std::cout << std::endl;
        std::cout << "Perform QBall reconstruction on an dwi file" << std::endl;
        std::cout << std::endl;
        std::cout << "usage: " << argv[0] << " <in-file> <lambda> <baseline-threshold> <out-file> " << std::endl;
        std::cout << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        RegisterDiffusionCoreObjectFactory();

        MITK_INFO << "Loading image ...";
        const std::string s1="", s2="";
        std::vector<BaseData::Pointer> infile = BaseDataIO::LoadBaseDataFromFile( argv[1], s1, s2, false );
        DiffusionImage<short>::Pointer dwi = dynamic_cast<DiffusionImage<short>*>(infile.at(0).GetPointer());

        typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,4,QBALL_ODFSIZE> FilterType;

        dwi->AverageRedundantGradients(0.001);

        FilterType::Pointer filter = FilterType::New();
        filter->SetGradientImage( dwi->GetDirections(), dwi->GetVectorImage() );
        filter->SetBValue(dwi->GetB_Value());
        filter->SetThreshold( boost::lexical_cast<int>(argv[3]) );
        filter->SetLambda(boost::lexical_cast<float>(argv[2]));
        filter->SetNormalizationMethod(FilterType::QBAR_SOLID_ANGLE);
        filter->Update();

        mitk::QBallImage::Pointer image = mitk::QBallImage::New();
        image->InitializeByItk( filter->GetOutput() );
        image->SetVolume( filter->GetOutput()->GetBufferPointer() );

        std::string outfilename = argv[4];
        MITK_INFO << "writing image " << outfilename;
        mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
        for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
        {
            if ( (*it)->CanWriteBaseDataType(image.GetPointer()) ) {
                (*it)->SetFileName( outfilename.c_str() );
                (*it)->DoWrite( image.GetPointer() );
            }
        }
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
