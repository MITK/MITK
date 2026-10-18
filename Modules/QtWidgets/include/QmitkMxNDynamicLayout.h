/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNDynamicLayout_h
#define QmitkMxNDynamicLayout_h

#include <MitkQtWidgetsExports.h>

#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkLookupTable.h>

#include <nlohmann/json.hpp>

#include <QString>

#include <map>
#include <string>
#include <variant>
#include <vector>

/**
 * \file
 * \brief Dynamic layouts of the MxN display: layouts built from the data they
 *        are to show.
 *
 * A template declares slots for the data it arranges and has typed options.
 * Its generator is a pure function from the data bound to the slots and the
 * options to a v3.0 layout document plus what the document cannot carry: the
 * data each selection group shows, renderer-specific appearance, and the view
 * fits to run. 'QmitkMxNMultiWidget::ApplyDynamicLayout' applies the result.
 *
 * A generator only arranges windows, links them, binds selection groups and
 * sets renderer-specific state of existing nodes; it never creates nodes and
 * never writes node-global properties.
 *
 * Data classes: an Image is an 'mitk::Image' that is not binary; an Overlay
 * is a multi-label segmentation, a surface, a point set or a binary image.
 * Helper and hidden objects are neither. A 3D representation is a multi-label
 * segmentation or a surface.
 */

/** \brief The dynamic layout templates. */
enum class QmitkMxNDynamicTemplate
{
  DatasetsByPlanes,
  OneViewPerDataset,
  SharedBase,
  FusionPair
};

/** \brief The slots across all templates. */
enum class QmitkMxNSlot
{
  Datasets,    // DatasetsByPlanes, OneViewPerDataset
  BaseImage,   // SharedBase
  Overlays,    // SharedBase
  Anatomy,     // FusionPair
  FusedImage   // FusionPair
};

/** \brief What a slot accepts; see the data classes above. */
enum class QmitkMxNSlotDataClass
{
  Image,
  Overlay
};

struct QmitkMxNSlotDescription
{
  QmitkMxNSlot slot;
  QString role;                     // the dialog's label, e.g. "Anatomy"
  QmitkMxNSlotDataClass dataClass;
  int min;                          // a required single slot has min == max == 1
  int max;
};

struct QmitkMxNTemplateDescription
{
  QmitkMxNDynamicTemplate id;
  QString name;
  std::vector<QmitkMxNSlotDescription> slotDescriptions;  // not 'slots', a Qt keyword
  bool swappableSlots = false;      // "Swap roles" exchanges the two slots
};

/** \brief Options of the datasets-by-planes template. */
struct QmitkMxNDatasetsByPlanesOptions
{
  bool aligned = false;
  bool threeDColumn = false;
  bool datasetsAsColumns = false;
};

enum class QmitkMxNDatasetView
{
  Axial,
  Sagittal,
  Coronal,
  ThreeD
};

enum class QmitkMxNDatasetOrder
{
  Selection,
  AcquisitionDate
};

/** \brief Options of the one-view-per-dataset template. */
struct QmitkMxNOneViewPerDatasetOptions
{
  bool aligned = false;
  QmitkMxNDatasetView view = QmitkMxNDatasetView::Axial;
  QmitkMxNDatasetOrder order = QmitkMxNDatasetOrder::Selection;
};

/** \brief Options of the shared-base template. */
struct QmitkMxNSharedBaseOptions
{
  bool threeDRow = false;
  bool allOverlaysWindow = false;
};

enum class QmitkMxNFusionColormap
{
  PetColour,
  HotIron,
  JetTransparentFloor
};

enum class QmitkMxNFusionOpacity
{
  Percent30,
  Percent50,
  Percent70
};

/** \brief Options of the fusion-pair template. It has no alignment option:
 *         its two images are aligned by contract. */
struct QmitkMxNFusionPairOptions
{
  QmitkMxNFusionColormap colormap = QmitkMxNFusionColormap::PetColour;
  bool mipWindow = true;
  QmitkMxNFusionOpacity opacity = QmitkMxNFusionOpacity::Percent50;
};

/** \brief The options of one template; the alternative selects the template. */
using QmitkMxNDynamicOptions = std::variant<QmitkMxNDatasetsByPlanesOptions,
                                            QmitkMxNOneViewPerDatasetOptions,
                                            QmitkMxNSharedBaseOptions,
                                            QmitkMxNFusionPairOptions>;

using QmitkMxNSlotBindings = std::map<QmitkMxNSlot, std::vector<mitk::DataNode::Pointer>>;

/** \brief A generated layout and everything 'ApplyDynamicLayout' writes after loading it. */
struct QmitkMxNDynamicLayout
{
  /** v3.0, strict mode, window ids qualified for the target widget. */
  nlohmann::json document;

  /** The nodes each bound selection group shows, bottom to top. */
  std::map<std::string, std::vector<mitk::DataNode::Pointer>> groupNodes;

