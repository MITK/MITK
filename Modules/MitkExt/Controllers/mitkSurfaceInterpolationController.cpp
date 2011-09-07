/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkSurfaceInterpolationController.h"
#include "vtkMarchingCubes.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "mitkInteractionConst.h"

#include "mitkVtkRepresentationProperty.h"
#include "vtkProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"


mitk::SurfaceInterpolationController::SurfaceInterpolationController()
: m_CurrentContourListID (0)
{
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();
  m_Contours = Surface::New();

  m_PolyData = vtkSmartPointer<vtkPolyData>::New();
  m_PolyData->SetPoints(vtkPoints::New());
  //m_PolyData->SetPolys(vtkCellArray::New());
  //m_Contours->SetVtkPolyData(m_PolyData);

  //m_PolyDataAppender = vtkSmartPointer<vtkAppendPolyData>::New();
}

mitk::SurfaceInterpolationController::~SurfaceInterpolationController()
{
  //for (unsigned int i = 0; i < m_PositionList.size(); i++)
  //{
  //  delete m_PositionList.at(i);
  //}
}

mitk::SurfaceInterpolationController* mitk::SurfaceInterpolationController::GetInstance()
{
  static mitk::SurfaceInterpolationController* m_Instance;

  if ( m_Instance == 0)
  {
    m_Instance = new SurfaceInterpolationController();
  }

  return m_Instance;
}

void mitk::SurfaceInterpolationController::AddNewContour (mitk::Surface::Pointer newContour ,RestorePlanePositionOperation* op)
{
  AffineTransform3D::Pointer transform = AffineTransform3D::New();
  transform = op->GetTransform();

  mitk::Vector3D direction = op->GetDirectionVector();
  int pos (-1);

  for (unsigned int i = 0; i < m_ListOfContourLists.at(m_CurrentContourListID).size(); i++)
  {
      itk::Matrix<float> diffM = transform->GetMatrix()-m_ListOfContourLists.at(m_CurrentContourListID).at(i).position->GetTransform()->GetMatrix();
      bool isSameMatrix(true);
      for (unsigned int j = 0; j < 3; j++)
      {
        if (fabs(diffM[j][0]) > 0.0001 && fabs(diffM[j][1]) > 0.0001 && fabs(diffM[j][2]) > 0.0001)
        {
          isSameMatrix = false;
          break;
        }
      }
      itk::Vector<float> diffV = m_ListOfContourLists.at(m_CurrentContourListID).at(i).position->GetTransform()->GetOffset()-transform->GetOffset();
      if ( isSameMatrix && m_ListOfContourLists.at(m_CurrentContourListID).at(i).position->GetPos() == op->GetPos() && (fabs(diffV[0]) < 0.0001 && fabs(diffV[1]) < 0.0001 && fabs(diffV[2]) < 0.0001) )
      {
        pos = i;
        break;
      }
    
  }

  if (pos == -1)
  {
    //MITK_INFO<<"New";
    mitk::RestorePlanePositionOperation* newOp = new mitk::RestorePlanePositionOperation (OpRESTOREPLANEPOSITION, op->GetWidth(), 
      op->GetHeight(), op->GetSpacing(), op->GetPos(), direction, transform);
    ContourPositionPair newData;
    newData.contour = newContour;
    newData.position = op;

    m_ReduceFilter->SetInput(m_ListOfContourLists.at(m_CurrentContourListID).size(), newContour);
    m_NormalsFilter->SetInput(m_ListOfContourLists.at(m_CurrentContourListID).size(), m_ReduceFilter->GetOutput(m_ListOfContourLists.at(m_CurrentContourListID).size()));
    m_InterpolateSurfaceFilter->SetInput(m_ListOfContourLists.at(m_CurrentContourListID).size(), m_NormalsFilter->GetOutput(m_ListOfContourLists.at(m_CurrentContourListID).size()));
    m_ListOfContourLists.at(m_CurrentContourListID).push_back(newData);
  }
  else
  {
    m_ListOfContourLists.at(m_CurrentContourListID).at(pos).contour = newContour;
    m_ReduceFilter->SetInput(pos, newContour);
  }

  this->Modified();
}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::Interpolate()
{
  if (m_ListOfContourLists.at(m_CurrentContourListID).size() < 2)
    return 0; 

  m_InterpolateSurfaceFilter->Update();

  Image::Pointer distanceImage = m_InterpolateSurfaceFilter->GetOutput();

  vtkSmartPointer<vtkMarchingCubes> mcFilter = vtkMarchingCubes::New();
  mcFilter->SetInput(distanceImage->GetVtkImageData());
  mcFilter->SetValue(0,0);
  mcFilter->Update();
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();

  mitk::Surface::Pointer surface = mitk::Surface::New();
  surface->SetVtkPolyData(mcFilter->GetOutput());
  Point3D origin = distanceImage->GetGeometry()->GetOrigin();
  surface->GetGeometry()->SetOrigin(distanceImage->GetGeometry()->GetOrigin());

  vtkSmartPointer<vtkAppendPolyData> polyDataAppender = vtkSmartPointer<vtkAppendPolyData>::New();
  for (unsigned int i = 0; i < m_ReduceFilter->GetNumberOfOutputs(); i++)
  {
    polyDataAppender->AddInput(m_ReduceFilter->GetOutput(i)->GetVtkPolyData());
  }
  polyDataAppender->Update();
  m_Contours->SetVtkPolyData(polyDataAppender->GetOutput());

  surface->DisconnectPipeline();
  return surface;
}

