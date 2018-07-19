error_string = None
del error_string
try:
    import SimpleITK as sitk
    import nibabel as nib
    import numpy as np
    from tractseg.TractSeg import run_tractseg

    data = sitk.GetArrayFromImage(in_image)
    data = np.nan_to_num(data)
    swapaxes = False
    if data.shape != (sx, sy, sz):
        data = np.swapaxes(data, 0, 2)
        swapaxes = True

    seg = run_tractseg(data=data, output_type=output_type, input_type="peaks", verbose=verbose, get_probs=get_probs)

    if swapaxes:
        seg = np.swapaxes(seg, 0, 2)

    if not output_type == "TOM":

        if not get_probs:
            if collapse:
                temp = np.zeros((seg.shape[0], seg.shape[1], seg.shape[2]))
                for l in range(seg.shape[3]):
                    temp[np.where(seg[:, :, :, l]>0)] = l + 1
                seg = temp
            segmentation = sitk.GetImageFromArray(seg.astype(np.uint8))
        else:
            segmentation = sitk.GetImageFromArray(seg)

        segmentation.SetOrigin(in_image.GetOrigin())
        segmentation.SetSpacing(in_image.GetSpacing())
        segmentation.SetDirection(in_image.GetDirection())
    else:
        if not collapse:
            for x in range(0, 20):
                tom = sitk.GetImageFromArray(seg[:, :, :, x * 3:x * 3 + 3])
                tom.SetOrigin(in_image.GetOrigin())
                tom.SetSpacing(in_image.GetSpacing())
                tom.SetDirection(in_image.GetDirection())
                globals()['tom%s' % x] = tom
        else:
            tom0 = sitk.GetImageFromArray(seg)
            tom0.SetOrigin(in_image.GetOrigin())
            tom0.SetSpacing(in_image.GetSpacing())
            tom0.SetDirection(in_image.GetDirection())



except Exception as e:
    error_string = str(e)
    print(error_string)
