# test_property_paths.py: Tests for WP-16 Property Path Helpers
# Tests for TemporoSpatialStringProperty, PropertyKeyPath, and DICOMTagPath

import pytest
import mitk

class TestTemporoSpatialStringProperty:
    """Tests for TemporoSpatialStringProperty class."""
    
    def test_construction_and_uniform_value(self):
        """Test basic construction and uniform value access."""
        prop = mitk.TemporoSpatialStringProperty("test value")
        assert prop.value == "test value"
        assert prop.is_uniform is True
        assert prop.has_value() is True
    
    def test_time_slice_specific_values(self):
        """Test setting and getting values at specific time steps and slices."""
        prop = mitk.TemporoSpatialStringProperty("initial")
        
        # Set specific values
        prop.set_value("value at 0,0", time_step=0, z_slice=0)
        prop.set_value("value at 1,0", time_step=1, z_slice=0)
        prop.set_value("value at 0,1", time_step=0, z_slice=1)
        
        # Check values
        assert prop.get_value(time_step=0, z_slice=0) == "value at 0,0"
        assert prop.get_value(time_step=1, z_slice=0) == "value at 1,0"
        assert prop.get_value(time_step=0, z_slice=1) == "value at 0,1"
        assert prop.is_uniform is False
    
    def test_set_value_omitted_dimensions(self):
        """Test setting values with omitted dimensions (sets all in that dimension)."""
        prop = mitk.TemporoSpatialStringProperty("initial")
        
        # Set values for all slices in time step 0
        prop.set_value("all slices in ts0", time_step=0)
        
        # Set values for all time steps in slice 1
        prop.set_value("all time steps in slice1", z_slice=1)
        
        # Verify the values were set correctly
        assert prop.get_value(time_step=0, z_slice=0) == "all slices in ts0"
        assert prop.get_value(time_step=0, z_slice=1) == "all time steps in slice1"
    
    def test_set_value_creates_missing_indices(self):
        """Test that set_value creates missing time steps or slices."""
        prop = mitk.TemporoSpatialStringProperty("initial")
        
        # Set value at a non-existent time step
        prop.set_value("new time step", time_step=5)
        assert prop.get_value(time_step=5, z_slice=0) == "new time step"
        
        # Set value at a non-existent slice
        prop.set_value("new slice", z_slice=10)
        assert prop.get_value(time_step=0, z_slice=10) == "new slice"
    
    def test_available_indices(self):
        """Test querying available time steps and slices."""
        prop = mitk.TemporoSpatialStringProperty("initial")
        
        prop.set_value("ts0_s0", time_step=0, z_slice=0)
        prop.set_value("ts0_s1", time_step=0, z_slice=1)
        prop.set_value("ts1_s0", time_step=1, z_slice=0)
        
        time_steps = prop.get_available_time_steps()
        assert set(time_steps) == {0, 1}
        
        slices = prop.get_available_slices()
        assert set(slices) == {0, 1}
        
        # Time steps for specific slice
        ts_for_slice0 = prop.get_available_time_steps(0)
        assert set(ts_for_slice0) == {0, 1}
        
        # Slices for specific time step
        slices_for_ts0 = prop.get_available_slices(0)
        assert set(slices_for_ts0) == {0, 1}
    
    def test_serialization(self):
        """Test to_dict and property_from_dict round-trip."""
        prop = mitk.TemporoSpatialStringProperty("initial")
        prop.set_value("ts0_s0", time_step=0, z_slice=0)
        prop.set_value("ts1_s1", time_step=1, z_slice=1)
        
        # Serialize to dict
        prop_dict = prop.to_dict()
        assert prop_dict["type"] == "TemporoSpatialStringProperty"
        assert "entries" in prop_dict
        
        # Deserialize back
        prop_back = mitk.property_from_dict(prop_dict)
        assert isinstance(prop_back, mitk.TemporoSpatialStringProperty)
        assert prop_back.get_value(time_step=0, z_slice=0) == "ts0_s0"
        assert prop_back.get_value(time_step=1, z_slice=1) == "ts1_s1"
    
    def test_property_list_serialization(self):
        """Test PropertyList serialization with TemporoSpatialStringProperty."""
        pl = mitk.PropertyList()
        
        ts_prop = mitk.TemporoSpatialStringProperty("test")
        ts_prop.set_value("value1", time_step=0, z_slice=0)
        ts_prop.set_value("value2", time_step=1, z_slice=1)
        
        pl.set_property("ts_property", ts_prop)
        pl.set_property("regular_prop", "regular value")
        
        # Serialize to dict
        pl_dict = pl.to_dict()
        assert "ts_property" in pl_dict
        assert "regular_prop" in pl_dict
        
        # Deserialize back
        pl_back = mitk.PropertyList.from_dict(pl_dict)
        retrieved_prop = pl_back.get_property("ts_property")
        assert isinstance(retrieved_prop, mitk.TemporoSpatialStringProperty)
        assert retrieved_prop.get_value(time_step=0, z_slice=0) == "value1"
        assert retrieved_prop.get_value(time_step=1, z_slice=1) == "value2"


