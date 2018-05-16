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


//VTK
#include <vtkPolyLine.h>
#include <vtkIdList.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkWarpVector.h>
#include <vtkCleanPolyData.h>
#include <vtkVectorNorm.h>
#include <vtkGlyph3D.h>
#include <vtkArrowSource.h>

//MITK
#include <mitkProperties.h>

//MITK
#include "mitkRegVisHelper.h"
#include "mitkRegVisPropertyTags.h"
#include "mitkRegVisDirectionProperty.h"
#include "mitkMAPRegistrationWrapper.h"

//MAP
#include "mapRegistration.h"

namespace mitk
{

  void GetGridGeometryFromNode(const mitk::DataNode* regNode, mitk::Geometry3D::Pointer& gridDesc, unsigned int& gridFrequ)
  {

    int internalFrequ = 1;

    if(!regNode->GetIntProperty(mitk::nodeProp_RegVisGridFrequence,internalFrequ))
    {
      mitkThrow() << "Cannot configure node correctly. Node property "<<mitk::nodeProp_RegVisGridFrequence<<" is not correctly defined.";
    }

    mitk::Vector3DProperty* valueProp = nullptr;
    mitk::Vector3D size;
    mitk::Vector3D spacing;

    if (regNode->GetProperty(valueProp, mitk::nodeProp_RegVisFOVSize))
    {
      size = valueProp->GetValue();
    }
    else
    {
      mitkThrow() << "Cannot configure node correctly. Node property "<<mitk::nodeProp_RegVisFOVSize<<" is not correctly defined.";
    }

    if (regNode->GetProperty(valueProp, mitk::nodeProp_RegVisFOVSpacing))
    {
      spacing = valueProp->GetValue();
    }
    else
    {
      mitkThrow() << "Cannot configure node correctly. Node property "<<mitk::nodeProp_RegVisFOVSpacing<<" is not correctly defined.";
    }

    mitk::Point3dProperty* originProp = nullptr;
    mitk::Point3D origin;
    if (regNode->GetProperty(originProp, mitk::nodeProp_RegVisFOVOrigin))
    {
      origin = originProp->GetValue();
    }
    else
    {
      mitkThrow() << "Cannot configure node correctly. Node property "<<mitk::nodeProp_RegVisFOVOrigin<<" is not correctly defined.";
    }

    gridDesc = mitk::Geometry3D::New();
    mitk::Geometry3D::BoundsArrayType bounds = gridDesc->GetBounds();
    bounds[0] = 0;
    bounds[1] = size[0]/spacing[0];
    bounds[2] = 0;
    bounds[3] = size[1]/spacing[1];
    bounds[4] = 0;
    bounds[5] = size[2]/spacing[2];
    gridDesc->SetBounds(bounds);
    gridDesc->SetSpacing(spacing);
    gridDesc->SetOrigin(origin);

    gridFrequ = internalFrequ;
  }


