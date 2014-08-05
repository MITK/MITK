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

#include "MitkDICOMReaderExports.h"

#include "mitkDICOMTagCache.h"

#include "mitkDICOMImageBlockDescriptor.h"

namespace mitk
{

// TODO Philips3D!
// TODO http://bugs.mitk.org/show_bug.cgi?id=11572 ?

/**
  \ingroup DICOMReaderModule
  \brief Interface for DICOM readers that produce mitk::Images.

  As described in \ref DICOMReaderModule, this class structures
  the reader's part in the process of analyzing a set of DICOM files
  and selecting the most appropriate reader.

  The overall loading process is as follows:
   - <b>Define input files</b>: a list of absolute filenames
   - <b>Analyze the potential output</b>: see what can be made of the input files, describe with DICOMImageBlockDescriptor%s
   - <b>Load pixel data</b>: an application will usually analyze files using multiple readers and only load with a single reader

  Sub-classes are required to implement a number of methods that
  reflect above structure. See mitk::DICOMITKSeriesGDCMReader for
  an example.

  To help applications in describing different readers to the user, each reader
  brings a number of methods that describe its configuration/specifics by
  means of a short label and a (longer) description.
*/
class MitkDICOMReader_EXPORT DICOMFileReader : public itk::Object
{
  public:

    mitkClassMacro( DICOMFileReader, itk::Object );

    /// Test whether a file is DICOM at all
    static bool IsDICOM(const std::string& filename);
    /// Indicate whether this reader can handle given file
    virtual bool CanHandleFile(const std::string& filename) = 0;

    /// This input files
    void SetInputFiles(StringList filenames);
    /// This input files
    const StringList& GetInputFiles() const;

    /// Analyze input files
    virtual void AnalyzeInputFiles() = 0;

    /// Number of outputs, only meaningful after calling AnalyzeInputFiles()
    unsigned int GetNumberOfOutputs() const;
    /// Individual outputs, only meaningful after calling AnalyzeInputFiles(). \throws std::invalid_argument
    const DICOMImageBlockDescriptor& GetOutput(unsigned int index) const;

    // void AllocateOutputImages(); TODO for later implementation of slice-by-slice loading

    /// Load the mitk::Image%s in our outputs, the DICOMImageBlockDescriptor. To be called only after AnalyzeInputFiles(). Take care of potential exceptions!
    virtual bool LoadImages() = 0;

    virtual DICOMTagList GetTagsOfInterest() const = 0;

    /// A way to provide external knowledge about files and tag values is appreciated.
    virtual void SetTagCache(DICOMTagCache::Pointer) = 0;

    /// Short label/name to describe this reader
    void SetConfigurationLabel(const std::string&);
    /// Short label/name to describe this reader
    std::string GetConfigurationLabel() const;
    /// One-sentence description of the reader's loading "strategy"
    void SetConfigurationDescription(const std::string&);
    /// One-sentence description of the reader's loading "strategy"
    std::string GetConfigurationDescription() const;

    /// Print configuration description to given stream, for human reader
    void PrintConfiguration(std::ostream& os) const;
    /// Print output description to given stream, for human reader
    void PrintOutputs(std::ostream& os, bool filenameDetails = false) const;

    virtual bool operator==(const DICOMFileReader& other) const = 0;

  protected:

    DICOMFileReader();
    virtual ~DICOMFileReader();

    DICOMFileReader(const DICOMFileReader& other);
    DICOMFileReader& operator=(const DICOMFileReader& other);

    void ClearOutputs();
    void SetNumberOfOutputs(unsigned int numberOfOutputs);
    void SetOutput(unsigned int index, const DICOMImageBlockDescriptor& output);

    /// non-const access to the DICOMImageBlockDescriptor
    DICOMImageBlockDescriptor& InternalGetOutput(unsigned int index);

    /// Configuration description for human reader, to be implemented by sub-classes
    virtual void InternalPrintConfiguration(std::ostream& os) const = 0;

  private:

    StringList m_InputFilenames;
    std::vector< DICOMImageBlockDescriptor > m_Outputs;

    std::string m_ConfigLabel;
    std::string m_ConfigDescription;
};

}

#endif