mitk::Surface* mitk::SurfaceInterpolationController::GetContoursAsSurface()
{
  return m_Contours;
}

void mitk::SurfaceInterpolationController::SetDataStorage(DataStorage &ds)
{
  m_DataStorage = &ds;
}

unsigned int mitk::SurfaceInterpolationController::GetNewListID()
{
  m_CurrentContourListID =  m_ListOfContourLists.size();
  ContourPositionPairList newList;
  m_ListOfContourLists.push_back(newList);
  this->SetCurrentListID(m_CurrentContourListID);
  return m_CurrentContourListID;
}

void mitk::SurfaceInterpolationController::SetCurrentListID ( int ID )
{
  MITK_INFO<<"Alte Konturenanzahl: "<<m_ListOfContourLists.at(m_CurrentContourListID).size();
  if ( ID < 0 || ID == m_CurrentContourListID)
    return;

  m_CurrentContourListID = ID;

  //if(m_ReduceFilter->GetNumberOfInputs() != 0)
  //{
  //  m_InterpolateSurfaceFilter->Delete();
  //  m_NormalsFilter->Delete();
  //  m_ReduceFilter->Delete();

  //  m_ReduceFilter->

  //    mitk::ReduceContourSetFilter::Pointer rf = mitk::ReduceContourSetFilter::New();
  //  mitk::ComputeContourSetNormalsFilter::Pointer cnf = mitk::ComputeContourSetNormalsFilter::New();
  //  mitk::CreateDistanceImageFromSurfaceFilter::Pointer csf = mitk::CreateDistanceImageFromSurfaceFilter::New();

  //  m_ReduceFilter = rf;
  //  m_NormalsFilter = cnf;
  //  m_InterpolateSurfaceFilter = csf;

  //  m_ReduceFilter->SetMinSpacing(m_MinSpacing);
  //  m_ReduceFilter->SetMaxSpacing(m_MaxSpacing);
  //  m_InterpolateSurfaceFilter->SetDistanceImageVolume(m_DistImageVolume);
  //}

  for(unsigned int i = 0; i < m_ReduceFilter->GetNumberOfInputs(); i++)
  {
    m_ReduceFilter->RemoveInputs(const_cast<mitk::Surface*>(m_ReduceFilter->GetInput(i)));
    m_NormalsFilter->RemoveInputs(const_cast<mitk::Surface*>(m_NormalsFilter->GetInput(i)));
    m_InterpolateSurfaceFilter->RemoveInputs(const_cast<mitk::Surface*>(m_ReduceFilter->GetInput(i)));
  }
  
  for (unsigned int i = 0; i < m_ListOfContourLists.at(m_CurrentContourListID).size(); i++)
  {
    m_ReduceFilter->SetInput(i, m_ListOfContourLists.at(m_CurrentContourListID).at(i).contour);
    m_NormalsFilter->SetInput(i,m_ReduceFilter->GetOutput(i));
    m_InterpolateSurfaceFilter->SetInput(i,m_NormalsFilter->GetOutput(i));
  }
  MITK_INFO<<"Neue Konturenanzahl: "<<m_ListOfContourLists.at(m_CurrentContourListID).size();
}