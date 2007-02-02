//#include <iostream>
//#include <iomanip>
#include "CreatePOVMesh.h"
#include <itksys/SystemTools.hxx>


int vtkPolyDataToPovRayMesh2Test(int /*argc*/ , char * /*argv*/[] )
{
  std::string fileOut = "ParameterizedMeshPOVWriterTest";
  fileOut = itksys::SystemTools::ConvertToOutputPath(fileOut.c_str());
  std::cout << "Output File to: " << fileOut<<std::endl;

  bool error = true;
  error = itkMeshToIndexedTriangleMeshFilterTest(fileOut);
  std::cout << "Writer returned error state: " << std::boolalpha << error << std::endl;

  if ( !error ) std::cout << "[TEST DONE]" << std::endl;
  else {std::cout << "[FAILED]" << std::endl;
        return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


bool itkMeshToIndexedTriangleMeshFilterTest(std::string outFile)
{
  // Construct test sphere
  vtkSphereSource *mySphereSource = vtkSphereSource::New();
  mySphereSource->SetRadius(5.);
  mySphereSource->Update();
  if ( !mySphereSource ) {
    std::cout << "Unable to create vtk::Sphere" << std::endl;
    return EXIT_FAILURE;
  }
  else std::cout << "Created test sphere." << std::endl;

  // Convert sphere to itk::Mesh
  MeshType::Pointer itkmesh = MeshType::New();
  //std::cout << "Created empty mesh" << std::endl;
  itkmesh = MeshUtilType::MeshFromPolyData( mySphereSource->GetOutput() );
  if ( ! itkmesh ) {
    std::cout << "Unable to convert test sphere to itk::Mesh" << std::endl;
    return EXIT_FAILURE;
  }
  else std::cout << "Converted test sphere to itk::Mesh." << std::endl;

  // Convert itk::Mesh to itk::IndexedTriangleMesh
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( itkmesh );
  filter->SetDeepCopy( false );
  filter->GenerateData();
  if ( !filter ) {
    std::cout << "Unable to instantiate filter" << std::endl;
    return EXIT_FAILURE;
  }

  ITMeshType::Pointer dummy = ITMeshType::New();
  dummy = filter->GetOutput();
    if ( !dummy ) {
      std::cout << "Unable to convert mesh to Indexed triangle mesh" << std::endl;
      return EXIT_FAILURE;
    }
    else std::cout << "Converted mesh to indexed triangle mesh." << std::endl;

  // Try if conversation to itk::SphericalParameterizedTriangleMesh is possible;
  // (would allow the use of UV Coordinates)
  ParameterizationType *mesh = dynamic_cast<ParameterizationType*>(dummy.GetPointer() );  

  // Write out all available information (checked internally)
  POVWriter::Pointer writer = POVWriter::New();
  if (mesh != 0 ) writer->SetInput( mesh );
  else writer->SetInput( dummy );
  writer->SetSubdivisionLevel(2);
  writer->SetTranslationInX(0);
  writer->SetTranslationInY(0);
  writer->SetFilePrefix(outFile.c_str());
  std::cout << "Writer initialized, starting write process: " << std::endl;

  if( writer->GetNumberOfInputs() < 1 )
  {
     std::cout << "[FAILED]: Writer has no outputs." << std::endl;
     writer->Delete();
     return EXIT_FAILURE;
  }
    else
  {
     writer->Write();
     std::cout << "[PASSED]" << std::endl;
  }
  return writer->GetWriteError();
}
