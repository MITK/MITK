/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIGTLMessageToNavigationDataFilter_h
#define mitkIGTLMessageToNavigationDataFilter_h

#include <mitkNavigationDataSource.h>
#include "mitkIGTLMessage.h"
#include "mitkIGTLMessageSource.h"
#include "MitkIGTExports.h"

namespace mitk
{
  /**Documentation
  * \brief IGTLinkMessageToNavigationDataFilter is a filter that receives
  * OpenIGTLink messages as input and produce NavigationDatas as output
  *
  * IGTLinkMessageToNavigationDataFilter is a filter that receives
  * OpenIGTLink messages as input and produce NavigationDatas as output.
  * If the OpenIGTLink message is not of the proper type the filter will not
  * do anything.
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT
      IGTLMessageToNavigationDataFilter : public NavigationDataSource
  {
  public:
    mitkClassMacro(IGTLMessageToNavigationDataFilter, NavigationDataSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    using Superclass::SetInput;

    /**
    * \brief Set the input of this filter
    *
    * \warning: this will set the number of outputs to the number of inputs,
    * deleting any extra outputs that might have been initialized.
    * Subclasses that have a different number of outputs than inputs
    * must overwrite the SetInput methods.
    */
    virtual void SetInput( const IGTLMessage* msg);

    /**
    * \brief Set input with id idx of this filter
    *
    * \warning: this will set the number of outputs to the number of inputs,
    * deleting any extra outputs that might have been initialized.
    * Subclasses that have a different number of outputs than inputs
    * must overwrite the SetInput methods.
    * If the last input is set to nullptr, the number of inputs will be decreased by
    * one (-> removing the last input). If other inputs are set to nullptr, the
    * number of inputs will not change.
    */
    virtual void SetInput( unsigned int idx, const IGTLMessage* msg);

    /** Set an input */
//    virtual void SetInput(const DataObjectIdentifierType & key, DataObject *input);

    /**
    * \brief Get the input of this filter
    */
    const IGTLMessage* GetInput(void) const;

    /**
    * \brief Get the input with id idx of this filter
    */
    const IGTLMessage* GetInput(unsigned int idx) const;

    /**
    * \brief Get the input with name messageName of this filter
    */
    const IGTLMessage* GetInput(std::string messageName) const;

    /**
    *\brief return the index of the input with name messageName,
    * throw std::invalid_argument exception if that name was not found
    *
    */
    DataObjectPointerArraySizeType GetInputIndex(std::string messageName);

    /**
    *\brief Connects the input of this filter to the outputs of the given
    * IGTLMessageSource
    *
    * This method does not support smartpointer. use FilterX.GetPointer() to
    * retrieve a dumbpointer.
    */
    virtual void ConnectTo(mitk::IGTLMessageSource * UpstreamFilter);

    /**
    *\brief Sets the number of expected outputs.
    *
    * Normally, this is done automatically by the filter concept. However, in our
    * case we can not know, for example, how many tracking elements are stored
    * in the incoming igtl message. Therefore, we have to set the number here to
    * the expected value.
    */
    void SetNumberOfExpectedOutputs(unsigned int numOutputs);

  protected:
    IGTLMessageToNavigationDataFilter();
    ~IGTLMessageToNavigationDataFilter() override;

    void GenerateData() override;
    void GenerateTransformData();
    void GenerateTrackingDataData();
    void GenerateQuaternionTrackingDataData();

    /**
    * \brief Create an output for each input
    *
    * This Method sets the number of outputs to the number of inputs
    * and creates missing outputs objects.
    * \warning any additional outputs that exist before the method is called are
    * deleted
    */
    void CreateOutputsForAllInputs();

    /**
    * \brief Defines how the input will be copied into the output
    */
    void GenerateOutputInformation() override;
  };
} // namespace mitk
#endif
