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
        print("Swapping axes!")
        data = np.swapaxes(data, 0, 2)
        swapaxes = True

    print('output_type', output_type)
    print('get_probs', get_probs)
    print('dropout_sampling', dropout_sampling)
    print('threshold', threshold)
    seg = run_tractseg(data=data, output_type=output_type, input_type="peaks", verbose=verbose, get_probs=get_probs, dropout_sampling=dropout_sampling, threshold=threshold)

    if swapaxes:
        seg = np.swapaxes(seg, 0, 2)

    if output_type == "tract_segmentation":

        print("tract_segmentation", get_probs, dropout_sampling)
        if not get_probs and not dropout_sampling:
            if collapse:
                temp = np.zeros((seg.shape[0], seg.shape[1], seg.shape[2]))
                for l in range(seg.shape[3]):
                    temp[np.where(seg[:, :, :, l] > 0)] = l + 1
                seg = temp
            segmentation = sitk.GetImageFromArray(seg.astype(np.uint8))
        else:
            segmentation = sitk.GetImageFromArray(seg)

        segmentation.SetOrigin(in_image.GetOrigin())
        segmentation.SetSpacing(in_image.GetSpacing())
        segmentation.SetDirection(in_image.GetDirection())

    elif output_type == "endings_segmentation":

        print("endings_segmentation", get_probs, dropout_sampling)
        if not get_probs and not dropout_sampling:

            # merge start and end into labelmap
            temp = np.zeros((seg.shape[0], seg.shape[1], seg.shape[2], seg.shape[3] // 2))
            for l in range(temp.shape[3]):
                temp[:, :, :, l][np.where(seg[:, :, :, l * 2] > 0)] = 1
                temp[:, :, :, l][np.where(seg[:, :, :, l * 2 + 1] > 0)] = 2
            seg = temp

            if collapse:
                temp = np.zeros((seg.shape[0], seg.shape[1], seg.shape[2]))
                for l in range(seg.shape[3]):
                    temp[np.where(seg[:, :, :, l] == 1)] = 2*l + 1
                    temp[np.where(seg[:, :, :, l] == 2)] = 2*l + 2
                seg = temp
            segmentation = sitk.GetImageFromArray(seg.astype(np.uint8))
        else:

            temp = np.zeros((seg.shape[0], seg.shape[1], seg.shape[2], seg.shape[3] // 2))
            for l in range(temp.shape[3]):
                temp[:, :, :, l] += seg[:, :, :, l * 2]
                temp[:, :, :, l] += seg[:, :, :, l * 2 + 1]
            seg = temp
            segmentation = sitk.GetImageFromArray(seg)

        segmentation.SetOrigin(in_image.GetOrigin())
        segmentation.SetSpacing(in_image.GetSpacing())
        segmentation.SetDirection(in_image.GetDirection())

    elif output_type == "TOM":

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
