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

#ifndef MITKNAVIGATIONDATASET_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATASET_H_HEADER_INCLUDED_

#include <MitkIGTExports.h>
#include "mitkBaseData.h"
#include "mitkNavigationData.h"

namespace mitk {
  /**
  * \brief Data structure which stores sets of mitk::NavigationData for
  * multiple tools.
  *
  *
  */
  class MitkIGT_EXPORT NavigationDataSet : public BaseData
  {
  public:

    typedef std::vector< std::vector<mitk::NavigationData::Pointer> >::iterator NavigationDataSetIterator;

    mitkClassMacro(NavigationDataSet, BaseData);

    mitkNewMacro1Param(Self, unsigned int);

    /**
    * \brief Add mitk::NavigationData of the given tool to the Set.
    *
    * @param toolIndex Index of the tool for which mitk::NavigationData should be added.
    * @param navigationData mitk::NavigationData object to be added.
    * @return true if object could be added to the set, false otherwise (e.g. tool with given index not existing)
    */
    bool AddNavigationDatas( std::vector<mitk::NavigationData::Pointer> navigationDatas );

    /**
    * \brief Get mitk::NavigationData from the given tool at given index.
    *
    * @param toolIndex Index of the tool from which mitk::NavigationData should be returned.
    * @param index Index of the mitk::NavigationData object that should be returned.
    * @return mitk::NavigationData at the specified indices, 0 if there is no object at the indices.
    */
    NavigationData::Pointer GetNavigationDataForIndex( unsigned int index, unsigned int toolIndex ) const;

    /**
    * \brief Get last mitk::Navigation object for given tool whose timestamp is less than the given timestamp.
    * @param toolIndex Index of the tool from which mitk::NavigationData should be returned.
    * @param timestamp Timestamp for selecting last object before.
    * @return Last mitk::NavigationData with timestamp less than given timestamp, 0 if there is no adequate object.
    */
    NavigationData::Pointer GetNavigationDataBeforeTimestamp( mitk::NavigationData::TimeStampType timestamp , unsigned int toolIndex ) const;

    unsigned int GetNumberOfTools();
    unsigned int GetNumberOfNavigationDatas(bool check = true);

    virtual NavigationDataSetIterator Begin();
    virtual NavigationDataSetIterator End();

    // virtual methods, that need to be implemented, but aren't reasonable for NavigationData
    virtual void SetRequestedRegionToLargestPossibleRegion( );
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion( );
    virtual bool VerifyRequestedRegion( );
    virtual void SetRequestedRegion( const itk::DataObject *data );

  protected:
    /**
    * \brief Constructs set with fixed number of tools.
    * @param numTools How many tools are used with this mitk::NavigationDataSet.
    */
    NavigationDataSet( unsigned int numTools );
    virtual ~NavigationDataSet( );

    /**
    * \brief Holds all the mitk::NavigationData objects managed by this class.
    *
    * The first dimension is the index of the navigation data, the second is the
    * tool to which this data belongs. i.e. the first dimension is usually the longer one.
    */
    std::vector<std::vector<NavigationData::Pointer> > m_NavigationDataVectors;

    /**
    * \brief The Number of Tools that this class is going to support.
    */
    int m_NumberOfTools;
  };
}

#endif // MITKNAVIGATIONDATASET_H_HEADER_INCLUDED_
