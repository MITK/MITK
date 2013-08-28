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
#include <mitkIOUtil.h>
#include "ctkCommandLineParser.h"
#include <boost/algorithm/string.hpp>

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
    ctkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", ctkCommandLineParser::String, "input image (tensor, Q-ball or FSL/MRTrix SH-coefficient image)", us::Any(), false);
    parser.addArgument("parameters", "p", ctkCommandLineParser::String, "parameter file (.gtp)", us::Any(), false);
    parser.addArgument("mask", "m", ctkCommandLineParser::String, "binary mask image");
    parser.addArgument("shConvention", "s", ctkCommandLineParser::String, "sh coefficient convention (FSL, MRtrix)", string("FSL"), true);
    parser.addArgument("outFile", "o", ctkCommandLineParser::String, "output fiber bundle (.fib)", us::Any(), false);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    string inFileName = us::any_cast<string>(parsedArgs["input"]);
    string paramFileName = us::any_cast<string>(parsedArgs["parameters"]);
    string outFileName = us::any_cast<string>(parsedArgs["outFile"]);

    try
    {
        RegisterDiffusionCoreObjectFactory();
        RegisterFiberTrackingObjectFactory();

        // instantiate gibbs tracker
        typedef itk::Vector<float, QBALL_ODFSIZE>   OdfVectorType;
        typedef itk::Image<OdfVectorType,3>         ItkQballImageType;
        typedef itk::GibbsTrackingFilter<ItkQballImageType> GibbsTrackingFilterType;
        GibbsTrackingFilterType::Pointer gibbsTracker = GibbsTrackingFilterType::New();

        // load input image
        const std::string s1="", s2="";
        std::vector<mitk::BaseData::Pointer> infile = mitk::BaseDataIO::LoadBaseDataFromFile( inFileName, s1, s2, false );

        // try to cast to qball image
        if( boost::algorithm::ends_with(inFileName, ".qbi") )
        {
            MITK_INFO << "Loading qball image ...";
            mitk::QBallImage::Pointer mitkQballImage = dynamic_cast<mitk::QBallImage*>(infile.at(0).GetPointer());
            ItkQballImageType::Pointer itk_qbi = ItkQballImageType::New();
            mitk::CastToItkImage<ItkQballImageType>(mitkQballImage, itk_qbi);
            gibbsTracker->SetQBallImage(itk_qbi.GetPointer());
        }
        else if( boost::algorithm::ends_with(inFileName, ".dti") )
        {
            MITK_INFO << "Loading tensor image ...";
            typedef itk::Image< itk::DiffusionTensor3D<float>, 3 >    ItkTensorImage;
            mitk::TensorImage::Pointer mitkTensorImage = dynamic_cast<mitk::TensorImage*>(infile.at(0).GetPointer());
            ItkTensorImage::Pointer itk_dti = ItkTensorImage::New();
            mitk::CastToItkImage<ItkTensorImage>(mitkTensorImage, itk_dti);
            gibbsTracker->SetTensorImage(itk_dti);
        }
        else if ( boost::algorithm::ends_with(inFileName, ".nii") )
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

            if (parsedArgs.count("shConvention"))
            {
                string convention = us::any_cast<string>(parsedArgs["shConvention"]).c_str();

                if ( boost::algorithm::equals(convention, "MRtrix") )
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
        if (parsedArgs.count("mask"))
        {
            typedef itk::Image<float,3> MaskImgType;
            mitk::Image::Pointer mitkMaskImage = mitk::IOUtil::LoadImage(us::any_cast<string>(parsedArgs["mask"]));
            MaskImgType::Pointer itk_mask = MaskImgType::New();
            mitk::CastToItkImage<MaskImgType>(mitkMaskImage, itk_mask);
            gibbsTracker->SetMaskImage(itk_mask);
        }

        gibbsTracker->SetDuplicateImage(false);
        gibbsTracker->SetLoadParameterFile( paramFileName );
//        gibbsTracker->SetLutPath( "" );
        gibbsTracker->Update();

        mitk::FiberBundleX::Pointer mitkFiberBundle = mitk::FiberBundleX::New(gibbsTracker->GetFiberBundle());

        mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
        for (mitk::CoreObjectFactory::FileWriterList::iterator it = fileWriters.begin() ; it != fileWriters.end() ; ++it)
        {
            if ( (*it)->CanWriteBaseDataType(mitkFiberBundle.GetPointer()) ) {
                (*it)->SetFileName( outFileName.c_str() );
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
    return EXIT_SUCCESS;
}
RegisterDiffusionMiniApp(GibbsTracking);
