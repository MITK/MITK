#include <CreatePOVMesh.h>
#include <itksys/SystemTools.hxx>

int vtkPolyDataToPovRayMesh2Test (int argc, char * argv[])
{
  std::string fileOut = "ParameterizedMeshPOVWriter";
  fileOut = itksys::SystemTools::ConvertToOutputPath(fileOut.c_str());
  std::cout<<"Output File to: "<<fileOut<<std::endl;

  bool error = true;
  error = itkMeshToIndexedTriangleMeshFilterTest(fileOut);

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
  if ( !mySphereSource ) return EXIT_FAILURE;

  // Convert sphere to itk::Mesh
  MeshType::Pointer itkmesh = MeshType::New();
  itkmesh = MeshUtilType::meshFromPolyData( mySphereSource->GetOutput() );
  if ( ! itkmesh ) return EXIT_FAILURE;

  // Convert itk::Mesh to itk::IndexedTriangleMesh
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( itkmesh );
  filter->SetDeepCopy( false );
  filter->GenerateData();
  if ( !filter ) return EXIT_FAILURE;

  ITMeshType::Pointer dummy = ITMeshType::New();
  dummy = filter->GetOutput();
    if ( !dummy ) return EXIT_FAILURE;


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

  if( writer->GetNumberOfInputs() < 1 )
  {
     std::cout<<"[FAILED]"<<std::endl;
     writer->Delete();
     return EXIT_FAILURE;
  }
    else
  {
     std::cout<<"[PASSED]"<<std::endl;
     writer->Write();
  }

  return writer->GetWriteError();
}
