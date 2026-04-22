# test_multilabel.py: Tests for WP-10 MultiLabelSegmentation bindings
# All test data built in-memory from numpy arrays via mitk.Image.from_numpy(...)

import pytest
import numpy as np
import mitk


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture
def ref_image():
    """A small 3D reference image for segmentation initialization."""
    arr = np.zeros((4, 8, 8), dtype=np.uint16)
    return mitk.Image.from_numpy(arr, spacing=(1.0, 1.0, 1.0))


@pytest.fixture
def seg(ref_image):
    """A fresh MultiLabelSegmentation initialized from a reference image."""
    return mitk.MultiLabelSegmentation(ref_image)


# ---------------------------------------------------------------------------
# Construction
# ---------------------------------------------------------------------------

class TestConstruction:

    def test_empty(self):
        seg = mitk.MultiLabelSegmentation()
        assert seg.num_groups == 0

    def test_from_reference_image(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        assert seg.num_groups == 1
        assert seg.time_steps == 1

    def test_from_time_geometry(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image.time_geometry)
        assert seg.num_groups == 1

    def test_from_base_geometry(self, ref_image):
        geom = ref_image.get_geometry(0)
        seg = mitk.MultiLabelSegmentation(geom)
        assert seg.num_groups == 1

    def test_from_labeled_image(self):
        arr = np.array([[[0, 1], [2, 0]]], dtype=np.uint16)
        img = mitk.Image.from_numpy(arr)
        seg = mitk.MultiLabelSegmentation.from_labeled_image(img)
        assert seg.num_groups == 1
        vals = seg.label_values
        assert 1 in vals
        assert 2 in vals

    def test_load_save_roundtrip(self, seg, tmp_path):
        seg.add_label("Liver", (1.0, 0.0, 0.0), 0)
        path = str(tmp_path / "test.nrrd")
        seg.save(path)
        loaded = mitk.MultiLabelSegmentation(path)
        assert loaded.num_groups >= 1
        assert len(loaded.label_values) >= 1

    def test_load_static(self, seg, tmp_path):
        seg.add_label("Kidney", (0.0, 1.0, 0.0), 0)
        path = str(tmp_path / "test2.nrrd")
        seg.save(path)
        loaded = mitk.MultiLabelSegmentation.load(path)
        assert loaded.num_groups >= 1

    def test_load_from_plain_image_uses_initialize_by_labeled_image(self, data_dir):
        # MultilabelSegmentation_group_0.nii.gz is a plain NIfTI image (not a
        # MultiLabelSegmentation file).  load() must detect this and fall back
        # to InitializeByLabeledImage, producing a segmentation whose label
        # values match the unique non-zero pixel values in the image.
        # According to MultilabelSegmentation_group.mitklabel.json group 0
        # contains labels with values 1, 2, 3, 4 and 5.
        path = str(data_dir / "Multilabel" / "MultilabelSegmentation_group_0.nii.gz")
        seg = mitk.MultiLabelSegmentation.load(path)

        assert seg is not None
        assert seg.num_groups == 1

        label_values = set(seg.label_values)
        expected_values = {1, 2, 3, 4, 5}
        assert expected_values == label_values, (
            f"Expected label values {expected_values}, got {label_values}"
        )

    def test_constructor_from_plain_image_uses_initialize_by_labeled_image(self, data_dir):
        # Same fallback semantics as load(): a plain NIfTI image passed to the
        # constructor must be auto-initialized via InitializeByLabeledImage.
        path = str(data_dir / "Multilabel" / "MultilabelSegmentation_group_0.nii.gz")
        seg = mitk.MultiLabelSegmentation(path)

        assert seg is not None
        assert seg.num_groups == 1
        assert set(seg.label_values) == {1, 2, 3, 4, 5}


# ---------------------------------------------------------------------------
# Re-initialize
# ---------------------------------------------------------------------------

class TestInitialize:

    def test_reinitialize_resets_geometry(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        seg.add_label("A", (1.0, 0.0, 0.0), 0)

        arr2 = np.zeros((2, 4, 4), dtype=np.uint16)
        img2 = mitk.Image.from_numpy(arr2, spacing=(2.0, 2.0, 2.0))
        seg.initialize(img2, reset_labels=True)
        assert seg.spacing == (2.0, 2.0, 2.0)
        assert len(seg.label_values) == 0


# ---------------------------------------------------------------------------
# Label CRUD
# ---------------------------------------------------------------------------

class TestLabelCRUD:

    def test_add_label_by_name_color(self, seg):
        lbl = seg.add_label("Liver", (1.0, 0.0, 0.0), 0)
        assert lbl.name == "Liver"
        assert lbl.value != 0
        assert lbl.color == pytest.approx((1.0, 0.0, 0.0), abs=1e-5)

    def test_add_label_object_clone_semantics(self, seg):
        original = mitk.Label(42, "Test")
        added = seg.add_label(original, 0)
        # Mutating original after add should not affect seg's copy
        original.name = "Changed"
        retrieved = seg.get_label(added.value)
        assert retrieved.name == "Test"

    def test_remove_label(self, seg):
        lbl = seg.add_label("Remove", (0.5, 0.5, 0.5), 0)
        val = lbl.value
        seg.remove_label(val)
        assert val not in seg.label_values

    def test_erase_label(self, seg):
        lbl = seg.add_label("Erase", (0.5, 0.5, 0.5), 0)
        val = lbl.value
        seg.erase_label(val)
        # Label still exists but pixels are cleared
        assert val in seg.label_values

    def test_rename_label(self, seg):
        lbl = seg.add_label("Old", (1.0, 1.0, 1.0), 0)
        seg.rename_label(lbl.value, "New", (0.0, 0.0, 1.0))
        updated = seg.get_label(lbl.value)
        assert updated.name == "New"
        assert updated.color == pytest.approx((0.0, 0.0, 1.0), abs=1e-5)

    def test_merge_labels(self, seg):
        l1 = seg.add_label("Target", (1.0, 0.0, 0.0), 0)
        l2 = seg.add_label("Source", (0.0, 1.0, 0.0), 0)
        seg.merge_labels(l1.value, [l2.value])
        # merge_labels only merges pixel content into target; labels themselves remain
        assert l1.value in seg.label_values
        assert l2.value in seg.label_values


# ---------------------------------------------------------------------------
# Mixed int|Label accept
# ---------------------------------------------------------------------------

class TestMixedValueAccept:

    def test_remove_label_with_label_obj(self, seg):
        lbl = seg.add_label("ByObj", (0.2, 0.3, 0.4), 0)
        seg.remove_label(lbl)  # Pass Label instead of int
        assert lbl.value not in seg.label_values

    def test_remove_labels_with_label_objs(self, seg):
        l1 = seg.add_label("A", (0.1, 0.1, 0.1), 0)
        l2 = seg.add_label("B", (0.2, 0.2, 0.2), 0)
        seg.remove_labels([l1, l2])
        assert l1.value not in seg.label_values
        assert l2.value not in seg.label_values

    def test_erase_label_with_label_obj(self, seg):
        lbl = seg.add_label("EraseObj", (0.3, 0.3, 0.3), 0)
        seg.erase_label(lbl)
        assert lbl.value in seg.label_values

    def test_get_labels_with_label_objs(self, seg):
        l1 = seg.add_label("X", (0.1, 0.1, 0.1), 0)
        l2 = seg.add_label("Y", (0.2, 0.2, 0.2), 0)
        result = seg.get_labels([l1, l2.value])
        assert len(result) == 2


# ---------------------------------------------------------------------------
# Group CRUD
# ---------------------------------------------------------------------------

class TestGroupCRUD:

    def test_add_empty_group(self, seg):
        assert seg.num_groups == 1
        idx = seg.add_group()
        assert seg.num_groups == 2
        assert idx == 1

    def test_add_group_with_labels(self, seg):
        lbl = mitk.Label(10, "InGroup")
        idx = seg.add_group(labels=[lbl])
        group_vals = seg.get_group_label_values(idx)
        assert len(group_vals) == 1

    def test_add_group_with_image(self, seg):
        group_img = seg.get_group_image(0)
        idx = seg.add_group(image=group_img)
        assert seg.num_groups == 2

    def test_remove_group(self, seg):
        seg.add_group()
        assert seg.num_groups == 2
        seg.remove_group(1)
        assert seg.num_groups == 1

    def test_clear_group_image(self, seg):
        seg.add_label("ClearTest", (1.0, 0.0, 0.0), 0)
        seg.clear_group_image(0)
        # Should not raise

    def test_clear_group_image_with_timestep(self, seg):
        seg.add_label("ClearTS", (1.0, 0.0, 0.0), 0)
        seg.clear_group_image(0, time_step=0)

    def test_set_group_name(self, seg):
        seg.set_group_name(0, "MyGroup")
        assert seg.get_group_name(0) == "MyGroup"


# ---------------------------------------------------------------------------
# Group queries
# ---------------------------------------------------------------------------

class TestGroupQueries:

    def test_groups_property(self, seg):
        seg.add_label("G0Label", (1.0, 0.0, 0.0), 0)
        groups = seg.groups
        assert len(groups) == 1
        assert groups[0].index == 0

    def test_get_group(self, seg):
        seg.add_label("G", (1.0, 0.0, 0.0), 0)
        g = seg.get_group(0)
        assert g.index == 0
        assert len(g.labels) >= 1

    def test_get_group_by_name(self, seg):
        seg.set_group_name(0, "Named")
        g = seg.get_group_by_name("Named")
        assert g is not None
        assert g.name == "Named"

    def test_get_group_by_name_not_found(self, seg):
        result = seg.get_group_by_name("DoesNotExist")
        assert result is None

    def test_get_group_class_names(self, seg):
        seg.add_label("Liver", (1.0, 0.0, 0.0), 0)
        seg.add_label("Liver", (0.0, 1.0, 0.0), 0)
        names = seg.get_group_class_names(0)
        assert "Liver" in names

    def test_get_group_of_label(self, seg):
        lbl = seg.add_label("Track", (1.0, 0.0, 0.0), 0)
        assert seg.get_group_of_label(lbl.value) == 0

    def test_get_group_image(self, seg):
        seg.add_label("ImgTest", (1.0, 0.0, 0.0), 0)
        img = seg.get_group_image(0)
        assert img is not None
        arr = img.as_numpy()
        assert arr.shape == (4, 8, 8)


# ---------------------------------------------------------------------------
# Label queries
# ---------------------------------------------------------------------------

class TestLabelQueries:

    def test_labels_property(self, seg):
        seg.add_label("A", (1.0, 0.0, 0.0), 0)
        seg.add_label("B", (0.0, 1.0, 0.0), 0)
        labels = seg.labels
        assert len(labels) == 2
        # Should be sorted by value
        assert labels[0].value < labels[1].value

    def test_label_values_sorted(self, seg):
        seg.add_label("X", (1.0, 0.0, 0.0), 0)
        seg.add_label("Y", (0.0, 1.0, 0.0), 0)
        vals = seg.label_values
        assert vals == sorted(vals)

    def test_get_label(self, seg):
        lbl = seg.add_label("Find", (1.0, 0.0, 0.0), 0)
        found = seg.get_label(lbl.value)
        assert found.name == "Find"

    def test_get_label_invalid(self, seg):
        with pytest.raises(ValueError):
            seg.get_label(9999)

    def test_get_label_values_by_name(self, seg):
        seg.add_label("Tumor", (1.0, 0.0, 0.0), 0)
        seg.add_label("Tumor", (0.0, 1.0, 0.0), 0)
        vals = seg.get_label_values_by_name("Tumor")
        assert len(vals) == 2

    def test_get_label_values_by_name_with_group(self, seg):
        seg.add_label("Bone", (1.0, 0.0, 0.0), 0)
        vals = seg.get_label_values_by_name("Bone", group=0)
        assert len(vals) == 1

    def test_label_class_names(self, seg):
        seg.add_label("Liver", (1.0, 0.0, 0.0), 0)
        seg.add_label("Kidney", (0.0, 1.0, 0.0), 0)
        names = seg.label_class_names
        assert "Liver" in names
        assert "Kidney" in names


# ---------------------------------------------------------------------------
# LabelVector semantics
# ---------------------------------------------------------------------------

class TestLabelVector:

    def test_len(self, seg):
        seg.add_label("A", (1.0, 0.0, 0.0), 0)
        seg.add_label("B", (0.0, 1.0, 0.0), 0)
        lv = seg.labels
        assert len(lv) == 2

    def test_indexing_positive(self, seg):
        seg.add_label("First", (1.0, 0.0, 0.0), 0)
        seg.add_label("Second", (0.0, 1.0, 0.0), 0)
        lv = seg.labels
        assert lv[0].value < lv[1].value

    def test_indexing_negative(self, seg):
        seg.add_label("A", (1.0, 0.0, 0.0), 0)
        lv = seg.labels
        assert lv[-1].name == "A"

    def test_indexing_out_of_range(self, seg):
        lv = seg.labels
        with pytest.raises(IndexError):
            _ = lv[999]

    def test_slicing(self, seg):
        seg.add_label("A", (1.0, 0.0, 0.0), 0)
        seg.add_label("B", (0.0, 1.0, 0.0), 0)
        seg.add_label("C", (0.0, 0.0, 1.0), 0)
        lv = seg.labels
        sliced = lv[0:2]
        assert isinstance(sliced, mitk.LabelVector)
        assert len(sliced) == 2

    def test_iteration(self, seg):
        seg.add_label("X", (1.0, 0.0, 0.0), 0)
        seg.add_label("Y", (0.0, 1.0, 0.0), 0)
        lv = seg.labels
        names = [l.name for l in lv]
        assert len(names) == 2

    def test_contains_int(self, seg):
        lbl = seg.add_label("In", (1.0, 0.0, 0.0), 0)
        lv = seg.labels
        assert lbl.value in lv
        assert 9999 not in lv

    def test_contains_label(self, seg):
        lbl = seg.add_label("LblIn", (1.0, 0.0, 0.0), 0)
        lv = seg.labels
        assert lbl in lv

    def test_values_property(self, seg):
        seg.add_label("V1", (1.0, 0.0, 0.0), 0)
        seg.add_label("V2", (0.0, 1.0, 0.0), 0)
        lv = seg.labels
        vals = lv.values
        assert isinstance(vals, list)
        assert len(vals) == 2

    def test_repr(self, seg):
        seg.add_label("R", (1.0, 0.0, 0.0), 0)
        lv = seg.labels
        r = repr(lv)
        assert "LabelVector" in r


# ---------------------------------------------------------------------------
# Label semantics
# ---------------------------------------------------------------------------

class TestLabel:

    def test_equality_by_value(self):
        l1 = mitk.Label(5, "A")
        l2 = mitk.Label(5, "B")
        assert l1 == l2

    def test_hash_by_value(self):
        l1 = mitk.Label(5, "A")
        l2 = mitk.Label(5, "B")
        d = {l1: "first"}
        assert d[l2] == "first"

    def test_color_roundtrip(self):
        lbl = mitk.Label(1, "Colored")
        lbl.color = (0.1, 0.5, 0.9)
        assert lbl.color == pytest.approx((0.1, 0.5, 0.9), abs=1e-5)

    def test_opacity(self):
        lbl = mitk.Label(1, "Op")
        lbl.opacity = 0.8
        assert lbl.opacity == pytest.approx(0.8, abs=1e-5)

    def test_visible(self):
        lbl = mitk.Label(1, "Vis")
        lbl.visible = False
        assert lbl.visible is False

    def test_locked(self):
        lbl = mitk.Label(1, "Lock")
        lbl.locked = False
        assert lbl.locked is False

    def test_repr(self):
        lbl = mitk.Label(7, "Test")
        r = repr(lbl)
        assert "7" in r
        assert "Test" in r


# ---------------------------------------------------------------------------
# Converter helpers
# ---------------------------------------------------------------------------

class TestConverterHelpers:

    def test_create_label_mask_binary(self, seg):
        lbl = seg.add_label("Mask", (1.0, 0.0, 0.0), 0)
        mask = seg.create_label_mask(lbl.value, binary=True)
        assert mask is not None
        arr = mask.as_numpy()
        assert arr.shape == (4, 8, 8)

    def test_create_label_mask_nonbinary(self, seg):
        lbl = seg.add_label("Mask2", (1.0, 0.0, 0.0), 0)
        mask = seg.create_label_mask(lbl.value, binary=False)
        assert mask is not None

    def test_create_filtered_group_image(self, seg):
        l1 = seg.add_label("F1", (1.0, 0.0, 0.0), 0)
        l2 = seg.add_label("F2", (0.0, 1.0, 0.0), 0)
        filtered = seg.create_filtered_group_image(0, [l1.value])
        assert filtered is not None

    def test_create_label_class_map(self, seg):
        l1 = seg.add_label("ClassA", (1.0, 0.0, 0.0), 0)
        l2 = seg.add_label("ClassB", (0.0, 1.0, 0.0), 0)
        result = seg.create_label_class_map(0)
        assert isinstance(result, tuple)
        assert len(result) == 2
        img, mapping = result
        assert img is not None
        assert isinstance(mapping, dict)

    def test_create_label_class_map_with_labels(self, seg):
        l1 = seg.add_label("CA", (1.0, 0.0, 0.0), 0)
        result = seg.create_label_class_map(0, labels=[l1.value])
        img, mapping = result
        assert img is not None


# ---------------------------------------------------------------------------
# Split helpers
# ---------------------------------------------------------------------------

class TestSplitHelpers:

    def test_split_labels_by_group(self, seg):
        l1 = seg.add_label("S1", (1.0, 0.0, 0.0), 0)
        seg.add_group()
        l2 = seg.add_label("S2", (0.0, 1.0, 0.0), 1)
        result = seg.split_labels_by_group([l1.value, l2.value])
        assert isinstance(result, dict)
        assert 0 in result
        assert 1 in result

    def test_split_labels_by_class_name(self, seg):
        seg.add_label("Liver", (1.0, 0.0, 0.0), 0)
        seg.add_label("Liver", (0.0, 1.0, 0.0), 0)
        seg.add_label("Kidney", (0.0, 0.0, 1.0), 0)
        result = seg.split_labels_by_class_name(0)
        assert isinstance(result, dict)
        assert "Liver" in result
        assert "Kidney" in result
        assert len(result["Liver"]) == 2

    def test_split_labels_by_class_name_with_labels(self, seg):
        l1 = seg.add_label("A", (1.0, 0.0, 0.0), 0)
        l2 = seg.add_label("B", (0.0, 1.0, 0.0), 0)
        result = seg.split_labels_by_class_name(0, labels=[l1.value])
        assert "A" in result


# ---------------------------------------------------------------------------
# Geometry parity
# ---------------------------------------------------------------------------

class TestGeometryParity:

    def test_spacing(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        assert seg.spacing == pytest.approx((1.0, 1.0, 1.0))

    def test_set_spacing(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        seg.spacing = (2.0, 2.0, 2.0)
        assert seg.spacing == pytest.approx((2.0, 2.0, 2.0))

    def test_origin(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        assert seg.origin == pytest.approx((0.0, 0.0, 0.0))

    def test_set_origin(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        seg.origin = (10.0, 20.0, 30.0)
        assert seg.origin == pytest.approx((10.0, 20.0, 30.0))

    def test_direction(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        d = seg.direction
        assert d.shape == (3, 3)

    def test_ndim(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        assert seg.ndim == 3

    def test_time_steps(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        assert seg.time_steps == 1

    def test_time_geometry(self, ref_image):
        seg = mitk.MultiLabelSegmentation(ref_image)
        tg = seg.time_geometry
        assert tg is not None


# ---------------------------------------------------------------------------
# Enums
# ---------------------------------------------------------------------------

class TestEnums:

    def test_merge_style_values(self):
        assert mitk.MergeStyle.REPLACE is not None
        assert mitk.MergeStyle.MERGE is not None

    def test_overwrite_style_values(self):
        assert mitk.OverwriteStyle.REGARD_LOCKS is not None
        assert mitk.OverwriteStyle.IGNORE_LOCKS is not None


# ---------------------------------------------------------------------------
# transfer_labels
# ---------------------------------------------------------------------------

class TestTransferLabels:

    def test_seg_to_seg_simple(self, ref_image):
        src = mitk.MultiLabelSegmentation(ref_image)
        dst = mitk.MultiLabelSegmentation(ref_image)
        l_src = src.add_label("Source", (1.0, 0.0, 0.0), 0)
        l_dst = dst.add_label("Dest", (0.0, 1.0, 0.0), 0)
        # Ensure same label values for mapping
        mitk.transfer_labels(src, dst, label_mapping=[(l_src.value, l_dst.value)])

    def test_string_enum_aliases(self, ref_image):
        src = mitk.MultiLabelSegmentation(ref_image)
        dst = mitk.MultiLabelSegmentation(ref_image)
        l_src = src.add_label("S", (1.0, 0.0, 0.0), 0)
        l_dst = dst.add_label("D", (0.0, 1.0, 0.0), 0)
        mitk.transfer_labels(
            src, dst,
            label_mapping=[(l_src.value, l_dst.value)],
            merge_style="merge",
            overwrite_style="ignore_locks")

    # --- Image-variant (source is a plain Image, not a MultiLabelSegmentation) ---

    @staticmethod
    def _seed_image_with_label(ref_image, label_value):
        """Return a plain Image with a small patch filled with *label_value*."""
        arr = np.zeros(ref_image.as_numpy().shape, dtype=np.uint16)
        arr[0, 0, :2] = label_value
        return mitk.Image.from_numpy(arr, spacing=ref_image.spacing)

    def test_image_source_basic_mapping(self, ref_image):
        src_img = self._seed_image_with_label(ref_image, 1)
        dst = mitk.MultiLabelSegmentation(ref_image)
        l_dst = dst.add_label("D", (0.0, 1.0, 0.0), 0)

        mitk.transfer_labels(
            src_img, dst.get_group_image(0),
            label_mapping=[(1, l_dst.value)],
            destination_labels=[l_dst])
        assert np.all(dst.get_group_image(0).as_numpy()[0, 0, :2] == l_dst.value)

    def test_image_source_accepts_label_objects_in_mapping(self, ref_image):
        src_img = self._seed_image_with_label(ref_image, 1)
        dst = mitk.MultiLabelSegmentation(ref_image)
        l_dst = dst.add_label("D", (0.0, 1.0, 0.0), 0)

        # label_mapping entries pass through _normalize_mapping: Label works as
        # either source or destination
        mitk.transfer_labels(
            src_img, dst.get_group_image(0),
            label_mapping=[(1, l_dst)],
            destination_labels=[l_dst])
        assert np.all(dst.get_group_image(0).as_numpy()[0, 0, :2] == l_dst.value)

    def test_image_source_full_kwargs(self, ref_image):
        src_img = self._seed_image_with_label(ref_image, 1)
        dst = mitk.MultiLabelSegmentation(ref_image)
        l_dst = dst.add_label("D", (0.0, 1.0, 0.0), 0)

        mitk.transfer_labels(
            src_img, dst.get_group_image(0),
            label_mapping=[(1, l_dst.value)],
            destination_labels=[l_dst],
            source_background=0,
            destination_background=0,
            destination_background_locked=False,
            merge_style=mitk.MergeStyle.REPLACE,
            overwrite_style=mitk.OverwriteStyle.IGNORE_LOCKS)

    def test_image_source_at_time_step(self, ref_image):
        src_img = self._seed_image_with_label(ref_image, 1)
        dst = mitk.MultiLabelSegmentation(ref_image)
        l_dst = dst.add_label("D", (0.0, 1.0, 0.0), 0)

        mitk.transfer_labels_at_time_step(
            src_img, dst.get_group_image(0), time_step=0,
            label_mapping=[(1, l_dst.value)],
            destination_labels=[l_dst])
        assert np.all(dst.get_group_image(0).as_numpy()[0, 0, :2] == l_dst.value)

    def test_image_source_at_time_step_full_kwargs(self, ref_image):
        src_img = self._seed_image_with_label(ref_image, 1)
        dst = mitk.MultiLabelSegmentation(ref_image)
        l_dst = dst.add_label("D", (0.0, 1.0, 0.0), 0)

        mitk.transfer_labels_at_time_step(
            src_img, dst.get_group_image(0), time_step=0,
            label_mapping=[(1, l_dst.value)],
            destination_labels=[l_dst],
            source_background=0,
            destination_background=0,
            destination_background_locked=False,
            merge_style="replace",
            overwrite_style="ignore_locks")


# ---------------------------------------------------------------------------
# clone / clear_group_images
# ---------------------------------------------------------------------------

class TestClone:

    def test_clone_preserves_label_structure(self, seg):
        seg.add_label("A", (1.0, 0.0, 0.0), 0)
        seg.add_label("B", (0.0, 1.0, 0.0), 0)
        cloned = seg.clone()
        assert cloned.num_groups == seg.num_groups
        assert cloned.label_values == seg.label_values
        assert cloned.get_label(cloned.label_values[0]).name == \
               seg.get_label(seg.label_values[0]).name

    def test_clone_is_independent(self, seg):
        seg.add_label("Original", (1.0, 0.0, 0.0), 0)
        cloned = seg.clone()
        cloned.add_label("Extra", (0.0, 0.0, 1.0), 0)
        assert len(seg.label_values) == 1
        assert len(cloned.label_values) == 2

    def test_clear_group_images_zeroes_pixels(self, seg):
        lbl = seg.add_label("C", (1.0, 0.0, 0.0), 0)
        # Write some pixels manually via numpy round-trip
        arr = seg.get_group_image(0).as_numpy().copy()
        arr[0, 0, 0] = lbl.value
        import mitk as _mitk
        seg.update_group_image(0, _mitk.Image.from_numpy(arr), 0, 0)
        assert seg.get_group_image(0).as_numpy()[0, 0, 0] == lbl.value

        seg.clear_group_images()
        assert seg.get_group_image(0).as_numpy()[0, 0, 0] == 0

    def test_clear_group_images_timestep(self, seg):
        seg.clear_group_images(time_step=0)  # should not raise


# ---------------------------------------------------------------------------
# relabel_to
# ---------------------------------------------------------------------------

class TestRelabelTo:

    def _make_seg_with_pixels(self, ref_image):
        """Helper: seg with two labels, each owning a distinct voxel region."""
        import numpy as np
        seg = mitk.MultiLabelSegmentation(ref_image)
        l1 = seg.add_label("L1", (1.0, 0.0, 0.0), 0)
        l2 = seg.add_label("L2", (0.0, 1.0, 0.0), 0)
        arr = seg.get_group_image(0).as_numpy().copy()
        arr[0, 0, :4] = l1.value
        arr[0, 1, :4] = l2.value
        seg.update_group_image(0, mitk.Image.from_numpy(arr), 0, 0)
        return seg, l1.value, l2.value

    def test_auto_clone_default(self, ref_image):
        """relabel_to with dest_seg=None returns a new independent segmentation."""
        import numpy as np
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        result = seg.relabel_to([(v1, v2)])
        # result is a different object
        assert result is not seg
        # original is untouched
        orig = seg.get_group_image(0).as_numpy()
        assert np.any(orig == v1)

    def test_pixels_transferred_to_destination_value(self, ref_image):
        """Pixels of v1 in source appear as v2 in result."""
        import numpy as np
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        result = seg.relabel_to([(v1, v2)])
        arr = result.get_group_image(0).as_numpy()
        # The region formerly holding v1 should now hold v2
        assert np.all(arr[0, 0, :4] == v2)

    def test_unmapped_label_becomes_unlabeled(self, ref_image):
        """Labels absent from the mapping are not transferred (remain 0)."""
        import numpy as np
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        # Only map v1 -> v2; the original v2 pixels (row 1) are not in the mapping
        # as a source, so they should remain UNLABELED in the result.
        result = seg.relabel_to([(v1, v2)])
        arr = result.get_group_image(0).as_numpy()
        assert np.all(arr[0, 1, :4] == 0)

    def test_explicit_dest_seg(self, ref_image):
        """Providing dest_seg uses it as the target and returns it."""
        import numpy as np
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        dest = seg.clone()
        dest.clear_group_images()
        returned = seg.relabel_to([(v1, v2)], dest_seg=dest)
        assert returned is dest
        arr = dest.get_group_image(0).as_numpy()
        assert np.any(arr == v2)

    def test_string_style_aliases(self, ref_image):
        """merge_style and overwrite_style accept lowercase string aliases."""
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        result = seg.relabel_to(
            [(v1, v2)],
            merge_style="replace",
            overwrite_style="ignore_locks",
        )
        assert result is not None

    def test_locked_label_ignored_with_regard_locks(self, ref_image):
        """REGARD_LOCKS protects existing destination voxels that hold a locked label.

        MergeStyle=Replace normally clears destination voxels whose label is absent
        from the source (i.e. source is background at those positions). With
        REGARD_LOCKS those voxels must be left untouched when the label is locked.
        """
        import numpy as np
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        # Destination is NOT cleared — it already has v2 pixels at row 1.
        dest = seg.clone()
        dest.get_label(v2).locked = True
        # Transfer v1 -> v2 with Replace style. Replace would normally blank the
        # destination v2 pixels at row 1 (source is background there), but
        # REGARD_LOCKS must prevent that because v2 is locked.
        result = seg.relabel_to(
            [(v1, v2)],
            dest_seg=dest,
            merge_style=mitk.MergeStyle.REPLACE,
            overwrite_style=mitk.OverwriteStyle.REGARD_LOCKS,
        )
        arr = result.get_group_image(0).as_numpy()
        # Row 1 originally held v2 in dest and v2 is locked — must survive.
        assert np.all(arr[0, 1, :4] == v2)

    def test_label_objects_accepted_in_mapping(self, ref_image):
        """Mapping entries may use Label objects instead of bare ints."""
        import numpy as np
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        lbl1 = seg.get_label(v1)
        lbl2 = seg.get_label(v2)
        result = seg.relabel_to([(lbl1, lbl2)])
        arr = result.get_group_image(0).as_numpy()
        assert np.any(arr == v2)

    def test_keep_untouched_labels_preserves_unmapped_pixels(self, ref_image):
        """keep_untouched_labels=True leaves pixels of unmapped labels in place."""
        import numpy as np
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        # Only remap v1 -> v2; v2's own pixels must survive in the result.
        result = seg.relabel_to([(v1, v2)], keep_untouched_labels=True)
        arr = result.get_group_image(0).as_numpy()
        # Row 0 (v1 source) is now v2; row 1 (original v2) is still v2.
        assert np.all(arr[0, 0, :4] == v2)
        assert np.all(arr[0, 1, :4] == v2)

    def test_keep_untouched_labels_false_clears_unmapped(self, ref_image):
        """keep_untouched_labels=False (default) clears unmapped label pixels."""
        import numpy as np
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        result = seg.relabel_to([(v1, v2)], keep_untouched_labels=False)
        arr = result.get_group_image(0).as_numpy()
        # Row 1 originally held v2 as a source but v2 is not in the source
        # side of the mapping, so those pixels must be 0 in the result.
        assert np.all(arr[0, 1, :4] == 0)

    def test_keep_untouched_labels_with_explicit_dest_raises(self, ref_image):
        """keep_untouched_labels=True combined with dest_seg must raise."""
        seg, v1, v2 = self._make_seg_with_pixels(ref_image)
        dest = seg.clone()
        with pytest.raises(ValueError, match="keep_untouched_labels"):
            seg.relabel_to([(v1, v2)], dest_seg=dest, keep_untouched_labels=True)


# ---------------------------------------------------------------------------
# Error paths
# ---------------------------------------------------------------------------

class TestBoundButUntestedAPI:
    """Coverage for bindings that other tests don't exercise directly."""

    def test_update_group_image_writes_pixels(self, seg):
        lbl = seg.add_label("U", (1.0, 0.0, 0.0), 0)
        arr = seg.get_group_image(0).as_numpy().copy()
        arr[0, 0, 0] = lbl.value
        seg.update_group_image(0, mitk.Image.from_numpy(arr), time_step=0, source_time_step=0)
        assert seg.get_group_image(0).as_numpy()[0, 0, 0] == lbl.value

    def test_get_label_values_at_coordinate(self, seg):
        lbl = seg.add_label("L", (1.0, 0.0, 0.0), 0)
        arr = seg.get_group_image(0).as_numpy().copy()
        arr[0, 0, 0] = lbl.value
        seg.update_group_image(0, mitk.Image.from_numpy(arr), time_step=0, source_time_step=0)

        # Voxel (x=0, y=0, z=0) with unit spacing → world coords (0, 0, 0).
        values = seg.get_label_values_at((0.0, 0.0, 0.0))
        assert lbl.value in values

    def test_split_label_value_mapping_by_source_and_target_group(self, ref_image):
        src = mitk.MultiLabelSegmentation(ref_image)
        dst = mitk.MultiLabelSegmentation(ref_image)
        l_src = src.add_label("S", (1.0, 0.0, 0.0), 0)
        l_dst = dst.add_label("D", (0.0, 1.0, 0.0), 0)
        result = mitk.mitk.split_label_value_mapping_by_source_and_target_group(
            src, dst, [(l_src.value, l_dst.value)])
        # Result is a per-(source_group, target_group) bucket; there is at least
        # one entry corresponding to (0, 0) since both labels live in group 0.
        assert result  # not empty / truthy


class TestErrorPaths:

    def test_get_label_invalid_value(self, seg):
        with pytest.raises(ValueError):
            seg.get_label(65000)

    def test_get_group_invalid_index(self, seg):
        with pytest.raises(ValueError):
            seg.get_group(999)


# ---------------------------------------------------------------------------
# DICOMCodeSequence
# ---------------------------------------------------------------------------

class TestDICOMCodeSequence:

    def test_default_construction(self):
        code = mitk.DICOMCodeSequence()
        assert code.is_empty

    def test_construction_with_args(self):
        code = mitk.DICOMCodeSequence("T-D0050", "SRT", "Tissue")
        assert code.value == "T-D0050"
        assert code.scheme == "SRT"
        assert code.meaning == "Tissue"
        assert not code.is_empty

    def test_property_setters(self):
        code = mitk.DICOMCodeSequence()
        code.value = "A"
        code.scheme = "B"
        code.meaning = "C"
        assert code.value == "A"
        assert code.scheme == "B"
        assert code.meaning == "C"

    def test_equality(self):
        a = mitk.DICOMCodeSequence("V", "S", "M")
        b = mitk.DICOMCodeSequence("V", "S", "M")
        c = mitk.DICOMCodeSequence("X", "S", "M")
        assert a == b
        assert a != c

    def test_repr(self):
        code = mitk.DICOMCodeSequence("T-D0050", "SRT", "Tissue")
        r = repr(code)
        assert "DICOMCodeSequence" in r
        assert "T-D0050" in r
        assert "SRT" in r
        assert "Tissue" in r


# ---------------------------------------------------------------------------
# DICOMCodeSequenceWithModifiers
# ---------------------------------------------------------------------------

class TestDICOMCodeSequenceWithModifiers:

    def test_default_construction(self):
        code = mitk.DICOMCodeSequenceWithModifiers()
        assert code.is_empty
        assert not code.has_modifiers
        assert code.modifier_count == 0

    def test_construction_with_args(self):
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        assert code.value == "T-D0050"
        assert not code.has_modifiers

    def test_construction_from_base(self):
        base = mitk.DICOMCodeSequence("T-D0050", "SRT", "Tissue")
        code = mitk.DICOMCodeSequenceWithModifiers(base)
        assert code.value == "T-D0050"

    def test_is_a_dicom_code_sequence(self):
        code = mitk.DICOMCodeSequenceWithModifiers("V", "S", "M")
        assert isinstance(code, mitk.DICOMCodeSequence)

    def test_add_modifier_object(self):
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        mod = mitk.DICOMCodeSequence("G-A100", "SRT", "Left")
        code.add_modifier(mod)
        assert code.has_modifiers
        assert code.modifier_count == 1
        assert code.get_modifier(0).value == "G-A100"

    def test_add_modifier_tuple(self):
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        code.add_modifier(("G-A100", "SRT", "Left"))
        assert code.modifier_count == 1

    def test_modifiers_returns_list(self):
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        code.add_modifier(("G-A100", "SRT", "Left"))
        mods = code.modifiers
        assert isinstance(mods, list)
        assert len(mods) == 1
        assert isinstance(mods[0], mitk.DICOMCodeSequence)

    def test_set_modifier(self):
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        code.add_modifier(("G-A100", "SRT", "Left"))
        code.set_modifier(0, ("G-A101", "SRT", "Right"))
        assert code.get_modifier(0).value == "G-A101"

    def test_clear_modifiers(self):
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        code.add_modifier(("G-A100", "SRT", "Left"))
        code.clear_modifiers()
        assert not code.has_modifiers

    def test_get_modifier_out_of_range(self):
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        with pytest.raises(IndexError):
            code.get_modifier(0)

    def test_equality(self):
        a = mitk.DICOMCodeSequenceWithModifiers("V", "S", "M")
        b = mitk.DICOMCodeSequenceWithModifiers("V", "S", "M")
        assert a == b

    def test_repr(self):
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        code.add_modifier(mitk.DICOMCodeSequence("G-A100", "SRT", "Left"))
        r = repr(code)
        assert "DICOMCodeSequenceWithModifiers" in r
        assert "T-D0050" in r
        assert "G-A100" in r


# ---------------------------------------------------------------------------
# AlgorithmType enum
# ---------------------------------------------------------------------------

class TestAlgorithmType:

    def test_values_exist(self):
        assert mitk.AlgorithmType.UNDEFINED is not None
        assert mitk.AlgorithmType.MANUAL is not None
        assert mitk.AlgorithmType.SEMIAUTOMATIC is not None
        assert mitk.AlgorithmType.AUTOMATIC is not None

    def test_assignable_to_label(self):
        lbl = mitk.Label(1, "Test")
        lbl.algorithm_type = mitk.AlgorithmType.MANUAL
        assert lbl.algorithm_type == mitk.AlgorithmType.MANUAL


# ---------------------------------------------------------------------------
# Label — extended simple properties
# ---------------------------------------------------------------------------

class TestLabelDescription:

    def test_description_default_empty(self):
        lbl = mitk.Label(1, "Test")
        assert lbl.description == ""

    def test_description_roundtrip(self):
        lbl = mitk.Label(1, "Test")
        lbl.description = "Primary tumor segment"
        assert lbl.description == "Primary tumor segment"

    def test_tracking_id_roundtrip(self):
        lbl = mitk.Label(1, "Test")
        lbl.tracking_id = "TRK-001"
        assert lbl.tracking_id == "TRK-001"

    def test_tracking_uid_roundtrip(self):
        lbl = mitk.Label(1, "Test")
        lbl.tracking_uid = "1.2.3.4.5"
        assert lbl.tracking_uid == "1.2.3.4.5"


# ---------------------------------------------------------------------------
# Label — algorithm properties
# ---------------------------------------------------------------------------

class TestLabelAlgorithm:

    def test_algorithm_type_default_undefined(self):
        lbl = mitk.Label(1, "Test")
        assert lbl.algorithm_type == mitk.AlgorithmType.UNDEFINED

    def test_algorithm_type_roundtrip(self):
        lbl = mitk.Label(1, "Test")
        lbl.algorithm_type = mitk.AlgorithmType.AUTOMATIC
        assert lbl.algorithm_type == mitk.AlgorithmType.AUTOMATIC

    def test_algorithm_type_str_roundtrip(self):
        lbl = mitk.Label(1, "Test")
        lbl.algorithm_type_str = "MANUAL"
        assert lbl.algorithm_type == mitk.AlgorithmType.MANUAL
        assert lbl.algorithm_type_str == "MANUAL"

    def test_algorithm_name_roundtrip(self):
        lbl = mitk.Label(1, "Test")
        lbl.algorithm_name = "TotalSegmentator"
        assert lbl.algorithm_name == "TotalSegmentator"

    def test_add_tool_use_enum(self):
        lbl = mitk.Label(1, "Test")
        lbl.add_tool_use(mitk.AlgorithmType.MANUAL, "PaintBrush")
        assert lbl.algorithm_type == mitk.AlgorithmType.MANUAL
        assert "PaintBrush" in lbl.algorithm_name

    def test_add_tool_use_string(self):
        lbl = mitk.Label(1, "Test")
        lbl.add_tool_use("AUTOMATIC", "SegmentationNet")
        assert lbl.algorithm_type == mitk.AlgorithmType.AUTOMATIC


# ---------------------------------------------------------------------------
# Label — center of mass
# ---------------------------------------------------------------------------

class TestLabelCenterOfMass:

    def test_update_and_read(self):
        lbl = mitk.Label(1, "Test")
        lbl.update_center_of_mass(index=(10.0, 20.0, 30.0), coordinates=(1.0, 2.0, 3.0))
        idx = lbl.center_of_mass_index
        assert idx == pytest.approx((10.0, 20.0, 30.0))
        coords = lbl.center_of_mass_coordinates
        assert coords == pytest.approx((1.0, 2.0, 3.0))

    def test_mtime_updated_after_set(self):
        lbl = mitk.Label(1, "Test")
        mtime_before = lbl.center_of_mass_mtime
        lbl.update_center_of_mass(index=(1.0, 1.0, 1.0), coordinates=(0.5, 0.5, 0.5))
        assert lbl.center_of_mass_mtime >= mtime_before

    def test_reset(self):
        lbl = mitk.Label(1, "Test")
        lbl.update_center_of_mass(index=(5.0, 5.0, 5.0), coordinates=(1.0, 1.0, 1.0))
        lbl.reset_center_of_mass()
        # mtime resets to 0 (the two CoM properties are removed).
        assert lbl.center_of_mass_mtime == 0


# ---------------------------------------------------------------------------
# Label — anatomic region
# ---------------------------------------------------------------------------

class TestLabelAnatomicRegion:

    def test_default_count_zero(self):
        lbl = mitk.Label(1, "Test")
        assert lbl.anatomic_region_count == 0

    def test_convenience_property_none_when_empty(self):
        lbl = mitk.Label(1, "Test")
        assert lbl.anatomic_region is None

    def test_set_via_object(self):
        lbl = mitk.Label(1, "Test")
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        lbl.set_anatomic_region(code)
        assert lbl.anatomic_region_count == 1
        retrieved = lbl.get_anatomic_region(0)
        assert retrieved.value == "T-D0050"

    def test_set_via_tuple(self):
        lbl = mitk.Label(1, "Test")
        lbl.set_anatomic_region(("T-D0050", "SRT", "Tissue"))
        assert lbl.anatomic_region_count == 1

    def test_convenience_property_setter(self):
        lbl = mitk.Label(1, "Test")
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        lbl.anatomic_region = code
        assert lbl.get_anatomic_region(0).value == "T-D0050"

    def test_remove(self):
        lbl = mitk.Label(1, "Test")
        lbl.set_anatomic_region(("T-D0050", "SRT", "Tissue"))
        lbl.remove_anatomic_region(0)
        assert lbl.anatomic_region_count == 0

    def test_with_modifier(self):
        lbl = mitk.Label(1, "Test")
        code = mitk.DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue")
        code.add_modifier(("G-A100", "SRT", "Left"))
        lbl.set_anatomic_region(code)
        retrieved = lbl.get_anatomic_region(0)
        assert retrieved.has_modifiers


# ---------------------------------------------------------------------------
# Label — primary anatomic structure
# ---------------------------------------------------------------------------

class TestLabelPrimaryAnatomicStructure:

    def test_default_count_zero(self):
        lbl = mitk.Label(1, "Test")
        assert lbl.primary_anatomic_structure_count == 0

    def test_convenience_property_none_when_empty(self):
        lbl = mitk.Label(1, "Test")
        assert lbl.primary_anatomic_structure is None

    def test_set_and_get(self):
        lbl = mitk.Label(1, "Test")
        lbl.set_primary_anatomic_structure(("T-D0050", "SRT", "Liver"))
        assert lbl.primary_anatomic_structure_count == 1
        assert lbl.get_primary_anatomic_structure(0).meaning == "Liver"

    def test_remove(self):
        lbl = mitk.Label(1, "Test")
        lbl.set_primary_anatomic_structure(("T-D0050", "SRT", "Liver"))
        lbl.remove_primary_anatomic_structure(0)
        assert lbl.primary_anatomic_structure_count == 0


# ---------------------------------------------------------------------------
# Label — segmented property
# ---------------------------------------------------------------------------

class TestLabelSegmentedProperty:

    def test_category_default_none(self):
        lbl = mitk.Label(1, "Test")
        assert lbl.segmented_property_category is None

    def test_category_set_and_get(self):
        lbl = mitk.Label(1, "Test")
        code = mitk.DICOMCodeSequence("T-D0050", "SRT", "Tissue")
        lbl.segmented_property_category = code
        cat = lbl.segmented_property_category
        assert cat is not None
        assert cat.value == "T-D0050"

    def test_category_set_via_tuple(self):
        lbl = mitk.Label(1, "Test")
        lbl.segmented_property_category = ("T-D0050", "SRT", "Tissue")
        assert lbl.segmented_property_category is not None

    def test_category_clear_with_none(self):
        lbl = mitk.Label(1, "Test")
        lbl.segmented_property_category = ("T-D0050", "SRT", "Tissue")
        lbl.segmented_property_category = None
        assert lbl.segmented_property_category is None

    def test_type_default_none(self):
        lbl = mitk.Label(1, "Test")
        assert lbl.segmented_property_type is None

    def test_type_set_and_get(self):
        lbl = mitk.Label(1, "Test")
        code = mitk.DICOMCodeSequenceWithModifiers("M-8000/3", "SRT", "Neoplasm")
        lbl.segmented_property_type = code
        t = lbl.segmented_property_type
        assert t is not None
        assert t.value == "M-8000/3"

    def test_type_clear_with_none(self):
        lbl = mitk.Label(1, "Test")
        lbl.segmented_property_type = ("M-8000/3", "SRT", "Neoplasm")
        lbl.segmented_property_type = None
        assert lbl.segmented_property_type is None


# ---------------------------------------------------------------------------
# Label — utility
# ---------------------------------------------------------------------------

class TestLabelUtility:

    def test_max_label_value(self):
        assert mitk.Label.MAX_LABEL_VALUE == 65535

    def test_update_from_template(self):
        src = mitk.Label(1, "Source")
        src.description = "From template"
        src.algorithm_type = mitk.AlgorithmType.MANUAL

        dst = mitk.Label(2, "Destination")
        dst.update(src)

        assert dst.description == "From template"
        assert dst.algorithm_type == mitk.AlgorithmType.MANUAL
        assert dst.value == 2  # value not updated by default

    def test_update_with_value(self):
        src = mitk.Label(5, "Source")
        dst = mitk.Label(2, "Destination")
        dst.update(src, update_value=True)
        assert dst.value == 5


# ---------------------------------------------------------------------------
# Label — repr
# ---------------------------------------------------------------------------

class TestLabelRepr:

    def test_repr_minimal(self):
        lbl = mitk.Label(1, "Liver")
        r = repr(lbl)
        assert r == "Label(value=1, name='Liver')"

    def test_repr_includes_description(self):
        lbl = mitk.Label(1, "Liver")
        lbl.description = "Main segment"
        r = repr(lbl)
        assert "description='Main segment'" in r

    def test_repr_includes_algorithm_type(self):
        lbl = mitk.Label(1, "Liver")
        lbl.algorithm_type = mitk.AlgorithmType.AUTOMATIC
        r = repr(lbl)
        assert "algorithm_type=AlgorithmType.AUTOMATIC" in r

    def test_repr_includes_non_default_locked(self):
        lbl = mitk.Label(1, "Liver")
        lbl.locked = False
        r = repr(lbl)
        assert "locked=False" in r

    def test_repr_omits_defaults(self):
        lbl = mitk.Label(1, "Liver")
        r = repr(lbl)
        # No extra fields for a freshly created label
        assert "description" not in r
        assert "algorithm_type" not in r
        assert "locked" not in r

    def test_repr_suffix_for_dicom_codes(self):
        lbl = mitk.Label(1, "Liver")
        lbl.set_anatomic_region(("T-D0050", "SRT", "Tissue"))
        r = repr(lbl)
        assert "+anatomic_region" in r
