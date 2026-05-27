/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "GeometryHelpers.h"
#include "SmartPointer.h"
#include "PropertyOwnerBindings.h"

#include <mitkDICOMCodeSequence.h>
#include <mitkDICOMCodeSequenceWithModifiers.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkLabel.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageConverter.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkProportionalTimeGeometry.h>

#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <algorithm>
#include <filesystem>
#include <optional>
#include <sstream>

namespace py = pybind11;
using namespace mitk;

// ---------------------------------------------------------------------------
// File-local helpers
// ---------------------------------------------------------------------------

namespace
{

Color TupleToColor(const std::tuple<float, float, float>& t)
{
  Color c;
  c.SetRed(std::get<0>(t));
  c.SetGreen(std::get<1>(t));
  c.SetBlue(std::get<2>(t));
  return c;
}

std::tuple<float, float, float> ColorToTuple(const Color& c)
{
  return {c.GetRed(), c.GetGreen(), c.GetBlue()};
}

Label::PixelType ToLabelValue(py::handle h)
{
  // Accept Label instance or integer
  if (py::isinstance<Label>(h))
    return h.cast<const Label&>().GetValue();
  return h.cast<Label::PixelType>();
}

MultiLabelSegmentation::LabelValueVectorType ToLabelValues(py::iterable seq)
{
  MultiLabelSegmentation::LabelValueVectorType result;
  for (auto item : seq)
    result.push_back(ToLabelValue(item));
  return result;
}

LabelValueMappingVector ToLabelMapping(py::iterable seq)
{
  LabelValueMappingVector result;
  for (auto item : seq)
  {
    auto pair = item.cast<py::tuple>();
    if (pair.size() != 2)
      throw py::value_error("label_mapping entries must be 2-tuples");
    result.push_back({ToLabelValue(pair[0]), ToLabelValue(pair[1])});
  }
  return result;
}

// Accept DICOMCodeSequence object or (value, scheme, meaning) 3-tuple
DICOMCodeSequence ToDICOMCodeSequence(py::handle h)
{
  if (py::isinstance<DICOMCodeSequence>(h))
    return h.cast<DICOMCodeSequence>();
  auto t = h.cast<py::tuple>();
  if (t.size() != 3)
    throw py::value_error("DICOMCodeSequence tuple must have 3 elements (value, scheme, meaning)");
  return DICOMCodeSequence(t[0].cast<std::string>(), t[1].cast<std::string>(), t[2].cast<std::string>());
}

// Accept DICOMCodeSequenceWithModifiers object, DICOMCodeSequence object, or (value, scheme, meaning) 3-tuple
DICOMCodeSequenceWithModifiers ToDICOMCodeSequenceWithModifiers(py::handle h)
{
  if (py::isinstance<DICOMCodeSequenceWithModifiers>(h))
    return h.cast<DICOMCodeSequenceWithModifiers>();
  if (py::isinstance<DICOMCodeSequence>(h))
    return DICOMCodeSequenceWithModifiers(h.cast<DICOMCodeSequence>());
  auto t = h.cast<py::tuple>();
  if (t.size() != 3)
    throw py::value_error("DICOMCodeSequenceWithModifiers tuple must have 3 elements (value, scheme, meaning)");
  return DICOMCodeSequenceWithModifiers(t[0].cast<std::string>(), t[1].cast<std::string>(), t[2].cast<std::string>());
}

// Convert Point3D to python tuple
std::tuple<double, double, double> Point3DToTuple(const mitk::Point3D& p)
{
  return {p[0], p[1], p[2]};
}

// Accept MergeStyle enum or case-insensitive string alias ("replace" / "merge")
MultiLabelSegmentation::MergeStyle ToMergeStyle(py::handle h)
{
  if (py::isinstance<MultiLabelSegmentation::MergeStyle>(h))
    return h.cast<MultiLabelSegmentation::MergeStyle>();
  const auto s = h.cast<std::string>();
  std::string upper;
  upper.reserve(s.size());
  for (const char c : s)
    upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
  if (upper == "REPLACE") return MultiLabelSegmentation::MergeStyle::Replace;
  if (upper == "MERGE")   return MultiLabelSegmentation::MergeStyle::Merge;
  throw py::value_error("Unknown MergeStyle: " + s);
}

// Accept OverwriteStyle enum or case-insensitive string alias ("regard_locks" / "ignore_locks")
MultiLabelSegmentation::OverwriteStyle ToOverwriteStyle(py::handle h)
{
  if (py::isinstance<MultiLabelSegmentation::OverwriteStyle>(h))
    return h.cast<MultiLabelSegmentation::OverwriteStyle>();
  const auto s = h.cast<std::string>();
  std::string upper;
  upper.reserve(s.size());
  for (const char c : s)
    upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
  if (upper == "REGARD_LOCKS") return MultiLabelSegmentation::OverwriteStyle::RegardLocks;
  if (upper == "IGNORE_LOCKS") return MultiLabelSegmentation::OverwriteStyle::IgnoreLocks;
  throw py::value_error("Unknown OverwriteStyle: " + s);
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// PyLabelVector — lightweight wrapper for sorted label vectors
// ---------------------------------------------------------------------------

struct PyLabelVector
{
  std::vector<Label::Pointer> items;
};

namespace
{

PyLabelVector MakeLabelVector(const LabelVector& src)
{
  PyLabelVector result;
  result.items.reserve(src.size());
  for (const auto& lbl : src)
    result.items.push_back(lbl);

  // Sort by label value for deterministic output
  std::sort(result.items.begin(), result.items.end(),
    [](const Label::Pointer& a, const Label::Pointer& b) {
      return a->GetValue() < b->GetValue();
    });

  return result;
}

Label::AlgorithmType ParseAlgorithmType(const std::string& s)
{
  if (s == "UNDEFINED") return Label::AlgorithmType::Undefined;
  if (s == "MANUAL") return Label::AlgorithmType::MANUAL;
  if (s == "SEMIAUTOMATIC") return Label::AlgorithmType::SEMIAUTOMATIC;
  if (s == "AUTOMATIC") return Label::AlgorithmType::AUTOMATIC;
  throw py::value_error("Unknown algorithm type string: " + s);
}

MultiLabelSegmentation::Pointer LoadSegmentationOrInitializeFromImage(const std::string& path)
{
  auto dv = IOUtil::Load(path);
  if (dv.empty())
    throw py::value_error("Could not load: " + path);

  MultiLabelSegmentation::Pointer seg = dynamic_cast<MultiLabelSegmentation*>(dv[0].GetPointer());

  if (seg.IsNull())
  {
    Image::Pointer image = dynamic_cast<Image*>(dv[0].GetPointer());
    if (image.IsNotNull())
    {
      seg = MultiLabelSegmentation::New();
      seg->InitializeByLabeledImage(image);
    }
  }

  if (seg.IsNull())
    throw py::value_error("Could not load: " + path);
  return seg;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// LabelGroup — snapshot view
// ---------------------------------------------------------------------------

struct LabelGroup
{
  std::size_t index;
  std::string name;
  PyLabelVector labels;
  std::vector<std::string> class_names;
  Image::Pointer image;
};

namespace
{
LabelGroup MakeLabelGroup(MultiLabelSegmentation& seg, unsigned int index)
{
  LabelGroup g;
  g.index = index;
  g.name = seg.GetGroupName(index);
  g.labels = MakeLabelVector(seg.GetLabelsByValue(seg.GetLabelValuesByGroup(index)));
  g.class_names = seg.GetLabelClassNamesByGroup(index);
  g.image = seg.GetGroupImage(index);
  return g;
}
} // namespace

// ---------------------------------------------------------------------------
// Module init
// ---------------------------------------------------------------------------

void InitMultiLabelSegmentation(py::module_& m)
{
  // =======================================================================
  // mitk.DICOMCodeSequence
  // =======================================================================
  py::class_<DICOMCodeSequence>(m, "DICOMCodeSequence",
    R"(DICOM coded entry: a ``(value, scheme, meaning)`` triple.

Used to identify standardized concepts in DICOM (anatomic regions,
segmented property types, ...). For example::

    DICOMCodeSequence("T-A0095", "SRT", "Brain")

Most MITK APIs that take a ``DICOMCodeSequence`` also accept a
``(value, scheme, meaning)`` 3-tuple for convenience.
)")
    .def(py::init<>(),
      "Construct an empty coded entry.")
    .def(py::init<const std::string&, const std::string&, const std::string&>(),
      py::arg("value"), py::arg("scheme"), py::arg("meaning"),
      R"(Construct a coded entry.

Args:
    value: Code value (e.g. ``"T-A0095"``).
    scheme: Coding scheme designator (e.g. ``"SRT"``).
    meaning: Human-readable meaning (e.g. ``"Brain"``).
)")
    .def_property("value", &DICOMCodeSequence::GetValue, &DICOMCodeSequence::SetValue)
    .def_property("scheme", &DICOMCodeSequence::GetScheme, &DICOMCodeSequence::SetScheme)
    .def_property("meaning", &DICOMCodeSequence::GetMeaning, &DICOMCodeSequence::SetMeaning)
    .def_property_readonly("is_empty", &DICOMCodeSequence::IsEmpty)
    .def("__eq__", &DICOMCodeSequence::operator==, py::is_operator())
    .def("__ne__", &DICOMCodeSequence::operator!=, py::is_operator())
    .def("__repr__", [](const DICOMCodeSequence& c) {
      std::ostringstream os;
      os << "DICOMCodeSequence(value='" << c.GetValue()
         << "', scheme='" << c.GetScheme()
         << "', meaning='" << c.GetMeaning() << "')";
      return os.str();
    });

  // =======================================================================
  // mitk.DICOMCodeSequenceWithModifiers
  // =======================================================================
  py::class_<DICOMCodeSequenceWithModifiers, DICOMCodeSequence>(m, "DICOMCodeSequenceWithModifiers",
    R"(:py:class:`DICOMCodeSequence` extended with optional modifier sequences.

Used by DICOM constructs that allow further qualification of a coded
entry (for example "Liver" + modifier "Left lobe").
)")
    .def(py::init<>(),
      "Construct an empty coded entry with no modifiers.")
    .def(py::init<const std::string&, const std::string&, const std::string&>(),
      py::arg("value"), py::arg("scheme"), py::arg("meaning"),
      R"(Construct a coded entry without modifiers.

Args:
    value: Code value.
    scheme: Coding scheme designator.
    meaning: Human-readable meaning.
)")
    .def(py::init<const DICOMCodeSequence&>(), py::arg("code"),
      R"(Promote a plain coded entry to one that can carry modifiers.

Args:
    code: Source coded entry to wrap.
)")
    .def("add_modifier",
      [](DICOMCodeSequenceWithModifiers& self, py::handle h) {
        self.AddModifier(ToDICOMCodeSequence(h));
      }, py::arg("modifier"))
    .def_property_readonly("modifiers",
      [](const DICOMCodeSequenceWithModifiers& self) {
        return std::vector<DICOMCodeSequence>(self.GetModifiers());
      })
    .def_property_readonly("modifier_count", &DICOMCodeSequenceWithModifiers::GetModifierCount)
    .def_property_readonly("has_modifiers", &DICOMCodeSequenceWithModifiers::HasModifiers)
    .def("get_modifier",
      [](const DICOMCodeSequenceWithModifiers& self, std::size_t index) {
        if (index >= self.GetModifierCount())
          throw py::index_error("modifier index out of range");
        return self.GetModifier(index);
      }, py::arg("index"))
    .def("set_modifier",
      [](DICOMCodeSequenceWithModifiers& self, std::size_t index, py::handle h) {
        if (index >= self.GetModifierCount())
          throw py::index_error("modifier index out of range");
        self.SetModifier(index, ToDICOMCodeSequence(h));
      }, py::arg("index"), py::arg("modifier"))
    .def("clear_modifiers", &DICOMCodeSequenceWithModifiers::ClearModifiers)
    .def("__eq__",
      [](const DICOMCodeSequenceWithModifiers& a, const DICOMCodeSequenceWithModifiers& b) {
        return a == b;
      }, py::is_operator())
    .def("__ne__",
      [](const DICOMCodeSequenceWithModifiers& a, const DICOMCodeSequenceWithModifiers& b) {
        return a != b;
      }, py::is_operator())
    .def("__repr__", [](const DICOMCodeSequenceWithModifiers& c) {
      std::ostringstream os;
      os << "DICOMCodeSequenceWithModifiers(value='" << c.GetValue()
         << "', scheme='" << c.GetScheme()
         << "', meaning='" << c.GetMeaning() << "'";
      if (c.HasModifiers())
      {
        os << ", modifiers=[";
        for (std::size_t i = 0; i < c.GetModifierCount(); ++i)
        {
          if (i > 0) os << ", ";
          const auto& mod = c.GetModifier(i);
          os << "DICOMCodeSequence(value='" << mod.GetValue()
             << "', scheme='" << mod.GetScheme()
             << "', meaning='" << mod.GetMeaning() << "')";
        }
        os << "]";
      }
      os << ")";
      return os.str();
    });

  // =======================================================================
  // mitk.AlgorithmType
  // =======================================================================
  py::enum_<Label::AlgorithmType>(m, "AlgorithmType",
    R"(Provenance of a :py:class:`Label`'s segmentation.

Tracks how a label was produced: manually drawn, semi-automatically
generated, fully automatic, or unspecified.
)")
    .value("UNDEFINED", Label::AlgorithmType::Undefined,
      "Provenance not recorded.")
    .value("MANUAL", Label::AlgorithmType::MANUAL,
      "Label was drawn by a human operator.")
    .value("SEMIAUTOMATIC", Label::AlgorithmType::SEMIAUTOMATIC,
      "Label was produced by a semi-automatic tool that required user input.")
    .value("AUTOMATIC", Label::AlgorithmType::AUTOMATIC,
      "Label was produced by a fully automatic algorithm.");

  // =======================================================================
  // mitk.Label
  // =======================================================================
  auto label_class = py::class_<Label, Label::Pointer>(m, "Label",
    R"(A single label inside a :py:class:`MultiLabelSegmentation`.

Carries the integer pixel value used in the label image, a human-readable
name, display attributes (color, opacity, visibility), provenance
metadata (algorithm type/name, tracking IDs), and structured DICOM
descriptors (anatomic region, primary anatomic structure, segmented
property category/type).

Labels expose the same property-owner interface as :py:class:`Image`, so
the :py:meth:`get_property` / :py:meth:`set_property` / :py:meth:`remove_property`
methods used on images work on labels too.
)");
  label_class
    .def(py::init([]() { return Label::New(); }),
      "Construct an unnamed label with a default value.")
    .def(py::init([](Label::PixelType value, const std::string& name) {
        return Label::New(value, name);
      }),
      py::arg("value"), py::arg("name"),
      R"(Construct a label with a given pixel value and name.

Args:
    value: Integer pixel value used to mark this label in the label
        image. Must be in ``[1, MAX_LABEL_VALUE]``; 0 is reserved.
    name: Human-readable label name.
)")
    .def_property_readonly("value", &Label::GetValue)
    .def_property("name",
      &Label::GetName,
      &Label::SetName)
    .def_property("color",
      [](const Label& l) { return ColorToTuple(l.GetColor()); },
      [](Label& l, const std::tuple<float, float, float>& c) { l.SetColor(TupleToColor(c)); })
    .def_property("opacity",
      &Label::GetOpacity,
      &Label::SetOpacity)
    .def_property("visible",
      &Label::GetVisible,
      &Label::SetVisible)
    .def_property("locked",
      &Label::GetLocked,
      &Label::SetLocked)

    // --- description, tracking ---
    .def_property("description", &Label::GetDescription, &Label::SetDescription)
    .def_property("tracking_id", &Label::GetTrackingID, &Label::SetTrackingID)
    .def_property("tracking_uid", &Label::GetTrackingUID, &Label::SetTrackingUID)

    // --- algorithm ---
    .def_property("algorithm_type", &Label::GetAlgorithmType, &Label::SetAlgorithmType)
    .def_property("algorithm_type_str", &Label::GetAlgorithmTypeStr,
      [](Label& l, const std::string& s) { l.SetAlgorithmTypeStr(s); })
    .def_property("algorithm_name", &Label::GetAlgorithmName, &Label::SetAlgorithmName)
    .def("add_tool_use",
      [](Label& l, py::handle algoType, const std::string& algoName) {
        const auto t = py::isinstance<Label::AlgorithmType>(algoType)
          ? algoType.cast<Label::AlgorithmType>()
          : ParseAlgorithmType(algoType.cast<std::string>());
        l.AddToolUse(t, algoName);
      },
      py::arg("algorithm_type"), py::arg("algorithm_name"))

    // --- center of mass ---
    .def_property_readonly("center_of_mass_index",
      [](const Label& l) { return Point3DToTuple(l.GetCenterOfMassIndex()); })
    .def_property_readonly("center_of_mass_coordinates",
      [](const Label& l) { return Point3DToTuple(l.GetCenterOfMassCoordinates()); })
    .def_property_readonly("center_of_mass_mtime",
      [](const Label& l) { return l.GetCenterOfMassMTime(); })
    .def("reset_center_of_mass", &Label::ResetCenterOfMass)
    .def("update_center_of_mass",
      [](Label& l, const std::array<double, 3>& index, const std::array<double, 3>& coords) {
        mitk::Point3D pi;
        pi[0] = index[0]; pi[1] = index[1]; pi[2] = index[2];
        mitk::Point3D pc;
        pc[0] = coords[0]; pc[1] = coords[1]; pc[2] = coords[2];
        l.UpdateCenterOfMass(pi, pc);
      },
      py::arg("index"), py::arg("coordinates"))

    // --- anatomic region ---
    .def_property_readonly("anatomic_region_count", &Label::GetAnatomicRegionCount)
    .def("get_anatomic_region",
      [](const Label& l, std::size_t index) { return l.GetAnatomicRegion(index); },
      py::arg("index") = 0)
    .def("set_anatomic_region",
      [](Label& l, py::handle code, std::size_t index) {
        l.SetAnatomicRegion(ToDICOMCodeSequenceWithModifiers(code), index);
      },
      py::arg("code"), py::arg("index") = 0)
    .def("remove_anatomic_region", &Label::RemoveAnatomicRegion, py::arg("index"))
    .def_property("anatomic_region",
      [](const Label& l) -> py::object {
        if (l.GetAnatomicRegionCount() == 0)
          return py::none();
        return py::cast(l.GetAnatomicRegion(0));
      },
      [](Label& l, py::handle code) {
        if (code.is_none())
          return;
        l.SetAnatomicRegion(ToDICOMCodeSequenceWithModifiers(code), 0);
      })

    // --- primary anatomic structure ---
    .def_property_readonly("primary_anatomic_structure_count", &Label::GetPrimaryAnatomicStructureCount)
    .def("get_primary_anatomic_structure",
      [](const Label& l, std::size_t index) { return l.GetPrimaryAnatomicStructure(index); },
      py::arg("index") = 0)
    .def("set_primary_anatomic_structure",
      [](Label& l, py::handle code, std::size_t index) {
        l.SetPrimaryAnatomicStructure(ToDICOMCodeSequenceWithModifiers(code), index);
      },
      py::arg("code"), py::arg("index") = 0)
    .def("remove_primary_anatomic_structure", &Label::RemovePrimaryAnatomicStructure, py::arg("index"))
    .def_property("primary_anatomic_structure",
      [](const Label& l) -> py::object {
        if (l.GetPrimaryAnatomicStructureCount() == 0)
          return py::none();
        return py::cast(l.GetPrimaryAnatomicStructure(0));
      },
      [](Label& l, py::handle code) {
        if (code.is_none())
          return;
        l.SetPrimaryAnatomicStructure(ToDICOMCodeSequenceWithModifiers(code), 0);
      })

    // --- segmented property category ---
    .def_property("segmented_property_category",
      [](const Label& l) -> py::object {
        const auto cat = l.GetSegmentedPropertyCategory();
        if (!cat.has_value() || cat->IsEmpty())
          return py::none();
        return py::cast(*cat);
      },
      [](Label& l, py::handle code) {
        if (code.is_none())
          l.SetSegmentedPropertyCategory(DICOMCodeSequence{});
        else
          l.SetSegmentedPropertyCategory(ToDICOMCodeSequence(code));
      })

    // --- segmented property type ---
    .def_property("segmented_property_type",
      [](const Label& l) -> py::object {
        const auto t = l.GetSegmentedPropertyType();
        if (!t.has_value() || t->IsEmpty())
          return py::none();
        return py::cast(*t);
      },
      [](Label& l, py::handle code) {
        if (code.is_none())
          l.SetSegmentedPropertyType(DICOMCodeSequenceWithModifiers{});
        else
          l.SetSegmentedPropertyType(ToDICOMCodeSequenceWithModifiers(code));
      })

    // --- utility ---
    .def("update",
      [](Label& l, const Label* tmpl, bool updateValue) {
        l.Update(tmpl, updateValue);
      },
      py::arg("template_label"), py::arg("update_value") = false)
    .def_readonly_static("MAX_LABEL_VALUE", &Label::MAX_LABEL_VALUE)

    .def("__eq__",
      [](const Label& a, const Label& b) { return a.GetValue() == b.GetValue(); },
      py::is_operator())
    .def("__hash__",
      [](const Label& l) { return std::hash<Label::PixelType>{}(l.GetValue()); })
    .def("__repr__",
      [](const Label& l) {
        // Compare against a freshly-constructed Label so defaults stay in sync
        // if Label::New() ever changes them.
        static const Label::Pointer defaults = Label::New();
        const auto colorsDiffer = [](const mitk::Color& a, const mitk::Color& b) {
          return std::abs(a.GetRed() - b.GetRed()) > 1e-5f ||
                 std::abs(a.GetGreen() - b.GetGreen()) > 1e-5f ||
                 std::abs(a.GetBlue() - b.GetBlue()) > 1e-5f;
        };

        std::ostringstream os;
        os << "Label(value=" << l.GetValue()
           << ", name='" << l.GetName() << "'";

        if (!l.GetDescription().empty())
          os << ", description='" << l.GetDescription() << "'";
        if (!l.GetTrackingID().empty())
          os << ", tracking_id='" << l.GetTrackingID() << "'";
        if (!l.GetTrackingUID().empty())
          os << ", tracking_uid='" << l.GetTrackingUID() << "'";
        if (l.GetAlgorithmType() != Label::AlgorithmType::Undefined)
          os << ", algorithm_type=AlgorithmType." << l.GetAlgorithmTypeStr();
        if (l.GetConstProperty("algorithm_name").IsNotNull())
          os << ", algorithm_name='" << l.GetAlgorithmName() << "'";
        if (l.GetLocked() != defaults->GetLocked())
          os << ", locked=" << (l.GetLocked() ? "True" : "False");
        if (l.GetVisible() != defaults->GetVisible())
          os << ", visible=" << (l.GetVisible() ? "True" : "False");
        const auto opacity = l.GetOpacity();
        if (std::abs(opacity - defaults->GetOpacity()) > 1e-5f)
          os << ", opacity=" << opacity;
        const auto color = l.GetColor();
        if (colorsDiffer(color, defaults->GetColor()))
          os << ", color=(" << color.GetRed() << ", " << color.GetGreen() << ", " << color.GetBlue() << ")";

        os << ")";

        std::vector<std::string> extras;
        if (l.GetAnatomicRegionCount() > 0)
          extras.push_back("anatomic_region");
        if (l.GetPrimaryAnatomicStructureCount() > 0)
          extras.push_back("primary_anatomic_structure");
        if (l.GetSegmentedPropertyCategory().has_value() && !l.GetSegmentedPropertyCategory()->IsEmpty())
          extras.push_back("segmented_property_category");
        if (l.GetSegmentedPropertyType().has_value() && !l.GetSegmentedPropertyType()->IsEmpty())
          extras.push_back("segmented_property_type");
        if (!extras.empty())
        {
          os << "  # +";
          for (std::size_t i = 0; i < extras.size(); ++i)
          {
            if (i > 0) os << ", +";
            os << extras[i];
          }
        }

        return os.str();
      });

  // Bind property owner methods on Label (it derives from PropertyList which is IPropertyOwner)
  bind_property_owner(label_class);

  // =======================================================================
  // mitk.LabelVector
  // =======================================================================
  py::class_<PyLabelVector>(m, "LabelVector",
    R"(Immutable, sliceable sequence of :py:class:`Label` objects.

Returned by methods like :py:meth:`MultiLabelSegmentation.get_labels` and
:py:meth:`MultiLabelSegmentation.get_group_labels`. Supports ``len``,
iteration, integer and slice indexing, membership testing, and exposes a
flat :py:attr:`values` list of the underlying label pixel values.
)")
    .def("__len__", [](const PyLabelVector& v) { return v.items.size(); },
      "Return the number of labels.")
    .def("__iter__",
      [](const PyLabelVector& v) { return py::make_iterator(v.items.begin(), v.items.end()); },
      py::keep_alive<0, 1>())
    .def("__getitem__",
      [](const PyLabelVector& v, py::ssize_t i) -> Label::Pointer {
        auto idx = i < 0 ? static_cast<py::ssize_t>(v.items.size()) + i : i;
        if (idx < 0 || idx >= static_cast<py::ssize_t>(v.items.size()))
          throw py::index_error("LabelVector index out of range");
        return v.items[static_cast<size_t>(idx)];
      })
    .def("__getitem__",
      [](const PyLabelVector& v, py::slice slice) {
        py::ssize_t start, stop, step, slicelength;
        if (!slice.compute(static_cast<py::ssize_t>(v.items.size()), &start, &stop, &step, &slicelength))
          throw py::error_already_set();

        PyLabelVector result;
        result.items.reserve(static_cast<size_t>(slicelength));
        for (py::ssize_t i = 0; i < slicelength; ++i)
          result.items.push_back(v.items[static_cast<size_t>(start + i * step)]);
        return result;
      })
    .def("__contains__",
      [](const PyLabelVector& v, py::handle h) {
        const auto val = ToLabelValue(h);
        return std::any_of(v.items.begin(), v.items.end(),
          [val](const Label::Pointer& l) { return l->GetValue() == val; });
      })
    .def_property_readonly("values",
      [](const PyLabelVector& v) {
        std::vector<Label::PixelType> vals;
        vals.reserve(v.items.size());
        for (const auto& l : v.items)
          vals.push_back(l->GetValue());
        return vals;
      })
    .def("__repr__",
      [](const PyLabelVector& v) {
        std::ostringstream os;
        os << "LabelVector([";
        for (size_t i = 0; i < v.items.size(); ++i)
        {
          if (i > 0) os << ", ";
          os << v.items[i]->GetValue();
        }
        os << "])";
        return os.str();
      });

  // =======================================================================
  // mitk.LabelGroup
  // =======================================================================
  py::class_<LabelGroup>(m, "LabelGroup",
    R"(One layer of a :py:class:`MultiLabelSegmentation`.

A multi-label segmentation is organized into one or more *groups*. Each
group has its own backing image and its own set of labels; labels in
different groups can overlap spatially because they live in separate
images.

A ``LabelGroup`` is a snapshot view: ``index``, ``name``, ``labels``,
``class_names``, and ``image`` are read-only fields representing the
group's state at the moment it was returned by the parent segmentation.
)")
    .def_readonly("index", &LabelGroup::index,
      "Zero-based index of the group inside its segmentation.")
    .def_readonly("name", &LabelGroup::name,
      "Display name of the group.")
    .def_readonly("labels", &LabelGroup::labels,
      "The :py:class:`LabelVector` for this group.")
    .def_readonly("class_names", &LabelGroup::class_names,
      "List of class-name strings for the labels in this group.")
    .def_readonly("image", &LabelGroup::image,
      "Backing :py:class:`Image` that stores this group's labels.")
    .def("__repr__",
      [](const LabelGroup& g) {
        std::ostringstream os;
        os << "LabelGroup(index=" << g.index
           << ", name='" << g.name
           << "', labels=" << g.labels.items.size() << ")";
        return os.str();
      });

  // =======================================================================
  // mitk.MergeStyle, mitk.OverwriteStyle
  // =======================================================================
  py::enum_<MultiLabelSegmentation::MergeStyle>(m, "MergeStyle",
    R"(How an incoming source image should combine with an existing destination
in a :py:class:`MultiLabelSegmentation` mutation.
)")
    .value("REPLACE", MultiLabelSegmentation::MergeStyle::Replace,
      "Overwrite destination pixels wherever the source provides a value.")
    .value("MERGE", MultiLabelSegmentation::MergeStyle::Merge,
      "Combine source and destination labels (the source layer is added on top).");

  py::enum_<MultiLabelSegmentation::OverwriteStyle>(m, "OverwriteStyle",
    R"(Whether a mutation should respect the per-label *locked* flag.

When a label is locked, edits at its pixels are normally prevented; this
enum lets callers force the operation through.
)")
    .value("REGARD_LOCKS", MultiLabelSegmentation::OverwriteStyle::RegardLocks,
      "Skip pixels of labels marked as locked.")
    .value("IGNORE_LOCKS", MultiLabelSegmentation::OverwriteStyle::IgnoreLocks,
      "Edit all pixels regardless of label lock state.");

  // =======================================================================
  // mitk.MultiLabelSegmentation
  // =======================================================================
  auto seg_class = py::class_<MultiLabelSegmentation, MultiLabelSegmentation::Pointer>(m, "MultiLabelSegmentation",
    R"(Multi-label, multi-group segmentation data structure.

A ``MultiLabelSegmentation`` represents one or more labeled regions over a
shared spatial geometry. It is organized into *groups* (see
:py:class:`LabelGroup`): each group has its own backing label image and
its own list of :py:class:`Label` instances. Labels in distinct groups
may overlap spatially because they live in separate images.

Construction is overloaded:

- ``MultiLabelSegmentation()`` constructs an empty segmentation.
- ``MultiLabelSegmentation(reference_image)`` constructs an empty
  segmentation with the geometry of *reference_image*.
- ``MultiLabelSegmentation(time_geometry)`` /
  ``MultiLabelSegmentation(base_geometry)`` use an explicit geometry.
- ``MultiLabelSegmentation(path)`` loads from disk.

The same surface is reachable via :py:meth:`load` and
:py:meth:`initialize`.
)");
  seg_class

    .def(py::init([] { return MultiLabelSegmentation::New(); }),
      "Construct an empty segmentation with no geometry and no labels.")
    .def(py::init([](const Image* ref) {
        auto s = MultiLabelSegmentation::New();
        s->Initialize(ref, true, true);
        return s;
      }),
      py::arg("reference_image"),
      R"(Construct a segmentation initialized from a reference image.

The segmentation adopts the spatial and time geometry of *reference_image*
and starts with no labels and one (empty) group.

Args:
    reference_image: An :py:class:`Image` whose geometry to copy.
)")
    .def(py::init([](const TimeGeometry* g) {
        auto s = MultiLabelSegmentation::New();
        s->Initialize(g, true, true);
        return s;
      }),
      py::arg("time_geometry"),
      R"(Construct a segmentation from an explicit time geometry.

Args:
    time_geometry: The :py:class:`TimeGeometry` to use.
)")
    .def(py::init([](const BaseGeometry* g) {
        auto tg = ProportionalTimeGeometry::New();
        tg->Initialize(g, 1);
        auto s = MultiLabelSegmentation::New();
        s->Initialize(tg, true, true);
        return s;
      }),
      py::arg("base_geometry"),
      R"(Construct a static (1-time-step) segmentation from a 3D geometry.

A :py:class:`ProportionalTimeGeometry` with a single time step is built
to wrap *base_geometry*.

Args:
    base_geometry: The 3D spatial geometry to use.
)")
    .def(py::init([](const std::filesystem::path& p) {
        return LoadSegmentationOrInitializeFromImage(p.string());
      }),
      py::arg("path"),
      R"(Load a segmentation from disk.

If the file is a plain image (not a multi-label segmentation format), the
image is interpreted as a single-group labeled image (each unique
non-zero pixel value becomes one :py:class:`Label`).

Args:
    path: Path to the segmentation file (``str`` or ``pathlib.Path``).

Raises:
    ValueError: If the file cannot be loaded.
)")

    .def_static("from_labeled_image",
      [](const Image* image) {
        auto s = MultiLabelSegmentation::New();
        s->InitializeByLabeledImage(image);
        return s;
      },
      py::arg("image"),
      R"(Construct a segmentation from a pre-labeled image.

Each unique non-zero pixel value in *image* becomes one :py:class:`Label`
in a single group of the resulting segmentation.

Args:
    image: An :py:class:`Image` containing integer label values.

Returns:
    A new :py:class:`MultiLabelSegmentation`.
)")
    .def_static("load",
      [](const std::string& path) {
        return LoadSegmentationOrInitializeFromImage(path);
      },
      py::arg("path"),
      R"(Load a segmentation from disk (explicit factory).

Equivalent to ``MultiLabelSegmentation(path)``.

Args:
    path: Path to the segmentation file.

Returns:
    The loaded segmentation.
)")

    .def("save",
      [](const MultiLabelSegmentation& seg, const std::string& path) {
        IOUtil::Save(&seg, path);
      },
      py::arg("path"),
      R"(Save the segmentation to a file.

The output format is inferred from the file extension. MITK ships with
writers for the segmentation file format (``.nrrd`` with auxiliary
metadata) and for DICOM SEG (``.dcm``).

Args:
    path: Output file path.
)")
    .def("initialize",
      [](MultiLabelSegmentation& seg, const Image* ref, bool resetLabels) {
        seg.Initialize(ref, resetLabels, true);
      },
      py::arg("reference_image"), py::arg("reset_labels") = true,
      R"(Re-initialize the segmentation from a reference image.

Args:
    reference_image: Image whose geometry to adopt.
    reset_labels: If True (default), discard all existing labels and
        groups. If False, keep them.
)")
    .def("initialize",
      [](MultiLabelSegmentation& seg, const TimeGeometry* g, bool resetLabels) {
        seg.Initialize(g, resetLabels, true);
      },
      py::arg("time_geometry"), py::arg("reset_labels") = true,
      R"(Re-initialize the segmentation with an explicit time geometry.

Args:
    time_geometry: The new geometry.
    reset_labels: If True (default), discard all existing labels and
        groups.
)")

    // ----------------------------------------------------------------
    // Label queries
    // ----------------------------------------------------------------
    .def_property_readonly("labels",
      [](MultiLabelSegmentation& seg) {
        return MakeLabelVector(seg.GetLabels());
      },
      "All labels across all groups, as a :py:class:`LabelVector`.")
    .def_property_readonly("label_values",
      [](const MultiLabelSegmentation& seg) {
        auto vals = seg.GetAllLabelValues();
        std::sort(vals.begin(), vals.end());
        return vals;
      },
      "Sorted list of all label pixel values across all groups.")
    .def("get_label",
      [](MultiLabelSegmentation& seg, Label::PixelType value) -> Label::Pointer {
        auto lbl = seg.GetLabel(value);
        if (lbl.IsNull())
          throw py::value_error("Label value " + std::to_string(value) + " not found");
        return lbl;
      },
      py::arg("value"),
      R"(Return the :py:class:`Label` with the given pixel value.

Args:
    value: Integer label pixel value.

Returns:
    The matching :py:class:`Label`.

Raises:
    ValueError: If no label with that value exists.
)")
    .def("get_labels",
      [](MultiLabelSegmentation& seg, py::iterable values) {
        return MakeLabelVector(seg.GetLabelsByValue(ToLabelValues(values)));
      },
      py::arg("values"),
      R"(Return the labels for a sequence of label values.

Args:
    values: Iterable of integer label values or :py:class:`Label`
        objects.

Returns:
    A :py:class:`LabelVector` (missing values are silently skipped).
)")
    .def("get_label_values_by_name",
      [](const MultiLabelSegmentation& seg, const std::string& name, std::optional<MultiLabelSegmentation::GroupIndexType> group) {
        if (group.has_value())
          return seg.GetLabelValuesByName(*group, name);
        return seg.GetLabelValuesByName(name);
      },
      py::arg("name"), py::arg("group") = py::none(),
      R"(Return the label values whose name matches *name*.

Args:
    name: Label name to look up.
    group: Optional group index to restrict the search to. If omitted,
        labels across all groups are considered.

Returns:
    A list of integer label values. Empty if no label matches.
)")
    .def("get_label_values_at",
      [](const MultiLabelSegmentation& seg, const std::array<double, 3>& coords,
         TimeStepType timeStep, std::optional<MultiLabelSegmentation::GroupIndexType> group) {
        Point3D pt;
        pt[0] = coords[0];
        pt[1] = coords[1];
        pt[2] = coords[2];
        return seg.GetLabelValuesByCoordinates(pt, timeStep, group);
      },
      py::arg("coordinates"), py::arg("time_step") = 0, py::arg("group") = py::none(),
      R"(Return the label values present at the given world-space coordinates.

For overlapping multi-group segmentations a single point can belong to
several label values across groups.

Args:
    coordinates: 3-element sequence ``(x, y, z)`` in world coordinates.
    time_step: Time-step index. Defaults to 0.
    group: Optional group index to restrict the lookup to. If omitted,
        all groups are queried.

Returns:
    A list of integer label values at the given point.
)")
    .def_property_readonly("label_class_names",
      &MultiLabelSegmentation::GetLabelClassNames,
      "Sorted list of unique class names assigned to labels in this "
      "segmentation.")

    .def_property_readonly("num_groups",
      &MultiLabelSegmentation::GetNumberOfGroups,
      "Number of label groups in this segmentation.")
    .def("get_group",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        if (!seg.ExistGroup(index))
          throw py::value_error("Group index " + std::to_string(index) + " does not exist");
        return MakeLabelGroup(seg, index);
      },
      py::arg("index"),
      R"(Return the :py:class:`LabelGroup` snapshot at *index*.

Args:
    index: Zero-based group index.

Raises:
    ValueError: If the index does not exist.
)")
    .def_property_readonly("groups",
      [](MultiLabelSegmentation& seg) {
        std::vector<LabelGroup> groups;
        const auto n = seg.GetNumberOfGroups();
        groups.reserve(n);
        for (unsigned int i = 0; i < n; ++i)
          groups.push_back(MakeLabelGroup(seg, i));
        return groups;
      },
      "List of :py:class:`LabelGroup` snapshots for every group.")
    .def("get_group_by_name",
      [](MultiLabelSegmentation& seg, const std::string& name) -> std::optional<LabelGroup> {
        const auto n = seg.GetNumberOfGroups();
        for (unsigned int i = 0; i < n; ++i)
        {
          if (seg.GetGroupName(i) == name)
            return MakeLabelGroup(seg, i);
        }
        return std::nullopt;
      },
      py::arg("name"),
      R"(Find a group by its display name.

Args:
    name: Group display name to look up.

Returns:
    The matching :py:class:`LabelGroup` snapshot, or ``None`` if no
    group has that name.
)")
    .def("get_group_name",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        return seg.GetGroupName(index);
      },
      py::arg("index"),
      "Return the display name of the group at *index*.")
    .def("get_group_labels",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        return MakeLabelVector(seg.GetLabelsByValue(seg.GetLabelValuesByGroup(index)));
      },
      py::arg("index"),
      "Return the :py:class:`LabelVector` for the group at *index*.")
    .def("get_group_label_values",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        return seg.GetLabelValuesByGroup(index);
      },
      py::arg("index"),
      "Return the integer label values for the group at *index*.")
    .def("get_group_class_names",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        return seg.GetLabelClassNamesByGroup(index);
      },
      py::arg("index"),
      "Return the list of class-name strings for the group at *index*.")
    .def("get_group_of_label",
      [](const MultiLabelSegmentation& seg, Label::PixelType value) {
        return seg.GetGroupIndexOfLabel(value);
      },
      py::arg("value"),
      R"(Return the group index that owns the label with pixel value *value*.

Args:
    value: Integer label pixel value.

Returns:
    The owning group's index.
)")
    .def("get_group_image",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) -> Image::Pointer {
        return seg.GetGroupImage(index);
      },
      py::arg("index"),
      "Return the backing :py:class:`Image` of the group at *index*.")
    .def("set_group_image",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index, const Image* image) {
        seg.UpdateGroupImage(index, image, 0, 0);
      },
      py::arg("index"), py::arg("image"),
      R"(Replace the backing image of the group at *index*.

