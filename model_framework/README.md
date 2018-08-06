ModelFramework
==============

NOTE: Atm its not even compiling because I'm reworking/rewriting large parts of it. 

Easy creation of complex models/functions in C++, with interfaces to data and fitting frameworks etc. Why create such a framework while there are so many things out there (Mathematica, Matlab, R, ROOT, etc.)?
Simple! None of what I found was really suited for my case (I need an open-source and free solution, that is powerful enough to construct rather complex models with easy-to-read/write code.

Just a dummy example to demonstrate the desired workflow:

```
BaseModel base;
AcceptanceModel acceptance;
AcceptanceCorrectedModel acc_cor_model = base * acceptance;
ResolutionModel resolution;
FullModel = acc_cor_model.Convolute(resolution);
```

Design goals:
-------------
General:
--------
- independent framework to create only the models, while other processes like visualization, fitting to data etc is done by other software parts that can be the users choice. Of course interfaces to such third party software have to be written and included in this framework. -> modular design
- keep the dependencies as small as possible (without reinventing the wheel of course). Atm only boost is planned to be a dependency.

Framwork specific:
------------------
- create "any" model (function) from a number of simple instructions/operations
- underlying tree-like structure for the models guarantee the handling of the parameters of "any" model type
- have parametrizations to automate the seting for a defined set of parameters
- no limit on dimensionality (but finite space of course)
- keep it as close as possible to mathematics


NOTE: As I'm working in the field of HEP (high energy physics) ROOT (see root.cern.ch) is the standard framework to be used in such cases. Therefore I have only interfaces to the ROOT Data structures and the Minuit Fitter included atm.
