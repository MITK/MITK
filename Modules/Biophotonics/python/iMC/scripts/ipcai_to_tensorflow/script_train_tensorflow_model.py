
import Image
import ImageEnhance
import logging
import datetime

import SimpleITK as sitk
import tensorflow as tf

from regression.tensorflow_estimator import multilayer_perceptron, cnn
from regression.tensorflow_dataset import read_data_set
from ipcai2016.tasks_common import *
import commons
from msi.io.nrrdreader import NrrdReader
import msi.normalize as norm
from regression.estimation import estimate_image_tensorflow

sc = commons.ScriptCommons()
sc.set_root("/media/wirkert/data/Data/2016_02_02_IPCAI/")
sc.create_folders()

ipcai_dir = sc.get_full_dir("INTERMEDIATES_FOLDER")

sc.add_dir("SMALL_BOWEL_DATA",
           os.path.join(sc.get_dir("DATA_FOLDER"), "small_bowel_images"))

sc.add_dir("TENSORFLOW_DATA",
           os.path.join(sc.get_dir("INTERMEDIATES_FOLDER"), "TensorflowModels"))

sc.add_dir("SMALL_BOWEL_RESULT", os.path.join(sc.get_dir("RESULTS_FOLDER"),
                                              "small_bowel_tensorflow"))

sc.add_dir("FILTER_TRANSMISSIONS",
           os.path.join(sc.get_dir("DATA_FOLDER"),
                        "filter_transmissions"))



def plot_image(image, axis):
    im2 = axis.imshow(image, interpolation='nearest', alpha=1.0)
    axis.xaxis.set_visible(False)


