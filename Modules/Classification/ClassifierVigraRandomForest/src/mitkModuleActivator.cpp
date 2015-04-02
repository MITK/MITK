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
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <mitkRandomForestIO.h>
//#include <mitkDummyLsetReader.h>
//#include "QmitkNodeDescriptorManager.h"
//#include "mitkVigraRandomForestClassifier.h"
#include "mitkNodePredicateDataType.h"

namespace mitk
{

  /**
  \brief Registers services for segmentation module.
  */
  class ModuleActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext*)
    {

      // *-----------------*
      // *        IO
      // *-----------------*

      m_DecisionForestIO = new mitk::RandomForestFileIO();
//      m_DummyLsetFileReader = new mitk::DummyLsetFileReader();
    }

    void Unload(us::ModuleContext*)
    {
      delete m_DecisionForestIO;
    }

  private:
    mitk::RandomForestFileIO * m_DecisionForestIO;
//    mitk::DummyLsetFileReader * m_DummyLsetFileReader;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::ModuleActivator)
