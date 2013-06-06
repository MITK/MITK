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

#include <mitkImageCast.h>
#include <mitkQBallImage.h>
#include <mitkTensorImage.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkFiberBundleX.h>
#include <itkGibbsTrackingFilter.h>
#include <itkDiffusionTensor3D.h>
#include <itkShCoefficientImageImporter.h>
#include <mitkImageToItk.h>

template<int shOrder>
typename itk::ShCoefficientImageImporter< float, shOrder >::QballImageType::Pointer TemplatedConvertShCoeffs(mitk::Image* mitkImg, int toolkit)
{
    typedef itk::ShCoefficientImageImporter< float, shOrder > FilterType;
    typedef mitk::ImageToItk< itk::Image< float, 4 > > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(mitkImg);
    caster->Update();

    typename FilterType::Pointer filter = FilterType::New();

    switch (toolkit)
    {
    case 0:
        filter->SetToolkit(FilterType::FSL);
        break;
    case 1:
        filter->SetToolkit(FilterType::MRTRIX);
        break;
    default:
        filter->SetToolkit(FilterType::FSL);
    }

    filter->SetInputImage(caster->GetOutput());
    filter->GenerateData();
    return filter->GetQballImage();
}

int GibbsTracking(int argc, char* argv[])
{
    if (argc<6)
    {
        std::cout << std::endl;
        std::cout << "Performes Gibbs Tracking" << std::endl;
        std::cout << std::endl;
        std::cout << "usage: " << argv[0] << " <in-filename> <parameter file name> <fib-out-filename> <mask-in-filename> <sh coefficient convention (FSL, MRtrix)>" << std::endl;
        std::cout << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        RegisterDiffusionCoreObjectFactory();
        RegisterFiberTrackingObjectFactory();

        std::cout << "input: " << argv[1] << std::endl;
        std::cout << "param: "  << argv[2] << std::endl;
        std::cout << "output: " << argv[3] << std::endl;

        // instantiate gibbs tracker
        typedef itk::Vector<float, QBALL_ODFSIZE>   OdfVectorType;
        typedef itk::Image<OdfVectorType,3>         ItkQballImageType;
        typedef itk::GibbsTrackingFilter<ItkQballImageType> GibbsTrackingFilterType;
        GibbsTrackingFilterType::Pointer gibbsTracker = GibbsTrackingFilterType::New();

        // load input image
        const std::string s1="", s2="";
        std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( argv[1], s1, s2, false );

        // try to cast to qball image
        QString inImageName(argv[1]);
        if( inImageName.endsWith(".qbi") )
        {
            MITK_INFO << "Loading qball image ...";
            mitk::QBallImage::Pointer mitkQballImage = dynamic_cast<mitk::QBallImage*>(infile.at(0).GetPointer());
            ItkQballImageType::Pointer itk_qbi = ItkQballImageType::New();
            mitk::CastToItkImage<ItkQballImageType>(mitkQballImage, itk_qbi);
            gibbsTracker->SetQBallImage(itk_qbi.GetPointer());
        }
        else if( inImageName.endsWith(".dti") )
        {
            MITK_INFO << "Loading tensor image ...";
            typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
            mitk::TensorImage::Pointer mitkTensorImage = dynamic_cast<mitk::TensorImage*>(infile.at(0).GetPointer());
            ItkTensorImage::Pointer itk_dti = ItkTensorImage::New();
            mitk::CastToItkImage<ItkTensorImage>(mitkTensorImage, itk_dti);
            gibbsTracker->SetTensorImage(itk_dti);
        }
        else if ( inImageName.endsWith(".nii") )
        {
            MITK_INFO << "Loading sh-coefficient image ...";
            mitk::Image::Pointer mitkImage = dynamic_cast<mitk::Image*>(infile.at(0).GetPointer());

            int nrCoeffs = mitkImage->GetLargestPossibleRegion().GetSize()[3];
            int c=3, d=2-2*nrCoeffs;
            double D = c*c-4*d;
            int shOrder;
            if (D>0)
            {
                shOrder = (-c+sqrt(D))/2.0;
                if (shOrder<0)
                    shOrder = (-c-sqrt(D))/2.0;
            }
            else if (D==0)
                shOrder = -c/2.0;

            MITK_INFO << "using SH-order " << shOrder;

            int toolkitConvention = 0;
            if(argc==6)
            {
                QString convention(argv[5]);
                if ( convention.compare("MRtrix", Qt::CaseInsensitive)==0 )
                {
                    toolkitConvention = 1;
                    MITK_INFO << "Using MRtrix style sh-coefficient convention";
                }
                else
                    MITK_INFO << "Using FSL style sh-coefficient convention";
            }
            else
                MITK_INFO << "Using FSL style sh-coefficient convention";

            switch (shOrder)
            {
            case 4:
                gibbsTracker->SetQBallImage(TemplatedConvertShCoeffs<4>(mitkImage, toolkitConvention));
                break;
            case 6:
                gibbsTracker->SetQBallImage(TemplatedConvertShCoeffs<6>(mitkImage, toolkitConvention));
                break;
            case 8:
                gibbsTracker->SetQBallImage(TemplatedConvertShCoeffs<8>(mitkImage, toolkitConvention));
                break;
            case 10:
                gibbsTracker->SetQBallImage(TemplatedConvertShCoeffs<10>(mitkImage, toolkitConvention));
                break;
            case 12:
                gibbsTracker->SetQBallImage(TemplatedConvertShCoeffs<12>(mitkImage, toolkitConvention));
                break;
            default:
                MITK_INFO << "SH-order " << shOrder << " not supported";
            }
        }
        else
            return EXIT_FAILURE;

        // global tracking
        if(argc>=5)
        {
            typedef itk::Image<float,3> MaskImgType;
            std::cout << "mask: " << argv[4]<< std::endl;
            infile = mitk::BaseDataIO::LoadBaseDataFromFile( argv[4], s1, s2, false );
            mitk::Image::Pointer mitkMaskImage = dynamic_cast<mitk::Image*>(infile.at(0).GetPointer());
            MaskImgType::Pointer itk_mask = MaskImgType::New();
            mitk::CastToItkImage<MaskImgType>(mitkMaskImage, itk_mask);
            gibbsTracker->SetMaskImage(itk_mask);
        }
        else{
            std::cout << "perform tracking without mask" << std::endl;
        }

        gibbsTracker->SetDuplicateImage(false);
        gibbsTracker->SetLoadParameterFile( argv[2] );
//        gibbsTracker->SetLutPath( "" );
        gibbsTracker->Update();

        mitk::FiberBundleX::Pointer mitkFiberBundle = mitk::FiberBundleX::New(gibbsTracker->GetFiberBundle());

        std::string outfilename = argv[3];
        MITK_INFO << "searching writer";
        mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
        for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
        {
            if ( (*it)->CanWriteBaseDataType(mitkFiberBundle.GetPointer()) ) {
                (*it)->SetFileName( outfilename.c_str() );
                (*it)->DoWrite( mitkFiberBundle.GetPointer() );
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
    return EXIT_SUCCESS;
}
RegisterFiberTrackingMiniApp(GibbsTracking);
