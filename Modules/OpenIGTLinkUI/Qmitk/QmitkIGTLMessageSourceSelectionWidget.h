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

#ifndef QmitkIGTLMessageSourceSelectionWidget_H
#define QmitkIGTLMessageSourceSelectionWidget_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkOpenIGTLinkUIExports.h"
#include "mitkIGTLMessageSource.h"

//us
#include <usServiceReference.h>

//ui header
#include "ui_QmitkIGTLMessageSourceSelectionWidgetControls.h"


 /** Documentation:
  *   \brief This widget allows the user to select a OpenIGTLink message source.
  *
  *          The widget lists all OpenIGTLink message sources which are available
  *          as microservice via the module context.
  *
  *          A signal is emmited whenever the selection changes.
  *
  *   \ingroup OpenIGTLinkUI
  */
class MITKOPENIGTLINKUI_EXPORT QmitkIGTLMessageSourceSelectionWidget :
    public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkIGTLMessageSourceSelectionWidget(QWidget* parent = 0,
                                          Qt::WindowFlags f = 0);
    ~QmitkIGTLMessageSourceSelectionWidget();

    /** @return Returns the currently selected OpenIGTLink message source.
     * Returns null if no source is selected at the moment. */
    mitk::IGTLMessageSource::Pointer GetSelectedIGTLMessageSource();

  signals:
    /** @brief This signal is emitted when a new OpenIGTLink message source is
      * selected.
      * @param source Holds the new selected OpenIGTLink device source. Is null
      * if the old source is deselected and no new source is selected.
      */
    void IGTLMessageSourceSelected(mitk::IGTLMessageSource::Pointer source);

  protected slots:

    void IGTLMessageSourceSelected(us::ServiceReferenceU s);


  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkIGTLMessageSourceSelectionWidgetControls* m_Controls;

    mitk::IGTLMessageSource::Pointer m_CurrentIGTLMessageSource;



};
#endif
