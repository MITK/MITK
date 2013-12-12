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

#ifndef MITKDIFFUSIONDICOMFILEREADER_H
#define MITKDIFFUSIONDICOMFILEREADER_H

#include "DiffusionCoreExports.h"

#include "mitkDICOMITKSeriesGDCMReader.h"

namespace mitk
{

class DiffusionCore_EXPORT DiffusionDICOMFileReader
        : public DICOMITKSeriesGDCMReader
{
public:

  mitkClassMacro( DiffusionDICOMFileReader, DICOMITKSeriesGDCMReader )
  mitkCloneMacro( DiffusionDICOMFileReader )

  itkNewMacro( DiffusionDICOMFileReader )

  virtual void AnalyzeInputFiles();

  virtual bool LoadImages();

  virtual bool CanHandleFile(const std::string &filename);

protected:
    DiffusionDICOMFileReader();
    virtual ~DiffusionDICOMFileReader();
};

}

#endif // MITKDIFFUSIONDICOMFILEREADER_H
