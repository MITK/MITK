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

#include "mitkTestingMacros.h"

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
#include <itkVectorImage.h>
#include <itkResampleImageFilter.h>

#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionImage.h>
#include <mitkQBallImage.h>
#include <mitkBaseData.h>
#include <QmitkCommonFunctionality.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkFiberBundleX.h>

/**
 * Short program to average redundant gradients in dwi-files
 */

mitk::FiberBundleX::Pointer LoadFib(std::string filename)
{
    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( filename, s1, s2, false );
    if( fibInfile.empty() )
        MITK_INFO << "File " << filename << " could not be read!";

    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    return dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
}

int mitkFiberProcessing(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkFiberBundleXTest");

    if ( argc<6 )
    {
        std::cout << argv[0] << " <input file> <point distance> <min. length threshold mm> <curvature threshold mm (radius)> <mirror SCT> <output file>" << std::endl;
        std::cout << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        RegisterDiffusionCoreObjectFactory();
        RegisterFiberTrackingObjectFactory();

        mitk::FiberBundleX::Pointer fib = LoadFib(argv[1]);
        int pointDist = QString(argv[2]).toFloat();
        float lenThres = QString(argv[3]).toFloat();
        float curvThres = QString(argv[4]).toFloat();

        std::string outfilename;
        if (argc==7)
            outfilename = argv[6];
        else
            outfilename = argv[1];

        if (lenThres>0)
            fib->RemoveShortFibers(lenThres);

        if (curvThres>0)
            fib->ApplyCurvatureThreshold(curvThres, false);

        if (pointDist>0)
            fib->ResampleFibers(pointDist);

        int axis = QString(argv[5]).toInt();
        if (axis/100==1)
            fib->MirrorFibers(0);

        if ((axis%100)/10==1)
            fib->MirrorFibers(1);

        if (axis%10==1)
            fib->MirrorFibers(2);

        mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
        for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
        {
            if ( (*it)->CanWriteBaseDataType(fib.GetPointer()) ) {
                MITK_INFO << "writing " << outfilename;
                (*it)->SetFileName( outfilename.c_str() );
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

    MITK_TEST_END();
}
