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
        assert img.get_property("name").value == "updated"


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
        
        # Test reading
        assert "name" in view
        assert view["name"].value == "test"
        assert len(view) == 2
        assert list(view.keys()) == ["name", "value"]

    def test_view_write_operations(self):
        """Test that view supports write operations for owned properties."""
        img = mitk.Image()
        view = img.properties
        
        # Test writing
        view["new_prop"] = "new_value"
        assert img.get_property("new_prop").value == "new_value"
        
        # Test deletion
        view["temp"] = "temp_value"
        del view["temp"]
        assert img.get_property("temp") is None


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
