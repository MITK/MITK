#include "mitkContour.h"
#include "mitkDataTreeNode.h"
#include "mitkContourMapper2D.h"
#include "mitkCommon.h"

#include <fstream>
int mitkContourMapper2DTest(int argc, char* argv[])
{
  mitk::Contour::Pointer contour;
  mitk::ContourMapper2D::Pointer contourMapper;
  mitk::DataTreeNode::Pointer node;
  contourMapper = mitk::ContourMapper2D::New();
  node = mitk::DataTreeNode::New();
  std::cout << "Testing mitk::ContourMapper2D::New(): ";

  contour = mitk::Contour::New();
  node->SetData(contour);

  if (contour.IsNull()) {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  } 

  contourMapper->SetInput( node );
  contourMapper->Update();

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
