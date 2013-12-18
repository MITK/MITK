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

#ifndef mitkDICOMFileReader_h
#define mitkDICOMFileReader_h

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "DICOMReaderExports.h"

#include "mitkDICOMImageBlockDescriptor.h"

namespace mitk
{

// TODO Philips3D!
// TODO http://bugs.mitk.org/show_bug.cgi?id=11572 ?
class DICOMReader_EXPORT DICOMFileReader : public itk::LightObject
{
  public:

    enum LoadingConfidence
    {
      NoSupport = 0,
      FullSupport = 1,
      PartialSupport = 2,
    };

    mitkClassMacro( DICOMFileReader, itk::LightObject )

    void SetInputFiles(StringList filenames);
    const StringList& GetInputFiles() const;

    virtual void AnalyzeInputFiles() = 0;
    unsigned int GetNumberOfOutputs() const;
    const DICOMImageBlockDescriptor& GetOutput(unsigned int index) const;

    // void AllocateOutputImages();
    virtual bool LoadImages() = 0;

    virtual bool CanHandleFile(const std::string& filename) = 0;

    void PrintOutputs(std::ostream& os, bool filenameDetails = false);

    static bool IsDICOM(const std::string& filename);

  protected:

    DICOMFileReader();
    virtual ~DICOMFileReader();

    DICOMFileReader(const DICOMFileReader& other);
    DICOMFileReader& operator=(const DICOMFileReader& other);

    void ClearOutputs();
    void SetNumberOfOutputs(unsigned int numberOfOutputs);
    void SetOutput(unsigned int index, const DICOMImageBlockDescriptor& output);

    DICOMImageBlockDescriptor& InternalGetOutput(unsigned int index);

  private:

    StringList m_InputFilenames;
    std::vector< DICOMImageBlockDescriptor > m_Outputs;
};

}

#endif
