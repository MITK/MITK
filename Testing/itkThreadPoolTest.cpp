#include <itkThreadPool.h>

int itkThreadPoolTest(int /*i*/, char** /*argv*/)
{
  itk::ThreadPool::Pointer threadPool = itk::ThreadPool::New();

  return EXIT_SUCCESS;
}

