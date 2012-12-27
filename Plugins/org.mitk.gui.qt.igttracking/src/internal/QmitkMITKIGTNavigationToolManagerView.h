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

#ifndef QmitkMITKIGTNavigationToolManagerView_h
#define QmitkMITKIGTNavigationToolManagerView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkMITKIGTNavigationToolManagerViewControls.h"



/*!
  \brief QmitkMITKIGTNavigationToolManagerView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkMITKIGTNavigationToolManagerView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkMITKIGTNavigationToolManagerView();
    QmitkMITKIGTNavigationToolManagerView(const QmitkMITKIGTNavigationToolManagerView& other)
    {
      Q_UNUSED(other)
      throw std::runtime_error("Copy constructor not implemented");
    }
    virtual ~QmitkMITKIGTNavigationToolManagerView();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

  protected slots:

  protected:

    Ui::QmitkMITKIGTNavigationToolManagerViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;
};



#endif // _QMITKMITKIGTNAVIGATIONTOOLMANAGERVIEW_H_INCLUDED
