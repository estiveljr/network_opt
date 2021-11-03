#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <limits>
#include <map>
#include <ilcplex/ilocplex.h>

using namespace std;

static string caminho = "F:\\OneDrive\\_each\\_Quali\\Artigo\\modelocpp\\";

struct ARCO {
    string tipo_de_arco;
    string i; //entrada da localidade ou origem da rota
    string j; //saida da localidade ou destino da rota
    string s; // sku
    float a; // CF
    float b; // CVL + icmsst + custos_fonecimento
    float c; // capacidade do arco
    float m; // icms - cred_pres + difal
    float n; // icms * (1 - anulacao)
};

vector<ARCO> lercsv(const std::string &nome_arquivo);

//EXEMPLO CPLEX BLEND

// -------------------------------------------------------------- -*- C++ -*-
// File: blend.cpp
// Version 20.1.0
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5655-Y21
// Copyright IBM Corporation 2000, 2020. All Rights Reserved.
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.
// --------------------------------------------------------------------------
//
// blend.cpp -- A blending problem

/* ------------------------------------------------------------

Problem Description
-------------------

Goal is to blend four sources to produce an alloy: pure metal, raw
materials, scrap, and ingots.

Each source has a cost.
Each source is made up of elements in different proportions.
Alloy has minimum and maximum proportion of each element.

Minimize cost of producing a requested quantity of alloy.

------------------------------------------------------------ */

/*
Cost:653.554
Pure metal:
0) 0
1) 0
2) 0
Raw material:
0) 0
1) 0
Scrap:
0) 17.059
1) 30.2311
Ingots :
0) 32.4769
Elements:
0) 3.55
1) 24.85
2) 42.6
*/

//ILOSTLBEGIN

IloInt nbElements, nbRaw, nbScrap, nbIngot;
IloNum alloy;
IloNumArray nm, nr, ns, ni, p, P;
IloNumArray2 PRaw, PScrap, PIngot;

void define_data(IloEnv env) {
    nbElements = 3;
    nbRaw      = 2;
    nbScrap    = 2;
    nbIngot    = 1;
    alloy      = 71;
    nm = IloNumArray(env, nbElements, 22.0, 10.0, 13.0);
    nr = IloNumArray(env, nbRaw, 6.0, 5.0);
    ns = IloNumArray(env, nbScrap, 7.0, 8.0);
    ni = IloNumArray(env, nbIngot, 9.0);
    p = IloNumArray(env, nbElements, 0.05, 0.30, 0.60);
    P = IloNumArray(env, nbElements, 0.10, 0.40, 0.80);
    PRaw   = IloNumArray2(env, nbElements);
    PScrap = IloNumArray2(env, nbElements);
    PIngot = IloNumArray2(env, nbElements);
    PRaw[0] = IloNumArray(env, nbRaw, 0.20, 0.01);
    PRaw[1] = IloNumArray(env, nbRaw, 0.05, 0.00);
    PRaw[2] = IloNumArray(env, nbRaw, 0.05, 0.30);

    PScrap[0] = IloNumArray(env, nbScrap, 0.00, 0.01);
    PScrap[1] = IloNumArray(env, nbScrap, 0.60, 0.00);
    PScrap[2] = IloNumArray(env, nbScrap, 0.40, 0.70);

    PIngot[0] = IloNumArray(env, nbIngot, 0.10);
    PIngot[1] = IloNumArray(env, nbIngot, 0.45);
    PIngot[2] = IloNumArray(env, nbIngot, 0.45);
}

int blend(int, char**);
int blend()
{
    IloEnv env;
    try {
        IloInt j;

        define_data(env);

        IloModel model(env);

        IloNumVarArray m(env, nbElements, 0.0, IloInfinity);
        IloNumVarArray r(env, nbRaw,   0.0, IloInfinity);
        IloNumVarArray s(env, nbScrap, 0.0, IloInfinity);
        IloNumVarArray i(env, nbIngot, 0.0, 100000);
        IloNumVarArray e(env, nbElements);

        // Objective Function: Minimize Cost
        model.add(IloMinimize(env, IloScalProd(nm, m) + IloScalProd(nr, r) +
                                   IloScalProd(ns, s) + IloScalProd(ni, i)  ));

        // Min and max quantity of each element in alloy
        for (j = 0; j < nbElements; j++) {
            e[j] = IloNumVar(env, p[j] * alloy, P[j] * alloy);
        }

        // Constraint: produce requested quantity of alloy
        model.add(IloSum(e) == alloy);

        // Constraints: Satisfy element quantity requirements for alloy
        for (j = 0; j < nbElements; j++) {
            model.add(e[j] == m[j] + IloScalProd(PRaw[j], r)
                              + IloScalProd(PScrap[j], s)
                              + IloScalProd(PIngot[j], i));
        }

        // Optimize
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());
        cplex.setWarning(env.getNullStream());
        cplex.solve();

        if (cplex.getStatus() == IloAlgorithm::Infeasible)
            env.out() << "No Solution" << endl;

        env.out() << "Solution status: " << cplex.getStatus() << endl;

        // Print results
        env.out() << "Cost:" << cplex.getObjValue() << endl;
        env.out() << "Pure metal:" << endl;
        for(j = 0; j < nbElements; j++)
            env.out() << j << ") " << cplex.getValue(m[j]) << endl;
        env.out() << "Raw material:" << endl;
        for(j = 0; j < nbRaw; j++)
            env.out() << j << ") " << cplex.getValue(r[j]) << endl;
        env.out() << "Scrap:" << endl;
        for(j = 0; j < nbScrap; j++)
            env.out() << j << ") " << cplex.getValue(s[j]) << endl;
        env.out() << "Ingots : " << endl;
        for(j = 0; j < nbIngot; j++)
            env.out() << j << ") " << cplex.getValue(i[j]) << endl;
        env.out() << "Elements:" << endl;
        for(j = 0; j < nbElements; j++)
            env.out() << j << ") " << cplex.getValue(e[j]) << endl;
    }
    catch (IloException& ex) {
        cerr << "Error: " << ex << endl;
    }
    catch (...) {
        cerr << "Error" << endl;
    }
    env.end();
    return 0;
}


