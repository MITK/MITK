#include "mitkLevelWindow.h"
#include <mitkImage.h>

int mitkLevelWindowTest(int, char* [])
{
  std::cout << "Testing mitk::LevelWindow  "<<std::endl;

  std::cout << "Testing mitk::LevelWindow constructor with Level and Window ";
  mitk::LevelWindow* levWin = new mitk::LevelWindow(256, 500);
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout << "Testing mitk::LevelWindow GetDefaultWindow ";
  mitk::ScalarType defaultWindow = levWin->GetDefaultWindow();
  if (!(defaultWindow == 500))
  {
    std::cout<<(int)(defaultWindow) + "[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetDefaultLevel ";
  mitk::ScalarType defaultLevel = levWin->GetDefaultLevel();
  if (!(defaultLevel == 256))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetWindow ";
  mitk::ScalarType window = levWin->GetWindow();
  if (!(window == 500))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetMin ";
  if (!(levWin->GetMin() == 6))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetMax ";
  if (!(levWin->GetMax() == 506))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetLevel ";
  mitk::ScalarType level = levWin->GetLevel();
  if (!(level == 256))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout << "Testing mitk::LevelWindow GetWindow : GetDefaultWindow ";
  if (!(defaultWindow == window))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetLevel : GetDefaultLevel ";
  if (!(defaultLevel == level))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetLevel ";
  levWin->SetLevel(20);
  if (!(levWin->GetLevel() == 20))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetWindow ";
  levWin->SetWindow(100);
  if (!(levWin->GetWindow() == 100))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetDefaultLevelWindow & SetLevelWindow ";
  levWin->SetLevelWindow(levWin->GetDefaultLevel(), levWin->GetDefaultWindow());
  if (!(levWin->GetLevel() == 256) || !(levWin->GetWindow() == 500))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetRangeMin ";
  mitk::ScalarType rangeMin = levWin->GetRangeMin();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetRangeMax ";
  mitk::ScalarType rangeMax = levWin->GetRangeMax();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetRange ";
  if (!((rangeMax - rangeMin) == levWin->GetRange()))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax ";
  levWin->SetRangeMinMax(-1000, 2000);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetRange ";
  if (!((levWin->GetRangeMax() - levWin->GetRangeMin()) == levWin->GetRange()))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetDefaultRangeMin ";
  if (!((rangeMin) == levWin->GetDefaultRangeMin()))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetDefaultRangeMax ";
  if (!((rangeMax) == levWin->GetDefaultRangeMax()))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  




  
























































  //Create Image out of nowhere
	mitk::Image::Pointer image;
	mitk::PixelType pt(typeid(int));
	unsigned int dim[]={100,100,20};

  std::cout << "Creating image: ";
	image = mitk::Image::New();
  //image->DebugOn();
	image->Initialize(mitk::PixelType(typeid(int)), 3, dim);
  int *p = (int*)image->GetData();

  int size = dim[0]*dim[1]*dim[2];
  int i;
  for(i=0; i<size; ++i, ++p)
    *p=i;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetAuto ";
  mitk::LevelWindow levelwindow;
  levelwindow.SetAuto( image );
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow constructor with mitkLevelWindow ";
  const mitk::LevelWindow* lw = new mitk::LevelWindow(levelwindow);
  std::cout<<"[PASSED]"<<std::endl;



















  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
