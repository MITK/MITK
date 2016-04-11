

import tensorflow as tf


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


def add_cnn_layer(input, n_inputs, n_outputs, kernel_size, padding='SAME'):
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
    return h_pool, W


def add_fully_connected_layer(_X, n_inputs, n_outputs, keep_prob):
    W = weight_variable([n_inputs, n_outputs])
    b = bias_variable([n_outputs])
    # Hidden layer with RELU activation
    new_layer = tf.nn.relu(tf.add(tf.matmul(_X, W), b))
    # Add dropout regularization
    new_layer_with_dropout = tf.nn.dropout(new_layer, keep_prob)

    return new_layer_with_dropout, W


# this is my exemplary convolutional network
def cnn(_X, n_classes):
    # two convolutional layers
    layer_1 = add_cnn_layer(_X, 1, 32, 6, padding='VALID')
    layer_2 = add_cnn_layer(layer_1, 32, 64, 4)
    # flatten last one to be able to apply it to fully connected layer
    final_number_of_dimensions = 4*64
    layer_2_flat = tf.reshape(layer_2, [-1, final_number_of_dimensions])

    # fully connected layer to bring information together
    h_fc1_drop = add_fully_connected_layer(layer_2_flat,
                                           final_number_of_dimensions,
                                           100)

    # return linear output layer
    W_fc2 = weight_variable([100, n_classes])
    b_fc2 = bias_variable([n_classes])
    return tf.matmul(h_fc1_drop, W_fc2) + b_fc2


# and this is the simpler multilayer perceptron
def multilayer_perceptron(x, n_bands, n_hidden, n_classes, keep_prob):
    flattend_input = tf.reshape(x, [-1, n_bands])
    layer_1, W_1 = add_fully_connected_layer(flattend_input, n_bands, n_hidden,
                                             keep_prob)
    layer_2, W_2 = add_fully_connected_layer(layer_1, n_hidden, n_hidden,
                                             keep_prob)
    last_hidden_layer, W_3 = add_fully_connected_layer(layer_2, n_hidden, n_hidden,
                                                       keep_prob)

    W_out = weight_variable([n_hidden, n_classes])
    b_out = bias_variable([n_classes])

    regularizers = (tf.nn.l2_loss(W_1) + tf.nn.l2_loss(W_2) +
              tf.nn.l2_loss(W_3) + tf.nn.l2_loss(W_out))

    return tf.matmul(last_hidden_layer, W_out) + b_out, regularizers
