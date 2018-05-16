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

#ifndef QmitkFreeSurferParcellationHandler_h
#define QmitkFreeSurferParcellationHandler_h

// MITK
#include <mitkEventStateMachine.h>
#include <mitkFreeSurferParcellationTranslator.h>
#include <mitkInteractionEventObserver.h>
// Qt
#include <QObject>

/**
 * @brief This handler returns either the pixelvalue or anatomical label of the current position of the crosshair inside a visible parcellation.
 * If no parcellation is visible no data is thrown.
 */
class QmitkFreeSurferParcellationHandler : public QObject, public EventStateMachine, public InteractionEventObserver
{
    Q_OBJECT
  public:
    mitkClassMacro(QmitkFreeSurferParcellationHandler, DataInteractor)
    itkNewMacro(QmitkFreeSurferParcellationHandler)
    QmitkFreeSurferParcellationHandler();
    ~QmitkFreeSurferParcellationHandler();


    // Signals
  Q_SIGNALS:
    /**
     * @brief Emitted when pixelValue has changed
     * @param value of the current selected pixel
     */
    void changed(int);
    /**
     * @brief Emitted when pixelValue has changed
     * @param anatomical label
     */
    void changed(const QString &);
    /**
     * @brief Thrown if and only if the left mousebutton is pressed and pixelValue has changed
     * @param value of the selected pixel
     */
    void clicked(int);
    /**
     * @brief Thrown if and only if the left mousebutton is pressed and pixelValue has changed
     * @param anatomical label
     */
    void clicked(const QString &);
    /**
     * @brief Thrown if mouse scrolled and pixelvalue changed
     * @param value of the selected pixel
     */
    void scrolled(int);
    /**
     * @brief Thrown if mouse scrolled and anatomcial labl changed
     * @param anatomical label
     */
    void scrolled(const QString &);


    // Methods
  protected:
    virtual void Notify(InteractionEvent* interactionEvent,bool isHandled);


    // Members
  protected:
    /**
     * @brief The current pixelvalue
     */
    int m_LastPixelvalue;
    /**
     * @brief Translator for coverting pixelvalue into anatomical label
     */
    mitk::FreeSurferParcellationTranslator::Pointer   m_Translator;
    /**
     * Reference to the service registration of the observer,
     * it is needed to unregister the observer on unload.
     */
    us::ServiceRegistration<InteractionEventObserver> m_ServiceRegistration;
};

#endif //QmitkPixelvalueHandler_h
