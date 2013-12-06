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

#ifndef mitkDICOMTagBasedSorter_h
#define mitkDICOMTagBasedSorter_h

#include "mitkDICOMDatasetSorter.h"

namespace mitk
{

/**
  \brief sort files based on filename (last resort).
*/
class DICOMReader_EXPORT DICOMTagBasedSorter : public DICOMDatasetSorter
{
  public:

    mitkClassMacro( DICOMTagBasedSorter, DICOMDatasetSorter )
    itkNewMacro( DICOMTagBasedSorter )

    void AddDistinguishingTag( const DICOMTag& );
    void AddSortCriterion( const DICOMTag& );

    virtual DICOMTagList GetTagsOfInterest();

    virtual void Sort();

  protected:

    DICOMTagBasedSorter();
    virtual ~DICOMTagBasedSorter();

    DICOMTagBasedSorter(const DICOMTagBasedSorter& other);
    DICOMTagBasedSorter& operator=(const DICOMTagBasedSorter& other);

    std::string BuildGroupID( DICOMDatasetAccess* dataset );

    void SplitGroups();
    void SortGroups();

    DICOMTagList m_DistinguishingTags;
    DICOMTagList m_SortCriteria;
};

}

#endif
