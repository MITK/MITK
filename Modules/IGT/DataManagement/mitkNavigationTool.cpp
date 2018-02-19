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

#include "mitkNavigationTool.h"
#include "mitkIGTException.h"
#include "mitkNavigationData.h"
#include "Poco/File.h"
#include "mitkUnspecifiedTrackingTypeInformation.h"

#include "vtkSphereSource.h"
#include "vtkConeSource.h"
#include "vtkLandmarkTransform.h"
#include "vtkLineSource.h"
#include "vtkCylinderSource.h"
#include "vtkTransformPolyDataFilter.h"
#include <vtkAppendPolyData.h>
#include "mitkTextAnnotation3D.h"
#include "mitkManualPlacementAnnotationRenderer.h"
#include "mitkBaseRenderer.h"


mitk::NavigationTool::NavigationTool() : m_Identifier("None"),
m_Type(mitk::NavigationTool::Unknown),
m_CalibrationFile("none"),
m_SerialNumber(""),
m_TrackingDeviceType(mitk::UnspecifiedTrackingTypeInformation::GetTrackingDeviceName()),
m_ToolRegistrationLandmarks(mitk::PointSet::New()),
m_ToolCalibrationLandmarks(mitk::PointSet::New()),
m_ToolAxisTransform(mitk::Quaternion(0, 0, 0, 1))
{
  m_ToolTipPosition[0] = 0;
  m_ToolTipPosition[1] = 0;
  m_ToolTipPosition[2] = 0;

  //m_ToolAxis[0] = 1;
  //m_ToolAxis[1] = 0;
  //m_ToolAxis[2] = 0;

  SetDefaultSurface();
}

itk::LightObject::Pointer mitk::NavigationTool::InternalClone() const
{
  Self::Pointer tool = new Self(*this);
  tool->UnRegister();
  return tool.GetPointer();
}

mitk::NavigationTool::NavigationTool(const NavigationTool &other)
  : Superclass()
{
  this->m_Identifier = other.m_Identifier;
  this->m_Type = other.m_Type;
  if (other.m_DataNode.IsNotNull())
  {
    this->m_DataNode = other.m_DataNode->Clone();
    this->m_DataNode->SetName(other.m_DataNode->GetName());
    if (other.m_DataNode->GetData())
    {
      this->m_DataNode->SetData(dynamic_cast<mitk::BaseData*>(other.m_DataNode->GetData()->Clone().GetPointer()));
    }
  }

  if (other.m_SpatialObject.IsNotNull())
    this->m_SpatialObject = other.m_SpatialObject->Clone();
  this->m_CalibrationFile = other.m_CalibrationFile;
  this->m_SerialNumber = other.m_SerialNumber;
  this->m_TrackingDeviceType = other.m_TrackingDeviceType;
  if (other.m_ToolRegistrationLandmarks.IsNotNull())
    this->m_ToolRegistrationLandmarks = other.m_ToolRegistrationLandmarks->Clone();
  if (other.m_ToolCalibrationLandmarks.IsNotNull())
    this->m_ToolCalibrationLandmarks = other.m_ToolCalibrationLandmarks->Clone();
  this->m_ToolTipPosition = other.m_ToolTipPosition;
  this->m_ToolAxisTransform = other.m_ToolAxisTransform;
  //this->m_ToolAxis = other.m_ToolAxis;
}

mitk::NavigationTool::~NavigationTool()
{
}

mitk::Point3D mitk::NavigationTool::GetToolAxis()
{
  // The tool axis in the sensor coordinate system is defined as the negative z-axis
  mitk::Vector3D toolAxisSensorCoordinateSystem;
  mitk::FillVector3D(toolAxisSensorCoordinateSystem, 0.0, 0.0, -1.0);
  // Apply inverse tool axis transform to calculate tool axis
  vnl_vector_fixed<mitk::ScalarType,3> toolAxisVector = m_ToolAxisTransform.inverse().rotate(toolAxisSensorCoordinateSystem.GetVnlVector());
  // Transfer to mitk::Point3D
  mitk::Point3D toolAxis;
  toolAxis[0] = toolAxisVector[0];
  toolAxis[1] = toolAxisVector[1];
  toolAxis[2] = toolAxisVector[2];
  return toolAxis;
}

