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

#ifndef MITKUSPOINTMARKINTERACTOR_H
#define MITKUSPOINTMARKINTERACTOR_H

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
    mitkClassMacro(USPointMarkInteractor, DataInteractor)
      itkNewMacro(Self)

      typedef Message1<DataNode*> PointMarkInteractorEvent;
    PointMarkInteractorEvent CoordinatesChangedEvent;

  protected:
    static void UpdateSurface(itk::SmartPointer<mitk::DataNode>);

    USPointMarkInteractor();
    virtual ~USPointMarkInteractor();

    /**
     * \brief Connects the functions from the state machine to methods of this class.
     */
    virtual void ConnectActionsAndFunctions() override;

    /**
     * \brief Sets empty surface as data for the new data node.
     * This is necessary as data nodes without data do not work
     * with data interactors.
     */
    virtual void DataNodeChanged() override;

    /**
     * \brief Set current position as origin to the data node.
     */
    void AddPoint(StateMachineAction*, InteractionEvent*);
  };
} // namespace mitk

#endif // MITKUSPOINTMARKINTERACTOR_H
