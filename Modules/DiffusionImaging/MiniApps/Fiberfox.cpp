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
#include <mitkDiffusionImage.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkIOUtil.h>
#include <mitkFiberBundleX.h>
#include <mitkFiberfoxParameters.h>
#include "mitkCommandLineParser.h"

#include <itkAddArtifactsToDwiImageFilter.h>
#include <itkTractsToDWIImageFilter.h>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/foreach.hpp"

/** TODO: Proritype signal komplett speichern oder bild mit speichern. */
/** TODO: Tarball aus images und parametern? */
/** TODO: Artefakte auf bild in miniapp */

using namespace mitk;

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output root:", "output root", us::Any(), false);
    parser.addArgument("parameters", "p", mitkCommandLineParser::InputFile, "Parameter file:", "fiberfox parameter file", us::Any(), false);
    parser.addArgument("fiberbundle", "f", mitkCommandLineParser::String, "Fiberbundle:", "", us::Any(), false);

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    string outName = us::any_cast<string>(parsedArgs["out"]);
    string paramName = us::any_cast<string>(parsedArgs["parameters"]);

    string fibFile = "";
    if (parsedArgs.count("fiberbundle"))
        fibFile = us::any_cast<string>(parsedArgs["fiberbundle"]);

    {
        FiberfoxParameters<double> parameters;
        parameters.LoadParameters(paramName);

        mitk::FiberBundleX::Pointer inputTractogram = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(fibFile)->GetData());

        itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
        tractsToDwiFilter->SetParameters(parameters);
        tractsToDwiFilter->SetFiberBundle(inputTractogram);
        tractsToDwiFilter->Update();

        DiffusionImage<short>::Pointer image = DiffusionImage<short>::New();
        image->SetVectorImage( tractsToDwiFilter->GetOutput() );
        image->SetReferenceBValue( parameters.m_SignalGen.m_Bvalue );
        image->SetDirections( parameters.m_SignalGen.GetGradientDirections() );
        image->InitializeFromVectorImage();

        mitk::IOUtil::Save(image, outName.c_str());
    }
    return EXIT_SUCCESS;
}

