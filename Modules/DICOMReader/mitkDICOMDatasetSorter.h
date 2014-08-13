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

#ifndef mitkDICOMDatasetSorter_h
#define mitkDICOMDatasetSorter_h

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "mitkDICOMDatasetAccess.h"

namespace mitk
{

/**
  \ingroup DICOMReaderModule
  \brief The sorting/splitting building-block of DICOMITKSeriesGDCMReader.

  This class describes the interface of the sorting/splitting process
  described as part of DICOMITKSeriesGDCMReader::AnalyzeInputFiles()
  (see \ref DICOMITKSeriesGDCMReader_LoadingStrategy).

  The prodecure is simple:
   - take a list of input datasets (DICOMDatasetAccess)
   - sort them (to be defined by sub-classes, based on specific tags)
   - return the sorting result as outputs (the single input might be distributed into multiple outputs)

  The simplest and most generic form of sorting is implemented in
  sub-class DICOMTagBasedSorter.
*/
class MitkDICOMReader_EXPORT DICOMDatasetSorter : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMDatasetSorter, itk::LightObject )

    /**
      \brief Return the tags of interest (to facilitate scanning)
    */
    virtual DICOMTagList GetTagsOfInterest() = 0;

    /// \brief Input for sorting
    void SetInput(DICOMDatasetList filenames);
    /// \brief Input for sorting
    const DICOMDatasetList& GetInput() const;

    /// \brief Sort input datasets into one or multiple outputs.
    virtual void Sort() = 0;

    /// \brief Output of the sorting process.
    unsigned int GetNumberOfOutputs() const;
    /// \brief Output of the sorting process.
    const DICOMDatasetList& GetOutput(unsigned int index) const;
    /// \brief Output of the sorting process.
    DICOMDatasetList& GetOutput(unsigned int index);

    /// \brief Print configuration details into stream.
    virtual void PrintConfiguration(std::ostream& os, const std::string& indent = "") const = 0;

    virtual bool operator==(const DICOMDatasetSorter& other) const = 0;

  protected:

    DICOMDatasetSorter();
    virtual ~DICOMDatasetSorter();

    DICOMDatasetSorter(const DICOMDatasetSorter& other);
    DICOMDatasetSorter& operator=(const DICOMDatasetSorter& other);

    void ClearOutputs();
    void SetNumberOfOutputs(unsigned int numberOfOutputs);
    void SetOutput(unsigned int index, const DICOMDatasetList& output);

  private:

    DICOMDatasetList m_Input;
    std::vector< DICOMDatasetList > m_Outputs;
};

}

#endif
