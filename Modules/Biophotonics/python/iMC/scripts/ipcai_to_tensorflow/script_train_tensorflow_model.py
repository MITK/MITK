
import os

import  numpy as np

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
learning_rate = 0.00001
training_epochs = 2000
batch_size = 100
display_step = 1

# Network Parameters
n_hidden = 100 # hidden layers number of elements
n_bands = 21 # number of features (wavelengths)
n_classes = 1 # number of outputs (one for oxygenation)

# tf Graph input

x = tf.placeholder("float", [None, n_bands, 1, 1])
y = tf.placeholder("float", [None, n_classes])

keep_prob = tf.placeholder("float")


def weight_variable(shape):
    initial = tf.truncated_normal(shape, stddev=0.1)
    return tf.Variable(initial)


def bias_variable(shape):
    initial = tf.constant(0.1, shape=shape)
    return tf.Variable(initial)


def conv2d(x, W, padding='SAME'):
    return tf.nn.conv2d(x, W, strides=[1, 1, 1, 1], padding=padding)


def max_pool_2x1(x):
    return tf.nn.max_pool(x, ksize=[1, 2, 1, 1],
                          strides=[1, 2, 1, 1], padding='SAME')


def add_hidden_layer(input, n_inputs, n_outputs, kernel_size, padding='SAME'):
    #w = weight_variable([n_inputs, n_outputs])
    #b = bias_variable([n_outputs])
    W = weight_variable([kernel_size, 1, n_inputs, n_outputs])
    b = bias_variable([n_outputs])
    # Hidden layer with RELU activation
    #new_layer = tf.nn.relu(tf.add(tf.matmul(input, w), b))
    h_conv = tf.nn.relu(conv2d(input, W, padding=padding) + b)
    # Add dropout regularization
    #new_layer_with_dropout = tf.nn.dropout(new_layer, keep_prob)
    h_pool = max_pool_2x1(h_conv)
    return h_pool


# Create model
def multilayer_perceptron(_X):
    # two convolutional layers
    layer_1 = add_hidden_layer(_X, 1, 32, 6, padding='VALID')
    layer_2 = add_hidden_layer(layer_1, 32, 64, 4)
    # flatten last one to be able to apply it to fully connected layer
    final_number_of_dimensions = 4*64
    layer_2_flat = tf.reshape(layer_2, [-1, final_number_of_dimensions])

    # fully connected layer to bring information together
    W_fc1 = weight_variable([final_number_of_dimensions, 100])
    b_fc1 = bias_variable([100])
    h_fc1 = tf.nn.relu(tf.matmul(layer_2_flat, W_fc1) + b_fc1)
    # with dropout regularization
    h_fc1_drop = tf.nn.dropout(h_fc1, keep_prob)

    # return linear output layer
    W_fc2 = weight_variable([100, n_classes])
    b_fc2 = bias_variable([n_classes])
    return tf.matmul(h_fc1_drop, W_fc2) + b_fc2

# Construct model
pred = multilayer_perceptron(x)

# Define loss and optimizer
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
            x_image = np.reshape(batch_xs, [-1, n_bands, 1, 1])
            sess.run(optimizer, feed_dict={x: x_image, y: batch_ys,
                                           keep_prob: 0.5})
            # Compute average loss
            avg_cost += sess.run(cost, feed_dict={x: x_image, y: batch_ys,
                                                  keep_prob: 1.0})/total_batch
        # Display logs per epoch step
        if epoch % display_step == 0:
            print "Epoch:", '%04d' % (epoch+1), "cost=", "{:.9f}".format(avg_cost)

    print "Optimization Finished!"

    # Test model
    accuracy = tf.reduce_mean(tf.cast(tf.abs(pred-y), "float"))
    x_test_image = np.reshape(ipcai.test.images, [-1, n_bands, 1, 1])
    print "Median testing error:", accuracy.eval({x: x_test_image,
                                                  y: ipcai.test.labels,
                                                  keep_prob:1.0})

