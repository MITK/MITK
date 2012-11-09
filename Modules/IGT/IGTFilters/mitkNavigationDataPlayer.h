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


#ifndef MITKNavigationDataPlayer_H_HEADER_INCLUDED_
#define MITKNavigationDataPlayer_H_HEADER_INCLUDED_

#include <mitkNavigationDataPlayerBase.h>
#include <mitkNavigationDataSource.h>
#include <mitkNavigationDataRecorder.h> //for the Recording Mode enum
#include "mitkTrackingDevice.h"

#include <itkMultiThreader.h>

#include "tinyxml.h"
#include <istream>


namespace mitk {
  /**Documentation
  * \brief This class is used to play recorded (see mitkNavigationDataRecorder class) files.
  *
  * If you want to play a file you have to set an input stream. This can be an own one (use StartPlaying(std::istream*))
  * or a preset (use StartPlaying()). The presets are NormalFile and ZipFile and can be set with the method
  * SetPlayerMode(PlayerMode). The presets need a FileName. Therefore the FileName must be set before the preset.
  * For pausing the player call Pause(). A call of Resume() will continue the playing.
  *
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataPlayer : public NavigationDataPlayerBase
  {
  public:
    mitkClassMacro(NavigationDataPlayer, NavigationDataPlayerBase);
    itkNewMacro(Self);

    /**
    * \brief sets the file name and path for the PlayerMode NormalFile and ZipFile
    */
    itkSetStringMacro(FileName);

    /**
    * \brief returns the file name and path for the PlayerMode NormalFile and ZipFile
    */
    itkGetStringMacro(FileName);


    /**
    * \brief Used for pipeline update just to tell the pipeline that we always have to update
    */
    virtual void UpdateOutputInformation();

    /**
     * \brief This method starts the player.
     *
     * Before the stream has to be set. Either with a PlayingMode (SetStream(PlayerMode)) and FileName. Or
     * with an own inputstream (SetStream(istream*)).
     *
     * @throw mitk::IGTIOException Throws an exception if the file cannot be opened.
     * @throw mitk::IGTIOException Throws an exception if there is no valid filename.
     * @throw mitk::IGTIOException Throws an exception if the file is damaged.
     * @throw mitk::IGTException Throws an exception if there is no stream (i.e stream=NULL).
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
     */
    void Pause();

    /**
     * \brief This method resumes the player when it was paused.
     */
    void Resume();


    /**
     * \brief This method checks if player arrived at end of file.
     *
     */
    bool IsAtEnd();

    /**
     * \brief The PlayerMode is used for generating a presetted output stream. You do not need to
     * set it if you want to use your own stream.
     *
     * There are:
     * NormalFile: ifstream
     * ZipFile: not implemented yet
     *
     *\warning The ZipFile Mode is not implemented yet
     */
    enum PlayerMode
    {
      NormalFile,
      ZipFile
    };

    /** @return Returns the current playing mode of the player. */
    itkGetMacro(PlayerMode,PlayerMode);

    /** @brief Sets the playing mode of the player. */
    itkSetMacro(PlayerMode,PlayerMode);

    /**
     * \brief Sets the stream of this player.
     * @throw mitk::IGTException Throws an exception if stream is NULL or if it is not good.
     */
    void SetStream(std::istream* stream);

  protected:
    NavigationDataPlayer();
    virtual ~NavigationDataPlayer();

    typedef mitk::NavigationData::TimeStampType TimeStampType;

    /**
     * \brief filter execute method
     */
    virtual void GenerateData();

    /**
     * \brief Creates a stream out of the filename given by the variable m_FileName.
     * The stream is then set to m_Stream.
     *
     * @throw mitk::IGTIOException Throws an exception if file does not exist
     * @throw mitk::IGTException Throws an exception if the stream is NULL
     */
    void CreateStreamFromFilename();

    /**
     * \brief Returns the file version out of the XML document.
     * @throw mitk::IGTException Throws an mitk::IGTException an exception if stream is NULL or not good.
     * @throw mitk::IGTIOException Throws an mitk::IGTIOException if the stream has an incompatible XML format.
     */
    unsigned int GetFileVersion(std::istream* stream);

    /**
     * \brief Returns the number of tracked tools out of the XML document.
     * @throw Throws an exception if stream is NULL.
     * @throw Throws an exception if the input stream has an XML incompatible format.
     */
    unsigned int GetNumberOfNavigationDatas(std::istream* stream);

    /**
     * \brief Gets the first data for initializing the player
     */
    void GetFirstData();

    /**
     * \brief This method reads one line of the XML document and returns the data as a NavigationData object
     * If there is a new file version another method must be added which reads this data.
     * @throw mitk::IGTException Throws an exceptions if file is damaged.
     */
    mitk::NavigationData::Pointer ReadVersion1();

    /**
     * \brief This method initializes the player with first data
     */
    void InitPlayer();

    std::istream* m_Stream; ///< stores a pointer to the input stream

    bool m_StreamSetOutsideFromClass; ///< stores if the stream was created in this class and must be deleted in the end

    PlayerMode m_PlayerMode; ///< stores the mode for the presetted PlayerMode sieh enum PlayerMode

    std::string m_FileName; ///< stores the filename

    unsigned int m_FileVersion; ///< indicates which XML encoding is used

    bool m_Playing; ///< indicates whether the generateoutput method generates new output or not

    bool m_Pause; ///< indicates if the player is paused

    unsigned int m_NumberOfOutputs; ///< stores the number of outputs known from the XML document

    TimeStampType m_StartPlayingTimeStamp; ///< the starttime of the playing set in the method StartPlaying()

    TimeStampType m_PauseTimeStamp; ///< stores the beginning of a pause

    std::vector<NavigationData::Pointer> m_NextToPlayNavigationData; ///< stores the next possible candidate for playing

    std::vector<TimeStampType> m_StartTimeOfData; ///< stores the start time of the different tools

    TiXmlElement * m_parentElement;

    TiXmlNode * m_currentNode;

    bool m_StreamEnd; ///< stores if the input stream arrived at end

    /**
     * @brief This is a helping method which gives an error message and throws an exception with the given message.
     *        It can be used if a stream is found to be invalid.
     *
     * @throw mitk::IGTIOException Always throws an exception.
     */
    void StreamInvalid(std::string message);  ///< help method which sets the stream invalid and displays an error

  };
} // namespace mitk

#endif /* MITKNavigationDataPlayer_H_HEADER_INCLUDED_ */
