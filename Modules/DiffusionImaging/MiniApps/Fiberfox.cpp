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

#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkProperties.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkFiberBundle.h>
#include <mitkFiberfoxParameters.h>
#include "mitkCommandLineParser.h"

#include <itkAddArtifactsToDwiImageFilter.h>
#include <itkTractsToDWIImageFilter.h>
#include <boost/lexical_cast.hpp>

/** TODO: Proritype signal komplett speichern oder bild mit speichern. */
/** TODO: Tarball aus images und parametern? */
/** TODO: Artefakte auf bild in miniapp */

using namespace mitk;

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    parser.setTitle("FiberFox");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setContributor("MBI");
    parser.setDescription(" ");
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output root:", "output root", us::Any(), false);
    parser.addArgument("parameters", "p", mitkCommandLineParser::InputFile, "Parameter file:", "fiberfox parameter file", us::Any(), false);
    parser.addArgument("fiberbundle", "f", mitkCommandLineParser::String, "Fiberbundle:", "", us::Any(), false);
    parser.addArgument("verbose", "v", mitkCommandLineParser::Bool, "Output additional images:", "", false, true);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    string outName = us::any_cast<string>(parsedArgs["out"]);
    string paramName = us::any_cast<string>(parsedArgs["parameters"]);

    string fibFile = "";
    if (parsedArgs.count("fiberbundle"))
        fibFile = us::any_cast<string>(parsedArgs["fiberbundle"]);

    bool verbose = false;
    if (parsedArgs.count("verbose"))
        verbose = us::any_cast<bool>(parsedArgs["verbose"]);

    FiberfoxParameters<double> parameters;
    parameters.LoadParameters(paramName);
    parameters.m_Misc.m_OutputPath = itksys::SystemTools::GetFilenamePath(outName)+"/";

    if (verbose)
        parameters.SaveParameters(outName+".ffp");

    mitk::FiberBundle::Pointer inputTractogram = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::LoadDataNode(fibFile)->GetData());

    itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
    tractsToDwiFilter->SetParameters(parameters);
    tractsToDwiFilter->SetFiberBundle(inputTractogram);
    tractsToDwiFilter->Update();

    mitk::Image::Pointer image = mitk::GrabItkImageMemory( tractsToDwiFilter->GetOutput() );
    image->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( parameters.m_SignalGen.GetGradientDirections() ) );
    image->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( parameters.m_SignalGen.m_Bvalue ) );
    mitk::DiffusionPropertyHelper propertyHelper( image );
    propertyHelper.InitializeImage();

    mitk::IOUtil::Save(image, outName+".dwi");

    if (verbose)
    {
        std::vector< itk::TractsToDWIImageFilter< short >::ItkDoubleImgType::Pointer > volumeFractions = tractsToDwiFilter->GetVolumeFractions();
        for (unsigned int k=0; k<volumeFractions.size(); k++)
        {
            mitk::Image::Pointer image = mitk::Image::New();
            image->InitializeByItk(volumeFractions.at(k).GetPointer());
            image->SetVolume(volumeFractions.at(k)->GetBufferPointer());
            mitk::IOUtil::Save(image, outName+"_Compartment"+boost::lexical_cast<string>(k)+".nrrd");
        }

        if (tractsToDwiFilter->GetPhaseImage().IsNotNull())
        {
            mitk::Image::Pointer image = mitk::Image::New();
            itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer itkPhase = tractsToDwiFilter->GetPhaseImage();
            image = mitk::GrabItkImageMemory( itkPhase.GetPointer() );
            mitk::IOUtil::Save(image, outName+"_Phase.nrrd");
        }

        if (tractsToDwiFilter->GetKspaceImage().IsNotNull())
        {
            mitk::Image::Pointer image = mitk::Image::New();
            itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer itkImage = tractsToDwiFilter->GetKspaceImage();
            image = mitk::GrabItkImageMemory( itkImage.GetPointer() );
            mitk::IOUtil::Save(image, outName+"_kSpace.nrrd");
        }
    }

    return EXIT_SUCCESS;
}

