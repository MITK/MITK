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

#include <itkImageFileWriter.h>
#include <itkResampleImageFilter.h>

#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>

#include <mitkCommandLineParser.h>
#include <mitkLexicalCast.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <mitkIOUtil.h>
#include <mitkIPythonService.h>

typedef itk::Image< float, 3 >    ItkImageType;

/*!
\brief
*/
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");
    parser.addArgument("image", "i", mitkCommandLineParser::InputFile, "Input image", "sh coefficient image", us::Any(), false);
    parser.addArgument("script", "s", mitkCommandLineParser::InputFile, "", "", us::Any(), false);
    parser.addArgument("params", "p", mitkCommandLineParser::InputFile, "", "", us::Any(), false);
    parser.addArgument("out_file", "o", mitkCommandLineParser::OutputDirectory, "Output image", "output image", us::Any(), false);

    parser.setCategory("TEST");
    parser.setTitle("TEST");
    parser.setDescription("TEST");
    parser.setContributor("MIC");

    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    std::string image_file = us::any_cast<std::string>(parsedArgs["image"]);
    std::string script = us::any_cast<std::string>(parsedArgs["script"]);
    std::string params = us::any_cast<std::string>(parsedArgs["params"]);
    std::string out_file = us::any_cast<std::string>(parsedArgs["out_file"]);

    mitk::Image::Pointer mitk_image = mitk::IOUtil::Load<mitk::Image>(image_file);
    us::ModuleContext* context = us::GetModuleContext();

    us::ServiceReference<mitk::IPythonService> m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
    mitk::IPythonService* m_PythonService = dynamic_cast<mitk::IPythonService*> ( context->GetService<mitk::IPythonService>(m_PythonServiceRef) );

    mitk::IPythonService::ForceLoadModule();

    m_PythonService->Execute("import SimpleITK as sitk");
    m_PythonService->Execute("import SimpleITK._SimpleITK as _SimpleITK");
    m_PythonService->Execute("import numpy");

    m_PythonService->CopyToPythonAsSimpleItkImage( mitk_image, "myvar");
    m_PythonService->Execute("myparams=\""+params+"\"");
    m_PythonService->ExecuteScript(script);

    mitk::Image::Pointer out_seg = m_PythonService->CopySimpleItkImageFromPython("out_image");
    mitk::IOUtil::Save(out_seg, out_file);

    return EXIT_FAILURE;
}
