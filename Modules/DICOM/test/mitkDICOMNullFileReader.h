/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
