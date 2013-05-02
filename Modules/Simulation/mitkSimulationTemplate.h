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
#include <utility>

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

    /** \brief Create simulation scene based on parsed simulation scene template.
     *
     * You must parse a simulation scene template before calling this method.
     * In case of error this method writes a message to MITK_DEBUG and returns an empty string.
     *
     * \return %Simulation scene as string or empty string in case of error.
     * \sa Parse()
     */
    std::string CreateSimulation() const;

    /** \brief Initialize class by parsing a simulation scene template.
     *
     * For more information on the syntax of simulation templates, see \ref org_mitk_gui_qt_simulationSimulationTemplates.
     * An mitk::Exception with a meaningful description is thrown for any possible error related to parsing.
     *
     * \param[in] contents %Simulation scene template.
     * \exception mitk::Exception An error occurred during parsing.
     * \return False if class is already initialized.
     */
    bool Parse(const std::string& contents);

    bool RequestedRegionIsOutsideOfTheBufferedRegion();

    /** \brief Add properties which represent simulation template variables to given data node.
     *
     * You must parse a simulation scene template before calling this method.
     * In case of error this method writes a message to MITK_DEBUG and returns false.
     *
     * \return False if class is not initialized, data node is null, or data node doesn't own this simulatiom template.
     * \sa Parse()
     */
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
