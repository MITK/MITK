from collections import OrderedDict
import numpy as np

import theano
import theano.tensor as T
from theano.sandbox.rng_mrg import MRG_RandomStreams as RandomStreams
#from theano.tensor.shared_randomstreams import RandomStreams
import time

#theano.config.compute_test_value = 'warn'
#theano.config.exception_verbosity='high'
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




finished = theano.shared(np.array(0, dtype='int8'))

xyz = theano.shared(np.zeros((photons,3), dtype=theano.config.floatX))

uvw_np = np.zeros((photons,3), dtype=theano.config.floatX)
uvw_np[:,2] = 1. # w = 1
uvw = theano.shared(uvw_np)

#weights_np = np.ones((photons,1), dtype=theano.config.floatX)
weight = theano.shared(np.ones((photons,1), dtype=theano.config.floatX))

heat_np = np.zeros((SHELL_MAX,1), dtype=theano.config.floatX)
heat = theano.shared(heat_np)








# while sum alive > 0

def l2_norm_along_columns(A):
    A_normed = T.sqrt(T.sum(T.sqr(A), axis=1))
    A_normed = A_normed.reshape((photons, 1))
    return A_normed

# move
random = rng.uniform((photons,1), low=0.00003, high=1.)
t = -T.log(random)
t = T.addbroadcast(t, 1)

xyz_moved = xyz + uvw*t
#theano.printing.Print('xyz_moved')(xyz_moved)

# absorb
shells = T.cast(l2_norm_along_columns(xyz_moved) * shells_per_mfp,
               'int32')
shells = T.clip(shells, 0, SHELL_MAX-1)
new_heats = (1.0 - albedo) * weight
new_weight = weight * albedo
theano.printing.Print('shells')(shells)

# new direction
xi12 = rng.uniform((photons,2), low=-1., high=1.)
xi_norm = l2_norm_along_columns(xi12)

t_xi = rng.uniform((photons,1), low=0.000000001, high=1.)
t_xi = T.addbroadcast(t_xi, 1)

# rescale xi12 to fit t_xi as norm
xi12 = xi12/xi_norm * T.sqr(t_xi)

u_new_direction = 2. * t_xi - 1.
vw_new_direction = xi12 * T.sqrt((1. - T.sqr(u_new_direction)) / t_xi)
uvw_new_direction = T.concatenate([u_new_direction, vw_new_direction], axis=1)

#theano.printing.Print('t_xi')(t_xi)
#theano.printing.Print('vw')(vw_new_direction)
#theano.printing.Print('uvw')(uvw_new_direction)
# roulette
weight_for_starting_roulette = 0.001
CHANCE = 0.1
partakes_roulette = T.switch(T.lt(new_weight, weight_for_starting_roulette),
                             1,
                             0)
roulette = rng.uniform((photons,1), low=0., high=1.)
loses_roulette = T.gt(roulette, CHANCE)
# if roulette decides to ter+minate the photon: set weight to 0
weight_after_roulette = T.switch(T.and_(partakes_roulette, loses_roulette),
                                 0.,
                                 new_weight)
# if partakes in roulette but does not get terminated
weight_after_roulette = T.switch(T.and_(partakes_roulette, T.invert(loses_roulette)),
                                 weight_after_roulette / CHANCE,
                                 weight_after_roulette)
#theano.printing.Print('new weight')(new_weight)
#theano.printing.Print('partakes_roulette')(partakes_roulette)
#theano.printing.Print('loses_roulette')(loses_roulette)
#theano.printing.Print('weight_after_roulette')(weight_after_roulette)


one_cycle = theano.function(inputs=[mu_a, mu_s, microns_per_shell],
                            outputs=[shells, new_heats],
                            updates=OrderedDict({xyz: xyz_moved, uvw: uvw_new_direction,
                                                 weight: weight_after_roulette,
                                                 finished: T.allclose(weight, 0.)}))


start = time.time()
print("start simulation")

while not finished.get_value():
    new_shells, new_heats = one_cycle(2, 20, 50)

end = time.time()
print("end simulation after: " + str(end - start) + " seconds")


