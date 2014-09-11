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

namespace mitk
{
  /**Documentation
  * \brief This class records NavigationData objects into NavigationDataSets.
  *
  * The recording is started with the call of the method StartRecording(). Now
  * every Update() stores the current state of the added NavigationDatas into the NavigationDataSet.
  * With StopRecording() the stream is stopped, but can be resumed anytime.
  * To start recording to a new NavigationDataSet, call ResetRecording();
  *
  * \warning Do not add inputs while the recorder ist recording. The recorder can't handle that and will cause a nullpointer exception.
  * \ingroup IGT
  */

  class MitkIGT_EXPORT NavigationDataRecorder : public NavigationDataToNavigationDataFilter
  {
  public:

    mitkClassMacro( NavigationDataRecorder, NavigationDataToNavigationDataFilter );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * \brief Returns whether the NavigationDataRecorder is currently recording or not
    */
    itkGetMacro(Recording, bool);

    /**
    * \brief Returns the set that contains all of the recorded data.
    */
    itkGetMacro(NavigationDataSet, mitk::NavigationDataSet::Pointer);

    /**
    * \brief Sets a limit of recorded data sets / frames. Recording will be stopped if the number is reached. values < 1 disable this behaviour. Default is -1.
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
    virtual void StartRecording();

    /**
    * \brief Stops StopsRecording to the NavigationDataSet.
    *
    * Recording can be resumed to the same Dataset by just calling StartRecording() again.
    * Call ResetRecording() to start recording to a new Dataset;
    */
    virtual void StopRecording();

    /**
    * \brief Resets the Datasets and the timestamp, so a new recording can happen.
    *
    * Do not forget to save the old Dataset, it will be lost after calling this function.
    */
    virtual void ResetRecording();

    /**
    * \brief Returns the number of time steps that were recorded in the current set.
    * Warning: This Method does NOT Stop Recording!
    */
    virtual int GetNumberOfRecordedSteps();

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
  };
}
#endif // #define _MITK_POINT_SET_SOURCE_H
