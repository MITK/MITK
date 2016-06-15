from collections import OrderedDict
import numpy as np

import theano
from theano.ifelse import ifelse
import theano.tensor as T
import theano.tensor.inplace as inplace
#from theano.sandbox.rng_mrg import MRG_RandomStreams as RandomStreams
from theano.tensor.shared_randomstreams import RandomStreams
import time


#theano.config.compute_test_value = 'warn'
#theano.config.exception_verbosity='high'
#theano.config.profile=True
#theano.config.mode = "FAST_RUN"
#theano.config.mode = "FAST_COMPILE"
#theano.config.scan.allow_gc =True
#theano.config.scan.allow_output_prealloc =False
#theano.optimizer_excluding="more_mem"

# initializing
rng = RandomStreams(seed=234)
photons = 10**6
SHELL_MAX = 101


mu_a = T.scalar('mu_a')
# provide Theano with a default test-value
mu_a.tag.test_value = 2.
mu_s = T.scalar('mu_s')
mu_s.tag.test_value = 20.
microns_per_shell = T.scalar('microns_per_shell')
microns_per_shell.tag.test_value = 50.

albedo = mu_s / (mu_s + mu_a)
shells_per_mfp = 1e4/microns_per_shell/(mu_a+mu_s)

heat = theano.shared(np.zeros(SHELL_MAX, dtype=theano.config.floatX))

x = T.scalar('x')
x.tag.test_value = 0

y = T.scalar('y')
y.tag.test_value = 0

z = T.scalar('z')
z.tag.test_value = 0

u = T.scalar('u')
u.tag.test_value = 0

v = T.scalar('v')
v.tag.test_value = 0

w = T.scalar('w')
w.tag.test_value = 1

weight = T.scalar('weight')
weight.tag.test_value = 1


def one_run(my_x, my_y, my_z,
            my_u, my_v, my_w,
            my_weight,
            my_heat, my_albedo, my_microns_per_shell):

    # move
    random = rng.uniform(low=0.00003, high=1.)
    t = -T.log(random)

    x_moved = my_x + my_u*t
    y_moved = my_y + my_v*t
    z_moved = my_z + my_w*t

    # absorb
    shell = T.cast(T.sqrt(T.sqr(x_moved) + T.sqr(y_moved) + T.sqr(z_moved))
                   * my_microns_per_shell, 'int32')
    shell = T.clip(shell, 0, SHELL_MAX-1)

    new_weight = my_weight * my_albedo

    # new direction
    xi1 = rng.uniform(low=-1., high=1.)
    xi2 = rng.uniform(low=-1., high=1.)
    xi_norm = T.sqrt(T.sqr(xi1) + T.sqr(xi2))

    t_xi = rng.uniform(low=0.000000001, high=1.)

    # rescale xi12 to fit t_xi as norm
    xi1 = xi1/xi_norm * T.sqr(t_xi)
    xi2 = xi2/xi_norm * T.sqr(t_xi)

    u_new_direction = 2. * t_xi - 1.
    v_new_direction = xi1 * T.sqrt((1. - T.sqr(u_new_direction)) / t_xi)
    w_new_direction = xi2 * T.sqrt((1. - T.sqr(u_new_direction)) / t_xi)

    # roulette
    weight_for_starting_roulette = 0.001
    CHANCE = 0.1
    partakes_roulette = T.switch(T.lt(new_weight, weight_for_starting_roulette),
                                 1,
                                 0)
    roulette = rng.uniform(low=0., high=1.)
    loses_roulette = T.gt(roulette, CHANCE)
    # if roulette decides to terminate the photon: set weight to 0
    weight_after_roulette = ifelse(T.and_(partakes_roulette, loses_roulette),
                                     0.,
                                     new_weight)
    # if partakes in roulette but does not get terminated
    weight_after_roulette = ifelse(T.and_(partakes_roulette, T.invert(loses_roulette)),
                                     weight_after_roulette / CHANCE,
                                     weight_after_roulette)

    new_heat = (1.0 - my_albedo) * my_weight
    heat_i = my_heat[shell]

    return (x_moved, y_moved, z_moved,\
           u_new_direction, v_new_direction, w_new_direction,\
           weight_after_roulette),\
           OrderedDict({my_heat: T.inc_subtensor(heat_i, new_heat)})


# one_photon_results, one_photon_updates = theano.scan(fn=one_run,
#                                                      outputs_info=[T.zeros_like(x),
#                                                                    T.zeros_like(y),
#                                                                    T.zeros_like(z),
#                                                                    T.zeros_like(u),
#                                                                    T.zeros_like(v),
#                                                                    T.ones_like(w),
#                                                                    T.ones_like(weight)],
#                                                      non_sequences=[heat,
#                                                                     albedo,
#                                                                     microns_per_shell],
#                                                      n_steps=100,
#                                                      strict=True)
#
# final_one_photon_heat_result = one_photon_updates[heat]

# heat_for_one_photon = theano.function(inputs=[x, y, z,
#                                               u, v, w,
#                                               weight,
#                                               mu_a, mu_s, microns_per_shell],
#                                       outputs=final_one_photon_heat_result,
#                                       updates=one_photon_updates)


def all_runs(my_x, my_y, my_z,
            my_u, my_v, my_w,
            my_weight,
            my_heat,
            my_albedo, my_microns_per_shell):
    my_one_photon_results, my_one_photon_updates = theano.scan(fn=one_run,
                                                               outputs_info=[T.zeros_like(my_x),
                                                                             T.zeros_like(my_y),
                                                                             T.zeros_like(my_z),
                                                                             T.zeros_like(my_u),
                                                                             T.zeros_like(my_v),
                                                                             T.ones_like(my_w),
                                                                             T.ones_like(my_weight)],
                                                               non_sequences=[my_heat,
                                                                              my_albedo,
                                                                              my_microns_per_shell],
                                                               n_steps=10,
                                                               strict=True)
    return {my_heat: my_one_photon_updates[my_heat]}


all_photon_results, all_photon_updates = theano.scan(fn=all_runs,
                                                     outputs_info=None,
                                                     non_sequences=[x, y, z,
                                                                    u, v, w,
                                                                    weight,
                                                                    heat,
                                                                    albedo, microns_per_shell],
                                                     n_steps=10,
                                                     strict=True)


heat_for_all_photons = theano.function(inputs=[x, y, z,
                                               u, v, w,
                                               weight,
                                               mu_a, mu_s, microns_per_shell],
                                       outputs=all_photon_updates[heat],
                                       updates=all_photon_updates)





start = time.time()

print("start simulation")

print(heat_for_all_photons(0., 0., 0.,
                           0., 0., 1.,
                           1.,
                           2., 20., 50.))

end = time.time()
print("end simulation after: " + str(end - start) + " seconds")


