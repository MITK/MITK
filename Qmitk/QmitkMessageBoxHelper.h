/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef QmitkMessageBoxCloserhincluded
#define QmitkMessageBoxCloserhincluded

#include <qobject.h>
#include <qtimer.h>
#include <qmutex.h>

#include <itkEventObject.h>
#include <mitkCommon.h>

class QmitkMessageBoxHelperDialogWaitThread;

/**
  * \brief Test helper class for closing message boxes.
  *
  * \todo cleanup!
  */
class QMITK_EXPORT QmitkMessageBoxHelper : public QObject
{
  Q_OBJECT

  public:

    QmitkMessageBoxHelper(QObject* parent, const char* name = NULL);

    void DialogFound();
    void DialogNotFound();

    QWidget* FindDialogItem(const char* widgetName, QWidget* parent);
  signals:

    /// can be NULL, if nothing is found after the specified waiting time
    void DialogFound( QWidget* );

  public slots:

    void SetFoundDialog(QWidget*);
    void OnFoundWidgetDestroyed();

    void CloseMessageBoxes(unsigned int whichButton = 0, unsigned int delay = 1000);
    void CloseMessageBoxesNow();
    QWidget* FindTopLevelWindow( const char* classname );

    /**
     * Spawn thread to wait for a toplevel window (dialog).
     * Has to be done threaded because there are modal dialogs that won't give back control until they are closed
     */
    void WaitForDialogAndCallback( const char* classname, int maxWaitSeconds = 20 );

    /**
     * Stops the thread which waits for a toplevel window (dialog).
     */
    void StopWaitForDialogAndCallback();

  protected slots:

    void OnLookForDialogTimeout();

  protected:

    friend class QmitkMessageBoxHelperDialogWaitThread;

    QTimer m_Timer;
    QTimer m_LookForDialogTimer;

    unsigned int m_WhichButton;

    QmitkMessageBoxHelperDialogWaitThread* m_DialogWaitingThread;

    QWidget* m_FoundWidget;

    QString m_ClassName;

    int m_NumberOfLooks;
    int m_MaxNumberOfLooks;
};

#endif