Args:
    index: Zero-based group index.
    image: New :py:class:`Image`. Must match the segmentation's geometry.
)")

    .def("add_label",
      [](MultiLabelSegmentation& seg, Label* label, MultiLabelSegmentation::GroupIndexType group) -> Label::Pointer {
        return seg.AddLabel(label, group, true, true);
      },
      py::arg("label"), py::arg("group") = 0,
      R"(Add an existing :py:class:`Label` to the segmentation.

The input *label* is **cloned**; the segmentation stores the copy. Edits
applied to the original *label* after this call do not affect the
segmentation. To modify the in-segmentation label, work with the returned
:py:class:`Label` instance instead.

Args:
    label: The label to clone and add. If its pixel value collides with
        an existing label, a new value is assigned to the clone.
    group: Target group index (default 0).

Returns:
    The cloned label as stored in the segmentation (its assigned pixel
    value may differ from the input). Use this object for further edits.
)")
    .def("add_label",
      [](MultiLabelSegmentation& seg, const std::string& name,
         const std::tuple<float, float, float>& color,
         MultiLabelSegmentation::GroupIndexType group) -> Label::Pointer {
        return seg.AddLabel(name, TupleToColor(color), group);
      },
      py::arg("name"), py::arg("color"), py::arg("group") = 0,
      R"(Add a new label by name and color.

Args:
    name: Label name.
    color: ``(r, g, b)`` tuple of floats in ``[0, 1]``.
    group: Target group index (default 0).

Returns:
    The newly created label.
)")
    .def("remove_label",
      [](MultiLabelSegmentation& seg, py::handle value) {
        seg.RemoveLabel(ToLabelValue(value));
      },
      py::arg("value"),
      R"(Remove a label entirely (metadata and pixels).

Args:
    value: Label pixel value or :py:class:`Label` object.
)")
    .def("remove_labels",
      [](MultiLabelSegmentation& seg, py::iterable values) {
        seg.RemoveLabels(ToLabelValues(values));
      },
      py::arg("values"),
      R"(Remove multiple labels at once.

Args:
    values: Iterable of label values or :py:class:`Label` objects.
)")
    .def("erase_label",
      [](MultiLabelSegmentation& seg, py::handle value) {
        seg.EraseLabel(ToLabelValue(value));
      },
      py::arg("value"),
      R"(Clear the pixels of a label without removing its metadata.

The :py:class:`Label` stays in the segmentation; only the pixels of its
backing image are reset. Use :py:meth:`remove_label` to delete the
label entirely.

Args:
    value: Label pixel value or :py:class:`Label` object.
)")
    .def("erase_labels",
      [](MultiLabelSegmentation& seg, py::iterable values) {
        seg.EraseLabels(ToLabelValues(values));
      },
      py::arg("values"),
      R"(Clear the pixels of multiple labels at once.

Args:
    values: Iterable of label values or :py:class:`Label` objects.
)")
    .def("rename_label",
      [](MultiLabelSegmentation& seg, py::handle value,
         const std::string& name, const std::tuple<float, float, float>& color) {
        seg.RenameLabel(ToLabelValue(value), name, TupleToColor(color));
      },
      py::arg("value"), py::arg("name"), py::arg("color"),
      R"(Change a label's display name and color.

Args:
    value: Label pixel value or :py:class:`Label` object.
    name: New display name.
    color: ``(r, g, b)`` tuple of floats in ``[0, 1]``.
)")
    .def("merge_labels",
      [](MultiLabelSegmentation& seg, py::handle target, py::iterable sources,
         py::handle overwriteStyle) {
        seg.MergeLabels(ToLabelValue(target), ToLabelValues(sources), ToOverwriteStyle(overwriteStyle));
      },
      py::arg("target"), py::arg("sources"),
      py::arg("overwrite_style") = MultiLabelSegmentation::OverwriteStyle::RegardLocks,
      R"(Merge multiple source labels into a single target label.

Pixels belonging to any of the source labels are reassigned to the
target. The source labels are removed.

Args:
    target: Destination label (pixel value or :py:class:`Label`).
    sources: Iterable of label values or :py:class:`Label` objects.
    overwrite_style: How to treat locked target pixels (see
        :py:class:`OverwriteStyle`). Defaults to ``REGARD_LOCKS``.
)")

    .def("add_group",
      [](MultiLabelSegmentation& seg, Image* image, std::optional<py::list> labels) -> MultiLabelSegmentation::GroupIndexType {
        ConstLabelVector labelVec;
        if (labels.has_value())
        {
          for (auto item : *labels)
            labelVec.push_back(Label::ConstPointer(item.cast<Label*>()));
        }
        if (image != nullptr)
          return seg.AddGroup(image, labelVec);
        return seg.AddGroup(labelVec);
      },
      py::arg("image") = py::none(), py::arg("labels") = py::none(),
      R"(Append a new group to the segmentation.

Args:
    image: Optional :py:class:`Image` to use as the group's backing
        image. If omitted, a fresh blank image is allocated.
    labels: Optional list of :py:class:`Label` objects to populate the
        new group with.

Returns:
    The index of the new group.
)")
    .def("remove_group",
      &MultiLabelSegmentation::RemoveGroup,
      py::arg("index"),
      R"(Remove the group at *index*.

Args:
    index: Zero-based group index.
)")
    .def("clear_group_image",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index,
         std::optional<TimeStepType> timeStep) {
        if (timeStep.has_value())
          seg.ClearGroupImage(index, *timeStep);
        else
          seg.ClearGroupImage(index);
      },
      py::arg("index"), py::arg("time_step") = py::none(),
      R"(Zero out the pixels of one group's backing image.

