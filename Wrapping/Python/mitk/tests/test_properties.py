# test_properties.py: Tests for WP-15 Property Interface Bindings
# Part of WP-15: Property Interface Bindings

import pytest
import mitk


class TestPropertyNotOwnedError:
    """Tests for PropertyNotOwnedError exception."""

    def test_is_attribute_error(self):
        """PropertyNotOwnedError inherits from AttributeError."""
        assert issubclass(mitk.PropertyNotOwnedError, AttributeError)

    def test_catchable_as_attribute_error(self):
        """Generic except AttributeError handlers catch it."""
        with pytest.raises(AttributeError):
            raise mitk.PropertyNotOwnedError("test")


class TestPropertyOwnerOnImage:
    """Tests for property ownership on Image class."""
    @pytest.fixture
    def img(self):
        """Create a test image."""
        return mitk.Image()
    
    def test_property_is_owned_existing(self, img):
        """Test property_is_owned for existing property."""
        img.set_property("name", "test")
        assert img.property_is_owned("name") is True

    def test_property_is_owned_missing(self, img):
        """Test property_is_owned for non-existing property."""
        assert img.property_is_owned("nonexistent") is False

    def test_set_property_creates_new(self, img):
        """Setting a new property should succeed (creates it)."""
        img.set_property("new_prop", "value")
        assert img.get_property("new_prop") is not None

    def test_set_property_updates_owned(self, img):
        """Updating an owned property should succeed."""
        img.set_property("name", "original")
        img.set_property("name", "updated")
        assert img.get_property("name") == "updated"


class TestPropertyList:
    """Tests for PropertyList class."""

    def test_property_is_owned_existing(self):
        """Test property_is_owned for existing property."""
        pl = mitk.PropertyList()
        pl.set_property("key", "value")
        assert pl.property_is_owned("key") is True

    def test_property_is_owned_missing(self):
        """Test property_is_owned for non-existing property."""
        pl = mitk.PropertyList()
        assert pl.property_is_owned("missing") is False

    def test_property_list_owns_all(self):
        """PropertyList should own all its properties."""
        pl = mitk.PropertyList()
        pl.set_property("prop1", "value1")
        pl.set_property("prop2", "value2")
        
        assert pl.property_is_owned("prop1") is True
        assert pl.property_is_owned("prop2") is True


class TestPropertyView:
    """Tests for PropertyView class."""

    def test_view_read_operations(self):
        """Test that view supports read operations."""
        img = mitk.Image()
        img.set_property("name", "test")
        img.set_property("value", 42)

        view = img.properties

        # Test reading -- values are coerced to Python-native types
        assert "name" in view
        assert view["name"] == "test"
        assert len(view) == 2
        assert list(view.keys()) == ["name", "value"]

    def test_view_write_operations(self):
        """Test that view supports write operations for owned properties."""
        img = mitk.Image()
        view = img.properties

        # Test writing
        view["new_prop"] = "new_value"
        assert img.get_property("new_prop") == "new_value"

        # Test deletion
        view["temp"] = "temp_value"
        del view["temp"]
        assert img.get_property("temp") is None


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
