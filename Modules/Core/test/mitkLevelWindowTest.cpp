/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkLevelWindow.h"
#include <mitkImage.h>
#include "mitkImageWriteAccessor.h"


/*
 * Reseting the Levelwindow to default values:
 * i.e. Range = -1000..1000, Level = 0 and Window = 500
 */
void resetLevelWindow(mitk::LevelWindow &lw)
{
  //Default window bounds
  lw.SetRangeMinMax(-10000,10000);
  lw.SetLevelWindow(0,500,false);
  if (lw.GetRangeMin() != -10000 || lw.GetRangeMax() != 10000 ||
      lw.GetLevel() != 0 || lw.GetWindow() != 500)
  {
    std::cout << "[Failed] To reset Levelwindow"<<std::endl;
  }
}

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
    std::cout<<defaultWindow<<" [FAILED]"<<std::endl;
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
  if (!(levWin->GetLowerWindowBound() == 6))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetMax ";
  if (!(levWin->GetUpperWindowBound() == 506))
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
  if (!(levWin->GetLevel() == 256) && !(levWin->GetWindow() == 500))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetDefaultLevelWindow ";
  levWin->SetDefaultLevelWindow(20, 200);
  if (!(levWin->GetDefaultLevel() == 20) && !(levWin->GetDefaultWindow() == 200))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow ResetDefaultLevelWindow ";
  levWin->SetLevelWindow(100, 50);
  levWin->ResetDefaultLevelWindow();
  //double a = levWin->GetLevel();
  //double d = levWin->GetWindow();

  if (!((levWin->GetLevel() == 20) &&(levWin->GetWindow() == 200)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetWindowBounds ";
  levWin->SetWindowBounds(0, 2);
  if (!((levWin->GetLowerWindowBound() == 0) && (levWin->GetUpperWindowBound() == 2) && (levWin->GetLevel() == 1) && (levWin->GetWindow() == 2)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with rangemin = rangemax";
  levWin->SetRangeMinMax(2000, 2000);
  if (!(levWin->GetRangeMin() == 1999 && levWin->GetRangeMax() == 2000))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with rangemin > rangemax";
  levWin->SetRangeMinMax(2100, 2000);
  if (!(levWin->GetRangeMin() == 2000 && levWin->GetRangeMax() == 2100))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
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

  std::cout << "Testing mitk::LevelWindow SetDefaultBoundaries with rangemin = rangemax";
  levWin->SetDefaultBoundaries(2000, 2000);
  if (!(levWin->GetDefaultLowerBound() == 1999 && levWin->GetDefaultUpperBound() == 2000))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetDefaultBoundaries with rangemin > rangemax";
  levWin->SetDefaultBoundaries(2100, 2000);
  if (!(levWin->GetDefaultLowerBound() == 2000 && levWin->GetDefaultUpperBound() == 2100))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetDefaultBoundaries ";
  levWin->SetDefaultBoundaries(-2000, 8000);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetDefaultLowerBound ";
  if (!(levWin->GetDefaultLowerBound() == -2000))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow GetDefaultUpperBound ";
  if (!(levWin->GetDefaultUpperBound() == 8000))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow ResetDefaultRangeMinMax ";
  levWin->ResetDefaultRangeMinMax();
  if (!((levWin->GetRangeMin() == levWin->GetDefaultLowerBound()) && (levWin->GetRangeMax() == levWin->GetDefaultUpperBound())))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow minRange > maxRange ";
  levWin->SetRangeMinMax(2000, 1000);
  if (!((levWin->GetRangeMin() == 1000) && (levWin->GetRangeMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(2000, -1000);
  if (!((levWin->GetRangeMin() == -1000) && (levWin->GetRangeMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(-2000, -3000);
  if (!((levWin->GetRangeMin() == -3000) && (levWin->GetRangeMax() == -2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(0, -1000);
  if (!((levWin->GetRangeMin() == -1000) && (levWin->GetRangeMax() == 0)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(2000, 0);
  if (!((levWin->GetRangeMin() == 0) && (levWin->GetRangeMax() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetRangeMinMax(-10000, 10000);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow defaultMinRange > defaultMaxRange ";
  levWin->SetDefaultBoundaries(2000, 1000);
  if (!((levWin->GetDefaultLowerBound() == 1000) && (levWin->GetDefaultUpperBound() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultBoundaries(2000, -1000);
  if (!((levWin->GetDefaultLowerBound() == -1000) && (levWin->GetDefaultUpperBound() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultBoundaries(-2000, -3000);
  if (!((levWin->GetDefaultLowerBound() == -3000) && (levWin->GetDefaultUpperBound() == -2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultBoundaries(0, -1000);
  if (!((levWin->GetDefaultLowerBound() == -1000) && (levWin->GetDefaultUpperBound() == 0)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultBoundaries(2000, 0);
  if (!((levWin->GetDefaultLowerBound() == 0) && (levWin->GetDefaultUpperBound() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetDefaultBoundaries(-10000, 10000);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min > max ";
  levWin->SetWindowBounds(2000, 1000);
  if (!((levWin->GetLowerWindowBound() == 1000) && (levWin->GetUpperWindowBound() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetWindowBounds(2000, -1000);
  if (!((levWin->GetLowerWindowBound() == -1000) && (levWin->GetUpperWindowBound() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetWindowBounds(-2000, -3000);
  if (!((levWin->GetLowerWindowBound() == -3000) && (levWin->GetUpperWindowBound() == -2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetWindowBounds(0, -1000);
  if (!((levWin->GetLowerWindowBound() == -1000) && (levWin->GetUpperWindowBound() == 0)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->SetWindowBounds(2000, 0);
  if (!((levWin->GetLowerWindowBound() == 0) && (levWin->GetUpperWindowBound() == 2000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;


  // minmax > maxrange, minmax < minrange, min<maxrange & max >maxrange, min < minrange & max > minrange
  // max < minrange & min > minrange, min > maxrange & max < maxrange, min < minrange & max > maxrange
  // min > maxrange & max < minrange
  std::cout << "Testing mitk::LevelWindow max > min > maxrange autoexpand = FALSE";
  levWin->SetWindowBounds(11000, 12000, false);
  if (!((levWin->GetLowerWindowBound() == 9999) && (levWin->GetUpperWindowBound() == 10000)))
  {
    std::cout<<"[FAILED]"<<levWin->GetLowerWindowBound()<<","<<levWin->GetUpperWindowBound()<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow max > min > maxrange autoexpand = TRUE";
  levWin->SetWindowBounds(11000, 12000);
  if (!((levWin->GetLowerWindowBound() == 11000) && (levWin->GetUpperWindowBound() == 12000)))
  {
    std::cout<<"[FAILED]"<<levWin->GetLowerWindowBound()<<","<<levWin->GetUpperWindowBound()<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  //Reset default window
  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min > max > maxrange autoexpand = FALSE";
  levWin->SetWindowBounds(12000, 11000, false);
  if (!((levWin->GetLowerWindowBound() == 9999) && (levWin->GetUpperWindowBound() == 10000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min > max > maxrange autoexpand = TRUE";
  levWin->SetWindowBounds(12000, 11000);
  if (!((levWin->GetLowerWindowBound() == 11000) && (levWin->GetUpperWindowBound() == 12000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min < max < minrange autoexpand = FALSE";
  levWin->SetWindowBounds(-12000, -11000, false);
  if (!((levWin->GetLowerWindowBound() == -10000) && (levWin->GetUpperWindowBound() == -9999)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min < max < minrange autoexpand = TRUE";
  levWin->SetWindowBounds(-12000, -11000);
  if (!((levWin->GetLowerWindowBound() == -12000) && (levWin->GetUpperWindowBound() == -11000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow max < min < minrange autoexpand = FALSE";
  levWin->SetWindowBounds(-11000, -12000, false);
  if (!((levWin->GetLowerWindowBound() == -10000) && (levWin->GetUpperWindowBound() == -9999)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow max < min < minrange autoexpand = TRUE";
  levWin->SetWindowBounds(-11000, -12000);
  if (!((levWin->GetLowerWindowBound() == -12000) && (levWin->GetUpperWindowBound() == -11000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min < maxrang & max > maxrange autoexpand = FALSE";
  levWin->SetWindowBounds(9999, 12000, false);
  if (!((levWin->GetLowerWindowBound() == 9999) && (levWin->GetUpperWindowBound() == 10000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min < maxrang & max > maxrange autoexpand = TRUE";
  levWin->SetWindowBounds(9999, 12000);
  if (!((levWin->GetLowerWindowBound() == 9999) && (levWin->GetUpperWindowBound() == 12000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min < minrange & max > minrange autoexpand = FALSE";
  levWin->SetWindowBounds(-11000, -9999, false);
  if (!((levWin->GetLowerWindowBound() == -10000) && (levWin->GetUpperWindowBound() == -9999)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min < minrange & max > minrange autoexpand = TRUE";
  levWin->SetWindowBounds(-11000, -9999);
  if (!((levWin->GetLowerWindowBound() == -11000) && (levWin->GetUpperWindowBound() == -9999)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min < minrange & max > maxrange autoexpand = FALSE";
  levWin->SetWindowBounds(-11000, 11000, false);
  if (!((levWin->GetLowerWindowBound() == -10000) && (levWin->GetUpperWindowBound() == 10000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min < minrange & max > maxrange autoexpand = TRUE";
  levWin->SetWindowBounds(-11000, 11000);
  if (!((levWin->GetLowerWindowBound() == -11000) && (levWin->GetUpperWindowBound() == 11000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow maxrange > min = max > minrange autoexpand = FALSE";
  levWin->SetWindowBounds(5000, 5000, false);
  if (!((levWin->GetLowerWindowBound() == 4999) && (levWin->GetUpperWindowBound() == 5000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow maxrange > min = max > minrange autoexpand = TRUE";
  levWin->SetWindowBounds(5000, 5000);
  if (!((levWin->GetLowerWindowBound() == 4999) && (levWin->GetUpperWindowBound() == 5000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min = max = minrange autoexpand = FALSE";
  levWin->SetWindowBounds(-10000, -10000, false);
  if (!((levWin->GetLowerWindowBound() == -10000) && (levWin->GetUpperWindowBound() == -9999)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min = max = minrange autoexpand = TRUE";
  levWin->SetWindowBounds(-10000, -10000);
  if (!((levWin->GetLowerWindowBound() == -10000) && (levWin->GetUpperWindowBound() == -9999)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min = max = maxrange autoexpand = FALSE";
  levWin->SetWindowBounds(10000, 10000, false);
  if (!((levWin->GetLowerWindowBound() == 9999) && (levWin->GetUpperWindowBound() == 10000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min = max = maxrange autoexpand = TRUE";
  levWin->SetWindowBounds(10000, 10000);
  if (!((levWin->GetLowerWindowBound() == 9999) && (levWin->GetUpperWindowBound() == 10000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min = max > maxrange autoexpand = FALSE";
  levWin->SetWindowBounds(11000, 11000, false);
  if (!((levWin->GetLowerWindowBound() == 9999) && (levWin->GetUpperWindowBound() == 10000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min = max > maxrange autoexpand = TRUE";
  levWin->SetWindowBounds(11000, 11000);
  if (!((levWin->GetLowerWindowBound() == 10999) && (levWin->GetUpperWindowBound() == 11000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min = max < minrange autoexpand = FALSE";
  levWin->SetWindowBounds(-11000, -11000, false);
  if (!((levWin->GetLowerWindowBound() == -10000) && (levWin->GetUpperWindowBound() == -9999)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min = max < minrange autoexpand = TRUE";
  levWin->SetWindowBounds(-11000, -11000);
  if (!((levWin->GetLowerWindowBound() == -11000) && (levWin->GetUpperWindowBound() == -10999)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow maxrange > min > minrange > max autoexpand = FALSE";
  levWin->SetWindowBounds(-9000, -11000, false);
  if (!((levWin->GetLowerWindowBound() == -10000) && (levWin->GetUpperWindowBound() == -9000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow maxrange > min > minrange > max autoexpand = TRUE";
  levWin->SetWindowBounds(-9000, -11000, true);
  if (!((levWin->GetLowerWindowBound() == -11000) && (levWin->GetUpperWindowBound() == -9000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow min > maxrange > minrange > max autoexpand = FALSE";
  levWin->SetWindowBounds(11000, -11000, false);
  if (!((levWin->GetLowerWindowBound() == -10000) && (levWin->GetUpperWindowBound() == 10000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow min > maxrange > minrange > max autoexpand = TRUE";
  levWin->SetWindowBounds(11000, -11000);
  if (!((levWin->GetLowerWindowBound() == -11000) && (levWin->GetUpperWindowBound() == 11000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with maxrange < min < max autoexpand = FALSE";
  levWin->SetRangeMinMax(-20000, -15000);
  if (!((levWin->GetLowerWindowBound() == -15001) && (levWin->GetUpperWindowBound() == -15000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with minrange > maxrange & maxrange < min < max ";
  levWin->ResetDefaultLevelWindow();
  levWin->SetRangeMinMax(-15000, -20000);
  if (!((levWin->GetLowerWindowBound() == -15001) && (levWin->GetUpperWindowBound() == -15000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with minrange < min < maxrange < max autoexpand = FALSE";
  levWin->SetRangeMinMax(-80, 1000);
  levWin->SetWindowBounds(-1000,110, false);
  if (!((levWin->GetLowerWindowBound() == -80) && (levWin->GetUpperWindowBound() == 110)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with minrange < min < maxrange < max autoexpand = TRUE";
  levWin->SetRangeMinMax(-80, 1000);
  levWin->SetWindowBounds(-1000,110);
  if (!((levWin->GetLowerWindowBound() == -1000) && (levWin->GetUpperWindowBound() == 110)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with maxrange < minrange & minrange < min < maxrange < max autoexpand = FALSE";
  levWin->SetRangeMinMax(1000,-80);
  levWin->SetWindowBounds(-1000,110, false );
  if (!((levWin->GetLowerWindowBound() == -80) && (levWin->GetUpperWindowBound() == 110)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with maxrange < minrange & minrange < min < maxrange < max autoexpand = TRUE";
  levWin->SetRangeMinMax(1000,-80);
  levWin->SetWindowBounds(-1000,110 );
  if (!((levWin->GetLowerWindowBound() == -1000) && (levWin->GetUpperWindowBound() == 110)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  resetLevelWindow(*levWin);

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with min < minrange < maxrange <max ";
  levWin->SetRangeMinMax(20, 110);
  if (!((levWin->GetLowerWindowBound() == 20) && (levWin->GetUpperWindowBound() == 110)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with minRange > maxRange & min < maxrange < max ";
  levWin->SetWindowBounds(-90,1000);
  levWin->SetRangeMinMax(100, -80);

  if (!((levWin->GetLowerWindowBound() == -80) && (levWin->GetUpperWindowBound() == 100)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with minRange > maxRange & min < minrange < maxrange <max ";
  levWin->SetRangeMinMax(20, 100);
  if (!((levWin->GetLowerWindowBound() == 20) && (levWin->GetUpperWindowBound() == 100)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with min < max < minrange ";
  levWin->SetRangeMinMax(20000, 15000);
  if (!((levWin->GetLowerWindowBound() == 15000) && (levWin->GetUpperWindowBound() == 15001)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with minrange > maxrange & min < max < minrange ";
  levWin->SetRangeMinMax(20000, 15000);
  if (!((levWin->GetLowerWindowBound() == 15000) && (levWin->GetUpperWindowBound() == 15001)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindow SetRangeMinMax with min < minrange <max ";
  levWin->SetRangeMinMax(-20000, -15000);
  if (!((levWin->GetLowerWindowBound() == -15001) && (levWin->GetUpperWindowBound() == -15000)))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  levWin->ResetDefaultRangeMinMax();
  levWin->ResetDefaultLevelWindow();
  std::cout<<"[PASSED]"<<std::endl;

  // auch für default levelwindow und default range

  //Create Image out of nowhere
  mitk::Image::Pointer image;
  //mitk::PixelType pt(typeid(int));
  unsigned int dim[]={100,100,20};

  std::cout << "Creating image: ";
  image = mitk::Image::New();
  //image->DebugOn();

  image->Initialize( mitk::MakePixelType<int, int, 1>(), 3, dim);
  mitk::ImageWriteAccessor imAccess(image);
  int *p = (int*)imAccess.GetData();

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

  mitk::LevelWindow levelWindow(50,100);
  levelWindow.SetRangeMinMax(0,100);
  // test range restriction/adaption for SetLevelWindow and SetWindowBounds
  std::cout << "Testing range restriction of mitk::LevelWindow::SetWindowBounds() autoexpand = FALSE";
  mitk::ScalarType initialUpperBound = levelWindow.GetUpperWindowBound();
  mitk::ScalarType initialLowerBound = levelWindow.GetLowerWindowBound();
  levelWindow.SetWindowBounds( -10, 110, false );
  if ( levelWindow.GetUpperWindowBound() != initialUpperBound ||
       levelWindow.GetLowerWindowBound() != initialLowerBound )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing range restriction of mitk::LevelWindow::SetWindowBounds() autoexpand = TRUE";
  levelWindow.SetWindowBounds( -10, 110 );
  if ( levelWindow.GetUpperWindowBound() != 110 ||
       levelWindow.GetLowerWindowBound() != -10 )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  levelWindow.SetRangeMinMax(0,100);
  levelWindow.SetLevelWindow(50,100);

  std::cout << "Testing range restriction of mitk::LevelWindow::SetLevelWindow() autoexpand = FALSE";
  levelWindow.SetLevelWindow( 60, 100, false );
  if ( levelWindow.GetUpperWindowBound() != initialUpperBound )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing range restriction of mitk::LevelWindow::SetLevelWindow() autoexpand = TRUE";
  levelWindow.SetLevelWindow( 60, 100 );
  if ( levelWindow.GetUpperWindowBound() != initialUpperBound+10 )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  levelWindow.SetRangeMinMax(0,100);
  levelWindow.SetLevelWindow(50,100);

  std::cout << "Testing range restriction of mitk::LevelWindow::SetLevelWindow() autoexpand = FALSE";
  levelWindow.SetLevelWindow( 40, 100, false );
  if ( levelWindow.GetLowerWindowBound() != initialLowerBound )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing range restriction of mitk::LevelWindow::SetLevelWindow() autoexpand = TRUE";
  levelWindow.SetLevelWindow( 40, 100 );
  if ( levelWindow.GetLowerWindowBound() != initialLowerBound-10 )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing range adaption of mitk::LevelWindow::SetWindowBounds()";
  levelWindow.SetWindowBounds(-10,90,true); // ture == force
  if ( levelWindow.GetUpperWindowBound() != 90.0 ||
       levelWindow.GetLowerWindowBound() != -10.0 )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing range adaption of mitk::LevelWindow::SetWindowBounds()";
  levelWindow.SetWindowBounds(-20,110,true); // ture == force
  if ( levelWindow.GetUpperWindowBound() != 110.0 ||
       levelWindow.GetLowerWindowBound() != -20.0 )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing range adaption of mitk::LevelWindow::SetLevelWindow()";
  levelWindow.SetLevelWindow(50,140,true); // ture == force
  if ( levelWindow.GetUpperWindowBound() != 120.0 ||
       levelWindow.GetLowerWindowBound() != -20.0 )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing c'tor with x-ray values (formerly did not expand range enough)";
  mitk::LevelWindow crLevelWindow(16352, 16444);
  if ( crLevelWindow.GetLevel() != 16352.0 ||
       crLevelWindow.GetWindow() != 16444.0 )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;



  std::cout<<"[PASSED]"<<std::endl;
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