  vtkSmartPointer<vtkPolyData> Generate3DDeformationGlyph(const mitk::BaseGeometry* gridDesc, const map::core::RegistrationKernelBase<3,3>* regKernel)
  {
    assert(gridDesc);

    vtkSmartPointer<vtkPoints>  points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkFloatArray> vectors = vtkSmartPointer<vtkFloatArray>::New();
    vectors->SetNumberOfComponents(3);
    vectors->SetName("DeformationVectors");

    long xExtend = itk::Math::Ceil<long,mitk::ScalarType>(gridDesc->GetExtent(0));
    long yExtend = itk::Math::Ceil<long,mitk::ScalarType>(gridDesc->GetExtent(1));
    long zExtend = itk::Math::Ceil<long,mitk::ScalarType>(gridDesc->GetExtent(2));

    mitk::Geometry3D::BoundsArrayType bounds = gridDesc->GetBounds();

    //Generate Points
    mitk::Point3D pos;
    for (pos[2] = bounds[4] ; pos[2]< bounds[4]+zExtend; ++(pos[2]))
    {
      for (pos[1] = bounds[2]; pos[1]< bounds[2]+yExtend; ++(pos[1]))
      {
        for (pos[0] = bounds[0]; pos[0]< bounds[0]+xExtend; ++(pos[0]))
        {
          mitk::Point3D worldPos;
          gridDesc->IndexToWorld(pos,worldPos);
          points->InsertNextPoint(worldPos[0],worldPos[1],worldPos[2]);

          if(regKernel)
          {
            map::core::RegistrationKernelBase<3,3>::InputPointType regInput(worldPos);
            map::core::RegistrationKernelBase<3,3>::OutputPointType regOutput;

            bool mapped = regKernel->mapPoint(regInput,regOutput);
            mitk::Vector3D vector(regOutput-regInput);
            if (!mapped)
            {
              vector.Fill(0.0);
            }

            vectors->InsertNextTuple(vector.GetDataPointer());
          }
        }
      }
    }

    vtkSmartPointer<vtkPolyData> input = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkFloatArray> vectorMagnitude;

    input->SetPoints(points);

    if(regKernel)
    {
      input->GetPointData()->SetVectors(vectors);
      input->GetPointData()->SetActiveVectors(vectors->GetName());

      vtkSmartPointer<vtkVectorNorm> norm = vtkSmartPointer<vtkVectorNorm>::New();
      norm->SetInputData(input);
      norm->SetAttributeModeToUsePointData();
      norm->Update();

      vectorMagnitude = vtkFloatArray::SafeDownCast(norm->GetOutput()->GetPointData()->GetScalars());
      vectorMagnitude->SetName("VectorMagnitude");
      input->GetPointData()->SetScalars(vectorMagnitude);
      input->GetPointData()->SetActiveScalars(vectorMagnitude->GetName());
    }

    vtkSmartPointer<vtkGlyph3D> glyphFilter = vtkSmartPointer<vtkGlyph3D>::New();
    glyphFilter->ScalingOn();
    glyphFilter->OrientOn();
    glyphFilter->SetVectorModeToUseVector();
    glyphFilter->SetScaleModeToScaleByVector();
    glyphFilter->SetInputData(input);

    vtkSmartPointer<vtkArrowSource> arrowSource = vtkSmartPointer<vtkArrowSource>::New();
    arrowSource->SetTipLength(0.2);

    glyphFilter->SetSourceConnection(arrowSource->GetOutputPort());

    glyphFilter->Update();

    vtkSmartPointer<vtkPolyData> output = glyphFilter->GetOutput();

    return output;
  }

  vtkSmartPointer<vtkPolyData> MITKMATCHPOINTREGISTRATION_EXPORT Generate3DDeformationGrid( const mitk::BaseGeometry* gridDesc, unsigned int gridFrequence, const map::core::RegistrationKernelBase<3,3>* regKernel /*= nullptr*/ )
  {
    assert(gridDesc);

    vtkSmartPointer<vtkPoints>  points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkFloatArray> vectors = vtkSmartPointer<vtkFloatArray>::New();
    vectors->SetNumberOfComponents(3);
    vectors->SetName("DeformationVectors");

    long xExtend = itk::Math::Ceil<long,mitk::ScalarType>(gridDesc->GetExtent(0))+1;
    long yExtend = itk::Math::Ceil<long,mitk::ScalarType>(gridDesc->GetExtent(1))+1;
    long zExtend = itk::Math::Ceil<long,mitk::ScalarType>(gridDesc->GetExtent(2))+1;
    long zOffset = xExtend*yExtend;

    mitk::Geometry3D::BoundsArrayType bounds = gridDesc->GetBounds();

    //Generate Points
    mitk::Point3D pos;
    for (pos[2] = bounds[4] ; pos[2]< bounds[4]+zExtend; ++(pos[2]))
    {
      for (pos[1] = bounds[2]; pos[1]< bounds[2]+yExtend; ++(pos[1]))
      {
        for (pos[0] = bounds[0]; pos[0]< bounds[0]+xExtend; ++(pos[0]))
        {
          mitk::Point3D worldPos;
          gridDesc->IndexToWorld(pos,worldPos);
          points->InsertNextPoint(worldPos[0],worldPos[1],worldPos[2]);

          if(regKernel)
          {
            map::core::RegistrationKernelBase<3,3>::InputPointType regInput(worldPos);
            map::core::RegistrationKernelBase<3,3>::OutputPointType regOutput;

            bool mapped = regKernel->mapPoint(regInput,regOutput);
            mitk::Vector3D vector(regOutput-regInput);
            if (!mapped)
            {
              vector.Fill(0.0);
            }

            vectors->InsertNextTuple(vector.GetDataPointer());
          }
        }
      }
    }

    vtkSmartPointer<vtkCellArray> gridLinesCell = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPolyLine> gridLine = vtkSmartPointer<vtkPolyLine>::New();
    gridLine->GetPointIds()->SetNumberOfIds(2);

    //generate the gridlines between the points
    for (long zPos = 0; zPos< zExtend-1; ++zPos)
    {
      for (long yPos = 0; yPos< yExtend-1; ++yPos)
      {
        for (long xPos = 0; xPos< xExtend-1; ++xPos)
        {
          gridLine->GetPointIds()->SetId(0, xPos+yPos*xExtend+zPos*zOffset);

          if ((yPos%gridFrequence == 0) && (zPos%gridFrequence == 0))
          {
            gridLine->GetPointIds()->SetId(1, (xPos+1)+yPos*xExtend+zPos*zOffset);
            gridLinesCell->InsertNextCell(gridLine);
          }

          if ((xPos%gridFrequence == 0) && (zPos%gridFrequence == 0))
          {
            gridLine->GetPointIds()->SetId(1, xPos+(yPos+1)*xExtend+zPos*zOffset);
            gridLinesCell->InsertNextCell(gridLine);
          }

          if ((xPos%gridFrequence == 0) && (yPos%gridFrequence == 0))
          {
            gridLine->GetPointIds()->SetId(1, xPos+yPos*xExtend+(zPos+1)*zOffset);
            gridLinesCell->InsertNextCell(gridLine);
          }
        }
      }
    }

    vtkSmartPointer<vtkPolyData> grid = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkFloatArray> vectorMagnitude;

    grid->SetPoints(points);
    grid->SetLines(gridLinesCell);

    if(regKernel)
    {
      grid->GetPointData()->SetVectors(vectors);
      grid->GetPointData()->SetActiveVectors(vectors->GetName());

      vtkSmartPointer<vtkVectorNorm> norm = vtkSmartPointer<vtkVectorNorm>::New();
      norm->SetInputData(grid);
      norm->SetAttributeModeToUsePointData();
      norm->Update();

      vectorMagnitude = vtkFloatArray::SafeDownCast(norm->GetOutput()->GetPointData()->GetScalars());
      vectorMagnitude->SetName("VectorMagnitude");
    }

    vtkSmartPointer<vtkPolyData> input = grid;

    if(regKernel)
    {
      vtkSmartPointer<vtkWarpVector> warp = vtkSmartPointer<vtkWarpVector>::New();

      warp->SetInputData(input);
      warp->SetScaleFactor(1);
      warp->Update();

      input = warp->GetPolyDataOutput();

      input->GetPointData()->SetScalars(vectorMagnitude);
      input->GetPointData()->SetActiveScalars(vectorMagnitude->GetName());
    }

    return input;
  }


