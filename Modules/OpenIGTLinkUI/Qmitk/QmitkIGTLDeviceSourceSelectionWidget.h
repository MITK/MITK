/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkIGTLDeviceSourceSelectionWidget_h
#define QmitkIGTLDeviceSourceSelectionWidget_h

//QT headers
#include <QWidget>

//mitk headers
#include "MitkOpenIGTLinkUIExports.h"
#include "mitkIGTLDeviceSource.h"
//#include <mitkNavigationToolStorage.h>
//#include <mitkNavigationDataSource.h>
#include <usServiceReference.h>
//ui header
#include "ui_QmitkIGTLDeviceSourceSelectionWidgetControls.h"


 /** Documentation:
  *   \brief This widget allows the user to select a OpenIGTLink device source.
  *
  *          The widget lists all OpenIGTLink device sources which are available
  *          as microservice via the module context.
  *
  *          A signal is emmited whenever the device selection changes.
  *
  *   \ingroup OpenIGTLinkUI
  */
class MITKOPENIGTLINKUI_EXPORT QmitkIGTLDeviceSourceSelectionWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkIGTLDeviceSourceSelectionWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkIGTLDeviceSourceSelectionWidget() override;

    /** @return Returns the currently selected OpenIGTLink device source.
     * Returns null if no source is selected at the moment. */
    mitk::IGTLDeviceSource::Pointer GetSelectedIGTLDeviceSource();

  signals:
    /** @brief This signal is emitted when a new OpenIGTLink device source is
      * selected.
      * @param source Holds the new selected OpenIGTLink device source. Is null
      * if the old source is deselected and no new source is selected.
      */
    void IGTLDeviceSourceSelected(mitk::IGTLDeviceSource::Pointer source);

  protected slots:

    void IGTLDeviceSourceSelected(us::ServiceReferenceU s);


  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkIGTLDeviceSourceSelectionWidgetControls* m_Controls;

    mitk::IGTLDeviceSource::Pointer m_CurrentIGTLDeviceSource;



};
#endif
