/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITKNNAVIGATIONDATATONAVIGATIONDATAFILTER_H_HEADER_INCLUDED_
#define MITKNNAVIGATIONDATATONAVIGATIONDATAFILTER_H_HEADER_INCLUDED_

#include <mitkNavigationDataSource.h>

namespace mitk
{

  /**Documentation
  * \brief NavigationDataToNavigationDataFilter is the base class of all filters that receive
  * NavigationDatas as input and produce NavigationDatas as output
  *
  * Base class that for all navigation filters that receive NavigationData objects as input
  * and produce NavigationData objects as output.
  * This class defines the input-interface for NavigationDataFilters.
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT NavigationDataToNavigationDataFilter : public NavigationDataSource
  {
  public:
    mitkClassMacro(NavigationDataToNavigationDataFilter, NavigationDataSource);

    using Superclass::SetInput;

    /**
    * \brief Set the input of this filter
    *
    * \warning: this will set the number of outputs to the number of inputs,
    * deleting any extra outputs that might have been initialized.
    * Subclasses that have a different number of outputs than inputs
    * must overwrite the SetInput methods.
    */
    virtual void SetInput( const NavigationData* nd);

    /**
    * \brief Set input with id idx of this filter
    *
    * \warning: this will set the number of outputs to the number of inputs,
    * deleting any extra outputs that might have been initialized.
    * Subclasses that have a different number of outputs than inputs
    * must overwrite the SetInput methods.
    * If the last input is set to nullptr, the number of inputs will be decreased by one
    * (-> removing the last input). If other inputs are set to nullptr, the number of inputs
    * will not change.
    */
    virtual void SetInput( unsigned int idx, const NavigationData* nd);

    /**
    * \brief Get the input of this filter
    */
    const NavigationData* GetInput(void) const;

    /**
    * \brief Get the input with id idx of this filter
    */
    const NavigationData* GetInput(unsigned int idx) const;

    /**
    * \brief Get the input with name navDataName of this filter
    */
    const NavigationData* GetInput(std::string navDataName) const;

    /**
    *\brief return the index of the input with name navDataName, throw std::invalid_argument exception if that name was not found
    *
    * \warning if a subclass has inputs that have different data type than mitk::NavigationData, they have to overwrite this method
    */
    DataObjectPointerArraySizeType GetInputIndex(std::string navDataName);

  /**
  *\brief Connects the input of this filter to the outputs of the given NavigationDataSource
  *
  * This method does not support smartpointer. use FilterX.GetPointer() to retrieve a dumbpointer.
  * E.g. calling Filter2->ConnectTo(Filter1) will result in a Pipeline where NavigationData flows from Filter1 to Filter2.
  */
  virtual void ConnectTo(mitk::NavigationDataSource * UpstreamFilter);

  protected:
    NavigationDataToNavigationDataFilter();
    ~NavigationDataToNavigationDataFilter() override;

    /**
    * \brief Create an output for each input
    *
    * This Method sets the number of outputs to the number of inputs
    * and creates missing outputs objects.
    * \warning any additional outputs that exist before the method is called are deleted
    */
    void CreateOutputsForAllInputs();
  };
} // namespace mitk
#endif /* MITKNAVIGATIONDATATONAVIGATIONDATAFILTER_H_HEADER_INCLUDED_ */
