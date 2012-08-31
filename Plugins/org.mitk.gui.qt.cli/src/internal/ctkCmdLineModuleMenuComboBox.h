/*=============================================================================

  Library: CTK

  Copyright (c) University College London

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#ifndef CTKCMDLINEMODULEMENUCOMBOBOX_H
#define CTKCMDLINEMODULEMENUCOMBOBOX_H

#include <QObject>
#include <ctkMenuComboBox.h>

/**
 * \class ctkCmdLineModuleMenuComboBox
 * \brief Subclass of ctkMenuComboBox to listen to ctkCmdLineModuleManager
 * moduleRegistered and moduleUnregistered signals, and update the menu accordingly.
 * \ingroup org_mitk_gui_qt_cli_internal
 */
class ctkCmdLineModuleMenuComboBox : public ctkMenuComboBox {

  Q_OBJECT

public:

  ctkCmdLineModuleMenuComboBox(QWidget* parent = 0);
  virtual ~ctkCmdLineModuleMenuComboBox();

};

#endif CTKCMDLINEMODULEMENUCOMBOBOX_H

