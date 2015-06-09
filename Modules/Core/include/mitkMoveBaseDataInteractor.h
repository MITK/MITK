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


#ifndef MITKMoveBaseDataInteractor_H_HEADER_INCLUDED
#define MITKMoveBaseDataInteractor_H_HEADER_INCLUDED

#include <mitkInteractor.h>

namespace mitk
{
  class DataNode;

  /**
   * \brief Interaction to move an object by the arrow keys. See tutorial step 10 for explanation.
   *
   * Left- and right-arrows move the x-direction.
   * Shift + up-and down-arrows move the y-direction.
   * Up- and down-arrows move the z-direction.
   * Every move is one unit in world coordinates (usually 1mm).
   * This class offers three properties:
   * - \b "MovingInteractor.SelectedColor": This color is used to mark that
   *    the object is selected and the interactor can be used.
   * - \b "MovingInteractor.DeselectedColor": Marks that the interactor is
   *    is added to the data node, but the node is not selected.
   * - \b "MovingInteractor.PriorColor":  Temporary property used to
   *    save the old color of the data node in order to restore it
   *    uppon removal of the interactor.
   *
   * All properties are deleted when the interactor is destroyed.
   *
   * \warning After moving an object, the user has to perform a reinit manually, in order to change the geoemtrie.
   *
   * \warning The picking seems not to work on a single point. If you want to move pointsets,
   *    they should atleast have 2 or more points.
   *
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT MoveBaseDataInteractor : public Interactor
  {
  public:
    mitkClassMacro(MoveBaseDataInteractor, Interactor);
    mitkNewMacro2Param(Self, const char*, DataNode*);

  protected:
    /**
     * \brief Constructor
     */
    MoveBaseDataInteractor(const char * type, DataNode* dataNode);

    /**
     * \brief Default Destructor
     **/
    virtual ~MoveBaseDataInteractor();

    /**
    * @brief Convert the given Actions to Operations and send to data and UndoController
    **/
    virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent ) override;
  };
}

/**
\example mitkMoveBaseDataInteractor.h
 * This is an example of how to implement a new Interactor.
 * See more details about this example in tutorial Step10.
 */

#endif /* MITKMoveBaseDataInteractor_H_HEADER_INCLUDED */
