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
  * \brief Data structure which stores streams of mitk::NavigationData for
  * multiple tools.
  *
  * Use mitk::NavigationDataRecorder to create these sets easily from pipelines.
  * Use mitk::NavigationDataPlayer to stream from these sets easily.
  *
  */
  class MitkIGT_EXPORT NavigationDataSet : public BaseData
  {
  public:

    /**
    * \brief This iterator iterates over the distinct time steps in this set.
    *
    * It returns an array of the length equal to GetNumberOfTools(), containing a
    * mitk::NavigationData for each tool..
    */
    typedef std::vector< std::vector<mitk::NavigationData::Pointer> >::iterator NavigationDataSetIterator;

    mitkClassMacro(NavigationDataSet, BaseData);

    mitkNewMacro1Param(Self, unsigned int);

    /**
    * \brief Add mitk::NavigationData of the given tool to the Set.
    *
    * @param navigationDatas vector of mitk::NavigationData objects to be added. Make sure that the size of the
    * vector equals the number of tools given in the constructor
    * @return true if object was be added to the set successfully, false otherwise
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

    ///**
    //* \brief Get last mitk::Navigation object for given tool whose timestamp is less than the given timestamp.
    //* @param toolIndex Index of the tool from which mitk::NavigationData should be returned.
    //* @param timestamp Timestamp for selecting last object before.
    //* @return Last mitk::NavigationData with timestamp less than given timestamp, 0 if there is no adequate object.
    //*/
    // Method not yet supported!
    //NavigationData::Pointer GetNavigationDataBeforeTimestamp( mitk::NavigationData::TimeStampType timestamp , unsigned int toolIndex ) const;

    /**
    * \brief Returns a vector that contains all tracking data for a given tool.
    *
    * This is a relatively expensive operation, as it requires the construction of a new vector.
    *
    * @param toolIndex Index of the tool for which the stream should be returned.
    * @return Returns a vector that contains all tracking data for a given tool.
    */
    virtual std::vector< mitk::NavigationData::Pointer > GetDataStreamForTool(unsigned int toolIndex);

    /**
    * \brief Returns a vector that contains NavigationDatas for each tool for a given timestep.
    *
    * If GetNumberOFTools() equals four, then 4 NavigationDatas will be returned.
    *
    * @param index Index of the timeStep for which the datas should be returned. cannot be larger than mitk::NavigationDataSet::Size()
    * @return Returns a vector that contains all tracking data for a given tool.
    */
    virtual std::vector< mitk::NavigationData::Pointer > GetTimeStep(unsigned int index);

    /**
    * \brief Returns the number of tools for which NavigationDatas are stored in this set.
    *
    * This is always equal to the number given in the constructor of this class.
    *
    * @return the number of tools for which NavigationDatas are stored in this set.
    */
    unsigned int GetNumberOfTools();

    /**
    * \brief Returns the number of time steps stored in this NavigationDataSet.
    *
    * This is not the total number of Navigation Datas stored in this set, but the number stored for each tool.
    * i.e. the total number of NavigationDatas equals Size() * GetNumberOfTools();
    *
    * @return Returns the number of time steps stored in this NavigationDataSet.
    */
    unsigned int Size();

    /**
    * \brief Returns an iterator pointing to the first TimeStep.
    *
    * @return Returns an iterator pointing to the first TimeStep.
    */
    virtual NavigationDataSetIterator Begin();

    /**
    * \brief Returns an iterator pointing behind to the last TimeStep.
    *
    * @return Returns an iterator pointing behind to the last TimeStep.
    */
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
