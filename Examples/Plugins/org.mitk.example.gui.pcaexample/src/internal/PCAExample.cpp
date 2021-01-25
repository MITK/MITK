/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "PCAExample.h"

// Qt
#include <QMessageBox>

// mitk image
#include <mitkImage.h>

const std::string PCAExample::VIEW_ID = "org.mitk.views.pcaexample";

void PCAExample::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void PCAExample::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(BtnPerfomPCAClicked()));

  //initialize point set widget and point set node
  mitk::DataNode::Pointer PointSetNode = mitk::DataNode::New();
  PointSetNode->SetName("PCA Example Pointset");
  mitk::PointSet::Pointer newPtSet = mitk::PointSet::New();
  PointSetNode->SetData(newPtSet);
  m_Controls.m_pointSetWidget->SetPointSetNode(PointSetNode);
  this->GetDataStorage()->Add(PointSetNode);
}

PCAExample::PCAExample()
{

}

PCAExample::~PCAExample()
{
  //clean up
  mitk::DataNode::Pointer ptSetNode = m_Controls.m_pointSetWidget->GetPointSetNode();
  m_Controls.m_pointSetWidget->SetPointSetNode(nullptr);
  this->GetDataStorage()->Remove(ptSetNode);
  this->GetDataStorage()->Remove(m_Axis1Node);
  this->GetDataStorage()->Remove(m_Axis2Node);
  this->GetDataStorage()->Remove(m_Axis3Node);
}

void PCAExample::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{
  // iterate all selected objects, adjust warning visibility
  foreach (mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNotNull() && dynamic_cast<mitk::Image *>(node->GetData()))
    {
      m_Controls.buttonPerformImageProcessing->setEnabled(true);
      return;
    }
  }

}

void PCAExample::BtnPerfomPCAClicked()
{
	std::vector<mitk::Vector3D> eigenVectors;
	std::vector<double> eigenValues;
	mitk::Vector3D mean;
	bool success = comutePCA(m_Controls.m_pointSetWidget->GetPointSet(), eigenVectors, eigenValues, mean);
	this->showEigenvectors(eigenVectors, eigenValues, mean);
	MITK_INFO << "PCA: " << success;
}

bool PCAExample::comutePCA(mitk::PointSet::Pointer input, std::vector<mitk::Vector3D> &eigenVectors, std::vector<double> &eigenValues, mitk::Vector3D &pointsMean)
{
//Step 1: Construct data matrix
vnl_matrix<double> dataMatrix(3, input->GetSize(), 0.0);
int size = input->GetSize();
for (int i=0; i<size; i++)
{
	dataMatrix[0][i] = input->GetPoint(i)[0];
	dataMatrix[1][i] = input->GetPoint(i)[1];
	dataMatrix[2][i] = input->GetPoint(i)[2];
}

//Step 2: Remove average for each row (Mittelwertbefreiung)
mitk::Vector3D mean;
for (int i = 0; i < size; i++) { mean += mitk::Vector3D(dataMatrix.get_column(i)); }
mean /= size;
for (int i = 0; i<size; i++)
	{
	mitk::Vector3D removedAverageI = mitk::Vector3D(dataMatrix.get_column(i)) - mean;
	dataMatrix.set_column(i, removedAverageI);
    }

//Step 3: Compute covariance matrix
vnl_matrix<double> covMatrix = (1.0 / (size - 1.0)) * dataMatrix * dataMatrix.transpose();

//Step 4: Singular value composition
vnl_svd<double> svd(covMatrix);

//Store results and print them to the console
MITK_INFO << "DataMatrix: " << "\n" << dataMatrix;
MITK_INFO << "CovMatrix: " << "\n" << covMatrix;
for (int i = 0; i < 3; i++)
{
	eigenVectors.push_back(svd.U().get_column(i));
	eigenValues.push_back(sqrt(svd.W(i)));
	MITK_INFO << "Eigenvector " << i << ": " << eigenVectors.at(i);
	MITK_INFO << "Eigenvalue " << i << ": " << eigenValues.at(i);
}

//Compute center of points
for (int i = 0; i < size; i++) { pointsMean += input->GetPoint(i).GetVectorFromOrigin(); }
pointsMean /= size;

return true;
}

void PCAExample::showEigenvectors(std::vector<mitk::Vector3D> eigenVectors, std::vector<double> eigenValues, mitk::Vector3D center)
{
	m_Axis1Node = mitk::DataNode::New();
	m_Axis1Node->SetName("Eigenvector 1");
	mitk::PointSet::Pointer axis1 = mitk::PointSet::New();
	axis1->InsertPoint(0, mitk::Point3D(center));
	axis1->InsertPoint(1, mitk::Point3D(center + eigenVectors.at(0)*eigenValues.at(0)));
	m_Axis1Node->SetData(axis1);
	m_Axis1Node->SetBoolProperty("show contour", true);
	m_Axis1Node->SetColor(1, 0, 0);
	this->GetDataStorage()->Add(m_Axis1Node);

	m_Axis2Node = mitk::DataNode::New();
	m_Axis2Node->SetName("Eigenvector 2");
	mitk::PointSet::Pointer axis2 = mitk::PointSet::New();
	axis2->InsertPoint(0, mitk::Point3D(center));
	axis2->InsertPoint(1, mitk::Point3D(center + eigenVectors.at(1)*eigenValues.at(1)));
	m_Axis2Node->SetData(axis2);
	m_Axis2Node->SetBoolProperty("show contour", true);
	m_Axis2Node->SetColor(1, 0, 0);
	this->GetDataStorage()->Add(m_Axis2Node);

	m_Axis3Node = mitk::DataNode::New();
	m_Axis3Node->SetName("Eigenvector 3");
	mitk::PointSet::Pointer axis3 = mitk::PointSet::New();
	axis3->InsertPoint(0, mitk::Point3D(center));
	axis3->InsertPoint(1, mitk::Point3D(center + eigenVectors.at(2)*eigenValues.at(2)));
	m_Axis3Node->SetData(axis3);
	m_Axis3Node->SetBoolProperty("show contour", true);
	m_Axis3Node->SetColor(1, 0, 0);
	this->GetDataStorage()->Add(m_Axis3Node);
}
