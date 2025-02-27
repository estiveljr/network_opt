# An Integer Programming Model for Supply Chain Network Design with Taxation

## Abstract
A supply chain network project is responsible for defining the logistical structure of a company's supply chain by choosing which locations will be used to establish facilities and the paths products must travel to reach the final customer. These choices are long-term and have high implementation costs for facilities and offer some flexibility for product flows. The decisions can focus on lower costs, reduced customer service time, or a combination of both.

Taxation is a highly relevant factor in supply chain network design when the focus is cost reduction, as a network project optimal when considering only logistical costs may cease to be optimal when tax costs are considered, since tax costs change according to product movement between states.

This study proposes an integer programming model for optimizing costs in a supply chain network design with taxation. It emphasizes the tax that most impacts network decisions, which is ICMS (Brazilian state VAT) and its derivatives, such as tax benefits, tax substitution, and tax rate differentials.

## Implementation
Program developed in C++ using IBM Cplex® solver
Data available in ./build/data folder
Linux environment through WSL
*.bat* scripts available with the data
Artificial data created using Python script in criar_instancia/criar_instancia.py

