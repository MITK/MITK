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


#ifndef MITKNavigationDataToPointSetPlayer_H_HEADER_INCLUDED_
#define MITKNavigationDataToPointSetPlayer_H_HEADER_INCLUDED_

#include <mitkNavigationDataPlayerBase.h>
#include <mitkNavigationDataToPointSetFilter.h>
#include <mitkPointSet.h>
#include "tinyxml.h"


namespace mitk
{

  /**Documentation
  * \brief This class is a slightly changed reimplementation of the
  * NavigationDataPlayer which does not care about timestamps and just
  * outputs the navigationdatas in their sequential order
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataToPointSetPlayer
    : public NavigationDataPlayerBase
  {
  public:
    mitkClassMacro(NavigationDataToPointSetPlayer, NavigationDataPlayerBase);
    itkNewMacro(Self);

    /**
    * \brief sets the file name and path (if XMLString is set, this is neglected)
    */
    void SetFileName(const std::string& _FileName);

    /**
    * \brief returns the file name and path
    */
    itkGetStringMacro(FileName);

    /**
    * \brief sets a xml string (by this, the xml string is not read from file)
    */
    void SetXMLString(const std::string& _XMLString);

    /**
    * \brief returns the current xml string
    */
    itkGetStringMacro(XMLString);

    ///
    /// set to true if the data player should repeat the outputs
    ///
    itkSetMacro(Repeat, bool);
    ///
    /// set if the data player should repeat the outputs
    ///
    itkGetMacro(Repeat, bool);
    ///
    /// \return the number of navigation data snapshots available in the file
    ///
    itkGetMacro(NumberOfSnapshots, unsigned int);

    ///
    /// advance the output to the i-th snapshot
    /// e.g. if you want to have the NavData of snapshot
    /// 17 then you can call GoToSnapshot(17). index begins at 1!
    /// you can then also go back to snapshot 1 with GoToSnapshot(1)
    ///
    void GoToSnapshot(int i);

    /**
    * \brief Used for pipeline update just to tell the pipeline
    * that we always have to update
    */
    virtual void UpdateOutputInformation();



    /**
    * \brief This method starts the player.
    *
    * Before the stream has to be set. Either with a PlayingMode (SetStream(PlayerMode)) and FileName. Or
    * with an own inputstream (SetStream(istream*)).
    */
    void StartPlaying();

    /**
    * \brief Stops the player and closes the stream. After a call of StopPlaying()
    * StartPlaying() must be called to get new output data
    *
    * \warning the output is generated in this method because we know first about the number of output after
    * reading the first lines of the XML file. Therefore you should assign your output after the call of this method
    */
    void StopPlaying();

    /**
    * \brief This method pauses the player. If you want to play again call Resume()
    *
    *\warning This method is not tested yet. It is not save to use!
    */
    void Pause();

    /**
    * \brief This method resumes the player when it was paused.
    *
    *\warning This method is not tested yet. It is not save to use!
    */
    void Resume();







    const bool IsAtEnd();

  protected:
    NavigationDataToPointSetPlayer();
    virtual ~NavigationDataToPointSetPlayer();
    void ReinitXML();
    mitk::NavigationData::Pointer ReadVersion1();
    ///
    /// do the work here
    ///
    virtual void GenerateData();


    NavigationDataToPointSetFilter::Pointer m_PointSetFilter;

    std::string m_FileName;
    std::string m_XMLString;
    TiXmlDocument* m_Doc;
    TiXmlElement* m_DataElem;
    TiXmlElement* m_CurrentElem;
    bool m_Repeat;
    unsigned int m_NumberOfSnapshots;
    int m_LastGoTo;

    std::vector<PointSet> m_NDPointSet;
  };
} // namespace mitk

#endif /* MITKNavigationDataToPointSetPlayer_H_HEADER_INCLUDED_ */
