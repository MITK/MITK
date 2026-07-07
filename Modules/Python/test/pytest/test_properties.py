# ============================================================================
#
# The Medical Imaging Interaction Toolkit (MITK)
#
# Copyright (c) German Cancer Research Center (DKFZ)
# All rights reserved.
#
# Use of this source code is governed by a 3-clause BSD license that can be
# found in the LICENSE file.
#
# ============================================================================

# Tests for property bindings.
#
# TODO: when MultiLabelSegmentation bindings land (WP-10), add coverage for
# properties that are provided read-only by an internal component -- reads
# succeed, writes raise mitk.PropertyNotOwnedError, property_is_owned is False.

import json

import pytest
import mitk


def _close(a, b, tol=1e-6):
    """Float (or float-sequence) comparison within tolerance.

    Used where float32 round-trip through mitk.Color loses precision
    relative to the double-precision Python input.
    """
    if isinstance(a, (list, tuple)):
        return len(a) == len(b) and all(abs(x - y) <= tol for x, y in zip(a, b))
    return abs(a - b) <= tol


class TestPropertyNotOwnedError:
    def test_is_attribute_error(self):
        assert issubclass(mitk.PropertyNotOwnedError, AttributeError)

    def test_catchable_as_attribute_error(self):
        try:
            raise mitk.PropertyNotOwnedError("test")
        except AttributeError:
            return
        assert False, "PropertyNotOwnedError should be catchable as AttributeError"


class TestPropertyOwnerOnImage:
    def test_property_is_owned_existing(self):
        img = mitk.Image()
        img.set_property("name", "test")
        assert img.property_is_owned("name") is True

    def test_property_is_owned_missing(self):
        img = mitk.Image()
        assert img.property_is_owned("nonexistent") is False

    def test_set_property_creates_new(self):
        img = mitk.Image()
        img.set_property("new_prop", "value")
        assert img.get_property("new_prop") is not None

    def test_set_property_updates_owned(self):
        img = mitk.Image()
        img.set_property("name", "original")
        img.set_property("name", "updated")
        assert img.get_property("name") == "updated"

    def test_get_property_missing_returns_none(self):
        img = mitk.Image()
        assert img.get_property("missing") is None

    def test_remove_property_existing(self):
        img = mitk.Image()
        img.set_property("k", "v")
        img.remove_property("k")
        assert img.get_property("k") is None


class TestPropertyList:
    def test_property_is_owned_existing(self):
        pl = mitk.PropertyList()
        pl.set_property("key", "value")
        assert pl.property_is_owned("key") is True

    def test_property_is_owned_missing(self):
        pl = mitk.PropertyList()
        assert pl.property_is_owned("missing") is False

    def test_property_list_owns_all(self):
        pl = mitk.PropertyList()
        pl.set_property("prop1", "value1")
        pl.set_property("prop2", "value2")

        assert pl.property_is_owned("prop1") is True
        assert pl.property_is_owned("prop2") is True

    def test_get_property_existing(self):
        pl = mitk.PropertyList()
        pl.set_property("key", "value")
        prop = pl.get_property("key")
        assert prop is not None
        assert prop == "value"

    def test_get_property_missing_returns_none(self):
        pl = mitk.PropertyList()
        assert pl.get_property("missing") is None

    def test_set_property_updates_existing(self):
        pl = mitk.PropertyList()
        pl.set_property("k", "first")
        pl.set_property("k", "second")
        assert pl.get_property("k") == "second"

    def test_remove_property_existing(self):
        pl = mitk.PropertyList()
        pl.set_property("k", "v")
        pl.remove_property("k")
        assert pl.get_property("k") is None

    def test_remove_missing_is_noop(self):
        # Underlying IPropertyOwner contract: "If the property does not exist,
        # nothing will be done." Must not raise.
        pl = mitk.PropertyList()
        pl.remove_property("missing")

    def test_property_keys_reflects_mutations(self):
        pl = mitk.PropertyList()
        pl.set_property("alpha", "a")
        pl.set_property("beta", "b")
        assert set(pl.property_keys) == {"alpha", "beta"}
        pl.remove_property("alpha")
        assert set(pl.property_keys) == {"beta"}

    def test_auto_wrap_python_types(self):
        # PropertyList.set_property must auto-wrap the same way Image does.
        pl = mitk.PropertyList()
        pl.set_property("s", "hello")
        pl.set_property("i", 42)
        pl.set_property("b", True)
        pl.set_property("d", 3.14)

        assert isinstance(pl.get_property("s", raw=True), mitk.StringProperty)
        assert isinstance(pl.get_property("i", raw=True), mitk.IntProperty)
        assert isinstance(pl.get_property("b", raw=True), mitk.BoolProperty)
        assert isinstance(pl.get_property("d", raw=True), mitk.DoubleProperty)


