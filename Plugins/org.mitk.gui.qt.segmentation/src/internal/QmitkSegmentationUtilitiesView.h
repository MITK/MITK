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


#ifndef QmitkSegmentationUtilities_h
#define QmitkSegmentationUtilities_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>

#include "ui_QmitkSegmentationUtilitiesViewControls.h"

namespace mitk
{
class Surface;
}


/**
  \brief QmitkSegmentationUtilities

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkSegmentationUtilitiesView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkSegmentationUtilitiesView();

  protected slots:

    void OnImageMaskingToggled(bool);
    void OnSurfaceMaskingToggled(bool);
    void OnMaskImagePressed();
    void OnMaskingDataSelectionChanged(const mitk::DataNode*);
    void OnMaskingReferenceDataSelectionChanged(const mitk::DataNode*);

    void OnMorphologicalOperationsDataSelectionChanged(const mitk::DataNode*);
    //TODO create slots for all morphological buttons

    void OnBooleanOperationsSegImage1SelectionChanged(const mitk::DataNode*);
    void OnBooleanOperationsSegImage2SelectionChanged(const mitk::DataNode*);
    //TODO create slots for all boolean buttons

  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes );

    Ui::QmitkSegmentationUtilitiesViewControls m_Controls;

private:

    void EnableBooleanButtons(bool);

    mitk::NodePredicateAnd::Pointer m_IsOfTypeImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsOfTypeSurface;
    mitk::NodePredicateProperty::Pointer m_IsBinaryPredicate;
    mitk::NodePredicateNot::Pointer m_IsNotBinaryPredicate;
    mitk::NodePredicateAnd::Pointer m_IsNotABinaryImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsABinaryImagePredicate;

};

#endif // QmitkSegmentationUtilities_h
