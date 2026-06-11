/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegmentationPropertyHelper.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace mitk;

// mitk.dicom is the user-facing parent for DICOM property contracts on
// MITK data types. Per-data-type sub-modules sit inside it so future DICOM
// IODs (image, paramap, ...) land as siblings of mitk.dicom.segmentation.

void InitDicom(py::module_& m)
{
  auto dicom = m.def_submodule("dicom",
    "DICOM property contracts for MITK data types.\n"
    "\n"
    "Each affected data type has its own sub-module here. Today only\n"
    ":py:mod:`mitk.dicom.segmentation` is bound; further IODs will be added\n"
    "as siblings.\n");

  auto segmentation = dicom.def_submodule("segmentation",
    "DICOM SEG property contract for :py:class:`mitk.MultiLabelSegmentation`.\n"
    "\n"
    "Concerned with the segmentation as a DICOM SEG instance: validating\n"
    "the property set required for export, synthesizing missing items, and\n"
    "transferring patient/study/frame-of-reference identity from a source\n"
    "image when the typical-case inheritance is wanted. Source-image\n"
    "relations themselves live in :py:mod:`mitk.relations.segmentation`.\n");

  py::class_<DICOMSegmentationPropertyHelper::MissingItem> missingItem(
    segmentation, "MissingItem",
    "One item missing from the seg's DICOM SEG property contract.\n"
    "\n"
    "Returned by :py:func:`validate` and :py:func:`complete`.\n");

  py::enum_<DICOMSegmentationPropertyHelper::MissingItem::Scope>(missingItem, "Scope",
    "Scope of a missing item: segmentation-, group-, or label-level.")
    .value("SEGMENTATION", DICOMSegmentationPropertyHelper::MissingItem::Scope::Segmentation,
           "Missing at the segmentation top level (identifying tag).")
    .value("GROUP", DICOMSegmentationPropertyHelper::MissingItem::Scope::Group,
           "Missing on a specific group; ``identifier`` is the group index.")
    .value("LABEL", DICOMSegmentationPropertyHelper::MissingItem::Scope::Label,
           "Missing on a specific label; ``identifier`` is the label value.");

  missingItem
    .def_readonly("scope", &DICOMSegmentationPropertyHelper::MissingItem::scope,
                  "Scope of the missing item (:py:class:`Scope`).")
    .def_readonly("identifier", &DICOMSegmentationPropertyHelper::MissingItem::identifier,
                  "Group index or label value, depending on scope.")
    .def_readonly("description", &DICOMSegmentationPropertyHelper::MissingItem::description,
                  "Human-readable description of what is missing.");

  py::class_<DICOMSegmentationPropertyHelper::CompletionOptions>(segmentation, "CompletionOptions",
    "Options controlling which items :py:func:`complete` may synthesize.\n"
    "\n"
    "Flags are independent and additive. :py:func:`complete` uses\n"
    "fill-only-if-missing semantics regardless of flag; values already set\n"
    "on the segmentation are never overwritten.\n")
    .def(py::init<>(),
         "Construct with all options off (no synthesis).")
    .def_readwrite("synthesize_missing_identity",
                   &DICOMSegmentationPropertyHelper::CompletionOptions::synthesizeMissingIdentity,
                   "Fill missing patient/study identifying tags from placeholder\n"
                   "constants and mint UIDs for ``StudyInstanceUID`` and\n"
                   "``FrameOfReferenceUID`` under the MITK synthesis namespace.\n"
                   "``SeriesInstanceUID`` is minted unconditionally at\n"
                   "construction, independent of this flag.")
    .def_readwrite("derive_geometry_from_segmentation",
                   &DICOMSegmentationPropertyHelper::CompletionOptions::deriveGeometryFromSegmentation,
                   "Mint a ``FrameOfReferenceUID`` for sourceless segmentations.");

  segmentation.def("validate",
    [](const MultiLabelSegmentation* seg) {
      return DICOMSegmentationPropertyHelper::Validate(seg);
    },
    py::arg("seg"),
    "Report what is missing from the seg's DICOM SEG property contract\n"
    "without mutating it.\n"
    "\n"
    "An empty list means the segmentation is writable as DICOM SEG in\n"
    "strict mode. Absence of a source-image relation is not reported here:\n"
    "it is DICOM-legal per the SEG IOD (``ReferencedSeriesSequence`` is\n"
    "type 1C).\n"
    "\n"
    ":param seg: The segmentation to inspect.\n"
    ":returns: List of :py:class:`MissingItem` describing each gap.\n");

  segmentation.def("complete",
    [](MultiLabelSegmentation* seg,
       const DICOMSegmentationPropertyHelper::CompletionOptions& options) {
      return DICOMSegmentationPropertyHelper::Complete(seg, options);
    },
    py::arg("seg"), py::arg("options"),
    "Fill missing items per ``options`` and return whatever remains missing.\n"
    "\n"
    "Uses fill-only-if-missing semantics; properties already set on the\n"
    "segmentation are never overwritten. Per-instance source\n"
    "``ReferencedSOPInstanceUID``\\ s are never fabricated because they\n"
    "would lie about data that exists in the world (unlike top-level\n"
    "placeholder UIDs which describe the segmentation itself).\n"
    "\n"
    "Side effect: every label whose segment algorithm type is still\n"
    "Undefined is stamped ``MANUAL`` so the written SEG carries a\n"
    "conformant SegmentAlgorithmType (0062,0008).\n"
    "\n"
    ":param seg: The segmentation to complete.\n"
    ":param options: Which synthesis paths are enabled.\n"
    ":returns: List of :py:class:`MissingItem` not (or per policy not)\n"
    "    synthesized.\n");

  segmentation.def("inherit_patient_from_source",
    [](MultiLabelSegmentation* seg, const Image* source, bool overwrite) {
      DICOMSegmentationPropertyHelper::InheritPatientFromSource(seg, source, overwrite);
    },
    py::arg("seg"), py::arg("source"), py::arg("overwrite") = true,
    "Copy PatientName (0010,0010) and PatientID (0010,0020) from ``source``\n"
    "onto ``seg``.\n"
    "\n"
    "Useful when the segmentation should appear under the source's patient\n"
    "in PACS or downstream tooling. Not entailed by establishing a\n"
    "source-image relation: cross-patient derivations are unusual but DICOM-legal.\n"
    "\n"
    ":param seg: The segmentation to mutate.\n"
    ":param source: Image to read the patient identity from.\n"
    ":param overwrite: If True (default), overwrite existing values on the\n"
    "    seg; if False, fill only missing slots. Default is overwrite\n"
    "    because the typical caller of inherit_* explicitly opts into the\n"
    "    transfer.\n");

  segmentation.def("inherit_study_from_source",
    [](MultiLabelSegmentation* seg, const Image* source, bool overwrite) {
      DICOMSegmentationPropertyHelper::InheritStudyFromSource(seg, source, overwrite);
    },
    py::arg("seg"), py::arg("source"), py::arg("overwrite") = true,
    "Copy StudyID (0020,0010) and StudyInstanceUID (0020,000d) from\n"
    "``source`` onto ``seg``.\n"
    "\n"
    "Useful when the segmentation should appear under the source's study in\n"
    "PACS. Not entailed by source-image derivation: cross-study derivations\n"
    "are valid DICOM but uncommon.\n"
    "\n"
    ":param seg: The segmentation to mutate.\n"
    ":param source: Image to read the study identity from.\n"
    ":param overwrite: See :py:func:`inherit_patient_from_source`.\n");

  segmentation.def("inherit_frame_of_reference_from_source",
    [](MultiLabelSegmentation* seg, const Image* source, bool overwrite) {
      DICOMSegmentationPropertyHelper::InheritFrameOfReferenceFromSource(seg, source, overwrite);
    },
    py::arg("seg"), py::arg("source"), py::arg("overwrite") = true,
    "Copy FrameOfReferenceUID (0020,0052) from ``source`` onto ``seg``.\n"
    "\n"
    "Typical when the seg's geometry was initialized from the source. Not\n"
    "entailed by source-image derivation: a resampled seg can derive from a\n"
    "source while living in a different frame of reference.\n"
    "\n"
    ":param seg: The segmentation to mutate.\n"
    ":param source: Image to read the frame-of-reference identity from.\n"
    ":param overwrite: See :py:func:`inherit_patient_from_source`.\n");
}