The group and its labels are preserved; only pixel data is cleared.

Args:
    index: Group index.
    time_step: If given, only that time step is cleared. Otherwise all
        time steps are cleared.
)")
    .def("clear_group_images",
      [](MultiLabelSegmentation& seg, std::optional<TimeStepType> timeStep) {
        if (timeStep.has_value())
          seg.ClearGroupImages(*timeStep);
        else
          seg.ClearGroupImages();
      },
      py::arg("time_step") = py::none(),
      R"(Zero out the pixels of every group's backing image.

Args:
    time_step: If given, only that time step is cleared across all
        groups. Otherwise all time steps are cleared.
)")
    .def("clone",
      [](const MultiLabelSegmentation& seg) -> MultiLabelSegmentation::Pointer {
        return seg.Clone();
      },
      "Return a deep copy of the segmentation.")
    .def("relabel_to",
      [](const MultiLabelSegmentation& self, py::iterable labelMapping,
         py::object destSeg,
         bool keepUntouchedLabels,
         py::handle mergeStyle, py::handle overwriteStyle) -> py::object {
        const bool destProvided = !destSeg.is_none();
        if (keepUntouchedLabels && destProvided)
          throw py::value_error(
            "keep_untouched_labels=True is only allowed when dest_seg is None. "
            "When an explicit dest_seg is provided the caller controls its "
            "content; clear it manually beforehand if a clean slate is needed.");

        py::object targetObj;
        MultiLabelSegmentation* targetPtr = nullptr;
        if (destProvided)
        {
          targetObj = destSeg;
          targetPtr = destSeg.cast<MultiLabelSegmentation*>();
        }
        else
        {
          MultiLabelSegmentation::Pointer clone = self.Clone();
          if (!keepUntouchedLabels)
            clone->ClearGroupImages();
          targetPtr = clone.GetPointer();
          targetObj = py::cast(clone);
        }

        TransferLabelContent(&self, targetPtr,
          ToLabelMapping(labelMapping),
          ToMergeStyle(mergeStyle),
          ToOverwriteStyle(overwriteStyle));
        return targetObj;
      },
      py::arg("label_mapping"),
      py::arg("dest_seg") = py::none(),
      py::arg("keep_untouched_labels") = false,
      py::arg("merge_style") = MultiLabelSegmentation::MergeStyle::Replace,
      py::arg("overwrite_style") = MultiLabelSegmentation::OverwriteStyle::IgnoreLocks,
      R"(Apply a label-value mapping to produce a new (or updated) segmentation.

Args:
    label_mapping: Iterable of ``(source, destination)`` pairs. Each
        side is a label pixel value or a :py:class:`Label` object.
    dest_seg: Target :py:class:`MultiLabelSegmentation`. If ``None``
        (default), ``self`` is cloned and used as the target (see
        *keep_untouched_labels* for how the clone is prepared). When an
        explicit target is supplied, every destination value referenced
        in *label_mapping* must already exist in *dest_seg*.
    keep_untouched_labels: Only valid when ``dest_seg`` is ``None``.

        * ``False`` (default): the clone's group images are cleared
          before the transfer. Only pixels of mapped labels appear in
          the result.
        * ``True``: the clone retains its original pixel content. Mapped
          labels are remapped in-place; every other label is left
          untouched. Combining this with an explicit ``dest_seg`` is
          rejected because the caller already controls the state of
          that object.
    merge_style: :py:class:`MergeStyle` (or lowercase string alias
        ``"replace"`` / ``"merge"``). Defaults to ``REPLACE``.
    overwrite_style: :py:class:`OverwriteStyle` (or lowercase string
        alias ``"regard_locks"`` / ``"ignore_locks"``). Defaults to
        ``IGNORE_LOCKS``.

Returns:
    The populated *dest_seg* (or the auto-created clone when
    *dest_seg* was ``None``).

Raises:
    ValueError: If ``keep_untouched_labels=True`` is combined with an
        explicit *dest_seg*.
)")
    .def("update_group_image",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index,
         const Image* source, TimeStepType timeStep, TimeStepType sourceTimeStep) {
        seg.UpdateGroupImage(index, source, timeStep, sourceTimeStep);
      },
      py::arg("index"), py::arg("source"), py::arg("time_step"), py::arg("source_time_step") = 0,
      R"(Copy a single time step into one group's backing image.