  bool GridIsOutdated(const mitk::DataNode* regNode, const itk::TimeStamp& reference)
  {

    if (PropertyIsOutdated(regNode,mitk::nodeProp_RegVisGridFrequence,reference))
    {
      return true;
    }

    if (PropertyIsOutdated(regNode,mitk::nodeProp_RegVisFOVSize,reference))
    {
      return true;
    }

    if (PropertyIsOutdated(regNode,mitk::nodeProp_RegVisFOVSpacing,reference))
    {
      return true;
    }

    if (PropertyIsOutdated(regNode,mitk::nodeProp_RegVisFOVOrigin,reference))
    {
      return true;
    }

    if (PropertyIsOutdated(regNode,mitk::nodeProp_RegVisDirection,reference))
    {
      return true;
    }

    return false;
  }

  bool PropertyIsOutdated(const mitk::DataNode* regNode, const std::string& propName, const itk::TimeStamp& reference)
  {
    if (!regNode)
    {
      mitkThrow() << "Cannot check for outdated property. Passed regNode is null.";
    }

    bool result = false;

    mitk::BaseProperty* prop = regNode->GetProperty(propName.c_str());
    if (prop)
    {
      result = reference < prop->GetMTime();
    }
    else
    {
      mitkThrow() << "Cannot check for outdated property. Property not defined in passed regNode. Property name: "<<propName;
    }

    return result;
  }

  const map::core::RegistrationKernelBase<3,3>* GetRelevantRegKernelOfNode(const mitk::DataNode* regNode)
  {
    const map::core::RegistrationKernelBase<3,3>* regKernel = nullptr;

    if (!regNode) return regKernel;

    const mitk::MAPRegistrationWrapper* regWrapper = dynamic_cast<const mitk::MAPRegistrationWrapper*>(regNode->GetData());

    if (!regWrapper) return regKernel;

    const map::core::Registration<3,3>* reg = dynamic_cast<const map::core::Registration<3,3>*>(regWrapper->GetRegistration());

    if (reg)
    {
      mitk::RegVisDirectionProperty* directionProp = nullptr;
      if (regNode->GetProperty(directionProp, mitk::nodeProp_RegVisDirection))
      {
        if (directionProp->GetValueAsId()==0)
        {
          regKernel = &(reg->getDirectMapping());
        }
        else
        {
          regKernel = &(reg->getInverseMapping());
        }
      }
    }
    return regKernel;
  }

}
