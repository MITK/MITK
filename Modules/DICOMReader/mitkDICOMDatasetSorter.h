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

class DICOMReader_EXPORT DICOMDatasetSorter : public itk::LightObject
{
  public:

    mitkClassMacro( DICOMDatasetSorter, itk::LightObject )

    virtual DICOMTagList GetTagsOfInterest() = 0;

    void SetInput(DICOMDatasetList filenames);
    const DICOMDatasetList& GetInput() const;

    virtual void Sort() = 0;

    unsigned int GetNumberOfOutputs() const;
    const DICOMDatasetList& GetOutput(unsigned int index) const;
    DICOMDatasetList& GetOutput(unsigned int index);

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