int contarLinhas(const std::string &nome_arquivo) {
    ifstream infile;
    infile.open(nome_arquivo);
    string linecount;
    int nlinhas = 0;
    if (!infile) {
        cerr << "Erro ao abrir o arquivo";
        exit(1);
    }
    while (infile >> linecount) {
        nlinhas++;
    }
    infile.close();
    return nlinhas;
}

float stringtofloat(string s){
    if (s.length() == 0) {
        return std::numeric_limits<float>::quiet_NaN();
    } else {
        return stof(s);
    }
}

vector<ARCO> lercsv(const std::string &nome_arquivo) {
    ifstream infile;
    string dados;
//    int nlinhas;
//    nlinhas = contarLinhas(nome_arquivo);
    vector<ARCO> grafo; // cria um array com elemente do tipo ARCO, formando um grafo orientado.

    infile.open(nome_arquivo);
    if (!infile) {
        cerr << "Erro ao abrir o arquivo";
        exit(1);
    }
    std::string line;
    int numlinha = 0;
    while (infile >> line) {
        std::string delimiter = ",";
        size_t pos = 0;
        std::string token;

        ARCO arco;
        grafo.push_back(arco);
        string dados_arco[9];
        int item_arco = 0;
        do {
            pos = line.find(delimiter);
            token = line.substr(0, pos);
            dados_arco[item_arco] = token;
            line.erase(0, pos + delimiter.length());
            item_arco++;
        } while (pos != std::string::npos);

        // ALOCA DADOS DE CADA LINHA DO STREAM NO GRAFO
        if (numlinha != 0) {
            grafo[numlinha - 1].tipo_de_arco = dados_arco[0];
            grafo[numlinha - 1].i = dados_arco[1];
            grafo[numlinha - 1].j = dados_arco[2];
            grafo[numlinha - 1].s = dados_arco[3];
            grafo[numlinha - 1].a = stringtofloat(dados_arco[4]);
            grafo[numlinha - 1].b = stringtofloat(dados_arco[5]);
            grafo[numlinha - 1].c = stringtofloat(dados_arco[6]);
            grafo[numlinha - 1].m = stringtofloat(dados_arco[7]);
            grafo[numlinha - 1].n = stringtofloat(dados_arco[8]);
        }
        numlinha++;
    }
    infile.close();
    // CRIANDO UM DICIONÁRIO À PARTIR DO GRAFO EM STRUCTS

    std::map<vector<string>, ARCO> grafomap;
    for (int i = 0; i < grafo.size(); ++i) {
        vector<string> key = {grafo[i].i, grafo[i].j, grafo[i].s};
        grafomap[key] = grafo[i];
    }

    // temp: checa se a leitura do dic está correta.
    vector<string> query = {"CD_0_entrada","CD_0_saida",""};
    cout << grafomap[query].a << endl;

    return grafo;
}

//ILOSTLBEGIN

//IloInt nbElements, nbRaw, nbScrap, nbIngot;
//IloNum alloy;
//IloNumArray nm, nr, ns, ni, p, P;
//IloNumArray2 PRaw, PScrap, PIngot;


