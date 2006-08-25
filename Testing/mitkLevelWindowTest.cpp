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

  std::cout << "Testing mitk::LevelWindow SetLevelWindow ";
  levWin->SetLevelWindow(20, 100);
  if (!(levWin->GetLevel() == 20))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  if (!(levWin->GetWindow() == 100))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetLevelWindow ";
  levWin->SetLevelWindow(levWin->GetDefaultLevel(), levWin->GetDefaultWindow());
  if (!(levWin->GetLevel() == 256) || !(levWin->GetWindow() == 500))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetDefaultLevelWindow ";
  levWin->SetDefaultLevelWindow(20, 200);
  if (!(levWin->GetDefaultLevel() == 20) || !(levWin->GetDefaultWindow() == 200))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow ResetDefaultLevelWindow ";
  levWin->ResetDefaultLevelWindow();
  if (!((levWin->GetLevel() == levWin->GetDefaultLevel()) &&(levWin->GetWindow() == levWin->GetDefaultWindow())))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetMinMax ";
  levWin->SetMinMax(0, 2);
  if (!((levWin->GetMin() == 0) && (levWin->GetMax() == 2) && (levWin->GetLevel() == 1) && (levWin->GetWindow() == 2)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax ";
  levWin->SetRangeMinMax(-1000, 2000);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetRangeMin ";
  if (!(levWin->GetRangeMin() == -1000))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetRangeMax ";
  if (!(levWin->GetRangeMax() == 2000))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetRange ";
  if (!((levWin->GetRangeMax() - levWin->GetRangeMin()) == levWin->GetRange()))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetDefaultRangeMinMax ";
  levWin->SetDefaultRangeMinMax(-2000, 8000);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetDefaultRangeMin ";
  if (!(levWin->GetDefaultRangeMin() == -2000))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetDefaultRangeMax ";
  if (!(levWin->GetDefaultRangeMax() == 8000))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow ResetDefaultRangeMinMax ";
  levWin->ResetDefaultRangeMinMax();
  if (!((levWin->GetRangeMin() == levWin->GetDefaultRangeMin()) && (levWin->GetRangeMax() == levWin->GetDefaultRangeMax())))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow minRange > maxRange ";
  levWin->SetRangeMinMax(2000, 1000);
  if (!((levWin->GetRangeMin() == 1000) || (levWin->GetRangeMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(2000, -1000);
  if (!((levWin->GetRangeMin() == -1000) || (levWin->GetRangeMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(-2000, -3000);
  if (!((levWin->GetRangeMin() == -3000) || (levWin->GetRangeMax() == -2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(0, -1000);
  if (!((levWin->GetRangeMin() == -1000) || (levWin->GetRangeMax() == 0)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(2000, 0);
  if (!((levWin->GetRangeMin() == 0) || (levWin->GetRangeMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(-10000, 10000);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow defaultMinRange > defaultMaxRange ";
  levWin->SetDefaultRangeMinMax(2000, 1000);
  if (!((levWin->GetDefaultRangeMin() == 1000) || (levWin->GetDefaultRangeMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultRangeMinMax(2000, -1000);
  if (!((levWin->GetDefaultRangeMin() == -1000) || (levWin->GetDefaultRangeMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultRangeMinMax(-2000, -3000);
  if (!((levWin->GetDefaultRangeMin() == -3000) || (levWin->GetDefaultRangeMax() == -2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultRangeMinMax(0, -1000);
  if (!((levWin->GetDefaultRangeMin() == -1000) || (levWin->GetDefaultRangeMax() == 0)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultRangeMinMax(2000, 0);
  if (!((levWin->GetDefaultRangeMin() == 0) || (levWin->GetDefaultRangeMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultRangeMinMax(-10000, 10000);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min > max ";
  levWin->SetMinMax(2000, 1000);
  if (!((levWin->GetMin() == 1000) || (levWin->GetMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetMinMax(2000, -1000);
  if (!((levWin->GetMin() == -1000) || (levWin->GetMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetMinMax(-2000, -3000);
  if (!((levWin->GetMin() == -3000) || (levWin->GetMax() == -2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetMinMax(0, -1000);
  if (!((levWin->GetMin() == -1000) || (levWin->GetMax() == 0)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetMinMax(2000, 0);
  if (!((levWin->GetMin() == 0) || (levWin->GetMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  //minmax > maxrange, minmax < minrange, min<maxrange & max >maxrange, min < minrange & max > minrange
  // max < minrange & min > minrange, min > maxrange & max < maxrange
  // gleiches für levelwindow
  // setmaxrange < max, < level, < min
  // setminrange > min, > level, > max
  // auch für default levelwindow und default range

  
























































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
  if (!(lw->GetRange() == levelwindow.GetRange()))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout << "Testing mitk::LevelWindow destructor ";
  delete levWin;
  delete lw;
  std::cout<<"[PASSED]"<<std::endl;


















  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
