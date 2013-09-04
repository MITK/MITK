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


#ifndef MITKNavigationDataPlayerBase_H_HEADER_INCLUDED_
#define MITKNavigationDataPlayerBase_H_HEADER_INCLUDED_

#include <mitkNavigationDataSource.h>
#include "mitkNavigationDataReaderInterface.h"
#include "tinyxml.h"


namespace mitk{

  /**Documentation
  * \brief This class is a slightly changed reimplementation of the
  * NavigationDataPlayer which does not care about timestamps and just
  * outputs the navigationdatas in their sequential order
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataPlayerBase
    : public NavigationDataSource
  {
  public:
    mitkClassMacro(NavigationDataPlayerBase, NavigationDataSource);

    /**
    * \brief Used for pipeline update just to tell the pipeline that we always have to update
    */
    virtual void UpdateOutputInformation();

    itkGetMacro(NavigationDataSet, NavigationDataSet::Pointer);
    void SetNavigationDataSet(NavigationDataSet::Pointer navigationDataSet);

    /**
     * @return Returns the number of navigation data snapshots available in the player
     */
    unsigned int GetNumberOfSnapshots();

    /**
     * \brief This method checks if player arrived at end of file.
     *
     */
    virtual bool IsAtEnd() = 0;

  protected:
    NavigationDataPlayerBase();
    virtual ~NavigationDataPlayerBase();

    virtual void GenerateData() = 0;

    /**
     * \brief Initializes the outputs of this NavigationDataSource.
     */
    void InitPlayer();

    /**
      *
      */
    void GraftEmptyOutput();

    NavigationDataSet::Pointer m_NavigationDataSet;
    mitk::NavigationDataSet::NavigationDataSetIterator m_NavigationDataSetIterator;

    unsigned int m_NumberOfOutputs; ///< stores the number of outputs known from NavigationDataSet

    /**
    * \brief Creates NavigationData from XML element and returns it
    * @throw mitk::Exception Throws an exception if elem is NULL.
    */
    mitk::NavigationData::Pointer ReadNavigationData(TiXmlElement* elem);
  };
} // namespace mitk

#endif /* MITKNavigationDataSequentialPlayer_H_HEADER_INCLUDED_ */
