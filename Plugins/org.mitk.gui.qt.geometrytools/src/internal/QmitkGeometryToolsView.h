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


#ifndef QmitkGeometryToolsView_h
#define QmitkGeometryToolsView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <mitkAffineDataInteractor3D.h>

#include "ui_QmitkGeometryToolsViewControls.h"

/**
  \brief QmitkGeometryToolsView to modify geometry of mitkBaseData via interaction.

  \warning This is an experimental view to play with the geometry of all mitkBaseDatas.
  The current implementation allows to translate, rotate and scale objects with the
  keyboard.

  \warning Scaling is not supported for images, yet, and will not happen in coordinate origin.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkGeometryToolsView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

protected slots:

    /**
     * @brief Add/remove the affine interactor.
     */
    void AddInteractor();
    void RemoveInteractor();

    /**
     * @brief Slots to adapt the step size for interaction.
     */
    void OnRotationSpinBoxChanged(double step);
    void OnScaleSpinBoxChanged(double factor);
    void OnTranslationSpinBoxChanged(double step);

    /**
     * @brief OnUsageInfoBoxChanged show help.
     * @param flag yes/no.
     */
    void OnUsageInfoBoxChanged(bool flag);

  protected:

    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes ) override;

    Ui::QmitkGeometryToolsViewControls m_Controls;
};

#endif // QmitkGeometryToolsView_h
