#include <mitkRingSpline.h>

#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>
#include <fstream>

int mitkRingSplineTest(int /*argc*/, char* /*argv*/[])
{  //Create RingSpline
  mitk::RingSpline::Pointer cs = mitk::RingSpline::New();

  //try to get the itkRingSpline
  std::cout << "Create a RingSpline and try to get the itkRingSpline";
  mitk::RingSpline::DataType::Pointer itkdata = NULL;
  itkdata = cs->GetPointSet();
  if (itkdata.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //fresh RingSpline has to be empty!
  std::cout << "Is the RingSpline empty?";
  if (cs->GetSize() != 0)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //create an operation and add a point.
  int position;
  mitk::Point3D point;
  for (int i = 1; i <6; ++i)
  {
    position = i-1;
    point.Fill( (i%2)?i:i*5 );
    mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpINSERT, point, position);
    cs->ExecuteOperation(doOp);
    
    //now check new condition!
    if ( (cs->GetSize()!= i) )
    {
      std::cout<<"[FAILED] while: "<<cs->GetSize()<<std::endl;
      return EXIT_FAILURE;
    }
  }

  if (!cs->SplineExist())
  {
    std::cout<<"Spline no exist: [FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }


  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
