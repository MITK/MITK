#include "mitkContour.h"
#include "mitkDataTreeNode.h"
#include "mitkContourMapper2D.h"
#include "mitkCommon.h"
#include "mitkDataTree.h"

#include "mitkOpenGLRenderer.h"
#include "mitkRenderWindow.h"

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
  mitk::Contour* testContour = (mitk::Contour*)contourMapper->GetInput();
  std::cout << testContour << std::endl;
  
  mitk::DataTree::Pointer dataTree = mitk::DataTree::New();
  mitk::DataTreePreOrderIterator it(dataTree);
  
  it.Add( node );
  
  mitk::BoundingBox::Pointer bounds = mitk::DataTree::ComputeBoundingBox(&it);
  
  std::cout << "bounds: " << bounds << std::endl;

  bounds = mitk::DataTree::ComputeVisibleBoundingBox(&it);
  std::cout << "visible bounds: " << bounds << std::endl;
  
  mitk::OpenGLRenderer::Pointer renderer = mitk::OpenGLRenderer::New();
  
  //  mitk::QmitkRenderWindow *rw = new mitk::QmitkRenderWindow(renderer, NULL, NULL);  

  
  std::cout<<"Testing mitk::BaseRenderer::SetData()"<<std::endl;
  
  renderer->SetData(&it);

  std::cout<<"testing mitk::OpenGLRenderer::GetRenderWindow()"<<std::endl;
  /*  mitk::RenderWindow* window = renderer->GetRenderWindow();
  if (window == NULL)
  {
      return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  */
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
