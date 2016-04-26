
import numpy as np
from sklearn.preprocessing import Normalizer


from imgmani import collapse_image


class Normalize():

    def __init__(self):
        pass

    def normalize(self, msi):
        pass


class NormalizeIQ(Normalize):
    """Normalize by image quotient"""
    def __init__(self, iqBand=None):
        if iqBand is None:
            iqBand = 0
        self.iqBand = iqBand

    def normalize(self, msi):
        # todo: guard if iqBand is outside of image dimension
        original_shape = msi.get_image().shape
        collapsed_image = collapse_image(msi.get_image())
        iqDimension = collapsed_image[ :, self.iqBand]
        normalized_image = collapsed_image / iqDimension[:, None]
        msi.set_image(np.reshape(normalized_image, original_shape))


class NormalizeMean(Normalize):
    """Normalize by image mean"""
    def __init__(self):
        pass

    def normalize(self, msi, norm="l1"):
        original_shape = msi.get_image().shape
        collapsed_image = collapse_image(msi.get_image())
        # temporarily save mask, since scipy normalizer removes mask
        is_masked_array = isinstance(msi.get_image(), np.ma.MaskedArray)
        if is_masked_array:
            mask = msi.get_image().mask
        normalizer = Normalizer(norm=norm)
        normalized_image = normalizer.transform(collapsed_image)
        if is_masked_array:
            normalized_image = np.ma.MaskedArray(normalized_image, mask=mask)
        msi.set_image(np.reshape(normalized_image, original_shape))


standard_normalizer = NormalizeMean()