class TensorFlowCreateOxyImageTask(luigi.Task):
    image_name = luigi.Parameter()
    df_prefix = luigi.Parameter()
    df_test_prefix = luigi.Parameter()

    def requires(self):
        return TensorflowTrainRegressor(df_prefix=self.df_prefix,
                                        df_test_prefix=self.df_test_prefix), \
               Flatfield(flatfield_folder=sc.get_full_dir("FLAT_FOLDER")), \
               SingleMultispectralImage(image=self.image_name), \
               Dark(dark_folder=sc.get_full_dir("DARK_FOLDER"))

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("SMALL_BOWEL_RESULT"),
                                              os.path.split(self.image_name)[1] +
                                              "_" + self.df_prefix +
                                              "_summary" + ".png"))


    def run(self):
        nrrd_reader = NrrdReader()
        tiff_ring_reader = TiffRingReader()
        # read the flatfield
        flat = nrrd_reader.read(self.input()[1].path)
        dark = nrrd_reader.read(self.input()[3].path)
        # read the msi
        nr_filters = len(sc.other["RECORDED_WAVELENGTHS"])
        msi, segmentation = tiff_ring_reader.read(self.input()[2].path,
                                                  nr_filters, resize_factor=0.5)
        # only take into account not saturated pixels.
        segmentation = np.logical_and(segmentation,
                                      (np.max(msi.get_image(),
                                              axis=-1) < 1000.))

        # correct image setup
        filter_nr = int(self.image_name[-6:-5])
        original_order = np.arange(nr_filters)
        new_image_order = np.concatenate((
                                original_order[nr_filters - filter_nr:],
                                original_order[:nr_filters - filter_nr]))
        # resort msi to restore original order
        msimani.get_bands(msi, new_image_order)
        # correct by flatfield
        msimani.image_correction(msi, flat, dark)

        # create artificial rgb
        rgb_image = msi.get_image()[:, :, [2, 3, 1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.

        # preprocess the image
        # sortout unwanted bands
        print "1"
        # zero values would lead to infinity logarithm, thus clip.
        msi.set_image(np.clip(msi.get_image(), 0.00001, 2. ** 64))
        # normalize to get rid of lighting intensity
        norm.standard_normalizer.normalize(msi)
        # transform to absorption
        msi.set_image(-np.log(msi.get_image()))
        # normalize by l2 for stability
        norm.standard_normalizer.normalize(msi, "l2")
        print "2"
        # estimate
        path = sc.get_full_dir("TENSORFLOW_DATA")
        sitk_image, time = estimate_image_tensorflow(msi, path)
        image = sitk.GetArrayFromImage(sitk_image)

        plt.figure()
        print "3"
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(10.)
        plotted_image = np.array(im)
        top_left_axis = plt.gca()
        top_left_axis.imshow(plotted_image, interpolation='nearest')
        top_left_axis.xaxis.set_visible(False)
        top_left_axis.yaxis.set_visible(False)

        plt.set_cmap("jet")
        print "4"
        # plot parametric maps
        segmentation[0, 0] = 1
        segmentation[0, 1] = 1
        oxy_image = np.ma.masked_array(image[:, :], ~segmentation)
        oxy_image[np.isnan(oxy_image)] = 0.
        oxy_image[np.isinf(oxy_image)] = 0.
        oxy_mean = np.mean(oxy_image)
        oxy_image[0, 0] = 0.0
        oxy_image[0, 1] = 1.

        plot_image(oxy_image[:, :], plt.gca())

        df_image_results = pd.DataFrame(data=np.expand_dims([self.image_name,
                                                             oxy_mean * 100.,
                                                             time], 0),
                                        columns=["image name",
                                                 "oxygenation mean [%]",
                                                 "time to estimate"])

        results_file = os.path.join(sc.get_full_dir("SMALL_BOWEL_RESULT"),
                                    "results.csv")
        if os.path.isfile(results_file):
            df_results = pd.read_csv(results_file, index_col=0)
            df_results = pd.concat((df_results, df_image_results)).reset_index(
                drop=True
            )
        else:
            df_results = df_image_results

        df_results.to_csv(results_file)

        plt.savefig(self.output().path,
                    dpi=250, bbox_inches='tight')
        plt.close("all")


class TensorflowTrainRegressor(luigi.Task):
    df_prefix = luigi.Parameter()
    df_test_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sc.get_full_dir("TENSORFLOW_DATA"),
                                              "model.ckpt"))

    def requires(self):
        return tasks_mc.SpectroCamBatch(self.df_prefix), \
            tasks_mc.SpectroCamBatch(self.df_test_prefix)

    def run(self):
        # extract data from the batch
        tensorflow_dataset = read_data_set(self.input()[0].path)
        test_dataset = read_data_set(self.input()[1].path)

        # train regressor
        # Construct the desired model

        # Network Parameters
        nr_filters = len(sc.other["RECORDED_WAVELENGTHS"])
        x = tf.placeholder("float", [None, nr_filters, 1, 1])
        # Construct the desired model
        keep_prob = tf.placeholder("float")
        # pred, regularizers = multilayer_perceptron(x, nr_filters, 100, 1,
        #                                            keep_prob)
        pred = cnn(x, 1, keep_prob)
        # define parameters
        learning_rate = 0.0001
        training_epochs = 300
        batch_size = 100
        display_step = 1

        # Define loss and optimizer

        y = tf.placeholder("float", [None, 1])
        cost = tf.reduce_mean(tf.square(pred - y))
        optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate).minimize(cost)

        # Initializing the variables
        init = tf.initialize_all_variables()

        saver = tf.train.Saver()  # defaults to saving all variables

        # Launch the graph
        with tf.Session() as sess:
            sess.run(init)

            # Training cycle
            for epoch in range(training_epochs):
                avg_cost = 0.
                total_batch = int(tensorflow_dataset.num_examples/batch_size)
                # Loop over all batches
                for i in range(total_batch):
                    batch_xs, batch_ys = tensorflow_dataset.next_batch(batch_size)
                    # Fit training using batch data
                    x_image = np.reshape(batch_xs, [-1, nr_filters, 1, 1])
                    sess.run(optimizer, feed_dict={x: x_image, y: batch_ys,
                                                   keep_prob: 0.75})
                    # Compute average loss
                    avg_cost += sess.run(cost, feed_dict={x: x_image, y: batch_ys,
                                                          keep_prob: 1.0})/total_batch
                # Display logs per epoch step
                if epoch % display_step == 0:
                    print "Epoch:", '%04d' % (epoch+1), "cost=", "{:.9f}".format(avg_cost)

            # Test model
            accuracy = tf.reduce_mean(tf.cast(tf.abs(pred-y), "float"))
            x_test_image = np.reshape(test_dataset.images, [-1, nr_filters, 1, 1])
            print "Mean testing error:", accuracy.eval({x: x_test_image,
                                                          y: test_dataset.labels,
                                                          keep_prob:1.0})

            print "Optimization Finished!"
            saver.save(sess, self.output().path)


if __name__ == '__main__':

    # create a folder for the results if necessary
    sc.set_root("/media/wirkert/data/Data/2016_02_02_IPCAI/")
    sc.create_folders()

    # root folder there the data lies
    logging.basicConfig(filename=os.path.join(sc.get_full_dir("LOG_FOLDER"),
                                 "small_bowel_images" +
                                 str(datetime.datetime.now()) +
                                 '.log'),
                        level=logging.INFO)
    luigi.interface.setup_interface_logging()
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    # determine files to process
    files = get_image_files_from_folder(sc.get_full_dir("SMALL_BOWEL_DATA"),
                                        suffix="F0.tiff", fullpath=True)

    for f in files:
        main_task = TensorFlowCreateOxyImageTask(image_name=f,
                df_prefix="ipcai_revision_colon_mean_scattering_train",
                df_test_prefix="ipcai_revision_colon_mean_scattering_test")
        w.add(main_task)
    w.run()

