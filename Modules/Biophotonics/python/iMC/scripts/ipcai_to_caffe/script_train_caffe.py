from pylab import *

import caffe
from caffe import layers as L, params as P


def ipcai(database, batch_size):
    # our version of LeNet: a series of linear and simple nonlinear transformations
    n = caffe.NetSpec()

    n.data, n.label = L.HDF5Data(batch_size=batch_size, source=database, ntop=2)

    n.fc1 = L.InnerProduct(n.data, num_output=25, weight_filler=dict(type='xavier'),
                           bias_filler=dict(type='constant', value=0.1))
    n.relu1 = L.ReLU(n.fc1, in_place=True)
    n.fc2 = L.InnerProduct(n.relu1, num_output=25, weight_filler=dict(type='xavier'),
                           bias_filler=dict(type='constant', value=0.1))
    n.relu2 = L.ReLU(n.fc2, in_place=True)
    n.score = L.InnerProduct(n.relu2, num_output=1, weight_filler=dict(type='xavier'),
                             bias_filler=dict(type='constant', value=0.1))
    n.loss = L.EuclideanLoss(n.score, n.label)

    return n.to_proto()

with open('ipcai_train.prototxt', 'w') as f:
    f.write(str(ipcai('ipcai_train_hdf5.h5_list.txt', 100)))

with open('ipcai_test.prototxt', 'w') as f:
    f.write(str(ipcai('ipcai_test_hdf5.h5_list.txt', 50)))

caffe.set_device(0)
caffe.set_mode_gpu()

### load the solver and create train and test nets
solver = None  # ignore this workaround for lmdb data (can't instantiate two solvers on the same data)
solver = caffe.SGDSolver('ipcai_solver.prototxt')

# each output is (batch size, feature dim, spatial dim)
print [(k, v.data.shape) for k, v in solver.net.blobs.items()]

# just print the weight sizes (we'll omit the biases)
print [(k, v[0].data.shape) for k, v in solver.net.params.items()]

solver.net.forward()  # train net
print solver.test_nets[0].forward()  # test net (there can be more than one)

niter = 100000
test_interval = 1000
# losses will also be stored in the log
train_loss = zeros(niter)
test_acc = zeros(int(np.ceil(niter / test_interval)))
output = zeros((niter, 8, 10))

# the main solver loop
for it in range(niter):
    solver.step(1)  # SGD by Caffe

    # store the train loss
    train_loss[it] = solver.net.blobs['loss'].data

    # store the output on the first test batch
    # (start the forward pass at fc1 to avoid loading new data)
    solver.test_nets[0].forward(start='fc1')
    output[it] = solver.test_nets[0].blobs['score'].data[:8]

    # run a full test every so often
    # (Caffe can also do this for us and write to a log, but we show here
    #  how to do it directly in Python, where more complicated things are easier.)
    if it % test_interval == 0:
        print 'Iteration', it, 'testing...'
        mean = 0.
        for i in range(100):
            solver.test_nets[0].forward()
            mean += np.sum(np.abs(np.squeeze(solver.test_nets[0].blobs['score'].data)
                - solver.test_nets[0].blobs['label'].data))
        mean = mean / 5000
        test_acc[it // test_interval] = mean * 100. # %

print "final testing accuracy: ", test_acc[-1]


print solver.test_nets[0].blobs['score'].data
print solver.test_nets[0].blobs['label'].data