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

#ifndef mitkExportMitkVisitor_h
#define mitkExportMitkVisitor_h

#include <mitkDataStorage.h>
#include <sofa/simulation/common/Visitor.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class MitkSimulation_EXPORT ExportMitkVisitor : public sofa::simulation::Visitor
  {
  public:
    explicit ExportMitkVisitor(DataStorage::Pointer dataStorage, const sofa::core::ExecParams* params = sofa::core::ExecParams::defaultInstance());
    ExportMitkVisitor(DataStorage::Pointer dataStorage, const std::string& visualModelName, const sofa::core::ExecParams* params = sofa::core::ExecParams::defaultInstance());
    ~ExportMitkVisitor();

    Result processNodeTopDown(sofa::simulation::Node* node);

  private:
    ExportMitkVisitor(const ExportMitkVisitor&);
    ExportMitkVisitor& operator=(const ExportMitkVisitor&);

    void processVisualModel(sofa::simulation::Node* node, sofa::core::visual::VisualModel* visualModel);

    DataStorage::Pointer m_DataStorage;
    std::string m_VisualModelName;
  };
}

#endif
