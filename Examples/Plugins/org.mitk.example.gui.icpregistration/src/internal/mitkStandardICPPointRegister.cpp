/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <math.h>
#include "mitkStandardICPPointRegister.h"

//vtk header
#include <vtkPoints.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkLandmarkTransform.h>
#include <vtkVertexGlyphFilter.h>


bool mitk::StandardICPPointRegister::StandardICPPointRegisterAlgorithm(  mitk::PointSet::Pointer MovingSet,
                                                    mitk::PointSet::Pointer StaticSet,
                                                    double Threshold,
                                                    itk::Matrix<double,3,3>& TransformationR,
                                                    itk::Vector<double,3>& TransformationT,
                                                    double& FRE,
                                                    int& n,
                                                    std::string& ErrorMessage,
                                                    int max_iterations
                                                    )
  {
  //################# clean up return variables at first ####################
  TransformationR = itk::Matrix<double,3,3>(); TransformationR.Fill(0);
  TransformationR[0][0] = 1; TransformationR[1][1] = 1; TransformationR[2][2] = 1;
  TransformationT = itk::Vector<double,3>(); TransformationT.Fill(0);
  n = 0;
  FRE = 0;
  ErrorMessage = "";

  //################# check for right input #################################
  if (MovingSet.IsNull() || StaticSet.IsNull())
    {
    ErrorMessage = "Error, at least one input value is missing";
    return false;
    }
  else if (MovingSet->GetSize()>StaticSet->GetSize())
    {
    ErrorMessage = "Error, static set is smaller than moving set.";
    return false;
    }

  //##################### convert input data ################################
  //convert source points (= moving points) to vtkPolyData:
  vtkSmartPointer<vtkPolyData> source = convertPointSetToVtkPolyData(MovingSet);
  //convert target points (= static points) to vtkPolyData:
  vtkSmartPointer<vtkPolyData> target = convertPointSetToVtkPolyData(StaticSet);

  //##################### setup ICP transform ###############################
  vtkSmartPointer<vtkIterativeClosestPointTransform> icp = vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
  icp->SetMaximumMeanDistance(Threshold);
  icp->SetSource(source);
  icp->SetTarget(target);
  icp->GetLandmarkTransform()->SetModeToRigidBody();
  icp->SetMaximumNumberOfIterations(max_iterations);
  icp->Modified();
  icp->Update();

  //##################### get results #######################################
  vtkSmartPointer<vtkMatrix4x4> m = icp->GetMatrix();
  TransformationR[0][0] = m->GetElement(0,0);
  TransformationR[0][1] = m->GetElement(0,1);
  TransformationR[0][2] = m->GetElement(0,2);
  TransformationR[1][0] = m->GetElement(1,0);
  TransformationR[1][1] = m->GetElement(1,1);
  TransformationR[1][2] = m->GetElement(1,2);
  TransformationR[2][0] = m->GetElement(2,0);
  TransformationR[2][1] = m->GetElement(2,1);
  TransformationR[2][2] = m->GetElement(2,2);
  TransformationT[0] = m->GetElement(0,3);
  TransformationT[1] = m->GetElement(1,3);
  TransformationT[2] = m->GetElement(2,3);
  n = icp->GetNumberOfIterations();

  return true;
  }


vtkSmartPointer<vtkPolyData> mitk::StandardICPPointRegister::convertPointSetToVtkPolyData(mitk::PointSet::Pointer PointSet)
  {
  vtkSmartPointer<vtkPolyData> returnValue = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for(int i=0; i<PointSet->GetSize(); i++)
    {
      double point[3] = {PointSet->GetPoint(i)[0],PointSet->GetPoint(i)[1],PointSet->GetPoint(i)[2]};
      points->InsertNextPoint(point);
    }
  vtkSmartPointer<vtkPolyData> temp = vtkSmartPointer<vtkPolyData>::New();
  temp->SetPoints(points);

  vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  vertexFilter->SetInputData(temp);
  vertexFilter->Update();

  returnValue->ShallowCopy(vertexFilter->GetOutput());

  return returnValue;
  }
