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

#include <sofa/simulation/common/Visitor.h>
#include <org_mitk_simulation_Export.h>

namespace mitk
{
  class SIMULATION_INIT_EXPORT ExportMitkVisitor : public sofa::simulation::Visitor
  {
  public:
    explicit ExportMitkVisitor(const sofa::core::ExecParams* params = sofa::core::ExecParams::defaultInstance());
    explicit ExportMitkVisitor(const std::string& visualModelName, const sofa::core::ExecParams* params = sofa::core::ExecParams::defaultInstance());
    ~ExportMitkVisitor();

    Result processNodeTopDown(sofa::simulation::Node* node);

  private:
    ExportMitkVisitor(const ExportMitkVisitor&);
    ExportMitkVisitor& operator=(const ExportMitkVisitor&);

    void processVisualModel(sofa::simulation::Node* node, sofa::core::visual::VisualModel* visualModel);

    std::string m_VisualModelName;
  };
}

#endif
