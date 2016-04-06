
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
learning_rate = 0.001
training_epochs = 1000
batch_size = 100
display_step = 1

# Network Parameters
n_hidden_1 = 100 # 1st layer num features
n_hidden_2 = 100 # 2nd layer num features
n_input = 21 # MNIST data input (img shape: 28*28)
n_classes = 1 # MNIST total classes (0-9 digits)

# tf Graph input

x = tf.placeholder("float", [None, n_input])
y = tf.placeholder("float", [None, n_classes])

keep_prob = tf.placeholder("float")

# Create model
def multilayer_perceptron(_X, _weights, _biases):
    layer_1 = tf.nn.relu(tf.add(tf.matmul(_X, _weights['h1']), _biases['b1'])) #Hidden layer with RELU activation
    layer_1_drop = tf.nn.dropout(layer_1, keep_prob)
    layer_2 = tf.nn.relu(tf.add(tf.matmul(layer_1_drop, _weights['h2']), _biases['b2'])) #Hidden layer with RELU activation
    layer_2_drop = tf.nn.dropout(layer_2, keep_prob)
    return tf.matmul(layer_2_drop, _weights['out']) + _biases['out']

# Store layers weight & bias
weights = {
    'h1': tf.Variable(tf.random_normal([n_input, n_hidden_1])),
    'h2': tf.Variable(tf.random_normal([n_hidden_1, n_hidden_2])),
    'out': tf.Variable(tf.random_normal([n_hidden_2, n_classes]))
}
biases = {
    'b1': tf.Variable(tf.random_normal([n_hidden_1])),
    'b2': tf.Variable(tf.random_normal([n_hidden_2])),
    'out': tf.Variable(tf.random_normal([n_classes]))
}

# Construct model
pred = multilayer_perceptron(x, weights, biases)

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

