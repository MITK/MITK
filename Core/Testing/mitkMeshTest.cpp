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


#include <mitkMesh.h>
#include <mitkVector.h>
#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>
#include <fstream>

int mitkMeshTest(int /*argc*/, char* /*argv*/[])
{
  //Create mesh
  mitk::Mesh::Pointer mesh;
  mesh = mitk::Mesh::New();

  //try to get the itkmesh
  std::cout << "Create a mesh and try to get the itkMesh";
  mitk::Mesh::DataType::Pointer itkdata = NULL;
  itkdata = mesh->GetMesh();
  if (itkdata.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //fresh mesh has to be empty!
  std::cout << "Is the mesh empty?";
  if (mesh->GetSize() != 0)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //create an operation and add a point.
  int position = 0;
  mitk::Point3D point;
  point.Fill(1);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, point, position);
	mesh->ExecuteOperation(doOp);

  //now check new condition!
  if ( (mesh->GetSize()!=1) ||
      (!mesh->IndexExists(position)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;
  
  //get the point and check if it is still the same
  std::cout << "Create an operation and add a point. Then try to get that point.";
  mitk::Point3D tempPoint;
  tempPoint.Fill(0);
  tempPoint = mesh->GetPoint(position);
  if (tempPoint != point)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }


  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
