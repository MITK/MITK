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

#ifndef QMITKIGTLSTREAMINGCONNECTOR_H
#define QMITKIGTLSTREAMINGCONNECTOR_H

//QT headers
#include <QTimer>

//mitk headers
#include "MitkOpenIGTLinkUIExports.h"
#include "mitkIGTLMessageSource.h"
#include "mitkIGTLMessageProvider.h"

 /** Documentation:
  *   \brief This class is used to stream messages from a IGTL message source
  *   into the sending queue of a message provider.
  *
  *   The data from the queue will be send to the requesting device.
  *
  *   This class is just needed because of the qtimer functionality. Check also
  *   the MessageProvider for more information.
  *
  *   \ingroup OpenIGTLinkUI
  */
class MITKOPENIGTLINKUI_EXPORT QmitkIGTLStreamingConnector : public QObject
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkIGTLStreamingConnector(QObject* parent = 0);
    ~QmitkIGTLStreamingConnector();

    /** @brief Sets the message source that is the end of the pipeline and the
     *  message provider which will send the message
    */
    void Initialize(mitk::IGTLMessageSource::Pointer msgSource,
                    mitk::IGTLMessageProvider::Pointer msgProvider);

  protected slots:
    /** @brief checks the fps of the message source, if it is unequal 0 the
     * streaming is started.
    */
    void OnCheckFPS();
    /** @brief updates the message source and sends the latest output to the
     * provider
    */
    void OnUpdateSource();

  protected:
    /** @brief holds the message source that has to stream its data */
    mitk::IGTLMessageSource::Pointer m_IGTLMessageSource;

    /** @brief holds the message provider that will send the stream data from the
     * source
    */
    mitk::IGTLMessageProvider::Pointer m_IGTLMessageProvider;

    /** @brief the timer that is configured depending on the fps, if it is
      * fired the pipeline is updated and the IGTLMessage added to the sending
      * queue
      */
    QTimer m_StreamingTimer;

    /** @brief Everytime this timer is fired the fps of the message source are
     *  checked and the streaming is started or stopped
     */
    QTimer m_CheckFPSTimer;

    static const unsigned int MILISECONDS_BETWEEN_FPS_CHECK;
};
#endif
