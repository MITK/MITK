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


#ifndef MITKSEGMENTATIONDATANODEREADER_H
#define MITKSEGMENTATIONDATANODEREADER_H

#include <mitkIDataNodeReader.h>

namespace mitk {

class SegmentationDataNodeReader : public mitk::IDataNodeReader
{

public:

  int Read(const std::string& fileName, mitk::DataStorage& storage);
};

}

#endif // MITKSEGMENTATIONDATANODEREADER_H