Args:
    index: Group index.
    source: Source :py:class:`Image`.
    time_step: Destination time step in the segmentation.
    source_time_step: Time step to read from *source* (default 0).
)")
    .def("set_group_name",
      &MultiLabelSegmentation::SetGroupName,
      py::arg("index"), py::arg("name"),
      R"(Set the display name of the group at *index*.

Args:
    index: Group index.
    name: New display name.
)")

    .def("create_label_mask",
      [](const MultiLabelSegmentation& seg, Label::PixelType labelValue, bool binary) {
        return CreateLabelMask(&seg, labelValue, binary);
      },
      py::arg("label"), py::arg("binary") = true,
      R"(Extract a mask image for a single label.

Args:
    label: Label pixel value to extract.
    binary: If True (default), produce a 0/1 mask. If False, the mask
        pixels carry the label's original pixel value.

Returns:
    A new :py:class:`Image` masking the requested label.
)")
    .def("create_filtered_group_image",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType group,
         py::iterable labels) {
        return CreateFilteredGroupImage(&seg, group, ToLabelValues(labels));
      },
      py::arg("group"), py::arg("labels"),
      R"(Return a copy of a group's image filtered to the given labels.

Pixels not belonging to any of *labels* are set to the unlabeled value.

Args:
    group: Group index.
    labels: Iterable of label values or :py:class:`Label` objects.

