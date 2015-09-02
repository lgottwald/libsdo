# Library sdo

Contains a parser for vensim mdl files and extensions to formulate an optimization problem:
- Parser for vpd-Files which contains the objective
- Parser for voc-Files which adds controls to the problem
- Parser for vop-Files which bundle vpd-, mdl- and voc-Files to define an optimization problem

## Build/Install

## List of supported vensim functions

### States
- INTEG
- SMOOTH/SMOOTHI (gets reformulated with INTEG creates 1 state)
- SMOOTH3/SMOOTH3I (gets reformulated with INTEG creates 3 states)
- DELAY1/DELAY1I (gets reformulated with INTEG creates 1 state)
- DELAY3/DELAY3I (gets reformulated with INTEG creates 3 states)
- DELAYP (gets reformulated with INTEG creates 3 states and 1 algebraic )

### Special constants
- FINAL TIME
- INITIAL TIME
- TIME STEP
- TIME (Use of current time in expression)

### Arithmetic
- Basic arithmetic: + - * /
- Infix pow operator: **
- EXP
- LN
- LOG
- POWER
- SQRT

### Other

- DELAY FIXED
- ACTIVE_INITIAL
- INITIAL
- WITH LOOKUP
- PULSE TRAIN
- PULSE
- RAMP
- STEP
- RANDOM UNIFORM

### Trigonometric
- SIN
- COS
- TAN
- TANH
- SINH
- COSH
- ARCSIN
- ARCCOS
- ARCTAN

### Nonsmooth
- ABS
- MIN
- MAX
- MODULO
- INTEGER

### Logical
- IF THEN ELSE
- > Operator
- >= Operator
- < Operator
- <= Operator
- = Operator
- :AND:
- :OR:
- :NOT:


