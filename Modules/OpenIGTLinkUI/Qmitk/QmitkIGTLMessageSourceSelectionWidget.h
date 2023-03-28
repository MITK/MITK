/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkIGTLMessageSourceSelectionWidget_h
#define QmitkIGTLMessageSourceSelectionWidget_h

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

    QmitkIGTLMessageSourceSelectionWidget(QWidget* parent = nullptr,
                                          Qt::WindowFlags f = nullptr);
    ~QmitkIGTLMessageSourceSelectionWidget() override;

    /** @return Returns the currently selected OpenIGTLink message source.
     * Returns null if no source is selected at the moment. */
    mitk::IGTLMessageSource::Pointer GetSelectedIGTLMessageSource();

    /** @brief Automatically selects the first available OpenIGTLink message source
      * of the messageSourceSelectionWidget as message source and assigns it to the
      * class member m_CurrentIGTLMessageSource.
      * If there is no OpenIGTLink message source available, a nullptr will be assigned
      * to the m_CurrentIGTLMessageSource. It is important to call this method whenever
      * a new OpenIGTLink client connects to the active OpenIGTLink server. Otherwise,
      * the connection between PLUS and MITK or between Slicer and MITK won't work
      * automatically.
      *
      * @return The pointer to the automatically selected message source. This might
      *         be a nullptr, if there is no message source available.
      */
    mitk::IGTLMessageSource::Pointer AutoSelectFirstIGTLMessageSource();

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
