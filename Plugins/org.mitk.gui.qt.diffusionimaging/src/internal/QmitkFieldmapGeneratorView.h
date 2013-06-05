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

#ifndef _QMITKQmitkFieldmapGeneratorView_H_INCLUDED
#define _QMITKQmitkFieldmapGeneratorView_H_INCLUDED

#include <berryISelectionListener.h>
#include <QmitkFunctionality.h>
#include "ui_QmitkFieldmapGeneratorViewControls.h"
#include <mitkPointSet.h>
#include <itkImage.h>

/*!
  \brief View displaying details of the orientation distribution function in the voxel at the current crosshair position.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/

class QmitkFieldmapGeneratorView : public QmitkFunctionality
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    QmitkFieldmapGeneratorView();
    QmitkFieldmapGeneratorView(const QmitkFieldmapGeneratorView& other)
    {
        Q_UNUSED(other)
        throw std::runtime_error("Copy constructor not implemented");
    }
    virtual ~QmitkFieldmapGeneratorView();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

    void OnSliceChanged(const itk::EventObject& e);

protected slots:

    void GenerateFieldmap();
    void PlaceFieldSource();
    void OnVarianceChanged(double value);
    void OnHeightChanged(double value);

protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    Ui::QmitkFieldmapGeneratorViewControls* m_Controls;
    QmitkStdMultiWidget*                    m_MultiWidget;

    /** observer flags */
    int m_SliceObserverTag1;
    int m_SliceObserverTag2;
    int m_SliceObserverTag3;
    int m_PropertyObserverTag;

    mitk::Point3D               m_WorldPoint;
    mitk::DataNode::Pointer     m_SelectedSource;
};



#endif // _QmitkFieldmapGeneratorView_H_INCLUDED

