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

#ifndef mitkSimulationTemplate_h
#define mitkSimulationTemplate_h

#include <mitkBaseData.h>
#include <mitkBaseProperty.h>
#include <mitkDataNode.h>
#include <SimulationExports.h>
#include <string>
#include <vector>

namespace mitk
{
  /** \brief %Simulation templates are extended simulation scenes which can contain variables.
   *
   * These variables are mapped to properties (mitk::BaseProperty) to be easily adjustable.
   * A simulation template is an intermediate format and cannot be loaded by SOFA as simulation scene.
   * However, a simulation scene based on a simulation template can be created easily.
   *
   * For information on the syntax of simulation templates, see \ref org_mitk_gui_qt_simulationSimulationTemplates.
   *
   * Call Parse() to initialize the class by parsing a simulation scene template.
   * Parsed templates are represented by properties which you must set manually for the corresponding data node (mitk::DataNode) by calling SetProperties().
   * To create a simulation scene based on the simulation template call CreateSimulation().
   */
  class Simulation_EXPORT SimulationTemplate : public BaseData
  {
  public:
    mitkClassMacro(SimulationTemplate, BaseData);
    itkNewMacro(Self);

    std::string CreateSimulation() const;
    bool Parse(const std::string& contents);
    bool RequestedRegionIsOutsideOfTheBufferedRegion();
    bool SetProperties(DataNode::Pointer dataNode) const;
    void SetRequestedRegion(const itk::DataObject* data);
    void SetRequestedRegionToLargestPossibleRegion();
    void UpdateOutputInformation();
    bool VerifyRequestedRegion();

  private:
    SimulationTemplate();
    ~SimulationTemplate();

    SimulationTemplate(Self&);
    Self& operator=(const Self&);

    bool m_IsInitialized;
    std::vector<std::string> m_StaticContents;
    std::vector<std::pair<std::string, BaseProperty::Pointer> > m_VariableContents;
  };
}

#endif
