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

#ifndef mitkDICOMNullFileReader_h
#define mitkDICOMNullFileReader_h

#include "mitkDICOMFileReader.h"

namespace mitk
{

class DICOMNullFileReader : public DICOMFileReader
{
  public:

    mitkClassMacro( DICOMNullFileReader, DICOMFileReader );
    mitkCloneMacro( DICOMNullFileReader );
    itkNewMacro( DICOMNullFileReader );

    void AnalyzeInputFiles() override;

    // void AllocateOutputImages();
    bool LoadImages() override;

    bool CanHandleFile(const std::string& filename) override;

    bool operator==(const DICOMFileReader& other) const override;

    DICOMTagPathList GetTagsOfInterest() const override { return DICOMTagPathList(); }
    void SetTagCache( const DICOMTagCache::Pointer& ) override {}

  protected:

    DICOMNullFileReader();
    ~DICOMNullFileReader() override;

    DICOMNullFileReader(const DICOMNullFileReader& other);
    DICOMNullFileReader& operator=(const DICOMNullFileReader& other);

    void InternalPrintConfiguration(std::ostream& os) const override;

  private:
};

}

#endif
