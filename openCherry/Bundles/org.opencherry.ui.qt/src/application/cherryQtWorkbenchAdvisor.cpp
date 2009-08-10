/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryQtWorkbenchAdvisor.h"
#include "../internal/cherryQtGlobalEventFilter.h"

#include <cherryPlatform.h>

#include <QApplication>

#include <vector>

namespace cherry
{

void QtWorkbenchAdvisor::Initialize(IWorkbenchConfigurer::Pointer configurer)
{
  WorkbenchAdvisor::Initialize(configurer);

  char** argv;
  int& argc = Platform::GetRawApplicationArgs(argv);

 // CHERRY_INFO << "QtWorkbenchAdvisor::Initialize argv[0] = " << argv[0] << " argv[1] = " << argv[1] << std::endl;

  QApplication* app = new QApplication(argc, argv);
  QObject* eventFilter = new QtGlobalEventFilter(app);
  app->installEventFilter(eventFilter);
  app->setStyleSheet("cherry--QCTabBar::tab { "
      "background: palette(window); "
      "min-height: 24px; "
      "border-top: 1px solid palette(mid); "
      "border-left: 1px solid palette(mid); "
      "border-bottom: 1px solid palette(mid); "
      "margin-top: -1px; }"

      "cherry--QCTabBar::tab:only-one { border-top-left-radius: 8px; "
                              "border-top-right-radius: 8px; "
                              "min-height: 25px; "
                              "margin-left: -1px }"

      "cherry--QCTabBar::tab:first { margin-left: -1px;"
      "border-left: 0px; "
      "border-top-left-radius: 8px; }"

      "cherry--QCTabBar::tab:last { border-right: 1px solid palette(mid); "
      "                    border-top-right-radius: 8px }"

      "cherry--QCTabBar::tab:previous-selected { border-left: 0px }"

      "cherry--QCTabBar::tab:selected { "
      "border-right: 1px solid palette(Mid); "
      "border-bottom: 0px; "
      "border-top-left-radius: 8px; "
      "border-top-right-radius: 8px; }"

      "cherry--QCTabBar QToolButton {"
      "border: 1px solid palette(window); "
      "max-height: 22px; "
      "background: palette(window);  }"

      "cherry--QCTabBar QToolButton#TabCloseButton {"
      "border: none; "
      "background: none; }"

      "QWidget#TabTopRightControls {"
      "border: 1px solid palette(mid); "
      "border-left: 0px; "
      "border-top: 0px; "
      "border-right: 0px; "
      "}"

      "QWidget#ViewForm {"
      "border: 1px solid palette(mid); "
      "border-top-left-radius: 8px; "
      "border-top-right-radius: 8px; }"

      "QWidget#StandaloneViewForm {"
      "border: 1px solid palette(mid); }"
  );

}

}
