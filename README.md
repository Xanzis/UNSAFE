# Utilitarian Numbers: Structural Analysis of Finite Elements (UNSAFE)

Does what it says on the box.
The goal: to simulate both rigid and compliant finite element problems in an environment built from scratch. The existing programs solve for beam stresses in a fully constrained truss.

## To Run:


## Current Features:
* Utility routines written from scratch:
  * Matrix manipulation (matutil)
  * Definition file reading (inutil)
  * Simple truss visualizer (visutil) (uses public domain image writing software)
* Rigid truss stress solver written and tested (unsafe-r)

## Future work:
- [ ] Truss deformation under load
- [ ] Constraint-free truss deformation (squishing against other objects)
- [ ] Improved documentation and readability
- [ ] Improved code usability (Makefile, reorganize file structure, informative executable names)
- [ ] Solid-body modeling, mesh generation, and solid part analysis (far future)
- [ ] Time-domain modeling (farer future)
