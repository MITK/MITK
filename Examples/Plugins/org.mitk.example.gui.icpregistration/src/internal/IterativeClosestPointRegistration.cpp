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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "IterativeClosestPointRegistration.h"

// Qt
#include <QMessageBox>

// Mitk
#include <mitkImage.h>
#include <mitkNodePredicateDataType.h>
#include <mitkSurface.h>
#include <mitkPointSet.h>
#include "mitkStandardICPPointRegister.h"

const std::string IterativeClosestPointRegistration::VIEW_ID = "org.mitk.views.iterativeclosestpointregistration";

void IterativeClosestPointRegistration::SetFocus()
{
}

void IterativeClosestPointRegistration::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.m_performICP, &QPushButton::clicked, this, &IterativeClosestPointRegistration::PerformICP);
  //initialize Combo Boxes
  m_Controls.m_comboBoxFixedSurface->SetDataStorage(this->GetDataStorage());
  m_Controls.m_comboBoxFixedSurface->SetAutoSelectNewItems(false);
  m_Controls.m_comboBoxFixedSurface->SetPredicate(mitk::NodePredicateDataType::New("Surface"));
  m_Controls.m_comboBoxMovingSurface->SetDataStorage(this->GetDataStorage());
  m_Controls.m_comboBoxMovingSurface->SetAutoSelectNewItems(false);
  m_Controls.m_comboBoxMovingSurface->SetPredicate(mitk::NodePredicateDataType::New("Surface"));

}

void IterativeClosestPointRegistration::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{

}

void IterativeClosestPointRegistration::PerformICP()
{
mitk::Surface::Pointer MovingSurface = dynamic_cast<mitk::Surface*>(m_Controls.m_comboBoxMovingSurface->GetSelectedNode()->GetData());
mitk::Surface::Pointer FixedSurface = dynamic_cast<mitk::Surface*>(m_Controls.m_comboBoxFixedSurface->GetSelectedNode()->GetData());
double Threshold = 0.0001;
itk::Matrix<double,3,3> TransformationR;
itk::Vector<double,3> TransformationT;
double FRE;
int n;
std::string ErrorMessage;
mitk::StandardICPPointRegister::StandardICPPointRegisterAlgorithm(MovingSurface->GetVtkPolyData(),
                                                                  FixedSurface->GetVtkPolyData(),
                                                                  Threshold,
                                                                  TransformationR,
                                                                  TransformationT,
                                                                  FRE, n, ErrorMessage);
mitk::AffineTransform3D::Pointer T = mitk::AffineTransform3D::New();
T->SetTranslation(TransformationT);
T->SetMatrix(TransformationR);
MovingSurface->GetGeometry()->SetIndexToWorldTransform(T);
MITK_INFO << "Performed ICP";
MITK_INFO << "n:" << n;
MITK_INFO << "R:" << TransformationR;
MITK_INFO << "T:" << TransformationT;
MITK_INFO << "Message:" << ErrorMessage;
}
