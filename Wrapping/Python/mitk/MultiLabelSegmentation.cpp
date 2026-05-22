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
  py::class_<DICOMCodeSequence>(m, "DICOMCodeSequence")
    .def(py::init<>())
    .def(py::init<const std::string&, const std::string&, const std::string&>(),
      py::arg("value"), py::arg("scheme"), py::arg("meaning"))
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
  py::class_<DICOMCodeSequenceWithModifiers, DICOMCodeSequence>(m, "DICOMCodeSequenceWithModifiers")
    .def(py::init<>())
    .def(py::init<const std::string&, const std::string&, const std::string&>(),
      py::arg("value"), py::arg("scheme"), py::arg("meaning"))
    .def(py::init<const DICOMCodeSequence&>(), py::arg("code"))
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
  py::enum_<Label::AlgorithmType>(m, "AlgorithmType")
    .value("UNDEFINED", Label::AlgorithmType::Undefined)
    .value("MANUAL", Label::AlgorithmType::MANUAL)
    .value("SEMIAUTOMATIC", Label::AlgorithmType::SEMIAUTOMATIC)
    .value("AUTOMATIC", Label::AlgorithmType::AUTOMATIC);

  // =======================================================================
  // mitk.Label
  // =======================================================================
  auto label_class = py::class_<Label, Label::Pointer>(m, "Label");
  label_class
    .def(py::init([]() { return Label::New(); }))
    .def(py::init([](Label::PixelType value, const std::string& name) {
        return Label::New(value, name);
      }),
      py::arg("value"), py::arg("name"))
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
  py::class_<PyLabelVector>(m, "LabelVector")
    .def("__len__", [](const PyLabelVector& v) { return v.items.size(); })
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
  py::class_<LabelGroup>(m, "LabelGroup")
    .def_readonly("index", &LabelGroup::index)
    .def_readonly("name", &LabelGroup::name)
    .def_readonly("labels", &LabelGroup::labels)
    .def_readonly("class_names", &LabelGroup::class_names)
    .def_readonly("image", &LabelGroup::image)
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
  py::enum_<MultiLabelSegmentation::MergeStyle>(m, "MergeStyle")
    .value("REPLACE", MultiLabelSegmentation::MergeStyle::Replace)
    .value("MERGE", MultiLabelSegmentation::MergeStyle::Merge);

  py::enum_<MultiLabelSegmentation::OverwriteStyle>(m, "OverwriteStyle")
    .value("REGARD_LOCKS", MultiLabelSegmentation::OverwriteStyle::RegardLocks)
    .value("IGNORE_LOCKS", MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

  // =======================================================================
  // mitk.MultiLabelSegmentation
  // =======================================================================
  auto seg_class = py::class_<MultiLabelSegmentation, MultiLabelSegmentation::Pointer>(m, "MultiLabelSegmentation");
  seg_class

    // ----------------------------------------------------------------
    // Construction
    // ----------------------------------------------------------------
    .def(py::init([] { return MultiLabelSegmentation::New(); }))
    .def(py::init([](const Image* ref) {
        auto s = MultiLabelSegmentation::New();
        s->Initialize(ref, true, true);
        // Shared with CreateNewSegmentationNode so the constructor and the
        // C++ factory cannot drift on what a "typical derived seg" means.
        LabelSetImageHelper::SetupDerivedSegmentation(s, ref);
        return s;
      }),
      py::arg("reference_image"),
      "Construct a segmentation from a reference image.\n"
      "\n"
      "The segmentation inherits its geometry from ``reference_image`` and\n"
      "additionally:\n"
      "\n"
      "* establishes a source-image relation via\n"
      "  :py:func:`mitk.relations.segmentation.connect_source_image`, and\n"
      "* transfers the reference image's patient identity, study identity,\n"
      "  and frame-of-reference UID via the corresponding\n"
      "  :py:mod:`mitk.dicom.segmentation` ``inherit_*`` functions.\n"
      "\n"
      "These steps are the common-case expectation when creating a derived\n"
      "seg. Failures are logged via MITK_WARN and swallowed; the\n"
      "constructor always returns a usable seg. Callers that want explicit\n"
      "control should use the underlying :py:mod:`mitk.relations.segmentation`\n"
      "/ :py:mod:`mitk.dicom.segmentation` functions directly.\n"
      "\n"
      ":param reference_image: The image the segmentation derives from.\n")
    .def(py::init([](const TimeGeometry* g) {
        auto s = MultiLabelSegmentation::New();
        s->Initialize(g, true, true);
        return s;
      }),
      py::arg("time_geometry"))
    .def(py::init([](const BaseGeometry* g) {
        auto tg = ProportionalTimeGeometry::New();
        tg->Initialize(g, 1);
        auto s = MultiLabelSegmentation::New();
        s->Initialize(tg, true, true);
        return s;
      }),
      py::arg("base_geometry"))
    .def(py::init([](const std::filesystem::path& p) {
        return LoadSegmentationOrInitializeFromImage(p.string());
      }),
      py::arg("path"))

    // ----------------------------------------------------------------
    // Class methods
    // ----------------------------------------------------------------
    .def_static("from_labeled_image",
      [](const Image* image) {
        auto s = MultiLabelSegmentation::New();
        s->InitializeByLabeledImage(image);
        return s;
      },
      py::arg("image"))
    .def_static("load",
      [](const std::string& path) {
        return LoadSegmentationOrInitializeFromImage(path);
      },
      py::arg("path"))

    // ----------------------------------------------------------------
    // Save / re-initialize
    // ----------------------------------------------------------------
    .def("save",
      [](const MultiLabelSegmentation& seg, const std::string& path) {
        IOUtil::Save(&seg, path);
      },
      py::arg("path"))
    .def("initialize",
      [](MultiLabelSegmentation& seg, const Image* ref, bool resetLabels) {
        seg.Initialize(ref, resetLabels, true);
      },
      py::arg("reference_image"), py::arg("reset_labels") = true)
    .def("initialize",
      [](MultiLabelSegmentation& seg, const TimeGeometry* g, bool resetLabels) {
        seg.Initialize(g, resetLabels, true);
      },
      py::arg("time_geometry"), py::arg("reset_labels") = true)

    // ----------------------------------------------------------------
    // Label queries
    // ----------------------------------------------------------------
    .def_property_readonly("labels",
      [](MultiLabelSegmentation& seg) {
        return MakeLabelVector(seg.GetLabels());
      })
    .def_property_readonly("label_values",
      [](const MultiLabelSegmentation& seg) {
        auto vals = seg.GetAllLabelValues();
        std::sort(vals.begin(), vals.end());
        return vals;
      })
    .def("get_label",
      [](MultiLabelSegmentation& seg, Label::PixelType value) -> Label::Pointer {
        auto lbl = seg.GetLabel(value);
        if (lbl.IsNull())
          throw py::value_error("Label value " + std::to_string(value) + " not found");
        return lbl;
      },
      py::arg("value"))
    .def("get_labels",
      [](MultiLabelSegmentation& seg, py::iterable values) {
        return MakeLabelVector(seg.GetLabelsByValue(ToLabelValues(values)));
      },
      py::arg("values"))
    .def("get_label_values_by_name",
      [](const MultiLabelSegmentation& seg, const std::string& name, std::optional<MultiLabelSegmentation::GroupIndexType> group) {
        if (group.has_value())
          return seg.GetLabelValuesByName(*group, name);
        return seg.GetLabelValuesByName(name);
      },
      py::arg("name"), py::arg("group") = py::none())
    .def("get_label_values_at",
      [](const MultiLabelSegmentation& seg, const std::array<double, 3>& coords,
         TimeStepType timeStep, std::optional<MultiLabelSegmentation::GroupIndexType> group) {
        Point3D pt;
        pt[0] = coords[0];
        pt[1] = coords[1];
        pt[2] = coords[2];
        return seg.GetLabelValuesByCoordinates(pt, timeStep, group);
      },
      py::arg("coordinates"), py::arg("time_step") = 0, py::arg("group") = py::none())
    .def_property_readonly("label_class_names",
      &MultiLabelSegmentation::GetLabelClassNames)

    // ----------------------------------------------------------------
    // Group queries
    // ----------------------------------------------------------------
    .def_property_readonly("num_groups",
      &MultiLabelSegmentation::GetNumberOfGroups)
    .def("get_group",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        if (!seg.ExistGroup(index))
          throw py::value_error("Group index " + std::to_string(index) + " does not exist");
        return MakeLabelGroup(seg, index);
      },
      py::arg("index"))
    .def_property_readonly("groups",
      [](MultiLabelSegmentation& seg) {
        std::vector<LabelGroup> groups;
        const auto n = seg.GetNumberOfGroups();
        groups.reserve(n);
        for (unsigned int i = 0; i < n; ++i)
          groups.push_back(MakeLabelGroup(seg, i));
        return groups;
      })
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
      py::arg("name"))
    .def("get_group_name",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        return seg.GetGroupName(index);
      },
      py::arg("index"))
    .def("get_group_labels",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        return MakeLabelVector(seg.GetLabelsByValue(seg.GetLabelValuesByGroup(index)));
      },
      py::arg("index"))
    .def("get_group_label_values",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        return seg.GetLabelValuesByGroup(index);
      },
      py::arg("index"))
    .def("get_group_class_names",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) {
        return seg.GetLabelClassNamesByGroup(index);
      },
      py::arg("index"))
    .def("get_group_of_label",
      [](const MultiLabelSegmentation& seg, Label::PixelType value) {
        return seg.GetGroupIndexOfLabel(value);
      },
      py::arg("value"))
    .def("get_group_image",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index) -> Image::Pointer {
        return seg.GetGroupImage(index);
      },
      py::arg("index"))
    .def("set_group_image",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index, const Image* image) {
        seg.UpdateGroupImage(index, image, 0, 0);
      },
      py::arg("index"), py::arg("image"))

    // ----------------------------------------------------------------
    // Mutation — Label CRUD
    // ----------------------------------------------------------------
    .def("add_label",
      [](MultiLabelSegmentation& seg, Label* label, MultiLabelSegmentation::GroupIndexType group) -> Label::Pointer {
        return seg.AddLabel(label, group, true, true);
      },
      py::arg("label"), py::arg("group") = 0)
    .def("add_label",
      [](MultiLabelSegmentation& seg, const std::string& name,
         const std::tuple<float, float, float>& color,
         MultiLabelSegmentation::GroupIndexType group) -> Label::Pointer {
        return seg.AddLabel(name, TupleToColor(color), group);
      },
      py::arg("name"), py::arg("color"), py::arg("group") = 0)
    .def("remove_label",
      [](MultiLabelSegmentation& seg, py::handle value) {
        seg.RemoveLabel(ToLabelValue(value));
      },
      py::arg("value"))
    .def("remove_labels",
      [](MultiLabelSegmentation& seg, py::iterable values) {
        seg.RemoveLabels(ToLabelValues(values));
      },
      py::arg("values"))
    .def("erase_label",
      [](MultiLabelSegmentation& seg, py::handle value) {
        seg.EraseLabel(ToLabelValue(value));
      },
      py::arg("value"))
    .def("erase_labels",
      [](MultiLabelSegmentation& seg, py::iterable values) {
        seg.EraseLabels(ToLabelValues(values));
      },
      py::arg("values"))
    .def("rename_label",
      [](MultiLabelSegmentation& seg, py::handle value,
         const std::string& name, const std::tuple<float, float, float>& color) {
        seg.RenameLabel(ToLabelValue(value), name, TupleToColor(color));
      },
      py::arg("value"), py::arg("name"), py::arg("color"))
    .def("merge_labels",
      [](MultiLabelSegmentation& seg, py::handle target, py::iterable sources,
         py::handle overwriteStyle) {
        seg.MergeLabels(ToLabelValue(target), ToLabelValues(sources), ToOverwriteStyle(overwriteStyle));
      },
      py::arg("target"), py::arg("sources"),
      py::arg("overwrite_style") = MultiLabelSegmentation::OverwriteStyle::RegardLocks)

    // ----------------------------------------------------------------
    // Mutation — Group CRUD
    // ----------------------------------------------------------------
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
      py::arg("image") = py::none(), py::arg("labels") = py::none())
    .def("remove_group",
      &MultiLabelSegmentation::RemoveGroup,
      py::arg("index"))
    .def("clear_group_image",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index,
         std::optional<TimeStepType> timeStep) {
        if (timeStep.has_value())
          seg.ClearGroupImage(index, *timeStep);
        else
          seg.ClearGroupImage(index);
      },
      py::arg("index"), py::arg("time_step") = py::none())
    .def("clear_group_images",
      [](MultiLabelSegmentation& seg, std::optional<TimeStepType> timeStep) {
        if (timeStep.has_value())
          seg.ClearGroupImages(*timeStep);
        else
          seg.ClearGroupImages();
      },
      py::arg("time_step") = py::none())
    .def("clone",
      [](const MultiLabelSegmentation& seg) -> MultiLabelSegmentation::Pointer {
        return seg.Clone();
      })
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
      "Return a segmentation whose pixel content is the result of applying "
      "label_mapping to self.\n\n"
      "Parameters\n"
      "----------\n"
      "label_mapping:\n"
      "    Iterable of (source_value, destination_value) pairs. Each entry is a\n"
      "    (int|Label, int|Label) 2-tuple.\n"
      "dest_seg:\n"
      "    Target MultiLabelSegmentation. If None (default), self is cloned and\n"
      "    used as the target (see keep_untouched_labels for how the clone is\n"
      "    prepared). All destination label values referenced in label_mapping\n"
      "    must already exist in dest_seg.\n"
      "keep_untouched_labels:\n"
      "    Only valid when dest_seg is None. Controls what happens to labels\n"
      "    that are not listed as a source in label_mapping:\n"
      "    * False (default) - the clone's group images are cleared before the\n"
      "      transfer. Only pixels of mapped labels appear in the result.\n"
      "    * True - the clone retains its original pixel content. Mapped labels\n"
      "      are remapped in-place; every other label is left untouched. Raises\n"
      "      ValueError when combined with an explicit dest_seg because the\n"
      "      caller already controls the state of that object.\n"
      "merge_style:\n"
      "    MergeStyle enum value or lowercase string alias ('replace' / 'merge').\n"
      "    Defaults to REPLACE.\n"
      "overwrite_style:\n"
      "    OverwriteStyle enum value or lowercase string alias ('regard_locks' /\n"
      "    'ignore_locks'). Defaults to IGNORE_LOCKS.\n\n"
      "Returns\n"
      "-------\n"
      "MultiLabelSegmentation\n"
      "    The populated dest_seg (or the auto-created clone when dest_seg was None).")
    .def("update_group_image",
      [](MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType index,
         const Image* source, TimeStepType timeStep, TimeStepType sourceTimeStep) {
        seg.UpdateGroupImage(index, source, timeStep, sourceTimeStep);
      },
      py::arg("index"), py::arg("source"), py::arg("time_step"), py::arg("source_time_step") = 0)
    .def("set_group_name",
      &MultiLabelSegmentation::SetGroupName,
      py::arg("index"), py::arg("name"))

    // ----------------------------------------------------------------
    // Converter helpers
    // ----------------------------------------------------------------
    .def("create_label_mask",
      [](const MultiLabelSegmentation& seg, Label::PixelType labelValue, bool binary) {
        return CreateLabelMask(&seg, labelValue, binary);
      },
      py::arg("label"), py::arg("binary") = true)
    .def("create_filtered_group_image",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType group,
         py::iterable labels) {
        return CreateFilteredGroupImage(&seg, group, ToLabelValues(labels));
      },
      py::arg("group"), py::arg("labels"))
    .def("create_label_class_map",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType group,
         std::optional<py::iterable> labels) {
        std::pair<Image::Pointer, IDToLabelClassNameMapType> result;
        if (labels.has_value())
          result = CreateLabelClassMap(&seg, group, ToLabelValues(*labels));
        else
          result = CreateLabelClassMap(&seg, group);

        // Convert map to Python dict (pybind11/stl.h handles this)
        return py::make_tuple(result.first, result.second);
      },
      py::arg("group"), py::arg("labels") = py::none())

    // ----------------------------------------------------------------
    // Split helpers (from LabelSetImageHelper)
    // ----------------------------------------------------------------
    .def("split_labels_by_group",
      [](const MultiLabelSegmentation& seg, py::iterable labels) {
        return LabelSetImageHelper::SplitLabelValuesByGroup(&seg, ToLabelValues(labels));
      },
      py::arg("labels"))
    .def("split_labels_by_class_name",
      [](const MultiLabelSegmentation& seg, MultiLabelSegmentation::GroupIndexType group,
         std::optional<py::iterable> labels) {
        if (labels.has_value())
          return LabelSetImageHelper::SplitLabelValuesByClassName(&seg, group, ToLabelValues(*labels));
        return LabelSetImageHelper::SplitLabelValuesByClassName(&seg, group);
      },
      py::arg("group"), py::arg("labels") = py::none())
    .def_readonly_static("UNLABELED_VALUE", &MultiLabelSegmentation::UNLABELED_VALUE);

  // Geometry parity via shared template
  BindGeometryAccessors<decltype(seg_class), MultiLabelSegmentation>(seg_class);

  // Bind property owner methods
  bind_property_owner(seg_class);

  // =======================================================================
  // Module-level helpers
  // =======================================================================

  // split_label_value_mapping_by_source_and_target_group
  m.def("split_label_value_mapping_by_source_and_target_group",
    [](const MultiLabelSegmentation* source, const MultiLabelSegmentation* target,
       py::iterable mapping) {
      return LabelSetImageHelper::SplitLabelValueMappingBySourceAndTargetGroup(
        source, target, ToLabelMapping(mapping));
    },
    py::arg("source"), py::arg("target"), py::arg("label_mapping"));

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
    py::arg("destination_background_locked") = false);

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
    py::arg("destination_background_locked") = false);

}
