#include <mitkMesh.h>
#include <mitkVector.h>
#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>
#include <fstream>

int mitkMeshTest(int argc, char* argv[])
{
  //Create mesh
  mitk::Mesh::Pointer mesh;
  mesh = mitk::Mesh::New();

  //try to get the itkmesh
  mitk::Mesh::DataType::Pointer itkdata = NULL;
  itkdata = mesh->GetMesh();
  if (itkdata.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //fresh mesh has to be empty!
  if (mesh->GetSize() != 0)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //create an operation and add a point.
  int position = 0;
  mitk::Point3D itkPoint;
  itkPoint.Fill(1);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, itkPoint, position);
	mesh->ExecuteOperation(doOp);

  //now check new condition!
  if ( (mesh->GetSize()!=1) ||
      (!mesh->IndexExists(position)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  
  //get the point and check if it is still the same
  mitk::Point3D tempPoint;
  tempPoint.Fill(0);
  tempPoint = mesh->GetItkPoint(position);
  if (tempPoint != itkPoint)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }


  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}