Returns:
    A new :py:class:`Image` containing only the requested labels.
)")
    .def("create_label_class_map",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType group,
         std::optional<py::iterable> labels) {
        std::pair<Image::Pointer, IDToLabelClassNameMapType> result;
        if (labels.has_value())
          result = CreateLabelClassMap(&seg, group, ToLabelValues(*labels));
        else
          result = CreateLabelClassMap(&seg, group);

        return py::make_tuple(result.first, result.second);
      },
      py::arg("group"), py::arg("labels") = py::none(),
      R"(Build a class-name map for one group.

Returns a tuple ``(image, id_to_class_name)`` where *image* is a remapped
image whose pixel values are dense IDs (``1, 2, 3, ...``), and
*id_to_class_name* is a dict keyed by those new IDs mapping to the
corresponding label class name. Pixel value ``0`` is reserved for
unlabeled (background) voxels and is not present in the returned map.

Args:
    group: Group index.
    labels: Optional iterable of labels to restrict the map to. If
        omitted, all labels in the group are included.

Returns:
    A 2-tuple ``(image, class_name_map)`` where *image* is an
    :py:class:`Image` and *class_name_map* is a ``dict[int, str]``
    mapping the remapped dense ID to its class name (background ID ``0``
    is omitted).
)")

    .def("split_labels_by_group",
      [](const MultiLabelSegmentation& seg, py::iterable labels) {
        return LabelSetImageHelper::SplitLabelValuesByGroup(&seg, ToLabelValues(labels));
      },
      py::arg("labels"),
      R"(Partition a list of label values by their owning group.

Args:
    labels: Iterable of label values or :py:class:`Label` objects.

Returns:
    A mapping from group index to the subset of *labels* in that group.
)")
    .def("split_labels_by_class_name",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType group,
         std::optional<py::iterable> labels) {
        if (labels.has_value())
          return LabelSetImageHelper::SplitLabelValuesByClassName(&seg, group, ToLabelValues(*labels));
        return LabelSetImageHelper::SplitLabelValuesByClassName(&seg, group);
      },
      py::arg("group"), py::arg("labels") = py::none(),
      R"(Partition a group's labels by class name.

