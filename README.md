# Welcome to the PackingGeneration project

The program allows hard-sphere packing generation and packing post-processing (see
http://en.wikipedia.org/wiki/Sphere_packing and http://en.wikipedia.org/wiki/Random_close_pack).

It supports Lubachevsky–Stillinger, Jodrey–Tory, and force-biased generation algorithms; it can
calculate pore-size entropy, Steinhardt Q6 global and local order measures, coordination numbers for
non-rattler particles, pair correlation function, structure factor, and reduced pressure after
pressure equilibration. It doesn't require any preinstalled libraries, is multiplatform (Windows/nix)
and can generate multiple packings simultaneously if compiled and run as an MPI application.

It was developed by me (Vasili Baranau) while doing research on hard-sphere packings in the group of
Prof. Dr. Ulrich Tallarek in Marburg, Germany, in 2012-2013. It is distributed under the MIT license
(see LICENSE.txt).

To get an overview of the group reserch directions, please see
http://www.uni-marburg.de/fb15/ag-tallarek and http://www.khirevich.com.

If you use this program in research projects, please cite the paper *Baranau et al. (2013) Pore-size
entropy of random hard-sphere packings*; DOI 10.1039/C3SM27374A;
http://pubs.rsc.org/en/content/articlelanding/2013/sm/c3sm27374a.

For building the program, please read Docs/Building.txt.

If you would like to understand the code or update it, please read Docs/Architecture.txt.

Sample files, used or produced by the program, can be found in Docs/Examples.

For program options and basic usage, please read further.

Table of Contents:
1. [Program execution](https://github.com/VasiliBaranov/packing-generation#program-execution)
2. [Packing generation](https://github.com/VasiliBaranov/packing-generation#packing-generation)
3. [Post-processing](https://github.com/VasiliBaranov/packing-generation#post-processing)
4. [Sample usage](https://github.com/VasiliBaranov/packing-generation#sample-usage)

1. Program execution
=================

The program will examine the current folder and all its subfolders recursively for the file
*generation.conf* and run generation (or post-processing) algorithm for each of these files.

All the packing generation and post-processing algorithms are serial (non-parallel).

If the program is compiled and is run as an MPI application (see Docs/Building.txt),
the packings to be generated or analyzed will be distributed randomly and evenly by MPI processes
(some supercomputers do not allow single process program runs, some may simply have higher priority
for MPI jobs).

There is no automatic dynamic rebalancing of packings between processes in the current version of
the program.

The program doesn't write log to a file automatically, use nix pipes instead, e.g.,
PackingGeneration.exe > log.txt or PackingGeneration.exe | tee log.txt.

1.1. Packing file format
-----------------

The program will read and write all the packings in the following file format: It should be a binary
file which stores sequentially sphere center x, y, z coordinates and diameter for each particle as
floating points in double precision in little-endian byte order. If the machine on which the program
is being run is big-endian, the program will detect it and will still read and write in
little-endian format.

1.2. *generation.conf* format (see also the Examples folder)
-----------------

> Particles count: 10000 
> Packing size: 20.0823593086113 20.0823593086113 20.0823593086113 
> Generation start: 0 
> Seed: 341 
> Steps to write: 1000 
> Boundaries mode: 1 
> Contraction rate: 1.328910e-005 
> Generation mode: 1 
> 1. boundaries mode: 1 - bulk; 2 - ellipse (inscribed in XYZ box, Z is length of an ellipse); 3 - rectangle 
> 2. generationMode = 1 (Poisson, R) or 2 (Poisson in cells, S) 

1.3. Parameter description
------------------

All floating-point parameters support exponential and decimal notations (%g format in printf).

1. Particles count: particles count in the packing.

2. Packing size: the size of the box where particles reside.

3. Generation start: if the usage of a pregenerated packing is specified (Generation start: 0), the 
program will expect a file *packing.xyzd* along with each *generation.conf* file; otherwise 
(Generation start: 1) the program will generate a poisson packing as initial configuration. 
If a packing should be generated, you may also supply a text file with particle diameters (one 
diameter per line), diameters.txt. If such a file is not provided, all particle diameters will be 
set to 1.0.

4. Seed : seed used for any random number generations (e.g., initial particle positions or particle 
velocities).

5. Steps to write: during the generation and when it is finished the *packing.xyzd* will be 
overwritten with a current packing state. The amount of algorithm iterations between writing a 
temporary state is specified by the Steps to write parameter in *generation.conf*.

6. Boundaries mode: currently just the bulk mode is supported (box which is periodic by all 
dimensions), so always use 1.

7. Contraction rate: the only variable parameter for all of the algorithms.

8. Generation mode: specifies which algorithm for initial packing generation to use, 1 (Poisson, R) 
or 2 (Poisson in cells, S). See *Khirevich et al. (2010) Statistical analysis of packed beds, the 
origin of short-range disorder, and its impact on eddy dispersion* for details. 
This is the only optional parameter.

Post-processing algorithms will use only Particles count, Packing size, Seed, Boundaries mode
parameters.

2. Packing generation
=================

After successful packing generation the program will write an additional file *packing.nfo* with 
packing generation statistics. At the end of every generation the program searches for the closest 
pair by the most naive implementation (two nested cycles by particles). If the program finds a pair 
of particles that are closer than expected by the generation algorithm, the generation is not 
successful, an error is issued and *packing.nfo* is not saved.

Usage and options:

1. PackingGeneration.exe: uses Lubachevsky–Stillinger algorithm. See *Lubachevscky, Stillinger 
(1990) Geometric properties of random disk packings*; or *Lubachevsky (1990) How to Simulate 
Billiards and Similar Systems*; or *Skoge (2006) Packing hyperspheres in high-dimensional Euclidean 
spaces*. NOTE: Lubachevsky–Stillinger code will work correctly only when the initial packing has 
relatively large density (0.4 - 0.6), otherwise collisions with far particles will be missed, because 
far particles will not be included as possible neighbors by cell list or verlet list algorithms. 
Therefore i used the following procedure: generated packings with the force-biased algorithm (see 
below) for densities 0.4-0.6, then called Lubachevsky–Stillinger algorithm. The same applies for the 
two other Lubachevsky–Stillinger variations below (-lsgd and -lsebc).

2. PackingGeneration.exe -ls: same Lubachevsky–Stillinger.

3. -lsgd: Lubachevsky–Stillinger with gradual densification: the LS generation is run until the 
non-equilibrium reduced pressure is high enough (e.g., a conventional value of 1e12), then 
compression rate is decreased (devided by 2) and the LS generation is run again, until the pressure 
is high enough again; this procedure is repeated until the compression rate is low enough (1e-4). 
The paper is not submitted yet.

4. -lsebc: Lubachevsky–Stillinger with equilibration between compressions, we modified the LS 
generation procedure and after each 20 collisions for each particle with compression we completely 
equilibrate the packings. The equilibration is done by performing sets of 20 collisions for each 
particle with zero compression rate in a loop until the relative difference of reduced pressures in 
the last two sets is less than 1%, so the pressure is stationary. When a packing is equilibrated 
we perform collisions with compression again. See *Baranau et al (2013) Pore-size entropy of 
random hard-sphere packings*.

5. -fba: force-biased algorithm. See *Mościński et al (1989) The Force-Biased Algorithm for the 
Irregular Close Packing of Equal Hard Spheres*; *Bezrukov et al (2002) Statistical Analysis of 
Simulated Random Packings of Spheres*.

NOTE: algorithms below have not been used by me for a long time and may not work well in some 
special cases. If packings contain large intersections after generation, an error will be issued, 
so you won't get incorrect results, but may still lose time. Anyway, original Jodrey–Tory and 
Jodrey–Tory modification by Khirevich are much slower than LS and FBA and can't produce dense 
packings.

6. -ojt: original Jodrey–Tory algorithm. See *Jodrey and Tory (1985) Computer simulation of close 
random packing of equal spheres*; *Bargieł and Mościński (1991) C-language program for the irregular 
close packing of hard spheres*.

7. -kjt: Jodrey–Tory algorithm modification by Khirevich. See *Khirevich et al. (2010) Statistical 
analysis of packed beds, the origin of short-range disorder, and its impact on eddy dispersion*.

NOTE: algorithms below most certainly do not work well at all. They are left here just for your 
information. If needed, you may test them, fix bugs, and use according to your own needs.

8. -mca: Monte Carlo algorithm. See *Maier et al. (2008) Sensitivity of pore-scale dispersion to 
the construction of random bead packs*. It produces highly crystalline packings.

9. -cja: Conjugate gradient. See paper *Xu et. al. (2005) Random close packing revisited: Ways to 
pack frictionless disks*; and other papers by O'Hern. This algorithm requires GNU Scientific library, 
and also some updates to compiling and linking options (see Docs/Building.txt).

3. Post-processing
=================

The program will run post-processing just in those packing folders, that contain *packing.nfo* files.

Usage and options:

1. -radii [optional integer to specify radii count]: generates the radii of pores with randomply 
chosen center positions. The default pores count is 1e7 (therefore computation will consume several 
minutes). These radii can be later used to calculate pore-size entropy. See *Baranau et al (2013) 
Pore-size entropy of random hard-sphere packings*. They are saved in a binary file 
*insertion_radii.txt* (as floating point numbers in double precision in little-endian byte order).

2. -entropy [optional integer to specify min radii count]: calculates pore-size entropy at once by 
the formula (12) from *Baranau et al (2013) Pore-size entropy of random hard-sphere packings*. The 
factor alpha is chosen as 2. The radii count is selected dynamically, so that adding 10000 pores 
changes the large pores quantity by no more than 1%. Optional integer will specify min pores quantity 
to generate, it's 1e7 by default (therefore computation will consume several minutes). Entropy is 
saved in a file *entropy.txt* (not as a binary float, just a simple text value).

3. -directions: calculates all unique directions between particles (uses closest periodic images 
for directions), saves it to a text file *particle_directions.txt*. It has five columns: zero-based 
first particle index, zero-based second particle index, x, y, z of the direction vector from the 
first particle center to the second particle center (of unity length).

4. -contraction: calculates energies of particle intersections after uniform packing contraction 
(or, equivalently, radii increases), as if particles were supplied with potential. Uses second-order 
harmonic potential (see *Xu et. al. (2005) Random close packing revisited: Ways to pack frictionless 
disks*) and zero-order potential which is equivalent to calculating coordination number per particle. 
For jammed packings coordination number should be close to 6. Results are saved into a text file 
*contraction_energies.txt* with the following columns: contraction ratio (1 / strain rate), potential 
order (0 or 2), total energy, non-rattler particles count, energy per non-rattler particle. 
Contraction ratios (strain rates) are hardcoded, see the resulting file. For details of rattler 
removal see option -rm below.

5. -order: calculates global and local Q6 orders, saves them to a text file *orders.txt*. See 
*Jin, Makse (2010) A first-order phase transition defines the random close packing of hard spheres*.

6. -md: conducts Lubachevscky-Stillinger simulation with zero contraction rate, i.e., molecular 
dynamics simulation, tracks reduced pressure, waits until packings are completely equilibrated 
(pressure is stationary) and saves stationary pressure into a text file 
*molecular_dynamics_statistics.txt*. You can supply this stationary pressure into the equation of 
state by Salsburg and Wood. See *Salsburg and Wood (1962) Equation of State of Classical Hard Spheres 
at High Density*. The results of this application by me are not yet submitted anywhere.

7. -rm: removes rattler particles. Particle is a rattler if it has less than 4 contacts if the 
packing is uniformely contracted with a strain rate 1e-7. Rattlers are removed recursively (because 
new rattlers may appear after removing the first wave of rattlers). At some point this recursive 
process terminates. The program overwrites *packing.xyzd*, *packing.nfo* and *generation.conf* 
with a new packing.

8. -pc: computes pair correlation function, saves its values into a text file 
*pair_correlation_function.txt*. It has three columns: binLeftEdge, binParticleCount, 
pairCorrelationFunctionValue.

9. -sf: computes structure factor. For the exact method see *Xu and Ching (2010) Effects of 
particle-size ratio on jamming of binary mixtures at zero temperature*. This method doesn't require 
any Fourier transforms. The program saves structure factor values into *structure_factor.txt*, 
which contains 2 columns: waveVectorLength structureFactorValue.

NOTE: options below have not been used by me for a long time and most probably do not work well. 
They are left here just for your information. If needed, you may test them, fix bugs, and use 
according to your own needs.

10. -hessian: computes a hessian matrix (see *Xu et. al. (2005) Random close packing revisited: 
Ways to pack frictionless disks*). Writes the matrix to a text file *hessian.txt*. It requires 
LAPACK, and also some updates to compiling and linking options (see Docs/Building.txt).

4. Sample usage
=================

This section includes sample codes to do the following: generate initial force-biased algorithm 
packing, then create a packing with a Lubachevsky–Stillinger protocol, then find the closest jamming 
density for the LS packing by the LS with gradual densification protocol, then calculate Q6 local 
and global order for the final packing.

PackingGeneration.exe -fba | tee log_fba.txt 
PackingGeneration.exe -ls | tee log_ls.txt 
PackingGeneration.exe -lsgd | tee log_lsgd.txt 
PackingGeneration.exe -order | tee log_entropy.txt 

