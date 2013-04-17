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


#include "mitkFileReaderInterface.h"

mitk::FileReaderInterface::~FileReaderInterface()
{

}

const std::string mitk::FileReaderInterface::US_EXTENSION = "org.mitk.services.FileReader.Extension";

/**
* @brief Used to indicate that an image should be read as an binary image.
*/
const std::string mitk::FileReaderInterface::OPTION_READ_AS_BINARY = "org.mitk.services.FileReader.Option.ReadAsBinary";
