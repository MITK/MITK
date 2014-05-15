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

#include "MitkSegmentationUIExports.h"

#include "mitkLabelSetImage.h"
#include "mitkColorSequenceRainbow.h"

#include <QTableWidget>

class QStringList;
class QWidgetAction;
class QString;

namespace mitk {
  class ToolManager;
}

class MitkSegmentationUI_EXPORT QmitkLabelSetTableWidget : public QTableWidget
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
    bool GetAutoSelectNewLabels();

    /// \brief A label was added to LabelSet.
    void LabelAdded();

    /// \brief A label was removed from the LabelSet.
    void LabelRemoved();

    /// \brief A label was modified in the LabelSet.
    void LabelModified(int rowIndex);

    /// \brief The active label was changed in the LabelSet.
    void ActiveLabelChanged(int rowIndex);

    /// \brief All labels were modified in the LabelSet.
    void AllLabelsModified();

    /// \brief
    void setEnabled(bool enabled);

    /// \brief Reset function to populate available labels again
    void Reset();

    /// Sets AutoSelectNewItems flag. If set to true new items will be automatically selected. Default is false.
    void SetAutoSelectNewItems(bool value);

    QStringList GetLabelStringList();

    void OnToolManagerWorkingDataModified();

  signals:

    /// \brief Send a signal when the selected label has changed.
    void activeLabelChanged(int);

    /// \brief Send a signal when it was requested to go to a label.
    void goToLabel(const mitk::Point3D&);

    /// \brief Send a signal when the string list with label names has changed.
    void labelListModified(const QStringList&);

    /// \brief Send a signal when user selects the Merge Label action.
    void mergeLabel(int);

    /// \brief Send a signal when it was requested to create a label.
    void newLabel();

    /// \brief Send a signal when it was requested to rename a label.
    void renameLabel(int index, const mitk::Color& color, const std::string& name);

    /// \brief Send a signal when it was requested to create a surface out of the selected label.
    void createSurface(int, bool);

    void toggleOutline(bool);

    /// \brief Send a signal when it was requested to create a surface out of a selection (range) of labels.
    void combineAndCreateSurface( const QList<QTableWidgetSelectionRange>& ranges );

    /// \brief Send a signal when it was requested to create a mask out of a selection (range) of labels.
    void combineAndCreateMask( const QList<QTableWidgetSelectionRange>& ranges );

    /// \brief Send a signal when it was requested to create a mask out of the selected label.
    void createMask(int);

    /// \brief Send a signal when it was requested to create a cropped mask out of the selected label.
    void createCroppedMask(int);

    void importSegmentation();

  protected:

    /// \brief Seaches for a given label and returns a valid index or -1 if the label was not found.
    int Find( const mitk::Label* label ) const;

    /// \brief Initialize widget properties, slot/signal connections, etc.
    void Initialize();

    /// \brief Set the LabelSetImage the widget should listen to.
    void SetActiveLabelSetImage(mitk::LabelSetImage* image);

    /// \brief Inserts a new label at the end of the table.
    virtual void InsertItem();

    void WaitCursorOn();

    void WaitCursorOff();

    void RestoreOverrideCursor();

  protected:

    mitk::LabelSetImage::Pointer m_LabelSetImage;

    mitk::ToolManager* m_ToolManager;

    //QStringList m_LabelStringList;

    mitk::ColorSequenceRainbow* m_ColorSequenceRainbow;

    bool m_BlockEvents;

    /// \brief If set to "true" new labels inserted will be automatically selected.
    bool m_AutoSelectNewLabels;

    bool m_AllOutlined;

    QSlider* m_OpacitySlider;

    QWidgetAction* m_OpacityAction;

  protected slots:

    /// \brief Slot for signal when the user selects another item.
    void OnItemClicked(QTableWidgetItem *item);

    void OnItemDoubleClicked(QTableWidgetItem *item);

    void OnLockedButtonClicked();

    void OnVisibleButtonClicked();

    void OnColorButtonClicked();

    void NodeTableViewContextMenuRequested( const QPoint & index );

    void OnRandomColor(bool);

    void OnRemoveLabel(bool);

    void OnCreateSmoothedSurface(bool);

    void OnCreateDetailedSurface(bool);

    void OnCreateMask(bool);

    void OnCreateCroppedMask(bool);

    void OnCreateMasks(bool);

    void OnCombineAndCreateSurface(bool);

    void OnCombineAndCreateMask(bool);

    void OnEraseLabel(bool);

    void OnMergeLabel(bool);

    void OnMergeLabels(bool);

    void OnToggleOutline(bool);

    void OnRemoveLabels(bool);

    void OnEraseLabels(bool);

    void OnNewLabel(bool);

    void OnRenameLabel(bool);

    void OnSetOnlyActiveLabelVisible(bool);

    void OnLockAllLabels(bool);

    void OnUnlockAllLabels(bool);

    void OnSetAllLabelsVisible(bool);

    void OnSetAllLabelsInvisible(bool);

    void OnRemoveAllLabels(bool);

    void OpacityChanged(int);

    void OnImportSegmentationSession(bool);

}; // mitk namespace

#endif // __QmitkLabelSetTableWidget_h
