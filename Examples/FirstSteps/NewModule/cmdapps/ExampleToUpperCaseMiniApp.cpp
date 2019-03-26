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

// std includes
#include <algorithm>
#include <string>

// CTK includes
#include "mitkCommandLineParser.h"

// MITK includes
#include "mitkExampleDataStructure.h"
#include <mitkIOUtil.h>

/** \brief Example MiniApp that converts a text file content to upper case
 *
 * This MiniApp provides the capability to convert a text file to content to upper case.
 * This is done loading the text file as a mitk::ExampleDataStructure, getting the string
 * content, using the std to transform it and writing it to file again.
 *
 * Supported file extensions are .txt and .example.
 *
 * You will need to specify an input and an output file path. The -v flag is optional and will
 * produce additional output.
 */

//! [main]
int main(int argc, char *argv[])
{
  //! [main]

  //! [create parser]
  mitkCommandLineParser parser;

  // set general information about your MiniApp
  parser.setCategory("MITK-Examples");
  parser.setTitle("To Upper Case");
  parser.setDescription("An example MiniApp that converts the contents of a test file to upper case.");
  parser.setContributor("MBI");
  //! [create parser]

  //! [add arguments]
  // how should arguments be prefixed
  parser.setArgumentPrefix("--", "-");
  // add each argument, unless specified otherwise each argument is optional
  // see mitkCommandLineParser::addArgument for more information
  parser.beginGroup("Required I/O parameters");
  parser.addArgument(
    "input", "i", mitkCommandLineParser::File, "Input file", "input file (.txt/.example)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output",
                     "o",
                     mitkCommandLineParser::File,
                     "Output file",
                     "where to save the output (.txt/.example)",
                     us::Any(),
                     false, false, false, mitkCommandLineParser::Output);
  parser.endGroup();

  parser.beginGroup("Optional parameters");
  parser.addArgument(
    "verbose", "v", mitkCommandLineParser::Bool, "Verbose Output", "Whether to produce verbose output");
  parser.endGroup();
  //! [add arguments]

  //! [parse the arguments]
  // parse arguments, this method returns a mapping of long argument names and their values
  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size() == 0)
    return EXIT_FAILURE;

  // parse, cast and set required arguments
  std::string inFilename = us::any_cast<std::string>(parsedArgs["input"]);
  std::string outFileName = us::any_cast<std::string>(parsedArgs["output"]);

  // default values for optional arguments
  bool verbose(false);

  // parse, cast and set optional arguments if given
  if (parsedArgs.count("verbose"))
  {
    verbose = us::any_cast<bool>(parsedArgs["verbose"]);
  }
  //! [parse the arguments]

  //! [do processing]
  try
  {
    // verbosity in this example is slightly over the top
    if (verbose)
    {
      MITK_INFO << "Trying to read file.";
    }

    std::vector<mitk::BaseData::Pointer> inVector = mitk::IOUtil::Load(inFilename);
    if (inVector.empty())
    {
      std::string errorMessage = "File at " + inFilename + " could not be read. Aborting.";
      MITK_ERROR << errorMessage;
      return EXIT_FAILURE;
    }
    mitk::BaseData *inBaseData = inVector.at(0);
    mitk::ExampleDataStructure *inExample = dynamic_cast<mitk::ExampleDataStructure *>(inBaseData);

    if (verbose)
    {
      MITK_INFO << "Converting string.";
    }
    mitk::ExampleDataStructure::Pointer outExample = mitk::ExampleDataStructure::New();
    std::string data = inExample->GetData();
    if (verbose)
    {
      MITK_INFO << "String before conversion: " << data;
    }
    std::transform(data.begin(), data.end(), data.begin(), ::toupper);
    if (verbose)
    {
      MITK_INFO << "String after conversion: " << data;
    }
    outExample->SetData(data);

    if (verbose)
    {
      MITK_INFO << "Trying to write to file.";
    }

    mitk::IOUtil::Save(outExample.GetPointer(), outFileName);

    return EXIT_SUCCESS;
  }
  //! [do processing]

  catch (itk::ExceptionObject e)
  {
    MITK_ERROR << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    MITK_ERROR << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    MITK_ERROR << "Unexpected error encountered.";
    return EXIT_FAILURE;
  }
}
