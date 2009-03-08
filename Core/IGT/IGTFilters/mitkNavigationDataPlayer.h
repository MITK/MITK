/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-02-10 18:08:54 +0100 (Di, 10 Feb 2009) $
Version:   $Revision: 16228 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNavigationDataPlayer_H_HEADER_INCLUDED_
#define MITKNavigationDataPlayer_H_HEADER_INCLUDED_

#include <mitkNavigationDataSource.h>
#include <mitkNavigationDataRecorder.h> //for the Recording Mode enum
#include "mitkTrackingDevice.h"

#include <itkMultiThreader.h>

//#define TIXML_USE_STL
#include "tinyxml.h"

namespace mitk {
  /**Documentation
  * \brief Connects a mitk::TrackingDevice to a MITK-IGT NavigationData-Filterpipeline
  *
  *
  * @ingroup Navigation
  */
  class NavigationDataPlayer : public NavigationDataSource
  {
  public:
    mitkClassMacro(NavigationDataPlayer, NavigationDataSource);
    itkNewMacro(Self);

    /**Documentation
    * \brief sets the file name for the InputMode NormalFile and ZipFile
    * \warning existing files will be overriden
    */
    itkSetStringMacro(FileName);

    /**Documentation
    * \brief sets path for the InputMode: NormalFile and ZipFile
    */
    itkSetStringMacro(FilePath);

    /**Documentation
    * \brief Used for pipeline update
    */
    virtual void UpdateOutputInformation();

    /**Documentation
    * \brief This method starts the player
    */
    void StartPlaying();

    /**Documentation
    * \brief This method starts the player with an own input stream
    */
    void StartPlaying(std::istream* stream);

    /**Documentation
    * \brief Stops the player and closes the stream. After a call of StopPlaying()
    * StartPlaying() must be called to get new output data
    */
    void StopPlaying();
    
    /**Documentation
    * \brief This method pauses the player. The first call pauses the player if it is in the playing mode.
    * The second call continues the playing mode.
    * 
    */
    void PauseContinuePlaying();
    


    enum PlayerMode
    {
      NormalFile,
      ZipFile
    };

    /**Documentation
    * \brief sets the recording mode which causes different types of output streams
    */
    void SetPlayerMode(PlayerMode mode);

  protected:
    NavigationDataPlayer();
    virtual ~NavigationDataPlayer();

    typedef mitk::NavigationData::TimeStampType TimeStampType;

    /**Documentation
    * \brief filter execute method
    *   
    */
    virtual void GenerateData();

    /**Documentation
    * \brief gets the file version out of the XML document
    *   
    */
    void GetFileVersion();

    /**Documentation
    * \brief gets the number of tracked tools out of the XML document
    *   
    */
    void GetNumberOfTrackedTools();

    /**Documentation
    * \brief gets the first data for initializing the player
    *   
    */
    void GetFirstData();

    /**Documentation
    * \brief This method reads one line of the XML document and returns the data as a NavigationData object
    * If there is a new file version another method must be added which reads this data.
    *   
    */
    mitk::NavigationData::Pointer ReadVersion1();


    /**Documentation
    * \brief Starts the thread for the thread based player
    * \warning This method is not used yet! If there is an event based polling of the data we will need this.
    */
    static ITK_THREAD_RETURN_TYPE ThreadStartPlaying(void* pInfoStruct);
   
    /**Documentation
    * \brief This method plays the data in a thread. It is neccessary if you want to throw events if "new" data arrived.
    *   
    */
    void PlayInThread();
    
    std::istream*  m_Stream;

    PlayerMode m_PlayerMode;

    std::string m_FilePath;

    std::string m_FileName;

    unsigned int m_FileVersion;

    bool m_Playing;

    bool m_Pause;

    unsigned int m_NumberOfOutputs;

    itk::MultiThreader::Pointer m_MultiThreader;

    int m_ThreadID;

    TiXmlDocument* m_XMLDocument;

    TimeStampType m_StartPlayingTimeStamp;

    TimeStampType m_PauseTimeStamp;

    std::vector<NavigationData::Pointer> m_NextToPlayNavigationData;

    std::vector<TimeStampType> m_StartTimeOfData;

    std::vector<TimeStampType> m_CurrentTimeOfData;
    
    //unsigned int m_NextToPlayOutputNumber;
    

  };
} // namespace mitk

#endif /* MITKNavigationDataPlayer_H_HEADER_INCLUDED_ */
