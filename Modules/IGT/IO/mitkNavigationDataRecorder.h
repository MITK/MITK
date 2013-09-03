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

#ifndef _MITK_NavigationDataRecorder_H
#define _MITK_NavigationDataRecorder_H

#include "mitkNavigationDataToNavigationDataFilter.h"
#include "mitkNavigationData.h"
#include "mitkNavigationDataSet.h"

#include <iostream>

#include <mitkRealTimeClock.h>

namespace mitk
{
  /**Documentation
  * \brief This class records NavigationData objects.
  *
  * The output of this class is formated as a XML document.
  *
  * Internal this class uses streams for recording NavigationData objects. Therefore different types of output are possible
  * and can be set with the SetOutputMode() method. The default output is directed to the console. If you want to save into a
  * file you have to set a file name and the path. The recording is started with the call of the method StartRecording(). Now
  * every Update() stores the current state of the added NavigationDatas. With StopRecording() the stream is stopped. With
  * another call of StartRecording() the output is written to a new file with incremented filename counter.
  *
  * \warning Do not add inputs while the recorder ist recording. The recorder can't handle that and will cause a nullpointer exception.
  * \ingroup IGT
  */

  class MitkIGT_EXPORT NavigationDataRecorder : public NavigationDataToNavigationDataFilter
  {
  public:

    mitkClassMacro( NavigationDataRecorder, NavigationDataToNavigationDataFilter );

    itkNewMacro( Self );

    /**
    * \brief Returns whether the NavigationDataRecorder is recording or not
    */
    itkGetMacro(Recording, bool);

    /**
    * \brief Returns the set that contains all of the recorded data.
    */
    itkGetMacro(NavigationDataSet, mitk::NavigationDataSet::Pointer);

    /**
    * \brief Sets a limit of recorded data sets / frames. Recording will be stopped if the number is reached. -1 disables the limit, -1 is default value as well.
    */
    itkSetMacro(RecordCountLimit, int);

    /**
    * \brief Returns whether to use the navigationdata's time stamp or to create a new one upon recording.
    */
    itkGetMacro(StandardizeTime, bool);

    /**
    * \brief If set to false, the navigationDatas Timestamp will be used. If set to false, the recorder
    * will generate a timestamp when it copies the data to the navigationdataset.
    */
    itkSetMacro(StandardizeTime, bool);

    /**
    * \brief Starts recording NavigationData into the NAvigationDataSet
    */
    void StartRecording();

    /**
    * \brief Stops StopsRecording to the NavigationDataSet.
    *
    * Recording can be resumed to the same Dataset by just calling StartRecording() again.
    * Call ResetRecording() to start recording to a new Dataset;
    */
    void StopRecording();

    /**
    * \brief Resets the Datasets and the timestamp, so a new recording can happen.
    *
    * Do not forget to save the old Dataset, it will be lost after calling this function.
    */
    void ResetRecording();

  protected:

    virtual void GenerateData();

    NavigationDataRecorder();

    virtual ~NavigationDataRecorder();

    unsigned int m_NumberOfInputs; ///< counts the numbers of added input NavigationDatas

    mitk::NavigationDataSet::Pointer m_NavigationDataSet;

    bool m_Recording; ///< indicates whether the recording is started or not

    bool m_StandardizeTime; //< indicates whether one should use the timestamps in NavigationData or create new timestamps upon recording

    bool m_StandardizedTimeInitialized; //< set to true the first time start recording is called.

    int m_RecordCountLimit; ///< limits the number of frames, recording will be stopped if the limit is reached. -1 disables the limit

    mitk::RealTimeClock::Pointer m_SystemTimeClock;  ///< system time clock for system time tag in output xml file
  };
}
#endif // #define _MITK_POINT_SET_SOURCE_H
