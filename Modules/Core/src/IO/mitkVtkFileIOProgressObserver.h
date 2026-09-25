/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkFileIOProgressObserver_h
#define mitkVtkFileIOProgressObserver_h

#include <vtkSmartPointer.h>

#include <functional>

class vtkAlgorithm;
class vtkCallbackCommand;

namespace mitk
{
  /**
   * \brief Forwards the progress of a VTK reader or writer while it runs.
   *
   * Reading and writing a file is a single call into VTK, so a file IO class
   * has nothing to report unless it listens to the algorithm doing the work:
   *
   * \code
   * auto writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
   * VtkFileIOProgressObserver progress(writer, [this](float p) { this->ReportProgress(p); });
   *
   * writer->Write();
   * \endcode
   *
   * The observer is removed again when it goes out of scope, so it cannot
   * outlive the callback it was given.
   */
  class VtkFileIOProgressObserver final
  {
  public:
    VtkFileIOProgressObserver(vtkAlgorithm* algorithm, std::function<void(float)> report);
    ~VtkFileIOProgressObserver();

    VtkFileIOProgressObserver(const VtkFileIOProgressObserver&) = delete;
    VtkFileIOProgressObserver& operator=(const VtkFileIOProgressObserver&) = delete;

  private:
    static void OnProgress(vtkObject* caller, unsigned long event, void* clientData, void* callData);

    vtkAlgorithm* m_Algorithm;
    vtkSmartPointer<vtkCallbackCommand> m_Command;
    unsigned long m_ObserverTag;
    std::function<void(float)> m_Report;
  };
}

#endif