void mitk::NavigationTool::SetToolAxis(mitk::Point3D toolAxis)
{
  // The tool axis in the sensor coordinate system is defined as the negative z-axis
  mitk::Vector3D toolAxisSensorCoordinateSystem;
  mitk::FillVector3D(toolAxisSensorCoordinateSystem, 0.0, 0.0, -1.0);
  // Normalize the tool axis as obtained by a tool axis calibration
  mitk::Vector3D toolAxisFromCalibration;
  mitk::FillVector3D(toolAxisFromCalibration, toolAxis[0], toolAxis[1], toolAxis[2]);
  toolAxisFromCalibration.Normalize();
  // Determine rotation angle
  mitk::ScalarType rotationAngle = acos(toolAxisSensorCoordinateSystem*toolAxisFromCalibration);
  // Determine rotation axis
  mitk::Vector3D rotationAxis = itk::CrossProduct(toolAxisSensorCoordinateSystem, toolAxisFromCalibration);
  // Calculate transform
  itk::AffineTransform<mitk::ScalarType>::Pointer sensorToToolAxisTransform = itk::AffineTransform<mitk::ScalarType>::New();
  sensorToToolAxisTransform->Rotate3D(rotationAxis, rotationAngle);
  // transfer to quaternion notation. Note that the vnl_quaternion expects the matrix in row major format, hence the transpose
  mitk::Quaternion toolAxisTransform(sensorToToolAxisTransform->GetMatrix().GetVnlMatrix().transpose());
  // Update the tool tip orientation
  m_ToolAxisTransform = toolAxisTransform;
}

mitk::AffineTransform3D::Pointer mitk::NavigationTool::GetToolTipTransform()
{
  mitk::NavigationData::Pointer returnValue = mitk::NavigationData::New();
  returnValue->SetPosition(this->m_ToolTipPosition);
  returnValue->SetOrientation(this->m_ToolAxisTransform);
  return returnValue->GetAffineTransform3D();
}

void mitk::NavigationTool::Graft(const DataObject *data)
{
  // Attempt to cast data to an NavigationData
  const Self* nd;
  try
  {
    nd = dynamic_cast<const Self *>(data);
  }
  catch (...)
  {
    mitkThrowException(mitk::IGTException) << "mitk::NavigationData::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name();
  }
  if (!nd)
  {
    // pointer could not be cast back down
    mitkThrowException(mitk::IGTException) << "mitk::NavigationData::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name();
  }
  // Now copy anything that is needed
  m_Identifier = nd->GetIdentifier();
  m_Type = nd->GetType();
  m_DataNode->SetName(nd->GetDataNode()->GetName());
  m_DataNode->SetData(nd->GetDataNode()->GetData());
  m_SpatialObject = nd->GetSpatialObject();
  m_CalibrationFile = nd->GetCalibrationFile();
  m_SerialNumber = nd->GetSerialNumber();
  m_TrackingDeviceType = nd->GetTrackingDeviceType();
  m_ToolRegistrationLandmarks = nd->GetToolRegistrationLandmarks();
  m_ToolCalibrationLandmarks = nd->GetToolCalibrationLandmarks();
  m_ToolTipPosition = nd->GetToolTipPosition();
  m_ToolAxisTransform = nd->GetToolAxisTransform();
}

bool mitk::NavigationTool::IsToolTipSet()
{
  if ((m_ToolTipPosition[0] == 0) &&
    (m_ToolTipPosition[1] == 0) &&
    (m_ToolTipPosition[2] == 0) &&
    (m_ToolAxisTransform.x() == 0) &&
    (m_ToolAxisTransform.y() == 0) &&
    (m_ToolAxisTransform.z() == 0) &&
    (m_ToolAxisTransform.r() == 1))
    return false;
  else return true;
}

