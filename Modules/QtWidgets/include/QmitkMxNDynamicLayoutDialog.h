/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNDynamicLayoutDialog_h
#define QmitkMxNDynamicLayoutDialog_h

#include <MitkQtWidgetsExports.h>

#include <QmitkMxNDynamicLayout.h>

#include <mitkDataStorage.h>

#include <QDialog>
#include <QList>

#include <set>
#include <utility>
#include <vector>

class QmitkAbstractNodeSelectionWidget;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QFormLayout;
class QLabel;
class QPushButton;

/**
 * \brief Chooses a dynamic layout: a template, the data for its slots and its
 *        options.
 *
 * The slots are filled from the selection the dialog opens with, and the
 * options from the data in the slots; switching the template fills both
 * again. A slot does not offer a node another slot holds. An option follows
 * the data until the user changes it. Apply stays
 * disabled, with a line naming the slot, while a slot holds too few or too
 * many nodes. Accepting generates the layout; the dialog applies nothing.
 */
class MITKQTWIDGETS_EXPORT QmitkMxNDynamicLayoutDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \param dataStorage      the storage the slots choose their data from
   * \param selection        the Data Manager selection when the dialog opens
   * \param widgetName       the target widget's name, for the window ids
   * \param editorAspectRatio  width / height of the editor area
   * \param initialTemplate  the template chosen last time in this session
   * \param parent           the dialog's parent widget
   *
   * \throws mitk::Exception on a null data storage.
   */
  QmitkMxNDynamicLayoutDialog(mitk::DataStorage* dataStorage,
                              const QList<mitk::DataNode::Pointer>& selection,
                              const QString& widgetName,
                              double editorAspectRatio,
                              QmitkMxNDynamicTemplate initialTemplate,
                              QWidget* parent = nullptr);

  ~QmitkMxNDynamicLayoutDialog() override;

  /** \brief The generated layout; valid after the dialog was accepted. */
  const QmitkMxNDynamicLayout& GetLayout() const;

  /** \brief The template the dialog ended on, for the next opening. */
  QmitkMxNDynamicTemplate GetTemplate() const;

  /** \brief The data in the slots as they are now. Exposed for verification. */
  QmitkMxNSlotBindings GetBindings() const;

  /** \brief The options as the controls show them now. Exposed for
   *         verification. */
  QmitkMxNDynamicOptions GetOptions() const;

  /** \brief Show 'templateId' with its slots filled from the opening
   *         selection and its options from that data. */
  void SelectTemplate(QmitkMxNDynamicTemplate templateId);

private:
  void BuildSlotRows();
  void BuildOptionRows();

  /** \brief Set the options the user has not changed to the data's
   *         defaults, and update what depends on the slots. */
  void OnSlotsChanged();

  void UpdateApplyState();
  void SwapRoles();
  void OnApply();

  /** \brief Connect a control so that a change by the user marks it touched. */
  void TrackTouched(QCheckBox* control);
  void TrackTouched(QComboBox* control);
  bool IsTouched(const QWidget* control) const;

  mitk::DataStorage::Pointer m_DataStorage;
  std::vector<mitk::DataNode::Pointer> m_Selection;
  QString m_WidgetName;
  double m_EditorAspectRatio;
  QmitkMxNDynamicTemplate m_Template;
  QmitkMxNDynamicLayout m_Layout;

  QButtonGroup* m_TemplateButtons = nullptr;
  QFormLayout* m_SlotRows = nullptr;
  QFormLayout* m_OptionRows = nullptr;
  QLabel* m_Message = nullptr;
  QPushButton* m_ApplyButton = nullptr;
  QPushButton* m_SwapButton = nullptr;
  std::vector<std::pair<QmitkMxNSlot, QmitkAbstractNodeSelectionWidget*>> m_SlotWidgets;

  // The option controls of the current template; null where it has none.
  QCheckBox* m_Aligned = nullptr;
  QCheckBox* m_ThreeDColumn = nullptr;
  QCheckBox* m_DatasetsAsColumns = nullptr;
  QComboBox* m_View = nullptr;
  QComboBox* m_Order = nullptr;
  QCheckBox* m_ThreeDRow = nullptr;
  QCheckBox* m_AllOverlaysWindow = nullptr;
  QComboBox* m_Colormap = nullptr;
  QCheckBox* m_MipWindow = nullptr;
  QComboBox* m_Opacity = nullptr;

  std::set<const QWidget*> m_Touched;
  bool m_Updating = false;
};

#endif