class TestPropertyKeyPath:
    """Tests for PropertyKeyPath class."""
    
    def test_construction(self):
        """Test various construction methods."""
        # Empty path
        empty_path = mitk.PropertyKeyPath()
        assert empty_path.is_empty is True
        assert len(empty_path) == 0
        
        # From string
        path = mitk.PropertyKeyPath.from_string("parent.child.grandchild")
        assert str(path) == "parent.child.grandchild"
        assert len(path) == 3
        assert path.is_empty is False
        
        # From single element
        single = mitk.PropertyKeyPath("single")
        assert str(single) == "single"
        assert len(single) == 1
    
    def test_path_operations(self):
        """Test path concatenation and operations."""
        base = mitk.PropertyKeyPath("parent")
        
        # Append with /
        child = base / "child"
        assert str(child) == "parent.child"
        assert len(child) == 2
        
        # Append another path
        grandchild = child / mitk.PropertyKeyPath("grandchild")
        assert str(grandchild) == "parent.child.grandchild"
        
        # Wildcard selection
        wildcard_path = mitk.PropertyKeyPath("sequence") / "item" / "[*]"
        assert str(wildcard_path) == "sequence.item.[*]"
        
        # Indexed selection
        indexed_path = mitk.PropertyKeyPath("sequence") / "item" / "[5]"
        assert str(indexed_path) == "sequence.item.[5]"

    def test_equality(self):
        """Test path equality."""
        path1 = mitk.PropertyKeyPath.from_string("a.b.c")
        path2 = mitk.PropertyKeyPath("a") / "b" / "c"
        path3 = mitk.PropertyKeyPath.from_string("a.b.d")
        
        assert path1 == path2
        assert path1 != path3
        assert not (path1 == path3)
    
    def test_copy_protocol(self):
        """Test copy and deepcopy."""
        import copy
        
        original = mitk.PropertyKeyPath("a") / "b"
        copied = copy.copy(original)
        deepcopied = copy.deepcopy(original)
        
        assert copied == original
        assert deepcopied == original
        assert copied is not original
        assert deepcopied is not original
    
    def test_find_indexed_properties(self):
        """Test find_indexed_properties function."""
        # Test with PropertyList
        pl = mitk.PropertyList()
        pl.set_property("seq.[0].name", "first")
        pl.set_property("seq.[1].name", "second")
        pl.set_property("seq.[2].name", "third")
        pl.set_property("other.prop", "value")
        
        path = mitk.PropertyKeyPath("seq") / "[*]"/ "name"
        result = mitk.find_indexed_properties(pl, path)
        
        assert result == {0: "seq.[0].name", 1: "seq.[1].name", 2: "seq.[2].name"}
        
        # Test with Image
        img = mitk.Image()
        img.set_property("annotation.[0].text", "slice 0")
        img.set_property("annotation.[1].text", "slice 1")
        
        path2 = mitk.PropertyKeyPath("annotation") / "[*]"/ "text"
        result2 = mitk.find_indexed_properties(img, path2)
        
        assert result2 == {0: "annotation.[0].text", 1: "annotation.[1].text"}
    
    def test_invalid_wildcard_count(self):
        """Test that find_indexed_properties validates wildcard count."""
        pl = mitk.PropertyList()
        
        # No wildcards
        path_no_wildcard = mitk.PropertyKeyPath("seq") / "name"
        with pytest.raises(ValueError, match=r"exactly one \[\*\] wildcard"):
            mitk.find_indexed_properties(pl, path_no_wildcard)
        
        # Multiple wildcards
        path_multi_wildcard = mitk.PropertyKeyPath("seq") / "[*]" / "child" / "[*]"
        with pytest.raises(ValueError, match=r"exactly one \[\*\] wildcard"):
            mitk.find_indexed_properties(pl, path_multi_wildcard)


