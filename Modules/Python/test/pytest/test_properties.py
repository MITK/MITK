import json

import pytest

import mitk


class TestPropertyNotOwnedError:
    def test_is_attribute_error(self):
        assert issubclass(mitk.PropertyNotOwnedError, AttributeError)

    def test_catchable_as_attribute_error(self):
        with pytest.raises(AttributeError):
            raise mitk.PropertyNotOwnedError("test")


class TestPropertyOwnerOnImage:
    @pytest.fixture
    def img(self):
        return mitk.Image()

    def test_property_is_owned_existing(self, img):
        img.set_property("name", "test")
        assert img.property_is_owned("name") is True

    def test_property_is_owned_missing(self, img):
        assert img.property_is_owned("nonexistent") is False

    def test_set_property_creates_new(self, img):
        img.set_property("new_prop", "value")
        assert img.get_property("new_prop") is not None

    def test_set_property_updates_owned(self, img):
        img.set_property("name", "original")
        img.set_property("name", "updated")
        assert img.get_property("name").value == "updated"


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


class TestPropertyView:
    def test_view_read_operations(self):
        img = mitk.Image()
        img.set_property("name", "test")
        img.set_property("count", 42)

        view = img.properties

        assert "name" in view
        assert view["name"].value == "test"
        assert len(view) == 2
        # GetPropertyKeys order is unspecified; compare as a set.
        assert set(view.keys()) == {"name", "count"}

    def test_view_write_operations(self):
        img = mitk.Image()
        view = img.properties

        view["new_prop"] = "new_value"
        assert img.get_property("new_prop").value == "new_value"

        view["temp"] = "temp_value"
        del view["temp"]
        assert img.get_property("temp") is None


class TestAutoWrap:
    """Python value -> property auto-wrap behavior."""

    def test_python_float_maps_to_double_property(self):
        # Python float is IEEE-754 double; must not truncate.
        img = mitk.Image()
        img.set_property("timestamp", 1.2345678901234567)
        prop = img.get_property("timestamp")
        assert isinstance(prop, mitk.DoubleProperty)
        assert prop.value == pytest.approx(1.2345678901234567, rel=0, abs=0)

    def test_python_int_maps_to_int_property(self):
        img = mitk.Image()
        img.set_property("n", 7)
        assert isinstance(img.get_property("n"), mitk.IntProperty)

    def test_python_bool_maps_to_bool_property(self):
        # bool is subclass of int in Python; the binding must handle that first.
        img = mitk.Image()
        img.set_property("flag", True)
        prop = img.get_property("flag")
        assert isinstance(prop, mitk.BoolProperty)
        assert prop.value is True

    def test_plain_tuple_does_not_auto_wrap_to_color(self):
        # 3-tuple is ambiguous (Color vs Point3D vs Vector3D); callers must be explicit.
        img = mitk.Image()
        with pytest.raises(TypeError):
            img.set_property("color", (1.0, 0.5, 0.0))


class TestColor:
    def test_construct_and_component_access(self):
        c = mitk.Color(0.25, 0.5, 1.0)
        assert c.r == pytest.approx(0.25)
        assert c.g == pytest.approx(0.5)
        assert c.b == pytest.approx(1.0)

    def test_index_access_matches_components(self):
        c = mitk.Color(0.1, 0.2, 0.3)
        assert c[0] == pytest.approx(0.1)
        assert c[1] == pytest.approx(0.2)
        assert c[2] == pytest.approx(0.3)
        assert len(c) == 3

    def test_index_out_of_range_raises(self):
        c = mitk.Color()
        with pytest.raises(IndexError):
            _ = c[3]

    def test_color_property_value_returns_color(self):
        cp = mitk.ColorProperty(mitk.Color(0.7, 0.8, 0.9))
        value = cp.value
        assert isinstance(value, mitk.Color)
        assert value.r == pytest.approx(0.7)

    def test_color_property_from_rgb(self):
        cp = mitk.ColorProperty.from_rgb(1.0, 0.5, 0.0)
        assert cp.value.r == pytest.approx(1.0)


class TestSerialization:
    """Round-trip through MITK's self-contained JSON format."""

    def test_simple_property_is_bare_primitive(self):
        # StringProperty is a "simple" type in the self-contained format; it
        # must serialize as a bare JSON primitive, not {"type": ..., "value": ...}.
        prop = mitk.StringProperty("hello")
        payload = json.loads(prop.to_json())
        assert payload == "hello"

    def test_int_property_is_bare_primitive(self):
        prop = mitk.IntProperty(42)
        assert json.loads(prop.to_json()) == 42

    def test_bool_property_is_bare_primitive(self):
        prop = mitk.BoolProperty(True)
        assert json.loads(prop.to_json()) is True

    def test_color_property_is_wrapped(self):
        # Complex types carry their class name; ColorProperty's value is a
        # [r, g, b] array (matching what mitk_workbench_remote expects).
        prop = mitk.ColorProperty(mitk.Color(1.0, 0.5, 0.0))
        payload = json.loads(prop.to_json())
        assert payload["type"] == "ColorProperty"
        assert payload["value"] == pytest.approx([1.0, 0.5, 0.0])

    def test_property_round_trip(self):
        original = mitk.StringProperty("round-trip")
        restored = mitk.property_from_json(original.to_json())
        assert restored.value == "round-trip"

    def test_property_list_round_trip_mixed_types(self):
        pl = mitk.PropertyList()
        pl.set_property("name", "segmentation")
        pl.set_property("count", 3)
        pl.set_property("enabled", True)
        pl.set_property("ratio", 0.25)
        pl.set_property("color", mitk.ColorProperty(mitk.Color(1.0, 0.0, 0.0)))

        restored = mitk.PropertyList.from_json(pl.to_json())

        assert restored.get_property("name").value == "segmentation"
        assert restored.get_property("count").value == 3
        assert restored.get_property("enabled").value is True
        assert restored.get_property("ratio").value == pytest.approx(0.25)
        assert restored.get_property("color").value.r == pytest.approx(1.0)

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
        assert round_tripped["vec"]["value"] == pytest.approx([1.5, 2.5, 3.5])


class TestNonOwnedProperties:
    """Stubs for properties provided read-only by internal components.

    Implemented once MultiLabelSegmentation bindings land (WP-10).
    """

    def test_get_non_owned_property(self):
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_set_non_owned_property_raises(self):
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_remove_non_owned_property_raises(self):
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_view_setitem_non_owned_raises(self):
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_view_delitem_non_owned_raises(self):
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")

    def test_property_is_owned_non_owned(self):
        pytest.skip("Requires MultiLabelSegmentation (WP-10)")