class TestPropertyView:
    def test_view_read_on_image(self):
        img = mitk.Image()
        img.set_property("name", "test")
        img.set_property("count", 42)

        view = img.properties

        assert "name" in view
        assert view["name"] == "test"
        assert len(view) == 2
        # GetPropertyKeys order is unspecified; compare as a set.
        assert set(view.keys()) == {"name", "count"}

    def test_view_write_on_image(self):
        img = mitk.Image()
        view = img.properties

        view["new_prop"] = "new_value"
        assert img.get_property("new_prop") == "new_value"

        view["temp"] = "temp_value"
        del view["temp"]
        assert img.get_property("temp") is None

    def test_view_read_on_property_list(self):
        pl = mitk.PropertyList()
        pl.set_property("name", "test")

        view = pl.properties

        assert "name" in view
        assert view["name"] == "test"
        assert len(view) == 1

    def test_view_write_on_property_list(self):
        pl = mitk.PropertyList()
        view = pl.properties

        view["k"] = "v"
        assert pl.get_property("k") == "v"

        del view["k"]
        assert pl.get_property("k") is None


class TestAutoWrap:
    """Python value -> property auto-wrap behavior."""

    def test_python_float_maps_to_double_property(self):
        # Python float is IEEE-754 double; must not truncate.
        img = mitk.Image()
        img.set_property("timestamp", 1.2345678901234567)
        prop = img.get_property("timestamp", raw=True)
        assert isinstance(prop, mitk.DoubleProperty)
        assert prop.value == 1.2345678901234567  # exact: double round-trip

    def test_python_int_maps_to_int_property(self):
        img = mitk.Image()
        img.set_property("n", 7)
        assert isinstance(img.get_property("n", raw=True), mitk.IntProperty)

    def test_python_bool_maps_to_bool_property(self):
        # bool is subclass of int in Python; the binding must handle that first.
        img = mitk.Image()
        img.set_property("flag", True)
        prop = img.get_property("flag", raw=True)
        assert isinstance(prop, mitk.BoolProperty)
        assert prop.value is True

    def test_color_auto_wraps_to_color_property(self):
        # mitk.Color is a bound type, so set_property accepts it directly.
        img = mitk.Image()
        img.set_property("color", mitk.Color(1.0, 0.5, 0.0))
        prop = img.get_property("color", raw=True)
        assert isinstance(prop, mitk.ColorProperty)
        assert prop.value.r == 1.0


class TestColor:
    def test_construct_and_component_access(self):
        # 0.25, 0.5, 1.0 are bit-exact in float32 -- direct equality is fine.
        c = mitk.Color(0.25, 0.5, 1.0)
        assert c.r == 0.25
        assert c.g == 0.5
        assert c.b == 1.0

    def test_index_access_matches_components(self):
        # 0.1, 0.2, 0.3 are NOT bit-exact in float32 -- use tolerance.
        c = mitk.Color(0.1, 0.2, 0.3)
        assert _close(c[0], 0.1)
        assert _close(c[1], 0.2)
        assert _close(c[2], 0.3)
        assert len(c) == 3

    def test_index_assignment(self):
        c = mitk.Color(0.0, 0.0, 0.0)
        c[0] = 1.0
        c[1] = 0.5
        c[2] = 0.25
        assert c.r == 1.0
        assert c.g == 0.5
        assert c.b == 0.25

    def test_index_out_of_range_raises(self):
        c = mitk.Color()
        try:
            _ = c[3]
        except IndexError:
            return
        assert False, "expected IndexError"

    def test_color_property_value_returns_color(self):
        cp = mitk.ColorProperty(mitk.Color(0.7, 0.8, 0.9))
        value = cp.value
        assert isinstance(value, mitk.Color)
        assert _close(value.r, 0.7)

    def test_color_property_from_rgb(self):
        cp = mitk.ColorProperty.from_rgb(1.0, 0.5, 0.0)
        assert cp.value.r == 1.0

    def test_float_property_construct_and_value(self):
        # FloatProperty stores float32; inputs are silently truncated to float
        # precision, so use a value that is bit-exact in both.
        fp = mitk.FloatProperty(0.5)
        assert fp.value == 0.5


