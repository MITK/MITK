/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITKNAVIGATIONDATATOIGTLMessageFILTER_H__
#define _MITKNAVIGATIONDATATOIGTLMessageFILTER_H__

#include "mitkCommon.h"
#include "mitkPointSet.h"
#include "mitkIGTLMessageSource.h"
#include "mitkNavigationData.h"
#include "mitkNavigationDataSource.h"

namespace mitk {
  /**Documentation
  *
  * \brief This filter creates IGTL messages from mitk::NavigaitionData objects
  *
  *
  * \ingroup IGT
  *
  */
  class MITKIGT_EXPORT NavigationDataToIGTLMessageFilter : public IGTLMessageSource
  {
  public:
    mitkClassMacro(NavigationDataToIGTLMessageFilter, IGTLMessageSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**Documentation
      * \brief There are four different operation modes.
      *
      * - ModeSendQTransMsg: every input NavigationData is processed into one
      * output message that contains a position and a orientation (quaternion).
      * - ModeSendTransMsg: every input NavigationData is processed into one
      * output message that contains a 4x4 transformation.
      * - ModeSendQTDataMsg:all input NavigationData is processed into one single
      * output message that contains a position and orientation (quaternion) for
      * each navigation data.
      * - ModeSendTDataMsg:all input NavigationData is processed into one single
      * output message that contains a 4x4 transformation for
      * each navigation data.
      */
    enum OperationMode
    {
      ModeSendQTransMsg,
      ModeSendTransMsg,
      ModeSendQTDataMsg,
      ModeSendTDataMsg
    };

    /**
    * \brief filter execute method
    */
    void GenerateData() override;

    using Superclass::SetInput;

    /**
    * \brief Sets one input NavigationData
    */
    virtual void SetInput(const mitk::NavigationData *NavigationData);

    /**
    * \brief Sets the input NavigationData at a specific index
    */
    virtual void SetInput(unsigned int idx, const NavigationData* nd);

    /**
    * \brief Returns the input of this filter
    */
    const mitk::NavigationData* GetInput();

    /**
    * \brief Returns the input number idx of this filter
    */
    const mitk::NavigationData* GetInput(unsigned int idx);

    /**
    * \brief Sets the mode of this filter.
    *
    * See OperationMode for the behavior in the different modes
    * \warning A call to this method will change the number of outputs of the filter.
    * After calling this method, all previously acquired pointers to outputs are invalid
    * Always set the operation mode first, then get the outputs with GetOutput()
    */
    virtual void SetOperationMode(OperationMode mode);

    /**
    * \brief returns the mode of this filter.
    *
    * See OperationMode for the behavior in the different modes
    */
    itkGetConstMacro(OperationMode, OperationMode);

    /**
    * empty implementation to prevent calling of the superclass method that
    * would try to copy information from the input NavigationData to the output
    * PointSet, which makes no sense!
    */
    void GenerateOutputInformation() override {};

    /**
    *\brief Connects the input of this filter to the outputs of the given
    * NavigationDataSource
    *
    * This method does not support smartpointer. use FilterX.GetPointer() to
    * retrieve a dumbpointer.
    */
    virtual void ConnectTo(mitk::NavigationDataSource * UpstreamFilter);

  protected:
    NavigationDataToIGTLMessageFilter();

    ~NavigationDataToIGTLMessageFilter() override;

    /**
    * \brief Generates the output
    *
    */
    //    virtual void GenerateData();

    /**
    * \brief Generates the output for ModeSendQTDataMsg
    *
    */
    virtual void GenerateDataModeSendQTDataMsg();

    /**
    * \brief Generates the output for ModeSendTDataMsg
    */
    virtual void GenerateDataModeSendTDataMsg();

    /**
    * \brief Generates the output for ModeSendQTransMsg
    *
    */
    virtual void GenerateDataModeSendQTransMsg();

    /**
    * \brief Generates the output for ModeSendTransMsg
    */
    virtual void GenerateDataModeSendTransMsg();

    /**
    * \brief create output objects according to OperationMode for all inputs
    */
    virtual void CreateOutputsForAllInputs();

    OperationMode m_OperationMode;  ///< Stores the mode. See enum OperationMode
    //    unsigned int m_RingBufferSize;  ///< Stores the ringbuffer size
    unsigned int m_CurrentTimeStep; ///< Indicates the current timestamp
    //    unsigned int m_NumberForMean;   ///< Number of Navigation Data, which should be averaged

    /** Converts a mitk::IGTTimestamp (double, milliseconds) to an OpenIGTLink timestamp */
    igtl::TimeStamp::Pointer ConvertToIGTLTimeStamp(double IGTTimeStamp);

    /** Measurement class to calculate latency and frame count */
  };
} // namespace mitk
#endif // _MITKNAVIGATIONDATATOIGTLMessageFILTER_H__
