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

#include "mitkNavigationDataSource.h"
#include "mitkNavigationDataSet.h"

namespace mitk{
  /**
  * \brief Base class for using mitk::NavigationData as a filter source.
  * Subclasses can play objects of mitk::NavigationDataSet.
  *
  * Each subclass has to check the state of m_Repeat and do or do not repeat
  * the playing accordingly.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataPlayerBase
    : public NavigationDataSource
  {
  public:
    mitkClassMacro(NavigationDataPlayerBase, NavigationDataSource)

    /**
    * \brief Set to true if the data player should repeat the outputs.
    */
    itkSetMacro(Repeat, bool)

    /**
    * \return Returns if the data player should repeat the outputs.
    */
    itkGetMacro(Repeat, bool)

    /**
    * \brief Used for pipeline update just to tell the pipeline that we always have to update.
    */
    virtual void UpdateOutputInformation();

    itkGetMacro(NavigationDataSet, NavigationDataSet::Pointer)

    /**
    * \brief Set mitk::NavigationDataSet for playing.
    * Player is initialized by call to mitk::NavigationDataPlayerBase::InitPlayer()
    * inside this method. Method must be called before this object can be used as
    * a filter source.
    *
    * @param navigationDataSet mitk::NavigationDataSet which will be played by this player.
    */
    void SetNavigationDataSet(NavigationDataSet::Pointer navigationDataSet);

    /**
    * \brief Getter for the size of the mitk::NavigationDataSet used in this object.
    *
    * @return Returns the number of navigation data snapshots available in the player.
    */
    unsigned int GetNumberOfSnapshots();

    unsigned int GetCurrentSnapshotNumber();

    /**
    * \brief This method checks if player arrived at end of file.
    *
    * @return true if last mitk::NavigationData object is in the outputs, false otherwise
    */
    bool IsAtEnd();

  protected:
    NavigationDataPlayerBase();
    virtual ~NavigationDataPlayerBase();

    /**
    * \brief Every subclass hast to implement this method. See ITK filter documentation for details.
    */
    virtual void GenerateData() = 0;

    /**
    * \brief Initializes the outputs of this NavigationDataSource.
    * Aftwer calling this method, the first Navigationdata from the loaded Navigationdataset is loaded into the outputs.
    */
    void InitPlayer();

    /**
    * \brief Convenience method for subclasses.
    * When there are no further mitk::NavigationData objects available, this
    * method can be called in the implementation of mitk::NavigationDataPlayerBase::GenerateData().
    */
    void GraftEmptyOutput();

    /**
    * \brief If the player should repeat outputs. Default is false.
    */
    bool m_Repeat;

    NavigationDataSet::Pointer m_NavigationDataSet;

    /**
    * \brief Iterator always points to the NavigationData object which is in the outputs at the moment.
    */
    mitk::NavigationDataSet::NavigationDataSetIterator m_NavigationDataSetIterator;
  };
} // namespace mitk

#endif /* MITKNavigationDataSequentialPlayer_H_HEADER_INCLUDED_ */
