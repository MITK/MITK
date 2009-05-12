#include <itkDemoThreadPool.h>
#include <itkTPDemoTask.h>

namespace itk
{
class ExceptionTask : public TPTask
{
  public:
    typedef ExceptionTask Self;
    typedef TPTask Superclass;
    typedef SmartPointer<Self> Pointer;
    itkNewMacro(Self);

    void DoAction()
    {
      itkExceptionMacro("This is a test exception!");
    }
};
}

// first of all a simple thread pool with one task
int testSimpleTask()
{
  std::cout << " *** Testing a simple task ***\n";

  itk::TPDemoTask::Pointer demoTask = itk::TPDemoTask::New();
  demoTask->SetSleepTime(1);
  
  itk::ThreadPool::Pointer threadPool = itk::ThreadPool::New();
  threadPool->EnqueueTask(demoTask.GetPointer());
  threadPool->Run(1);
  
  std::cout << " *** [TEST PASSED] ***\n";

  return EXIT_SUCCESS;
}

// now test with the demo task if new tasks can be created
int testTaskCreate()
{
  std::cout << " *** Testing the creation of new tasks ***\n";

  itk::TPDemoTask::Pointer demoTask = itk::TPDemoTask::New();
  demoTask->SetSleepTime(1);
  
  itk::DemoThreadPool::Pointer threadPool = itk::DemoThreadPool::New();
  threadPool->SetMaxSleepedSeconds(2);
  threadPool->EnqueueTask(demoTask.GetPointer());
  threadPool->Run(1);

  unsigned int runnedTasks = threadPool->GetRunnedTasks();

  if(runnedTasks < 2) 
  {
    std::cout << "No tasks have been created!" << std::endl;
    return EXIT_FAILURE;
  }
  
  std::cout << " *** [TEST PASSED] ***\n";

  return EXIT_SUCCESS;
}

int testExceptionTask()
{
  std::cout << " *** Testing the exception handling ***\n";

  try
  {
    itk::ExceptionTask::Pointer exceptionTask = itk::ExceptionTask::New();
  
    itk::ThreadPool::Pointer threadPool = itk::ThreadPool::New();
    threadPool->EnqueueTask(exceptionTask.GetPointer());
    threadPool->Run(1);
  }
  catch (itk::ExceptionObject &e)
  {
    std::cout << "Catched exception " << e << std::endl;
    std::cout << " *** [TEST PASSED] ***\n";
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

int itkThreadPoolTest(int /*i*/, char* argv[])
{
  std::list<int> resultList;
  int failedCount = 0;
  int testCount;
  float failRatio;

  resultList.push_back(testSimpleTask());
  resultList.push_back(testTaskCreate());
  resultList.push_back(testExceptionTask());

  std::cout << " *** [ALL TESTS DONE] ***\n";

  testCount = resultList.size();
  bool allSuccess = true;

  while (resultList.size() > 0)
  {
    int value = resultList.front();
    if(value == EXIT_FAILURE) 
    { 
      failedCount++; 
      allSuccess = false;
    }
    resultList.pop_front();
  }

  failRatio = 100 * (float) failedCount / (float) testCount;

  std::cout << "Result: " << failedCount << "/" << testCount << " Tests failed (" << failRatio << " %)" << std::endl;

  if(allSuccess)
  {
    return EXIT_SUCCESS;
  }
  else 
  {
    return EXIT_FAILURE;
  }
}