class TestColorProperty:
    """Tests for tuple-based color property ergonomics."""

    @pytest.fixture
    def img(self):
        return mitk.Image()

    def test_set_color_tuple_typed_path(self, img):
        """set_property accepts a (r,g,b) tuple when an existing ColorProperty is present."""
        img.set_property("color", mitk.ColorProperty(mitk.Color(1.0, 0.0, 0.0)))
        img.set_property("color", (0.0, 1.0, 0.0))
        assert img.get_property("color") == (0.0, 1.0, 0.0)

    def test_set_color_tuple_untyped_path(self, img):
        """set_property creates a ColorProperty from a 3-element tuple with no prior property."""
        img.set_property("new_color", (0.5, 0.5, 0.5))
        assert img.get_property("new_color") == (0.5, 0.5, 0.5)
        assert isinstance(img.get_property("new_color", raw=True), mitk.ColorProperty)

    def test_set_color_mitk_color_still_works(self, img):
        """set_property still accepts a mitk.Color object (backwards compatible)."""
        img.set_property("color", mitk.Color(1.0, 0.5, 0.0))
        assert img.get_property("color") == pytest.approx((1.0, 0.5, 0.0))

    def test_get_color_returns_tuple(self, img):
        """get_property returns an (r,g,b) tuple for a ColorProperty by default."""
        img.set_property("color", mitk.ColorProperty(mitk.Color(0.2, 0.4, 0.6)))
        result = img.get_property("color")
        assert isinstance(result, tuple)
        assert len(result) == 3
        assert result == pytest.approx((0.2, 0.4, 0.6))

    def test_get_color_raw_returns_property_object(self, img):
        """get_property(raw=True) returns the mitk.ColorProperty object."""
        img.set_property("color", mitk.ColorProperty(mitk.Color(1.0, 0.0, 0.0)))
        prop = img.get_property("color", raw=True)
        assert isinstance(prop, mitk.ColorProperty)

    def test_color_roundtrip(self, img):
        """Setting and getting a color tuple is a lossless round-trip."""
        original = (0.1, 0.2, 0.9)
        img.set_property("color", original)
        assert img.get_property("color") == pytest.approx(original)

    def test_mitk_color_iter(self):
        """mitk.Color supports iteration: tuple(color) and unpacking work."""
        c = mitk.Color(0.3, 0.6, 0.9)
        assert tuple(c) == pytest.approx((0.3, 0.6, 0.9))
        r, g, b = c
        assert (r, g, b) == pytest.approx((0.3, 0.6, 0.9))


class TestSerialization:
    """Round-trip through MITK's self-contained JSON format."""

    def test_simple_property_is_bare_primitive(self):
        # StringProperty is a "simple" type in the self-contained format; it
        # must serialize as a bare JSON primitive, not {"type": ..., "value": ...}.
        prop = mitk.StringProperty("hello")
        assert json.loads(prop.to_json()) == "hello"

    def test_int_property_is_bare_primitive(self):
        prop = mitk.IntProperty(42)
        assert json.loads(prop.to_json()) == 42

    def test_bool_property_is_bare_primitive(self):
        prop = mitk.BoolProperty(True)
        assert json.loads(prop.to_json()) is True

    def test_color_property_is_wrapped(self):
        # Complex types carry their class name; ColorProperty's value is an
        # [r, g, b] array (matching what mitk_workbench_remote expects).
        prop = mitk.ColorProperty(mitk.Color(1.0, 0.5, 0.0))
        payload = json.loads(prop.to_json())
        assert payload["type"] == "ColorProperty"
        assert _close(payload["value"], [1.0, 0.5, 0.0])

    def test_property_round_trip(self):
        original = mitk.StringProperty("round-trip")
        restored = mitk.BaseProperty.from_json(original.to_json())
        assert restored.value == "round-trip"

    def test_property_list_round_trip_mixed_types(self):
        pl = mitk.PropertyList()
        pl.set_property("name", "segmentation")
        pl.set_property("count", 3)
        pl.set_property("enabled", True)
        pl.set_property("ratio", 0.25)
        pl.set_property("color", mitk.ColorProperty(mitk.Color(1.0, 0.0, 0.0)))

        restored = mitk.PropertyList.from_json(pl.to_json())

        assert restored.get_property("name") == "segmentation"
        assert restored.get_property("count") == 3
        assert restored.get_property("enabled") is True
        assert restored.get_property("ratio") == 0.25
        assert restored.get_property("color")[0] == 1.0

    def test_unbound_property_type_round_trips_via_property_list(self):
        # Vector3DProperty has no dedicated Python binding, but the wire
        # format routes through IPropertyDeserialization on the C++ side,
        # so a self-contained JSON payload naming it round-trips regardless.
        payload = json.dumps(
            {
                "vec": {
                    "type": "Vector3DProperty",
                    "value": [1.5, 2.5, 3.5],
                }
            }
        )
        restored = mitk.PropertyList.from_json(payload)
        assert restored.get_property("vec") is not None

        # Serializing back must preserve the type tag and values. This is the
        # actual proof that IPropertyDeserialization round-tripped the unbound
        # type correctly -- the returned Python object is just a BaseProperty
        # handle since Vector3DProperty has no dedicated binding.
        round_tripped = json.loads(restored.to_json())
        assert round_tripped["vec"]["type"] == "Vector3DProperty"
        assert _close(round_tripped["vec"]["value"], [1.5, 2.5, 3.5])



class TestNonOwnedProperties:
    """Tests for properties that are provided read-only (not owned).

    These tests are stubs that will be fully implemented when
    MultiLabelSegmentation bindings are available (WP-10).
    """

    def test_get_non_owned_property(self):
        """Reading non-owned properties should work."""
        # This will be implemented when MultiLabelSegmentation is available
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_set_non_owned_property_raises(self):
        """Setting non-owned property should raise PropertyNotOwnedError."""
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_remove_non_owned_property_raises(self):
        """Removing non-owned property should raise PropertyNotOwnedError."""
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_view_setitem_non_owned_raises(self):
        """View setitem on non-owned property should raise PropertyNotOwnedError."""
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_view_delitem_non_owned_raises(self):
        """View delitem on non-owned property should raise PropertyNotOwnedError."""
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_property_is_owned_non_owned(self):
        """property_is_owned should return False for non-owned properties."""
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")