void mitk::NavigationTool::SetCalibrationFile(const std::string filename)
{
  //check if file does exist:
  if (filename == "")
  {
    m_CalibrationFile = "none";
  }
  else
  {
    Poco::File myFile(filename);
    if (myFile.exists())
      m_CalibrationFile = filename;
    else
      m_CalibrationFile = "none";
  }
}

std::string mitk::NavigationTool::GetToolName()
{
  if (this->m_DataNode.IsNull()) { return ""; }
  else { return m_DataNode->GetName(); }
}

mitk::Surface::Pointer mitk::NavigationTool::GetToolSurface()
{
  if (this->m_DataNode.IsNull()) { return nullptr; }
  else if (this->m_DataNode->GetData() == nullptr) { return nullptr; }
  else { return dynamic_cast<mitk::Surface*>(m_DataNode->GetData()); }
}

void mitk::NavigationTool::SetDefaultSurface()
{
  if (m_DataNode.IsNull())
    m_DataNode = mitk::DataNode::New();

  mitk::Surface::Pointer mySphere = mitk::Surface::New();

  double axisLength = 5.;

  vtkSmartPointer<vtkSphereSource> vtkSphere = vtkSmartPointer<vtkSphereSource>::New();
  vtkSmartPointer<vtkConeSource> vtkCone = vtkSmartPointer<vtkConeSource>::New();
  vtkSmartPointer<vtkCylinderSource> vtkCylinder = vtkSmartPointer<vtkCylinderSource>::New();
  vtkSmartPointer<vtkPolyData> axis = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkLineSource> vtkLine = vtkSmartPointer<vtkLineSource>::New();
  vtkSmartPointer<vtkLineSource> vtkLine2 = vtkSmartPointer<vtkLineSource>::New();
  vtkSmartPointer<vtkLineSource> vtkLine3 = vtkSmartPointer<vtkLineSource>::New();

  vtkSmartPointer<vtkAppendPolyData> appendPolyData = vtkSmartPointer<vtkAppendPolyData>::New();
  vtkSmartPointer<vtkPolyData> surface = vtkSmartPointer<vtkPolyData>::New();

  //Y-Axis (start with y, cause cylinder is oriented in y by vtk default...)
  vtkCone->SetDirection(0, 1, 0);
  vtkCone->SetHeight(1.0);
  vtkCone->SetRadius(0.4f);
  vtkCone->SetResolution(16);
  vtkCone->SetCenter(0.0, axisLength, 0.0);
  vtkCone->Update();

  vtkCylinder->SetRadius(0.05);
  vtkCylinder->SetHeight(axisLength);
  vtkCylinder->SetCenter(0.0, 0.5*axisLength, 0.0);
  vtkCylinder->Update();

  appendPolyData->AddInputData(vtkCone->GetOutput());
  appendPolyData->AddInputData(vtkCylinder->GetOutput());
  appendPolyData->Update();
  axis->DeepCopy(appendPolyData->GetOutput());

  //y symbol
  vtkLine->SetPoint1(-0.5, axisLength + 2., 0.0);
  vtkLine->SetPoint2(0.0, axisLength + 1.5, 0.0);
  vtkLine->Update();

  vtkLine2->SetPoint1(0.5, axisLength + 2., 0.0);
  vtkLine2->SetPoint2(-0.5, axisLength + 1., 0.0);
  vtkLine2->Update();

  appendPolyData->AddInputData(vtkLine->GetOutput());
  appendPolyData->AddInputData(vtkLine2->GetOutput());
  appendPolyData->AddInputData(axis);
  appendPolyData->Update();
  surface->DeepCopy(appendPolyData->GetOutput());

  //X-axis
  vtkSmartPointer<vtkTransform> XTransform = vtkSmartPointer<vtkTransform>::New();
  XTransform->RotateZ(-90);
  vtkSmartPointer<vtkTransformPolyDataFilter> TrafoFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  TrafoFilter->SetTransform(XTransform);
  TrafoFilter->SetInputData(axis);
  TrafoFilter->Update();

  //x symbol
  vtkLine->SetPoint1(axisLength + 2., -0.5, 0.0);
  vtkLine->SetPoint2(axisLength + 1., 0.5, 0.0);
  vtkLine->Update();

  vtkLine2->SetPoint1(axisLength + 2., 0.5, 0.0);
  vtkLine2->SetPoint2(axisLength + 1., -0.5, 0.0);
  vtkLine2->Update();

  appendPolyData->AddInputData(vtkLine->GetOutput());
  appendPolyData->AddInputData(vtkLine2->GetOutput());
  appendPolyData->AddInputData(TrafoFilter->GetOutput());
  appendPolyData->AddInputData(surface);
  appendPolyData->Update();
  surface->DeepCopy(appendPolyData->GetOutput());

  //Z-axis
  vtkSmartPointer<vtkTransform> ZTransform = vtkSmartPointer<vtkTransform>::New();
  ZTransform->RotateX(90);
  TrafoFilter->SetTransform(ZTransform);
  TrafoFilter->SetInputData(axis);
  TrafoFilter->Update();

  //z symbol
  vtkLine->SetPoint1(-0.5, 0.0, axisLength + 2.);
  vtkLine->SetPoint2(0.5, 0.0, axisLength + 2.);
  vtkLine->Update();

  vtkLine2->SetPoint1(-0.5, 0.0, axisLength + 2.);
  vtkLine2->SetPoint2(0.5, 0.0, axisLength + 1.);
  vtkLine2->Update();

  vtkLine3->SetPoint1(0.5, 0.0, axisLength + 1.);
  vtkLine3->SetPoint2(-0.5, 0.0, axisLength + 1.);
  vtkLine3->Update();

  appendPolyData->AddInputData(vtkLine->GetOutput());
  appendPolyData->AddInputData(vtkLine2->GetOutput());
  appendPolyData->AddInputData(vtkLine3->GetOutput());
  appendPolyData->AddInputData(TrafoFilter->GetOutput());
  appendPolyData->AddInputData(surface);
  appendPolyData->Update();
  surface->DeepCopy(appendPolyData->GetOutput());

  //Center
  vtkSphere->SetRadius(0.5f);
  vtkSphere->SetCenter(0.0, 0.0, 0.0);
  vtkSphere->Update();

  appendPolyData->AddInputData(vtkSphere->GetOutput());
  appendPolyData->AddInputData(surface);
  appendPolyData->Update();
  surface->DeepCopy(appendPolyData->GetOutput());

  //Scale
  vtkSmartPointer<vtkTransform> ScaleTransform = vtkSmartPointer<vtkTransform>::New();
  ScaleTransform->Scale(20., 20., 20.);

  TrafoFilter->SetTransform(ScaleTransform);
  TrafoFilter->SetInputData(surface);
  TrafoFilter->Update();

  mySphere->SetVtkPolyData(TrafoFilter->GetOutput());

  //vtkCone->Delete();
  //vtkSphere->Delete();
  //vtkLine->Delete();
  //vtkLine2->Delete();
  //vtkLine3->Delete();
  //vtkCylinder->Delete();
  //ZTransform->Delete();
  //XTransform->Delete();
  //ScaleTransform->Delete();
  //TrafoFilter->Delete();
  //appendPolyData->Delete();
  //surface->Delete();

  this->GetDataNode()->SetData(mySphere);
}

std::string mitk::NavigationTool::GetStringWithAllToolInformation() const
{
  std::stringstream _info;
  _info << "  Identifier: " << this->m_Identifier << "\n"
    << "  NavigationToolType: " << m_Type << "\n"
    << "  Calibration file: " << m_CalibrationFile << "\n"
    << "  Serial number: " << m_SerialNumber << "\n"
    << "  TrackingDeviceType: " << m_TrackingDeviceType << "\n"
    << "  ToolTip Position: " << m_ToolTipPosition << "\n"
    << "  Tool Axis Transform: " << m_ToolAxisTransform << "\n"
    << "  Tool Axis: " << m_ToolAxisTransform.rotation_matrix_transpose().get_row(2);
    ;

  return _info.str();
}
