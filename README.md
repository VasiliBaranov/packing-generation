# Welcome to the PackingGeneration project

The program allows hard-sphere packing generation and packing post-processing (see the [sphere packing](
http://en.wikipedia.org/wiki/Sphere_packing) and the [random-close packing](http://en.wikipedia.org/wiki/Random_close_pack) wikipedia pages).

It supports the Lubachevsky–Stillinger, Jodrey–Tory, and force-biased generation algorithms; it can
calculate the particle insertion probability, Steinhardt Q6 global and local order measures, coordination numbers for
non-rattler particles, pair correlation function, structure factor, and reduced pressure after
pressure equilibration. It doesn't require any preinstalled libraries and is multiplatform (Windows/nix).

It was developed by me (Vasili Baranau) while doing research on hard-sphere packings in the group of
[Prof. Ulrich Tallarek](http://www.uni-marburg.de/fb15/ag-tallarek) in Marburg, Germany, in 2012-2013. It is distributed under the MIT license (see [LICENSE.txt](https://github.com/VasiliBaranov/packing-generation/blob/master/LICENSE.txt)).
This code (release v1.0.1.28) **has a DOI**: [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.580324.svg)](https://doi.org/10.5281/zenodo.580324).

If you use this program in research projects, please **cite the paper** 
*Baranau and Tallarek (2017) Another resolution of the configurational entropy paradox as applied to hard spheres*, 
[doi:10.1063/1.4999483](https://aip.scitation.org/doi/abs/10.1063/1.4999483). 
Or, alternatively, *Baranau and Tallarek (2021) Beyond Salsburg–Wood: Glass equation of state for polydisperse hard spheres*, 
[doi:10.1063/5.0036411](https://aip.scitation.org/doi/full/10.1063/5.0036411).

Sample generated packings look like this:

![SamplePackings](https://github.com/VasiliBaranov/packing-generation/blob/master/Docs/SamplePackings.png)
Left: a monodisperse packing of 10000 particles; 
taken from [Fig. 1](http://pubs.rsc.org/en/content/articlehtml/2016/sm/c6sm00567e#imgfig1) in 
[Baranau et al., 2016](http://pubs.rsc.org/en/content/articlelanding/2016/sm/c6sm00567e). 
Right: a polydisperse packing of 10000 particles; 
taken from [Fig. 1](http://pubs.rsc.org/en/content/articlehtml/2014/sm/c3sm52959b#imgfig1) in 
[Baranau and Tallarek, 2014](http://pubs.rsc.org/en/content/articlelanding/2014/sm/c3sm52959b).

For program options and basic usage, please read further.

For building the program, please read the 
[Compilation](https://github.com/VasiliBaranov/packing-generation/wiki/Compilation) wiki page or the 
[Docs/Compilation.txt](https://github.com/VasiliBaranov/packing-generation/tree/master/Docs/Compilation.txt) source file.

If you would like to understand the code or update it, please read the 
[Architecture](https://github.com/VasiliBaranov/packing-generation/wiki/Architecture) wiki page or the 
[Docs/Architecture.txt](https://github.com/VasiliBaranov/packing-generation/tree/master/Docs/Architecture.txt) source file.

This code was initialy hosted on [Google Code](https://code.google.com/p/packing-generation/), 
before Google closed the Google Code hosting.

# Main links in the project
1. This page, below : program options, their description, and basic usage
2. [Latest release](https://github.com/VasiliBaranov/packing-generation/releases/latest)
3. [Wiki](https://github.com/VasiliBaranov/packing-generation/wiki)
4. [Example](https://github.com/VasiliBaranov/packing-generation/tree/master/Docs/Examples): Sample files, used and produced by the program
5. [Matlab scripts for reading resulting packings](https://github.com/VasiliBaranov/packing-generation/tree/master/Docs/MATLAB%20scripts%20for%20interpreting%20results) (the [main script file](https://github.com/VasiliBaranov/packing-generation/blob/master/Docs/MATLAB%20scripts%20for%20interpreting%20results/ReadPackingScript.m))

___
Table of contents for the contents below:

1. [Program execution](https://github.com/VasiliBaranov/packing-generation#1-program-execution)
2. [Packing generation](https://github.com/VasiliBaranov/packing-generation#2-packing-generation)
3. [Post-processing](https://github.com/VasiliBaranov/packing-generation#3-post-processing)
4. [Sample usage](https://github.com/VasiliBaranov/packing-generation#4-sample-usage)

# 1. Program execution

The program will examine the current folder and all its subfolders recursively for the file
*generation.conf* and run generation (or post-processing) algorithm for each of these files.

All the packing generation and post-processing algorithms are serial (non-parallel).

If the program is compiled and is run as an MPI application (see the 
[compilation wiki page](https://github.com/VasiliBaranov/packing-generation/wiki/Compilation) or
[Docs/Compilation.txt](https://github.com/VasiliBaranov/packing-generation/tree/master/Docs/Compilation.txt)),
the packings to be generated or analyzed will be distributed randomly and evenly by MPI processes
(some supercomputers do not allow single process program runs, some may simply have higher priority
for MPI jobs).

There is no automatic dynamic rebalancing of packings between processes in the current version of
the program.

The program doesn't write log to a file automatically, use nix pipes instead, e.g.,
PackingGeneration.exe > log.txt or PackingGeneration.exe | tee log.txt.

## 1.1. Packing file format

The program will read and write all the packings in the following file format: It should be a binary
file which stores sequentially sphere center x, y, z coordinates and diameter for each particle as
floating points in double precision in little-endian byte order. If the machine on which the program
is being run is big-endian, the program will detect it and will still read and write in
little-endian format.

## 1.2. *generation.conf* format (see also the Examples folder)

> Particles count: 10000 <br>
> Packing size: 20.0823593086113 20.0823593086113 20.0823593086113 <br>
> Generation start: 0 <br>
> Seed: 341 <br>
> Steps to write: 1000 <br>
> Boundaries mode: 1 <br>
> Contraction rate: 1.328910e-002 <br>
> Generation mode: 1 <br>
> 1. boundaries mode: 1 - bulk; 2 - ellipse (inscribed in XYZ box, Z is length of an ellipse); 3 - rectangle <br>
> 2. generationMode = 1 (Poisson, R) or 2 (Poisson in cells, S) <br>

## 1.3. Parameter description

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
origin of short-range disorder, and its impact on eddy dispersion*, [doi:10.1016/j.chroma.2010.05.019](http://www.sciencedirect.com/science/article/pii/S0021967310006631) for details. 
This is the only optional parameter.

Post-processing algorithms will use only Particles count, Packing size, Seed, Boundaries mode
parameters.

## 1.4. Note on final diameters

If you specify the desired diameters through *diameters.txt* or *packing.xyzd*, 
this does not mean that the final packing will possess them. The final diameters will only be *proportional* to them. 
*I.e.*, you specify the diameters up to a constant scaling factor (unknown before generation).

In general, at the beginning of generation all algorithms select some initial scaling factor to make the closest pair 
of particles touch each other (this scaling factor will be small for Poisson configurations). Then, they gradually increase 
this scaling factor during the generation until the termination criterion is met. The final scaling factor will usually 
be below unity. For the force-biased algorithm, it is always below or very slightly above unity. 
For the Lubachevsky–Stillinger algorithm, it can in principle also reach values higher than unity 
(but when starting the generation from the Poisson configuration, it will almost always be below unity at the end).

In case of the Lubachevsky–Stillinger algorithm, the final scaling factor is determined by the contraction rate 
(the lower the rate, the larger the final diameters). In case of the force-biased algorithm, 
the final scaling factor is determined by the contraction rate and indirectly by the original (expected, theoretical) diameters 
(more precisely, by the original packing density as expected from the original diameters). The higher the original density, 
the higher the final density. The lower the contraction rate, the closer the final density is to the original density. 
See the papers in the descriptions of the algorithms below for details.

You can find the dependence of the final packing density on the contraction rate for packings with different 
radii distributions (monodisperse and log-normal with relative standard deviations 0.05-0.3) for the Lubachevsky–Stillinger
and force-biased algorithms in [Figs. 2a and c](http://pubs.rsc.org/en/content/articlehtml/2014/sm/c3sm52959b#imgfig2) in 
[Baranau and Tallarek, (2014)](http://pubs.rsc.org/en/content/articlelanding/2014/sm/c3sm52959b). Please note that the plot 
for the FB algorithm assumes a certain dependence of expected packings densities *φ*<sub>exp</sub> on the contraction rate *γ*:
*φ*<sub>exp</sub> = *φ*<sub>min</sub> + (*φ*<sub>max</sub> - *φ*<sub>min</sub>) / 
(ln*γ*<sub>slow</sub> - ln*γ*<sub>fast</sub>) \* (ln*γ*<sub>fast</sub> - ln*γ*). 
It is basically a line in the plane (ln*γ*, *φ*<sub>exp</sub>) through the points 
(ln*γ*<sub>fast</sub>, *φ*<sub>min</sub>) and (ln*γ*<sub>slow</sub>, *φ*<sub>max</sub>), 
where the following parameters were used: *γ*<sub>fast</sub> = 10<sup>-3</sup>, *φ*<sub>min</sub> = 0.6, 
*γ*<sub>slow</sub> = 10<sup>-7</sup>, *φ*<sub>max</sub> = 0.7.


# 2. Packing generation

After a successful packing generation the program will write an additional file *packing.nfo* with 
packing generation statistics. At the end of every generation the program searches for the closest 
pair of particles by the most naive implementation (two nested cycles by particles). If the program finds a pair 
of particles that are closer than expected by the generation algorithm, the generation is not 
successful, an error is issued and *packing.nfo* is not saved.

## 2.1. Note on final diameters

As mentioned, the final diameters after generation will only be proportinal to the 
original ones. The final packing file will store correct particle centers, but due to historical reasons i do not scale 
the diameters in the output packing file with the final scaling factor. I.e., they will be the same as in the diameters.txt or 
the original packing.xyzd (if they were present). You have to **scale the diameters manually** as explained in the 
[script for reading packings](https://github.com/VasiliBaranov/packing-generation/blob/master/Docs/MATLAB%20scripts%20for%20interpreting%20results/ReadPackingScript.m). 
Basically, you have to multiply the diameters by 
*(finalDensity / theoreticalDensity)^(1/3) = ((1 - finalPorosity) / (1 - theoreticalPorosity))^(1/3)* 
where the values of *finalPorosity* and *theoreticalPorosity* can be taken from the *packing.nfo* file.

At the same time, because all generation algorithms select the initial scaling factor (prior to generation) to make 
the closest pair of particles touch each other, you may run one generation algorithm after another without 
manually rescaling the diameters, because the proper initial scaling factor will anyway be ensured. 

## 2.2. Program usage and options for generation

1. PackingGeneration.exe: uses Lubachevsky–Stillinger algorithm. See *Lubachevsky, Stillinger 
(1990) Geometric properties of random disk packings*, [doi:10.1007/BF01025983](http://dx.doi.org/10.1007/BF01025983); 
or *Lubachevsky (1990) How to Simulate Billiards and Similar Systems*, 
[doi:10.1016/0021-9991(91)90222-7](http://www.sciencedirect.com/science/article/pii/0021999191902227); 
or *Skoge (2006) Packing hyperspheres in high-dimensional Euclidean 
spaces*, [doi:10.1103/PhysRevE.74.041127](http://link.aps.org/doi/10.1103/PhysRevE.74.041127). 
NOTE: The Lubachevsky–Stillinger code will work correctly only when the initial packing has 
relatively large density (0.4 - 0.6), otherwise collisions with far particles will be missed, because 
far particles will not be included as possible neighbors by cell list or verlet list algorithms. 
Therefore i used the following procedure: generated packings with the force-biased algorithm (see 
below) for densities 0.4-0.6, then called the Lubachevsky–Stillinger algorithm. The same applies for the 
two other Lubachevsky–Stillinger variations below (-lsgd and -lsebc).

2. PackingGeneration.exe -ls: same Lubachevsky–Stillinger.

3. -fba: force-biased algorithm. See 
*Mościński et al (1989) The Force-Biased Algorithm for the Irregular Close Packing of Equal Hard Spheres*, 
[doi:10.1080/08927028908031373](http://www.tandfonline.com/doi/abs/10.1080/08927028908031373); 
*Bezrukov et al (2002) Statistical Analysis of Simulated Random Packings of Spheres*, 
[doi:10.1002/1521-4117(200205)19:2&lt;111::AID-PPSC111&gt;3.0.CO;2-M](http://onlinelibrary.wiley.com/doi/10.1002/1521-4117(200205)19:2&lt;111::AID-PPSC111&gt;3.0.CO;2-M/abstract) (yes, that's the real doi).

4. -lsgd: Lubachevsky–Stillinger with gradual densification: the LS generation is run until the 
non-equilibrium reduced pressure is high enough (e.g., a conventional value of 1e12), then 
compression rate is decreased (devided by 2) and the LS generation is run again, until the pressure 
is high enough again; this procedure is repeated until the compression rate is low enough (1e-4). 
See *Baranau and Tallarek (2014) Random-close packing limits for monodisperse and polydisperse hard spheres*, 
[doi:10.1039/C3SM52959B](http://pubs.rsc.org/en/content/articlelanding/2014/sm/c3sm52959b).

5. -lsebc: Lubachevsky–Stillinger with equilibration between compressions, we modified the LS 
generation procedure and after each 20 collisions for each particle with compression we completely 
equilibrate the packings. The equilibration is done by performing sets of 20 collisions for each 
particle with zero compression rate in a loop until the relative difference of reduced pressures in 
the last two sets is less than 1%, so the pressure is stationary. When a packing is equilibrated 
we perform collisions with compression again. 
See *Baranau et al (2013) Pore-size entropy of random hard-sphere packings*, 
[doi:10.1039/C3SM27374A](http://pubs.rsc.org/en/content/articlelanding/2013/sm/c3sm27374a).

NOTE: algorithms below have not been used by me for a long time and may not work well in some 
special cases. If packings contain large intersections after generation, an error will be issued, 
so you won't get incorrect results, but may still lose time. Anyway, original Jodrey–Tory and 
Jodrey–Tory modification by Khirevich are much slower than LS and FBA and can't produce dense 
packings.

6. -ojt: original Jodrey–Tory algorithm. See 
*Jodrey and Tory (1985) Computer simulation of close random packing of equal spheres*, 
[doi:10.1103/PhysRevA.32.2347](http://link.aps.org/doi/10.1103/PhysRevA.32.2347); 
*Bargieł and Mościński (1991) C-language program for the irregular close packing of hard spheres*, 
[doi:10.1016/0010-4655(91)90060-X](http://www.sciencedirect.com/science/article/pii/001046559190060X).

7. -kjt: Jodrey–Tory algorithm modification by Khirevich. See 
*Khirevich et al. (2010) Statistical analysis of packed beds, the origin of short-range disorder, 
and its impact on eddy dispersion*, 
[doi:10.1016/j.chroma.2010.05.019](http://www.sciencedirect.com/science/article/pii/S0021967310006631).

NOTE: algorithms below most certainly do not work well at all. They are left here just for your 
information. If needed, you may test them, fix bugs, and use according to your own needs.

8. -mca: Monte Carlo algorithm. See 
*Maier et al. (2008) Sensitivity of pore-scale dispersion to the construction of random bead packs*, 
[doi:200810.1029/2006WR005577](http://www.agu.org/pubs/crossref/2008/2006WR005577.shtml). 
It produces highly crystalline packings.

9. -cja: Conjugate gradient. See paper 
*Xu et. al. (2005) Random close packing revisited: Ways to pack frictionless disks*, 
[doi:10.1103/PhysRevE.71.061306](http://link.aps.org/doi/10.1103/PhysRevE.71.061306)
and other papers by O'Hern. This algorithm requires GNU Scientific library, 
and also some updates to compiling and linking options (see the 
[compilation wiki page](https://github.com/VasiliBaranov/packing-generation/wiki/Compilation) or
[Docs/Compilation.txt](https://github.com/VasiliBaranov/packing-generation/tree/master/Docs/Compilation.txt)).

# 3. Post-processing

The program will run post-processing just in those packing folders, that contain *packing.nfo* files.
To do the post processing after generation, you have to **manually scale particle diameters** after generation, 
as explained in the [note on final diameters](https://github.com/VasiliBaranov/packing-generation#21-note-on-final-diameters).
Though it is not crucial for post-processing types that depend only on particle positions (like *-directions* 
or *-order* below).

Usage and options:

1. -radii [optional integer to specify radii count]: generates the radii of pores with randomply 
chosen center positions. The default pores count is 1e7 (therefore computation will consume several 
minutes). These radii can be later used to calculate particle insertion probability. 
See *Baranau et al (2013) Pore-size entropy of random hard-sphere packings*, 
[doi:10.1039/C3SM27374A](http://pubs.rsc.org/en/content/articlelanding/2013/sm/c3sm27374a); or 
[doi:10.1063/1.4953079](http://scitation.aip.org/content/aip/journal/jcp/144/21/10.1063/1.4953079). 
They are saved in a binary file *insertion_radii.txt* 
(as floating point numbers in double precision in little-endian byte order).

2. -entropy [optional integer to specify min radii count]: calculates the "pore-size entropy" at once by 
the formula (12) from *Baranau et al. (2013) Pore-size entropy of random hard-sphere packings*, 
[doi:10.1039/C3SM27374A](http://pubs.rsc.org/en/content/articlelanding/2013/sm/c3sm27374a). The 
factor alpha is chosen as 2. The radii count is selected dynamically, so that adding 10000 pores 
changes the large pores quantity by no more than 1%. Optional integer will specify min pores quantity 
to generate, it's 1e7 by default (therefore computation will consume several minutes). Entropy is 
saved in a file *entropy.txt* (not as a binary float, just a simple text value). See also 
*Baranau and Tallarek (2016)*, [doi:10.1063/1.4953079](http://aip.scitation.org/doi/full/10.1063/1.4953079).

3. -directions: calculates all unique directions between particles (uses closest periodic images 
for directions), saves it to a text file *particle_directions.txt*. It has five columns: zero-based 
first particle index, zero-based second particle index, x, y, z of the direction vector from the 
first particle center to the second particle center (of unity length).

4. -contraction: calculates energies of particle intersections after uniform packing contraction 
(or, equivalently, radii increases), as if particles were supplied with potential. Uses second-order 
harmonic potential (see *Xu et al. (2005) Random close packing revisited: Ways to pack frictionless 
disks*, [doi:10.1103/PhysRevE.71.061306](http://link.aps.org/doi/10.1103/PhysRevE.71.061306)) 
and zero-order potential which is equivalent to calculating coordination number per particle. 
For jammed packings coordination number should be close to 6. Results are saved into a text file 
*contraction_energies.txt* with the following columns: contraction ratio (1 / strain rate), potential 
order (0 or 2), total energy, non-rattler particles count, energy per non-rattler particle. 
Contraction ratios (strain rates) are hardcoded, see the resulting file. For details of rattler 
removal see option -rm below.

5. -order: calculates global and local Q6 orders, saves them to a text file *orders.txt*. See 
*Jin, Makse (2010) A first-order phase transition defines the random close packing of hard spheres*,
[doi:10.1016/j.physa.2010.08.010](http://www.sciencedirect.com/science/article/pii/S0378437110006928).

6. -md [optional integer to specify number of LS steps]: conducts Lubachevscky-Stillinger simulation 
with zero contraction rate, i.e., molecular dynamics simulation. It tracks the reduced pressure, 
self-diffusion coefficient, full intermediate scattering function (ISF),
and self-part of the ISF. Terminates when the pressure changes sufficiently slowly *and* 
the normalized full ISF crosses the critical value *e<sup>-1</sup>* at least ten times. 
The optional integer specifies the number of LS steps (each step is 20 collisions per particle).
Some computed parameters are displayd in stdout as a log (thus, one can use *-md | tee log.txt*).
Scattering function values are saved in the *ScatteringFunctions* folder.
For more advanced options, refer to the source code 
([MolecularDynamicsStatistics.cpp, CalculateStationaryStatistics](https://github.com/VasiliBaranov/packing-generation/blob/master/PackingGeneration/Generation/PackingServices/PostProcessing/Source/MolecularDynamicsService.cpp#L117)).
At the end, saves the stationary reduced pressure into a text file 
*molecular_dynamics_statistics.txt*. You can supply this stationary pressure into the equation of 
state by Salsburg and Wood, if packing was close enough to jamming. 
See *Salsburg and Wood (1962) Equation of State of Classical Hard Spheres 
at High Density*, [doi:10.1063/1.1733163](http://jcp.aip.org/resource/1/jcpsa6/v37/i4/p798_s1).

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

10. -hessian: computes a hessian matrix 
(see *Xu et. al. (2005) Random close packing revisited: Ways to pack frictionless disks*, 
[doi:10.1103/PhysRevE.71.061306](http://link.aps.org/doi/10.1103/PhysRevE.71.061306)). 
Writes the matrix to a text file *hessian.txt*. It requires 
LAPACK, and also some updates to compiling and linking options (see the 
[compilation wiki page](https://github.com/VasiliBaranov/packing-generation/wiki/Compilation) or
[Docs/Compilation.txt](https://github.com/VasiliBaranov/packing-generation/tree/master/Docs/Compilation.txt)).

# 4. Sample usage

This section includes sample codes to do the following: generate initial force-biased algorithm 
packing, then create a packing with a Lubachevsky–Stillinger protocol, then find the closest jamming 
density for the LS packing by the LS with gradual densification protocol, then calculate Q6 local 
and global order for the final packing.

PackingGeneration.exe -fba | tee log_fba.txt <br>
PackingGeneration.exe -ls | tee log_ls.txt <br>
PackingGeneration.exe -lsgd | tee log_lsgd.txt <br>
Rescale particle diameters as done in the [MATLAB script](https://github.com/VasiliBaranov/packing-generation/blob/master/Docs/MATLAB%20scripts%20for%20interpreting%20results/ReadPackingScript.m) and explained in the [note on final diameters](https://github.com/VasiliBaranov/packing-generation#21-note-on-final-diameters) <br>
PackingGeneration.exe -order | tee log_entropy.txt

As explained in the [note on final diameters](https://github.com/VasiliBaranov/packing-generation#21-note-on-final-diameters),
you can omit rescaling the packing between different generation steps (*-fba*, *-ls*, *-lsgd*) because each of them scales 
the packing prior to generation anyway. When using only the Q6 order measure (*-order*), you can actually omit rescaling the 
packing at all, because *-order* depends only on particle positions and not on diameters, 
but rescaling is needed in the general case, of course.