Args:
    group: Group index.
    labels: Optional iterable of label values to restrict the partition
        to. If omitted, all labels in the group are partitioned.

Returns:
    A mapping from class-name string to the subset of labels with that
    class name.
)")
    .def_readonly_static("UNLABELED_VALUE", &MultiLabelSegmentation::UNLABELED_VALUE,
      "Pixel value used to mark unlabeled regions in a group image.");

  // Geometry parity via shared template
  BindGeometryAccessors<decltype(seg_class), MultiLabelSegmentation>(seg_class);

  // Bind property owner methods
  bind_property_owner(seg_class);

  // =======================================================================
  // Module-level helpers
  // =======================================================================

  m.def("split_label_value_mapping_by_source_and_target_group",
    [](const MultiLabelSegmentation* source, const MultiLabelSegmentation* target,
       py::iterable mapping) {
      return LabelSetImageHelper::SplitLabelValueMappingBySourceAndTargetGroup(
        source, target, ToLabelMapping(mapping));
    },
    py::arg("source"), py::arg("target"), py::arg("label_mapping"),
    R"(Partition a label-value mapping by source-group and target-group pair.

Used internally by :py:func:`transfer_labels` to schedule cross-group
transfers correctly.

Args:
    source: Source :py:class:`MultiLabelSegmentation`.
    target: Target :py:class:`MultiLabelSegmentation`.
    label_mapping: Iterable of ``(source_value, destination_value)``
        pairs.

Returns:
    A mapping keyed by ``(source_group_index, target_group_index)`` to
    the subset of *label_mapping* whose endpoints live in those groups.
)");

  // ----------------------------------------------------------------
  // transfer_labels — type-dispatched (Segmentation or Image)
  // ----------------------------------------------------------------

  m.def("transfer_labels",
    [](py::object source, py::object destination,
       py::iterable labelMapping,
       py::handle mergeStyle, py::handle overwriteStyle,
       py::object destinationLabels,
       Label::PixelType srcBg,
       Label::PixelType dstBg,
       bool dstBgLocked) {
      const auto mapping = ToLabelMapping(labelMapping);
      const auto ms = ToMergeStyle(mergeStyle);
      const auto os = ToOverwriteStyle(overwriteStyle);

      if (py::isinstance<MultiLabelSegmentation>(source))
      {
        TransferLabelContent(
          source.cast<const MultiLabelSegmentation*>(),
          destination.cast<MultiLabelSegmentation*>(),
          mapping, ms, os);
        return;
      }

      ConstLabelVector dstLabelVec;
      if (!destinationLabels.is_none())
      {
        for (auto item : destinationLabels.cast<py::iterable>())
          dstLabelVec.push_back(Label::ConstPointer(item.cast<Label*>()));
      }
      TransferLabelContent(
        source.cast<const Image*>(),
        destination.cast<Image*>(),
        dstLabelVec, srcBg, dstBg, dstBgLocked,
        mapping, ms, os);
    },
    py::arg("source"), py::arg("destination"),
    py::arg("label_mapping"),
    py::arg("merge_style") = MultiLabelSegmentation::MergeStyle::Replace,
    py::arg("overwrite_style") = MultiLabelSegmentation::OverwriteStyle::RegardLocks,
    py::arg("destination_labels") = py::none(),
    py::arg("source_background") = MultiLabelSegmentation::UNLABELED_VALUE,
    py::arg("destination_background") = MultiLabelSegmentation::UNLABELED_VALUE,
    py::arg("destination_background_locked") = false,
    R"(Transfer label content from *source* to *destination*.

Dispatches based on the type of *source*:

- If *source* is a :py:class:`MultiLabelSegmentation`, *destination*
  must also be one. The cross-group transfer is performed in-place.
- If *source* is a plain :py:class:`Image`, *destination* must also be
  one. *destination_labels* must be supplied to interpret the pixel
  values, and *source_background* / *destination_background* identify
  the unlabeled values in each image.

Args:
    source: Source segmentation or image.
    destination: Target segmentation or image (must match *source*'s
        type).
    label_mapping: Iterable of ``(source_value, destination_value)``
        pairs.
    merge_style: :py:class:`MergeStyle`. Defaults to ``REPLACE``.
    overwrite_style: :py:class:`OverwriteStyle`. Defaults to
        ``REGARD_LOCKS``.
    destination_labels: Required for image-mode transfers. Iterable of
        :py:class:`Label` objects describing the destination image's
        labels.
    source_background: Unlabeled value in *source* (image mode only).
    destination_background: Unlabeled value in *destination* (image
        mode only).
    destination_background_locked: If True, treat the destination
        background pixel as locked.
)");

  m.def("transfer_labels_at_time_step",
    [](py::object source, py::object destination,
       TimeStepType timeStep,
       py::iterable labelMapping,
       py::handle mergeStyle, py::handle overwriteStyle,
       py::object destinationLabels,
       Label::PixelType srcBg,
       Label::PixelType dstBg,
       bool dstBgLocked) {
      const auto mapping = ToLabelMapping(labelMapping);
      const auto ms = ToMergeStyle(mergeStyle);
      const auto os = ToOverwriteStyle(overwriteStyle);

      if (py::isinstance<MultiLabelSegmentation>(source))
      {
        TransferLabelContentAtTimeStep(
          source.cast<const MultiLabelSegmentation*>(),
          destination.cast<MultiLabelSegmentation*>(),
          timeStep, mapping, ms, os);
        return;
      }

      ConstLabelVector dstLabelVec;
      if (!destinationLabels.is_none())
      {
        for (auto item : destinationLabels.cast<py::iterable>())
          dstLabelVec.push_back(Label::ConstPointer(item.cast<Label*>()));
      }
      TransferLabelContentAtTimeStep(
        source.cast<const Image*>(),
        destination.cast<Image*>(),
        dstLabelVec, timeStep, srcBg, dstBg, dstBgLocked,
        mapping, ms, os);
    },
    py::arg("source"), py::arg("destination"),
    py::arg("time_step"),
    py::arg("label_mapping"),
    py::arg("merge_style") = MultiLabelSegmentation::MergeStyle::Replace,
    py::arg("overwrite_style") = MultiLabelSegmentation::OverwriteStyle::RegardLocks,
    py::arg("destination_labels") = py::none(),
    py::arg("source_background") = MultiLabelSegmentation::UNLABELED_VALUE,
    py::arg("destination_background") = MultiLabelSegmentation::UNLABELED_VALUE,
    py::arg("destination_background_locked") = false,
    R"(Single-time-step variant of :py:func:`transfer_labels`.

Identical to :py:func:`transfer_labels` but restricted to the given
*time_step*. See its documentation for argument semantics.

Args:
    source: Source segmentation or image.
    destination: Target segmentation or image.
    time_step: Time step to transfer.
    label_mapping: Iterable of ``(source_value, destination_value)``
        pairs.
    merge_style: :py:class:`MergeStyle`. Defaults to ``REPLACE``.
    overwrite_style: :py:class:`OverwriteStyle`. Defaults to
        ``REGARD_LOCKS``.
    destination_labels: Image-mode only; see :py:func:`transfer_labels`.
    source_background: Image-mode only.
    destination_background: Image-mode only.
    destination_background_locked: Image-mode only.
)");
}