  struct LookupTableAssignment
  {
    QString windowId;  // a member of a LUT group; the table reaches every member
    mitk::DataNode::Pointer node;
    mitk::LookupTable::LookupTableType type;
  };
  std::vector<LookupTableAssignment> lookupTables;

  struct OpacityAssignment
  {
    QString windowId;
    mitk::DataNode::Pointer node;
    float opacity;
  };
  std::vector<OpacityAssignment> opacities;

  struct VolumeRenderingAssignment
  {
    QString windowId;  // a 3D window
    mitk::DataNode::Pointer node;
    int blendMode;     // a vtkVolumeMapper blend mode
  };
  std::vector<VolumeRenderingAssignment> volumeRenderings;

  struct Fit
  {
    QString windowId;
    /** True: fit the window's slice/orientation component from this window
        ('ReinitSyncGroupGeometry'). False: fit this window alone to the
        nodes its selection group shows. */
    bool wholeComponent;
  };
  std::vector<Fit> fits;
};

namespace QmitkMxNDynamicLayouts
{
  /** \brief The four templates, in dialog order. */
  MITKQTWIDGETS_EXPORT const std::vector<QmitkMxNTemplateDescription>& Templates();

  /** \brief The description of one template. */
  MITKQTWIDGETS_EXPORT const QmitkMxNTemplateDescription& Template(QmitkMxNDynamicTemplate templateId);

  /** \brief Whether 'node' belongs to the data class (see the file description). */
  MITKQTWIDGETS_EXPORT bool IsOfDataClass(const mitk::DataNode& node, QmitkMxNSlotDataClass dataClass);

  /** \brief Whether an image, with its direct derivations, has a 3D
   *         representation: a multi-label segmentation or a surface among
   *         its direct derivations of the Overlay class. */
  MITKQTWIDGETS_EXPORT bool HasThreeDRepresentation(const mitk::DataNode& image,
                                                    const mitk::DataStorage& dataStorage);

  /**
  * \brief Fill the slots from a node selection.
  *
  *   A slot takes the selection's nodes of its data class, in the order the
  *   selection reports them: a single slot the first one no earlier slot
  *   took, a multi slot up to its maximum. The shared-base template takes as
  *   base the first selected image, else the one direct source all selected
  *   overlays share; as overlays the selected ones, else the base's direct
  *   derivations of the Overlay class. The fusion pair takes a CT as anatomy
  *   and a PT as fused image when the selection has exactly one of each
  *   (DICOM Modality). Nodes the template cannot use are left out; a slot
  *   may stay empty.
  */
  MITKQTWIDGETS_EXPORT QmitkMxNSlotBindings Prefill(QmitkMxNDynamicTemplate templateId,
                                                    const std::vector<mitk::DataNode::Pointer>& selection,
                                                    const mitk::DataStorage& dataStorage);

  /**
  * \brief The automatic options for the given bindings; fields without a
  *        rule keep their declared default. The returned alternative is the
  *        template's.
  *
  *   Datasets count as aligned when all have identical geometry at time step
  *   0, or all share one Frame of Reference UID. A 3D column or row is on
  *   when any dataset or overlay has a 3D representation. Datasets are
  *   ordered by acquisition date when every dataset has one. A fused image
  *   whose DICOM Modality is known and not PT gets hot iron and no MIP
  *   window.
  */
  MITKQTWIDGETS_EXPORT QmitkMxNDynamicOptions DefaultOptions(QmitkMxNDynamicTemplate templateId,
                                                             const QmitkMxNSlotBindings& bindings,
                                                             const mitk::DataStorage& dataStorage);

  /** \brief The template that 'options' selects. */
  MITKQTWIDGETS_EXPORT QmitkMxNDynamicTemplate TemplateOf(const QmitkMxNDynamicOptions& options);

  /**
  * \brief Build the layout of the template that 'options' selects.
  *
  * \param bindings           the data in the template's slots.
  * \param options            the template's options; the alternative selects
  *                           the template.
  * \param widgetName         the target widget's name, for the window ids.
  * \param editorAspectRatio  width / height of the editor area, for the grid
  *                           of the one-view-per-dataset template.
  * \param dataStorage        the storage the bound nodes live in, for their
  *                           derivations.
  *
  * \throws mitk::Exception when a slot of the template is missing or holds
  *         fewer or more nodes than its cardinality, a node does not match
  *         its slot's data class, a binding names a slot the template does
  *         not have, the 3D view of one view per dataset is chosen without
  *         every dataset having a 3D representation, the widget name is not
  *         a valid MxN widget name, or the aspect ratio is not positive.
  */
  MITKQTWIDGETS_EXPORT QmitkMxNDynamicLayout Generate(const QmitkMxNSlotBindings& bindings,
                                                      const QmitkMxNDynamicOptions& options,
                                                      const QString& widgetName,
                                                      double editorAspectRatio,
                                                      const mitk::DataStorage& dataStorage);
}

#endif
