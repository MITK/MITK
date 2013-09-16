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

#ifndef __QmitkLabelSetTableWidget_h
#define __QmitkLabelSetTableWidget_h


#include "QmitkExtExports.h"

#include "mitkLabelSet.h"
#include "mitkLabelSetImage.h"
#include "mitkColorSequenceRainbow.h"

#include <QTableWidget>

class QStringList;
class QWidgetAction;
class QString;


class QmitkExt_EXPORT QmitkLabelSetTableWidget : public QTableWidget
{

  Q_OBJECT

  public:

    QmitkLabelSetTableWidget(QWidget* parent = 0);

    ~QmitkLabelSetTableWidget();

    enum TableColumns { NAME_COL=0, LOCKED_COL, COLOR_COL, VISIBLE_COL };

    /// \brief Sets the active label. It will be shown as selected.
    void SetActiveLabel( int );

    /// \brief Sets the active label. It will be shown as selected.
    void SetActiveLabel( const std::string& );

    /// \brief Returns whether a new labels is automatically set as active.
    virtual bool GetAutoSelectNewLabels();

    /// \brief Set the LabelSetImage the widget should listen to.
    void SetActiveLabelSetImage(mitk::LabelSetImage*);

    int RetrieveIndex(const QString&);

    /// \brief Toggles all labels visibility.
    virtual void SetAllLabelsVisible(bool);

    /// \brief A label was added to LabelSet.
    virtual void LabelAdded();

    /// \brief A label was removed from the LabelSet.
    virtual void LabelRemoved();

    /// \brief A label was modified in the LabelSet.
    virtual void LabelModified(int);

    /// \brief All labels were modified in the LabelSet.
    virtual void AllLabelsModified();

    /// \brief Init-function this class with the given dataStorage and _Predicate. This function is called by all ctors.
    void Init();

    /// Sets AutoSelectNewItems flag. If set to true new Nodes will be automatically selected. Default is false.
    virtual void SetAutoSelectNewItems(bool _AutoSelectNewItems);

    const QStringList& GetLabelList();

  signals:

    /// \brief Send a signal when the selected label has changed.
    void activeLabelChanged(int);

    /// \brief Send a signal when it was requested to go to a label.
    void goToLabel(const mitk::Point3D&);

    /// \brief Send a signal when the string list with label names has changed.
    void labelListModified(const QStringList&);

    /// \brief Send a signal when it was requested to create a label.
    void newLabel();

    /// \brief Send a signal when it was requested to rename a label.
    void renameLabel(int index, const mitk::Color& color, const std::string& name);

    /// \brief Send a signal when it was requested to create a surface out of a label.
    void createSurface(int);

    /// \brief Send a signal when it was requested to create a surface out of a selection (range) of labels.
    void combineAndCreateSurface( const QList<QTableWidgetSelectionRange>& ranges );

    /// \brief Send a signal when it was requested to create a mask out of a selection (range) of labels.
    void combineAndCreateMask( const QList<QTableWidgetSelectionRange>& ranges );

    /// \brief Send a signal when it was requested to create a mask out of a label.
    void createMask(int);

    void createMasks(int);

    void createSurfaces(int);

  protected:

    /// \brief Seaches for a given label and returns a valid index or -1 if the label was not found.
    int Find( const mitk::Label* _label ) const;

  protected slots:
    ///
    /// \brief Slot for signal when the user selects another item.
    ///
    void OnItemClicked(QTableWidgetItem *item);

    void OnItemDoubleClicked(QTableWidgetItem *item);

    void OnLockedButtonClicked();

    void OnVisibleButtonClicked();

    void OnColorButtonClicked();

    void NodeTableViewContextMenuRequested( const QPoint & index );

  public slots:
    ///
    /// \brief Slot for signal when user wants to set a node as current selected node.
    ///
  //  void SetSelectedNode(mitk::DataNode::Pointer node);

  protected:

    /// \brief Inserts a new label at the end of the table.
    virtual void InsertItem();

    void WaitCursorOn();

    void BusyCursorOn();

    void WaitCursorOff();

    void BusyCursorOff();

    void RestoreOverrideCursor();

    /// \brief Reset function to populate available labels again
    void Reset();

  protected:

    mitk::LabelSetImage::Pointer m_LabelSetImage;

    QStringList m_LabelStringList;

    mitk::ColorSequenceRainbow::Pointer m_ColorSequenceRainbow;

    bool m_BlockEvents;

    /// \brief If set to "true" new labels inserted will be automatically selected.
    bool m_AutoSelectNewLabels;

    bool m_AllVisible;

    bool m_AllLocked;

    QSlider* m_OpacitySlider;

    QWidgetAction* m_OpacityAction;

  protected slots:

    void OnRandomColor(bool);

    void OnRemoveLabel(bool);

    void OnCreateSurface(bool);

    void OnCreateSurfaces(bool);

    void OnCreateMask(bool);

    void OnCreateMasks(bool);

    void OnCombineAndCreateSurface(bool);

    void OnCombineAndCreateMask(bool);

    void OnEraseLabel(bool);

    void OnMergeLabels(bool);

    void OnRemoveLabels(bool);

    void OnEraseLabels(bool);

    void OnNewLabel(bool);

    void OnRenameLabel(bool);

    void OnSetOnlyActiveLabelVisible(bool);

    void OnLockAllLabels(bool);

    void OnSetAllLabelsVisible(bool);

    void OnRemoveAllLabels(bool);

    void OpacityChanged(int);

}; // mitk namespace

#endif // __QmitkLabelSetTableWidget_h
