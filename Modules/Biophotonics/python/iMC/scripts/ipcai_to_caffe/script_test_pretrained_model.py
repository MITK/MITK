
import time

import caffe

from ipcai2016.tasks_common import plot_image

model_def = 'ipcai_test_image.prototxt'
model_weights = 'snapshot_iter_100000.caffemodel'

net = caffe.Net(model_def,      # defines the structure of the model
                model_weights,  # contains the trained weights
                caffe.TEST)     # use test mode (e.g., don't perform dropout)

### perform classification

start = time.time()
output = net.forward()
end = time.time()
estimation_time = end - start

print "time necessary for estimating image parameters: " + str(estimation_time) + "s"

image = output['score'].reshape(1029,1228)

plot_image(image)