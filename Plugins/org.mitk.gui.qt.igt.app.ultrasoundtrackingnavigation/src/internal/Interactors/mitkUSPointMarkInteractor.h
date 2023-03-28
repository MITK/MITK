/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUSPointMarkInteractor_h
#define mitkUSPointMarkInteractor_h

#include "mitkDataInteractor.h"

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk
{
  /**
   * \brief Simple interactor for getting just one position on mouse click.
   */
  class USPointMarkInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(USPointMarkInteractor, DataInteractor);
    itkNewMacro(Self);

      typedef Message1<DataNode*> PointMarkInteractorEvent;
    PointMarkInteractorEvent CoordinatesChangedEvent;

  protected:
    static void UpdateSurface(itk::SmartPointer<mitk::DataNode>);

    USPointMarkInteractor();
    ~USPointMarkInteractor() override;

    /**
     * \brief Connects the functions from the state machine to methods of this class.
     */
    void ConnectActionsAndFunctions() override;

    /**
     * \brief Sets empty surface as data for the new data node.
     * This is necessary as data nodes without data do not work
     * with data interactors.
     */
    void DataNodeChanged() override;

    /**
     * \brief Set current position as origin to the data node.
     */
    void AddPoint(StateMachineAction*, InteractionEvent*);
  };
} // namespace mitk

#endif
