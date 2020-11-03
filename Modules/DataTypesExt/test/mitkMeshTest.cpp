/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <fstream>
#include <mitkInteractionConst.h>
#include <mitkMesh.h>
#include <mitkNumericTypes.h>
#include <mitkPointOperation.h>

int mitkMeshTest(int /*argc*/, char * /*argv*/ [])
{
  // Create mesh
  mitk::Mesh::Pointer mesh;
  mesh = mitk::Mesh::New();

  // try to get the itkmesh
  std::cout << "Create a mesh and try to get the itkMesh";
  mitk::Mesh::DataType::Pointer itkdata = nullptr;
  itkdata = mesh->GetMesh();
  if (itkdata.IsNull())
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // fresh mesh has to be empty!
  std::cout << "Is the mesh empty?";
  if (mesh->GetSize() != 0)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // create an operation and add a point.
  int position = 0;
  mitk::Point3D point;
  point.Fill(1);
  auto doOp = new mitk::PointOperation(mitk::OpINSERT, point, position);
  mesh->ExecuteOperation(doOp);

  // now check new condition!
  if ((mesh->GetSize() != 1) || (!mesh->IndexExists(position)))
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  delete doOp;

  // get the point and check if it is still the same
  std::cout << "Create an operation and add a point. Then try to get that point.";
  mitk::Point3D tempPoint;
  tempPoint.Fill(0);
  tempPoint = mesh->GetPoint(position);
  if (tempPoint != point)
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // well done!!! Passed!
  std::cout << "[PASSED]" << std::endl;

  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