class TestDICOMTagPath:
    """Tests for DICOMTagPath class."""
    
    def test_construction(self):
        """Test various construction methods."""
        # Empty path
        empty = mitk.DICOMTagPath()
        assert empty.is_empty is True
        assert len(empty) == 0
        
        # From group/element
        tag_path = mitk.DICOMTagPath(0x0010, 0x0010)  # Patient Name
        assert str(tag_path) == "(0010,0010)"
        assert len(tag_path) == 1
        
        # From DICOMTag object
        tag = mitk.DICOMTag(0x0010, 0x0010)
        tag_path2 = mitk.DICOMTagPath(tag)
        assert tag_path == tag_path2
    
    def test_parsing(self):
        """Test parsing methods."""
        # From string format
        path1 = mitk.DICOMTagPath.from_string("(0010,0010)")
        assert str(path1) == "(0010,0010)"
        
        # From property name format
        path2 = mitk.DICOMTagPath.from_property_name("DICOM.0010.0010")
        assert str(path2) == "(0010,0010)"
        
        # They should be equal
        assert path1 == path2
    
    def test_path_operations(self):
        """Test path concatenation."""
        base = mitk.DICOMTagPath(0x0008, 0x1115)  # Referenced Series Sequence
        child = mitk.DICOMTagPath(0x0020, 0x000E)  # Series Instance UID
        
        combined = base / child
        assert len(combined) == 2
        
        # String representation should show the hierarchy
        assert "(0008,1115)" in str(combined)
        assert "(0020,000E)" in str(combined)
    
    def test_property_name_conversion(self):
        """Test conversion to property name format."""
        path = mitk.DICOMTagPath(0x0010, 0x0010)
        prop_name = path.to_property_name()
        assert prop_name == "DICOM.0010.0010"
    
    def test_readable_name(self):
        """Test human-readable name conversion."""
        path = mitk.DICOMTagPath(0x0010, 0x0010)  # Patient Name
        
        readable = path.to_readable_name()
        assert "Patient" in readable or "Name" in readable  # Exact text depends on DCMTK dictionaries
        
        readable_with_tags = path.to_readable_name(include_tags=True)
        assert "(0010,0010)" in readable_with_tags
    
    def test_equality(self):
        """Test path equality."""
        path1 = mitk.DICOMTagPath(0x0010, 0x0010)
        path2 = mitk.DICOMTagPath.from_string("(0010,0010)")
        path3 = mitk.DICOMTagPath(0x0010, 0x0020)
        
        assert path1 == path2
        assert path1 != path3
        assert not (path1 == path3)
    
    def test_copy_protocol(self):
        """Test copy and deepcopy."""
        import copy
        
        original = mitk.DICOMTagPath(0x0010, 0x0010)
        copied = copy.copy(original)
        deepcopied = copy.deepcopy(original)
        
        assert copied == original
        assert deepcopied == original
        assert copied is not original
        assert deepcopied is not original
    

class TestIntegration:
    """Integration tests combining multiple features."""
    
    def test_ts_property_with_image(self):
        """Test TemporoSpatialStringProperty with Image."""
        img = mitk.Image()
        
        # Create and set TS property
        ts_prop = mitk.TemporoSpatialStringProperty("initial")
        ts_prop.set_value("slice 0 annotation", time_step=0, z_slice=0)
        ts_prop.set_value("slice 1 annotation", time_step=0, z_slice=1)
        
        img.set_property("dicom.annotations", ts_prop)
        
        # Retrieve and verify
        retrieved = img.get_property("dicom.annotations")
        assert isinstance(retrieved, mitk.TemporoSpatialStringProperty)
        assert retrieved.get_value(time_step=0, z_slice=0) == "slice 0 annotation"
        assert retrieved.get_value(time_step=0, z_slice=1) == "slice 1 annotation"
    
    def test_property_path_with_ts_property(self):
        """Test PropertyKeyPath with TemporoSpatialStringProperty."""
        pl = mitk.PropertyList()
        
        # Create TS properties with indexed keys
        for i in range(3):
            ts_prop = mitk.TemporoSpatialStringProperty(f"value {i}")
            ts_prop.set_value(f"detailed {i}", time_step=0, z_slice=i)
            pl.set_property(f"annotations.[{i}].text", ts_prop)
        
        # Find indexed properties
        path = mitk.PropertyKeyPath("annotations") / "[*]"/ "text"
        result = mitk.find_indexed_properties(pl, path)
        
        assert len(result) == 3
        for i in range(3):
            assert f"annotations.[{i}].text" in result.values()
            
            # Verify the property is still a TS property
            prop = pl.get_property(result[i])
            assert isinstance(prop, mitk.TemporoSpatialStringProperty)
            assert prop.get_value(time_step=0, z_slice=i) == f"detailed {i}"
    
    def test_serialization_round_trip_complex(self):
        """Test complex serialization round-trip with mixed property types."""
        pl = mitk.PropertyList()
        
        # Add various property types
        pl.set_property("string_prop", "string value")
        pl.set_property("int_prop", 42)
        pl.set_property("bool_prop", True)
        
        # Add TS property
        ts_prop = mitk.TemporoSpatialStringProperty("ts initial")
        ts_prop.set_value("ts value 0,0", time_step=0, z_slice=0)
        ts_prop.set_value("ts value 1,1", time_step=1, z_slice=1)
        pl.set_property("ts_prop", ts_prop)
        
        # Serialize and deserialize
        pl_dict = pl.to_dict()
        pl_back = mitk.PropertyList.from_dict(pl_dict)
        
        # Verify all properties
        assert pl_back.get_property("string_prop") == "string value"
        assert pl_back.get_property("int_prop") == 42
        assert pl_back.get_property("bool_prop") is True
        assert pl_back.get_property("string_prop", raw=True).value == "string value"
        assert pl_back.get_property("int_prop", raw=True).value == 42
        assert pl_back.get_property("bool_prop", raw=True).value is True     

        ts_back = pl_back.get_property("ts_prop")
        assert isinstance(ts_back, mitk.TemporoSpatialStringProperty)
        assert ts_back.get_value(time_step=0, z_slice=0) == "ts value 0,0"
        assert ts_back.get_value(time_step=1, z_slice=1) == "ts value 1,1"


if __name__ == "__main__":
    pytest.main([__file__, "-v"])