//void define_data(IloEnv env) {
//    nbElements = 3;
//    nbRaw      = 2;
//    nbScrap    = 2;
//    nbIngot    = 1;
//    alloy      = 71;
//    nm = IloNumArray(env, nbElements, 22.0, 10.0, 13.0);
//    nr = IloNumArray(env, nbRaw, 6.0, 5.0);
//    ns = IloNumArray(env, nbScrap, 7.0, 8.0);
//    ni = IloNumArray(env, nbIngot, 9.0);
//    p = IloNumArray(env, nbElements, 0.05, 0.30, 0.60);
//    P = IloNumArray(env, nbElements, 0.10, 0.40, 0.80);
//    PRaw   = IloNumArray2(env, nbElements);
//    PScrap = IloNumArray2(env, nbElements);
//    PIngot = IloNumArray2(env, nbElements);
//    PRaw[0] = IloNumArray(env, nbRaw, 0.20, 0.01);
//    PRaw[1] = IloNumArray(env, nbRaw, 0.05, 0.00);
//    PRaw[2] = IloNumArray(env, nbRaw, 0.05, 0.30);
//
//    PScrap[0] = IloNumArray(env, nbScrap, 0.00, 0.01);
//    PScrap[1] = IloNumArray(env, nbScrap, 0.60, 0.00);
//    PScrap[2] = IloNumArray(env, nbScrap, 0.40, 0.70);
//
//    PIngot[0] = IloNumArray(env, nbIngot, 0.10);
//    PIngot[1] = IloNumArray(env, nbIngot, 0.45);
//    PIngot[2] = IloNumArray(env, nbIngot, 0.45);
//}

//IloInt nbElements, nbRaw, nbScrap, nbIngot;
//IloNum alloy;
//IloNumArray nm, nr, ns, ni, p, P;
//IloNumArray2 PRaw, PScrap, PIngot;



void criar_grafo(IloEnv env) {
    vector<ARCO> grafo;
    grafo = lercsv(caminho.append("arcos_consolidados.csv"));
}

int flow(){
    IloEnv env;
    try {
        criar_grafo(env);
//        IloModel model(env);

    }
    catch (IloException& ex) {
        cerr << "Error: " << ex << endl;
    }
    catch (...) {
        cerr << "Error" << endl;
    }
    env.end();
    return 0;
}

//int blend(int, char**);
//int blend()
//{
//    IloEnv env;
//    try {
//        IloInt j;
//
//        define_data(env);
//
//        IloModel model(env);
//
//        IloNumVarArray m(env, nbElements, 0.0, IloInfinity);
//        IloNumVarArray r(env, nbRaw,   0.0, IloInfinity);
//        IloNumVarArray s(env, nbScrap, 0.0, IloInfinity);
//        IloNumVarArray i(env, nbIngot, 0.0, 100000);
//        IloNumVarArray e(env, nbElements);
//
//        // Objective Function: Minimize Cost
//        model.add(IloMinimize(env, IloScalProd(nm, m) + IloScalProd(nr, r) +
//                                   IloScalProd(ns, s) + IloScalProd(ni, i)  ));
//
//        // Min and max quantity of each element in alloy
//        for (j = 0; j < nbElements; j++) {
//            e[j] = IloNumVar(env, p[j] * alloy, P[j] * alloy);
//        }
//
//        // Constraint: produce requested quantity of alloy
//        model.add(IloSum(e) == alloy);
//
//        // Constraints: Satisfy element quantity requirements for alloy
//        for (j = 0; j < nbElements; j++) {
//            model.add(e[j] == m[j] + IloScalProd(PRaw[j], r)
//                              + IloScalProd(PScrap[j], s)
//                              + IloScalProd(PIngot[j], i));
//        }
//
//        // Optimize
//        IloCplex cplex(model);
//        cplex.setOut(env.getNullStream());
//        cplex.setWarning(env.getNullStream());
//        cplex.solve();
//
//        if (cplex.getStatus() == IloAlgorithm::Infeasible)
//            env.out() << "No Solution" << endl;
//
//        env.out() << "Solution status: " << cplex.getStatus() << endl;
//
//        // Print results
//        env.out() << "Cost:" << cplex.getObjValue() << endl;
//        env.out() << "Pure metal:" << endl;
//        for(j = 0; j < nbElements; j++)
//            env.out() << j << ") " << cplex.getValue(m[j]) << endl;
//        env.out() << "Raw material:" << endl;
//        for(j = 0; j < nbRaw; j++)
//            env.out() << j << ") " << cplex.getValue(r[j]) << endl;
//        env.out() << "Scrap:" << endl;
//        for(j = 0; j < nbScrap; j++)
//            env.out() << j << ") " << cplex.getValue(s[j]) << endl;
//        env.out() << "Ingots : " << endl;
//        for(j = 0; j < nbIngot; j++)
//            env.out() << j << ") " << cplex.getValue(i[j]) << endl;
//        env.out() << "Elements:" << endl;
//        for(j = 0; j < nbElements; j++)
//            env.out() << j << ") " << cplex.getValue(e[j]) << endl;
//    }
//    catch (IloException& ex) {
//        cerr << "Error: " << ex << endl;
//    }
//    catch (...) {
//        cerr << "Error" << endl;
//    }
//    env.end();
//    return 0;
//}

int main() {
//    lercsv(caminho.append("arcos_consolidados.csv"));
    flow();
//    blend();
    return 0;
}