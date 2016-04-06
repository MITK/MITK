
import os

import commons

sc = commons.ScriptCommons()
sc.set_root("/media/wirkert/data/Data/2016_02_02_IPCAI/")
sc.create_folders()

'''
A Multilayer Perceptron implementation example using TensorFlow library.
This example is using the MNIST database of handwritten digits (http://yann.lecun.com/exdb/mnist/)
Author: Aymeric Damien
Project: https://github.com/aymericdamien/TensorFlow-Examples/
'''

# Import MINST data
import input_data
#mnist = input_data.read_data_sets("/tmp/data/", one_hot=True)

ipcai_dir = os.path.join(sc.get_full_dir("INTERMEDIATES_FOLDER"))

import input_ipcai_data
ipcai = input_ipcai_data.read_data_sets(ipcai_dir)

import tensorflow as tf

# Parameters
learning_rate = 0.0001
training_epochs = 300
batch_size = 100
display_step = 1

# Network Parameters
n_hidden = 100 # hidden layers number of elements
n_input = 21 # number of features (wavelengths)
n_classes = 1 # number of outputs (one for oxygenation)

# tf Graph input

x = tf.placeholder("float", [None, n_input])
y = tf.placeholder("float", [None, n_classes])

keep_prob = tf.placeholder("float")


def weight_variable(shape):
    initial = tf.truncated_normal(shape, stddev=0.1)
    return tf.Variable(initial)


def bias_variable(shape):
    initial = tf.constant(0.1, shape=shape)
    return tf.Variable(initial)


def add_fully_connected_layer(input, n_inputs, n_outputs):
    w = weight_variable([n_inputs, n_outputs])
    b = bias_variable([n_outputs])
    # Hidden layer with RELU activation
    new_layer = tf.nn.relu(tf.add(tf.matmul(input, w), b))
    # Add dropout regularization
    new_layer_with_dropout = tf.nn.dropout(new_layer, keep_prob)
    return new_layer_with_dropout


# Create model
def multilayer_perceptron(_X):

    layer_1 = add_fully_connected_layer(_X, n_input, n_hidden)
    layer_2 = add_fully_connected_layer(layer_1, n_hidden, n_hidden)
    last_hidden_layer = add_fully_connected_layer(layer_2, n_hidden, n_hidden)

    # linear output layer
    w_out = weight_variable([n_hidden, n_classes])
    b_out = bias_variable([n_classes])
    return tf.matmul(last_hidden_layer, w_out) + b_out


# Construct model
pred = multilayer_perceptron(x)

# Define loss and optimizer

#cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(pred, y)) # Softmax loss
#cost = -tf.reduce_mean(y*tf.log(pred)+(1-y)*tf.log(1-pred))
cost = tf.reduce_mean(tf.square(pred - y))
optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate).minimize(cost) # Adam Optimizer

# Initializing the variables
init = tf.initialize_all_variables()

# Launch the graph
with tf.Session() as sess:
    sess.run(init)

    # Training cycle
    for epoch in range(training_epochs):
        avg_cost = 0.
        total_batch = int(ipcai.train.num_examples/batch_size)
        # Loop over all batches
        for i in range(total_batch):
            batch_xs, batch_ys = ipcai.train.next_batch(batch_size)
            # Fit training using batch data
            sess.run(optimizer, feed_dict={x: batch_xs, y: batch_ys,
                                           keep_prob: 0.5})
            # Compute average loss
            avg_cost += sess.run(cost, feed_dict={x: batch_xs, y: batch_ys,
                                                  keep_prob: 1.0})/total_batch
        # Display logs per epoch step
        if epoch % display_step == 0:
            print "Epoch:", '%04d' % (epoch+1), "cost=", "{:.9f}".format(avg_cost)

    print "Optimization Finished!"

    # Test model
    accuracy = tf.reduce_mean(tf.cast(tf.abs(pred-y), "float"))
    print "Median testing error:", accuracy.eval({x: ipcai.test.images,
                                                  y: ipcai.test.labels,
                                                  keep_prob:1.0})

