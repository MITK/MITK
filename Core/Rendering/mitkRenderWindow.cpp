#include "mitkRenderWindow.h"
#include <assert.h>

std::set<mitk::RenderWindow*> mitk::RenderWindow::instances;

//void mitk::RenderWindow::MakeCurrent() 
//{
//  assert(false);
//};
//
//void mitk::RenderWindow::SwapBuffers() 
//{
//  assert(false);
//};
//
//bool mitk::RenderWindow::IsSharing () const
//{
//  assert(false);
//  return false;
//}
//
//void mitk::RenderWindow::Update()
//{
//  assert(false);
//}