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

#ifndef mitkSimulation_h
#define mitkSimulation_h

#include "mitkSimulationDrawTool.h"
#include <mitkSurface.h>
#include <SimulationExports.h>
#include <sofa/simulation/common/Simulation.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /** \brief Encapsulates a SOFA simulation.
   *
   * Call GetSimulation() to access the initially default constructed SOFA simulation.
   * Directly after loading a scene into the SOFA simulation you must set its root node by calling SetRootNode().
   * You should set the simulation scene's default time step as well by calling SetDefaultDT().
   * Make sure to manually initilalize the SOFA simulation.
   *
   * SOFA can conceptionally handle only one simulation at a time, however, MITK %Simulation allows to load several simulation scenes in parallel.
   * Therefore it is necessary to set a simulation as active simulation when using it.
   * You can do this either by calling SetAsActiveSimulation() or by calling the static SetActiveSimulation() method.
   *
   * You can take snapshots of simulations represented as 3D surfaces by calling TakeSnapshot() or easily record simulations as 3D+t surfaces by calling AppendSnapshot().
   */
  class Simulation_EXPORT Simulation : public BaseData
  {
  public:
    mitkClassMacro(Simulation, BaseData);
    itkNewMacro(Self);

    /** \brief %Set the active simulation.
     *
     * \param[in] simulation The new active simulation or NULL.
     * \sa SetAsActiveSimulation()
     */
    static void SetActiveSimulation(Self* simulation);

    /** \brief Scale factor by which all objects in a simulation scene are scaled for visualization.
     */
    static const float ScaleFactor;

    /** \brief Append snapshot of scene at current simulation time to given Surface.
     *
     * \param[in] surface Surface to which the current scene is appended as 3D surface.
     * \return Returns true if a snapshot was successfully appended to the surface.
     * \sa TakeSnapshot()
     */
    bool AppendSnapshot(Surface::Pointer surface) const;

    /** \brief Get default time step of simulation scene.
     *
     * You need to manually set the default time step by calling SetDefaultDT().
     *
     * \return The default time step of the simulation scene if available or 0.0 otherwise.
     */
    double GetDefaultDT() const;

    /** \brief Get simulation draw tool.
     *
     * Usually called only by the corresponding mapper or to reset the draw tool manually.
     *
     * \return The simulation draw tool.
     */
    SimulationDrawTool* GetDrawTool();

    /** \brief Get root node of simulation scene.
     *
     * You must manually set the root node by calling SetRootNode() first.
     * This is usually done directly after setting/loading a simulation scene.
     *
     * \return The root node of the encapsulated SOFA simulation scene.
     */
    sofa::simulation::Node::SPtr GetRootNode() const;

    /** \brief Get SOFA simulation.
     *
     * The encapsulated SOFA simulation is default constructed during construction.
     * Use this method to gain access to it or to initially load a scene into the simulation.
     * Make sure to also call SetRootNode() and SetDefaultDT() in the latter case.
     *
     * \return The encapsulated SOFA simulation.
     */
    sofa::simulation::Simulation::SPtr GetSimulation() const;

    bool RequestedRegionIsOutsideOfTheBufferedRegion();

    /** \brief %Set this simulation as active simulation.
     *
     * You must set a simulation as active simulation prior to use it.
     *
     * \sa SetActiveSimulation()
     */
    void SetAsActiveSimulation();

    /** \brief %Set default simulation scene time step.
     *
     * You should call this method after you loaded a scene into the encapsulated SOFA simulation and set the root node.
     *
     * \param[in] dt Default time step of encapsulated simulation scene.
     * \sa GetSimulation(), SetRootNode()
     */
    void SetDefaultDT(double dt);

    void SetRequestedRegion(const itk::DataObject* data);

    void SetRequestedRegionToLargestPossibleRegion();

    /** \brief %Set the simulation scene root node of the encapsulated simulation.
     *
     * This is usually the first method you call after you accessed the encapsulated simulation by calling GetSimulation() to set or load the simulation scene.
     *
     * \param[in] rootNode Root Node of the simulation scene corresponding to the encapsulated SOFA simulation.
     */
    void SetRootNode(sofa::simulation::Node* rootNode);

    /** \brief Take a snapshot of the encapsulated simulation scene at current simulation time.
     *
     * Snapshots cannot be created if the simulation scene contains no visual models or only hidden visual models.
     *
     * \return Snapshot represented as 3D surface or NULL.
     */
    Surface::Pointer TakeSnapshot() const;

    void UpdateOutputInformation();

    bool VerifyRequestedRegion();

  private:
    Simulation();
    ~Simulation();

    Simulation(Self&);
    Self& operator=(const Self&);

    vtkSmartPointer<vtkPolyData> CreateSnapshot() const;

    sofa::simulation::Simulation::SPtr m_Simulation;
    sofa::simulation::Node::SPtr m_RootNode;
    SimulationDrawTool m_DrawTool;
    double m_DefaultDT;
  };
}

#endif
