/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkSegSourceImageRelationRule.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace mitk;

// mitk.relations is the user-facing parent for all relation-rule operations.
// Per-data-type sub-modules sit inside it so future relations (image-to-image
// source, model-fit input, paramap source, ...) can land as siblings of
// mitk.relations.segmentation without churn on the data classes.

void InitRelations(py::module_& m)
{
  auto relations = m.def_submodule("relations",
    "Relation rules between MITK data objects.\n"
    "\n"
    "Each affected data type has its own sub-module here. Today only\n"
    ":py:mod:`mitk.relations.segmentation` is bound; further rule types will\n"
    "be added as siblings.\n");

  auto segmentation = relations.def_submodule("segmentation",
    "Relations a :py:class:`mitk.MultiLabelSegmentation` participates in.\n"
    "\n"
    "Currently only the DICOM-SEG source-image relation. Establishing a\n"
    "relation captures the source's identification (per-instance SOP UIDs\n"
    "and series UID) but does NOT transfer the source's patient/study/\n"
    "frame-of-reference identity to the seg; that is a separate concern\n"
    "handled by :py:mod:`mitk.dicom.segmentation`.\n");

  py::class_<SegSourceImageRelationRule::SourceImageRelation>(segmentation, "SourceImageRelation",
    "One established source-image relation.\n"
    "\n"
    "Returned by :py:func:`get_source_image_relations`. The relation UID is\n"
    "the durable handle. The source series UID is empty when the relation\n"
    "was established against a source without DICOM identifying tags (i.e.\n"
    "the relation lives only on the ID layer).\n")
    .def_readonly("relation_uid",
                  &SegSourceImageRelationRule::SourceImageRelation::relationUID,
                  "Durable identifier of the relation.")
    .def_readonly("source_series_instance_uid",
                  &SegSourceImageRelationRule::SourceImageRelation::sourceSeriesInstanceUID,
                  "Source series UID captured at Connect time. Empty when the\n"
                  "source had no DICOM identifying tags.");

  segmentation.def("connect_source_image",
    [](MultiLabelSegmentation* seg, const Image* sourceImage) {
      return SegSourceImageRelationRule::Connect(seg, sourceImage);
    },
    py::arg("seg"), py::arg("source_image"),
    "Establish an additional source-image relation on ``seg``.\n"
    "\n"
    "The :py:class:`mitk.MultiLabelSegmentation` constructor already\n"
    "auto-connects when given an :py:class:`mitk.Image`; use this function\n"
    "for the multi-source case (e.g. multi-modal fusion or longitudinal\n"
    "segmentations referencing several source series) and from the reader\n"
    "side when a fresh seg is built from a loaded DICOM SEG file.\n"
    "\n"
    "Sources without DICOM identifying tags (e.g. NIfTI loads) still get an\n"
    "ID-layer relation; the DICOM SEG writer will not emit a\n"
    "``ReferencedSeriesSequence`` entry for them, but in-session consumers\n"
    "(statistics, ROI, radiomics) can resolve the source via the relation\n"
    "UID.\n"
    "\n"
    "Does not transfer patient/study/frame-of-reference identity from\n"
    "source to seg; see :py:mod:`mitk.dicom.segmentation` for that.\n"
    "\n"
    ":param seg: The destination segmentation.\n"
    ":param source_image: The source image the segmentation derives from.\n"
    ":returns: The newly created relation UID.\n"
    ":raises mitk.Exception: If ``seg`` or ``source_image`` is None.\n");

  segmentation.def("get_source_image_relations",
    [](const MultiLabelSegmentation* seg) {
      return SegSourceImageRelationRule::GetSourceImageRelations(seg);
    },
    py::arg("seg"),
    "Enumerate source-image relations established on ``seg``.\n"
    "\n"
    "One entry per Connect call. Includes relations established via the\n"
    ":py:class:`mitk.MultiLabelSegmentation` constructor's auto-connect, via\n"
    ":py:func:`connect_source_image`, and via the C++ SEG reader.\n"
    "\n"
    ":param seg: The segmentation to enumerate.\n"
    ":returns: List of :py:class:`SourceImageRelation`.\n");

  segmentation.def("canonical_purpose_tag",
    []() { return SegSourceImageRelationRule::CanonicalPurposeTag(); },
    "Return the DICOM Code Value used in the SEG's Source Image Sequence\n"
    "Purpose Of Reference Code item.\n"
    "\n"
    "Useful for tag-comparison code (e.g. regression tests) that wants to\n"
    "assert against the canonical value without duplicating the literal.\n"
    "\n"
    ":returns: The canonical DICOM Code Value string.\n");
}
