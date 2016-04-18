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

#ifndef MITKUSZONESINTERACTOR_H
#define MITKUSZONESINTERACTOR_H

#include "mitkDataInteractor.h"

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk
{
  class Surface;
  class DataNode;

  /**
   * \brief DataInteractor for creating a sphere at a specific coordinate.
   * The origin is set by mitk::USZonesInteractor::AddCenter() and then the
   * radius can be updated by mitk::USZonesInteractor::ChangeRadius(). An
   * updated sphere is rendered every time the radius changes.
   *
   * DataNode attributes set by this class:
   * * zone.size (float) - the radius of the sphere
   * * zone.created (bool) - determines if the interactor reached its final state
   *
   * Two state machines for this interactor are available as resources of the
   * USNavigation module.
   * * USZoneInteractions.xml: Create a sphere by clicking left, moving the mouse
   *   and clicking left againg. Aborting is possible by right mouse click.
   * * USZoneInteractionsHold.xml: Create a sphere by pressing the left mouse button,
   *   moving the mouse while holding it pressed and finally release the button.
   *   Aborting is possible by right mouse click.
   */
  class USZonesInteractor : public DataInteractor
  {
  public:
    static const char* DATANODE_PROPERTY_SIZE;
    static const char* DATANODE_PROPERTY_CREATED;

    mitkClassMacro(USZonesInteractor, DataInteractor)
      itkNewMacro(Self)

      /**
       * \brief Creates Vtk Sphere according to current radius.
       * The radius is gotten from the float property "zone.size" of the
       * data node.
       */
       static void UpdateSurface(itk::SmartPointer<mitk::DataNode>);

  protected:
    USZonesInteractor();
    virtual ~USZonesInteractor();

    /**
     * \brief Connects the functions from the state machine to methods of this class.
     */
    virtual void ConnectActionsAndFunctions();

    /**
     * \brief Sets empty surface as data for the new data node.
     * This is necessary as data nodes without data do not work
     * with data interactors.
     */
    virtual void DataNodeChanged();

    /**
     * \brief Sets origin of the data node to the coordinates of the position event.
     * \return false if interaction event isn't a position event, true otherwise
     */
    void AddCenter(StateMachineAction*, InteractionEvent*);

    /**
     * \brief Updates radius attribute according to position event.
     * Calculates distance between the data node origin and the current position
     * event and updates the data node attribue "zone.size" accordingly. The
     * mitk::USZonesInteractor::UpdateSurface() function is called then.
     *
     * \return false if interaction event isn't a position event, true otherwise
     */
    void ChangeRadius(StateMachineAction*, InteractionEvent*);

    /**
     * \brief Sets the property "zone.created" of the data node to true.
     * \return always true
     */
    void EndCreation(StateMachineAction*, InteractionEvent*);

    /**
     * \brief Removes Vtk Sphere from data node.
     * \return always true
     */
    void AbortCreation(StateMachineAction*, InteractionEvent*);
  };
} // namespace mitk

#endif // MITKUSZONESINTERACTOR_H
