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


#ifndef DcmRTV_h
#define DcmRTV_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <mitkDataNodeFactory.h>

#include <mitkDicomRTReader.h>

#include "ui_DcmRTVControls.h"

#include <mitkLookupTable.h>
#include <vtkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkRenderingModeProperty.h>
#include <mitkDicomSeriesReader.h>
//#include <QmitkFileOpenAction.h>

#include <vtkContourFilter.h>
#include <vtkPolyData.h>
#include <mitkSurface.h>
#include <vtkDICOMImageReader.h>
#include <vtkMarchingSquares.h>
#include <vtkMarchingCubes.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>

#include <mitkImage.h>
#include <mitkPixelType.h>

#include <mitkExtractImageFilter.h>

#include <mitkCoreServices.h>
#include <mitkIShaderRepository.h>
#include <mitkShaderProperty.h>

#include <QFileDialog>

/**
  \brief DcmRTV

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class DcmRTV : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    DcmRTV();
    virtual ~DcmRTV();
    static const std::string VIEW_ID;

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void DoImageProcessing();
    void LoadRTDoseFile();
    void UpdateIsoLines(int);
    void LoadContourModel();
    void LoadIsoLines();

  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes );

    Ui::DcmRTVControls m_Controls;

};

#endif // DcmRTV_h
