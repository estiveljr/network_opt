#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <sstream>
#include <limits>
#include <map>
#include <ilcplex/ilocplex.h>
#include <variant>
#include <algorithm>
#include <stack>
#include <set>

using namespace std;

//caminho sin
//static string caminho = "F:\\OneDrive\\_each\\_Quali\\Artigo\\modelocpp\\";

//caminho wsl
static string caminho = "/mnt/f/OneDrive/_each/_Quali/Artigo/modelocpp/";

//bool DEBUG = 1;
bool DEBUG = true;

enum tipo_grafo{
    original,
    ciclo,
    componente,
};

struct ARCO {
    int index;
    string tipo_de_arco = "";
    string i = ""; //entrada da localidade ou origem da rota
    string j = ""; //saida da localidade ou destino da rota
    string s = ""; // sku
    float a = 0; // CF
    float b = 0 ; // CVL + icmsst + custos_fonecimento
    float c = 0 ; // capacidade do arco
    float m = 0 ; // icms - cred_pres + difal
    float n = 0 ; // icms * (1 - anulacao)
};

struct ARCO_SIMPLES{
    int index;
    string tipo_de_arco = "";
    string i = ""; //entrada da localidade ou origem da rota
    string j = ""; //saida da localidade ou destino da rota
    float a = 0;
    float c= 0;
};

struct VERTICE{
    string vertice;
    string uf;
    string tipo;
    public:
        bool operator==(const VERTICE &v) const{
            if(vertice == v.vertice){
                return true;
            }else{
                return false;
            }
        }
        bool operator<(const VERTICE &v) const{
            if(vertice < v.vertice){
                return true;
            }else{
                return false;
            }
        }
        bool operator!=(const VERTICE &v) const{
            if(vertice != v.vertice){
                return true;
            }else{
                return false;
            }
        }
};

struct DEMANDA{
    string vertice;
    string s;
    string uf;
    string tipo;
    float d;
    float o;
};

float stringtofloat(string s){
    if (s.length() == 0) {
        return std::numeric_limits<float>::quiet_NaN();
    } else {
        return stof(s);
    }
}

vector<ARCO> ler_csv_grafo(const std::string &nome_arquivo) {
    ifstream infile;
    string dados;
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
            grafo[numlinha - 1].index = numlinha - 1; //indice do arco no grafo
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
    grafo.resize(grafo.size() - 1 );

    return grafo;
}
vector<VERTICE> ler_csv_vertices(const std::string &nome_arquivo) {
    ifstream infile;
    string dados;
    vector<VERTICE> vertices_ufs;

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

        VERTICE vertice_uf;
        vertices_ufs.push_back(vertice_uf);
        string dados_vertice[3];
        int item_arco = 0;
        do {
            pos = line.find(delimiter);
            token = line.substr(0, pos);
            dados_vertice[item_arco] = token;
            line.erase(0, pos + delimiter.length());
            item_arco++;
        } while (pos != std::string::npos);

        // ALOCA DADOS DE CADA LINHA DO STREAM NO GRAFO
        if (numlinha != 0) {
            vertices_ufs[numlinha - 1].vertice = dados_vertice[0];
            vertices_ufs[numlinha - 1].uf = dados_vertice[1];
            vertices_ufs[numlinha - 1].tipo = dados_vertice[2];
//            vertices_ufs[numlinha - 1].demanda = stringtofloat(dados_vertice[3]);
//            vertices_ufs[numlinha - 1].fornecimento = stringtofloat(dados_vertice[4]);
        }
        numlinha++;
    }
    infile.close();
    vertices_ufs.resize(vertices_ufs.size() - 1 );

    return vertices_ufs;
}
vector<DEMANDA> ler_csv_demandas(const std::string &nome_arquivo) {
    ifstream infile;
    string dados;
    vector<DEMANDA> demandas;

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

        DEMANDA demanda;
        demandas.push_back(demanda);
        string dados_demandas[6];
        int item_arco = 0;
        do {
            pos = line.find(delimiter);
            token = line.substr(0, pos);
            dados_demandas[item_arco] = token;
            line.erase(0, pos + delimiter.length());
            item_arco++;
        } while (pos != std::string::npos);

        // ALOCA DADOS DE CADA LINHA DO STREAM NO GRAFO
        if (numlinha != 0) {
            demandas[numlinha - 1].vertice = dados_demandas[0];
            demandas[numlinha - 1].s = dados_demandas[1];
            demandas[numlinha - 1].uf = dados_demandas[2];
            demandas[numlinha - 1].tipo = dados_demandas[3];
            demandas[numlinha - 1].d = stringtofloat(dados_demandas[4]);
            demandas[numlinha - 1].o = stringtofloat(dados_demandas[5]);
        }
        numlinha++;
    }
    infile.close();
    demandas.resize(demandas.size() - 1 );

    return demandas;
}

struct GRAFO{
    string nome;
    tipo_grafo tipo;
    vector<ARCO> completo; // arco ijs
    map<vector<string>, ARCO> completo_dic; // retorna struct ARCO a partir da entrada ijs
    map<vector<string>, unsigned int> completo_idx; // retorna indice do arco completo ijs
    vector<ARCO_SIMPLES> simples; //arco ij
    map<vector<string>, ARCO_SIMPLES> simples_dic; // retorna struct ARCO_SIMPLES a partir da entrada ij
    map<vector<string>, unsigned int> simples_idx; // retorna indice do arco simpes ij
    vector<VERTICE> vertices_completo;
    map<string,VERTICE> vertices_completo_dic;
    vector<string> vertices;
    vector<DEMANDA> demandas;
    std::map<vector<string>,DEMANDA> demandas_dic;
    vector<string> produtos;
    vector<GRAFO> subgrafos;
    int qnt_arcos_localidade = 0;
    int qnt_arcos_transporte = 0;
    int qnt_arcos = 0;
    int qnt_arcos_simples = 0;
    int qnt_vertices = 0;
    int qnt_produtos = 0;
    int qnt_localidades = 0;


    // retona todas as origens i de um destino j, sem considerar o sku
    vector<string> orgs_de(string &vertice){
        vector<string> origens;
        for (ARCO_SIMPLES &arco: simples){
            if (vertice == arco.j){
                origens.push_back(arco.i);
            }
        }
        return origens;
    };

    // retona todas as origens i de um destino j, sem considerar o sku
    vector<string> dest_de(string &vertice){
        vector<string> destinos;
        for (ARCO_SIMPLES &arco: simples){
            if (vertice == arco.i){
                destinos.push_back(arco.j);
            }
        }
        return destinos;
    };
    vector<VERTICE> dest_de(VERTICE &vertice){
        vector<VERTICE> destinos;
        for (ARCO_SIMPLES &arco: simples){
            if (vertice.vertice == arco.i){
                destinos.push_back(vertices_completo_dic[arco.j]);
            }
        }
        return destinos;
    };

    // retorna todos os arcos de transporte que possuem destino na UF especificada
    vector<ARCO> entradas_uf(string &uf){
        vector<ARCO> entradas;
        for(VERTICE vertice: vertices_completo){
            if (vertice.uf == uf){
                for (ARCO &arco: completo){
                    if (vertice.vertice == arco.j and arco.tipo_de_arco == "transporte"){
                        entradas.push_back(arco);
                    }
                }
            }
        }
        return entradas;
    }

    static bool encontrar_elemento(string elem, vector<string> lista){
        for (int i = 0; i < (int) lista.size(); ++i) {
            if(elem == lista[i]){
                return true;
            }
        }
        return false;
    }

    explicit GRAFO( tipo_grafo tg = tipo_grafo::original, \
                    const vector<VERTICE>& vertices_comp = {}, \
                    const vector<ARCO>& grafo_comp= {}, \
                    const vector<DEMANDA>& demandas_comp = {}) {
        tipo = tg;
        vertices_completo = vertices_comp;
        completo = grafo_comp;
        demandas = demandas_comp;
        std::map<vector<string>, ARCO> grafo_completo_dic;
        std::map<vector<string>, unsigned int> grafo_completo_idx;
        vector<ARCO_SIMPLES> grafo_simples;
        std::map<vector<string>, ARCO_SIMPLES> grafo_simples_dic;
        std::map<vector<string>, unsigned int> grafo_simples_idx;

        // criar grafo_completo a partir do arquivo de dados
        if(vertices_completo.empty() and completo.empty() and demandas.empty()){
            // dados para dev componentes
            completo = ler_csv_grafo(caminho + "ciclos_arcos.csv");
            vertices_completo = ler_csv_vertices(caminho + "ciclos_vertices.csv");
            demandas = ler_csv_demandas(caminho + "ciclos_demandas.csv");

            // dados para dev ciclos
//            completo = ler_csv_grafo(caminho + "arcos_consolidados.csv");
//            vertices_completo = ler_csv_vertices(caminho + "vertices_completo.csv");
//            demandas = ler_csv_demandas(caminho + "demandas_fornecimento.csv");

            //dados instancia malha
//    grafo_completo = ler_csv_grafo(caminho + "dados_comp_arcos.csv");
//    vertices_completo = ler_csv_vertices(caminho + "dados_comp_vertices.csv");
//    demandas = ler_csv_demandas(caminho + "dados_comp_dem_forn.csv");
        }


        // tranforma o vertices_completo em um dicionário para consulta por nós e salva uma lista com os vértices
        for (VERTICE &vertice : vertices_completo) {
            string key = vertice.vertice;
            vertices_completo_dic[key] = vertice;
            vertices.push_back(vertice.vertice);

        }

        // tranforma "demandas" em um dicionário para consulta por "nó, sku" e salva uma lista com os vértices
        for (int i = 0; i < int(demandas.size()); ++i) {
            DEMANDA demanda = demandas[i];
            vector<string> key = {demanda.vertice, demanda.s};
            demandas_dic[key] = demanda;
//        vertices.push_back(demanda.demanda);

        }
        // tranforma o grafo_completo em um dicionário para consulta por par de nós e sku
        for (unsigned int i = 0; i < completo.size(); ++i) {
            vector<string> key = {completo[i].i, completo[i].j, completo[i].s};
            grafo_completo_dic[key] = completo[i];
            grafo_completo_idx[key] = i;
        }

        // quantidades de cada tipo de arco
        for (int arco = 0; arco < (int) completo.size(); arco++){
            if (completo[arco].tipo_de_arco == "localidade"){
                ++qnt_arcos_localidade;
            }else if (completo[arco].tipo_de_arco == "transporte"){
                ++qnt_arcos_transporte;
            }
        }

        // grafo simples, sem considerar o sku
        int cont_arco_s = 0; // contagem de arcos simples é menor porque precisa nao considera sku
        for (int arco = 0; arco < (int) completo.size(); arco++){
            if (arco == 0){
                ARCO_SIMPLES arco_simples;
                arco_simples.index = cont_arco_s;
                arco_simples.tipo_de_arco = completo[arco].tipo_de_arco;
                arco_simples.i = completo[arco].i ;
                arco_simples.j = completo[arco].j ;
                arco_simples.a = completo[arco].a ;
                arco_simples.c = completo[arco].c ;
                grafo_simples.push_back(arco_simples);
                ++cont_arco_s;
            }else if (!(completo[arco].i == completo[arco - 1].i and completo[arco].j == completo[arco - 1].j)) {
                ARCO_SIMPLES arco_simples;
                arco_simples.index = cont_arco_s;
                arco_simples.tipo_de_arco = completo[arco].tipo_de_arco;
                arco_simples.i = completo[arco].i ;
                arco_simples.j = completo[arco].j ;
                arco_simples.a = completo[arco].a ;
                arco_simples.c = completo[arco].c ;
                grafo_simples.push_back(arco_simples);
                ++cont_arco_s;
            }
        }

        // tranforma o grafo_simples em um dicionário para consulta por par de nós
        for (unsigned int i = 0; i < grafo_simples.size(); ++i) {
            vector<string> key = {grafo_simples[i].i, grafo_simples[i].j};
            grafo_simples_dic[key] = grafo_simples[i];
            grafo_simples_idx[key] = i;
        }

        // encontra quais os produtos "s" presentes no grafo
        for (int arco = 0; arco < (int) completo.size() ; ++arco) {
            string produto = completo[arco].s;
            if (produto == "")continue;
            if(!encontrar_elemento(produto,produtos)){
                produtos.push_back(produto);
                qnt_produtos++;
            }
        }

        // quantidade de localidades
        for (ARCO_SIMPLES arco: grafo_simples){
            if (arco.tipo_de_arco == "localidade") {
                ++qnt_localidades;
            }
        }


        // atribuindo valores ao struct do grafo
        completo_dic = grafo_completo_dic;
        completo_idx = grafo_completo_idx;
        simples = grafo_simples;
        simples_dic = grafo_simples_dic;
        simples_idx = grafo_simples_idx;
        vertices_completo_dic = vertices_completo_dic;
        vertices = vertices;
        produtos = produtos;
        qnt_arcos_transporte = qnt_arcos_transporte;
        qnt_arcos_localidade = qnt_arcos_localidade;
        qnt_arcos = (int) completo.size();
        qnt_localidades = qnt_localidades;
        qnt_arcos_simples = (int) simples.size();
        qnt_vertices = (int) vertices_completo.size();
        qnt_produtos = qnt_produtos;
        demandas_dic = demandas_dic;
    }

    // retona todos os arcos de transporte que possuem origem na UF especificada
    vector<ARCO> saidas_uf(string &uf){
        vector<ARCO> saidas;
        for(VERTICE vertice : vertices_completo){
            if (vertice.uf == uf){
                for (ARCO &arco: completo){
                    if (vertice.vertice == arco.i and arco.tipo_de_arco == "transporte"){
                        saidas.push_back(arco);
                    }
                }
            }
        }
        return saidas;
    }

    //adicionar um subgrafo ao grafo.
    void add_subgrafo(tipo_grafo tg, vector<VERTICE> &sub_vertices){
        vector<ARCO> sub_arcos;
        vector<DEMANDA> sub_demanda;
        for (ARCO a: GRAFO::completo) {
            for (VERTICE u: sub_vertices) {
                for (VERTICE v: sub_vertices) {
                    if (a.i == u.vertice and a.i == v.vertice) {
                        sub_arcos.push_back(a);
                    }
                }
            }
        }
        for (VERTICE u: sub_vertices) {
            for (DEMANDA d: sub_demanda) {
                if (u.vertice == d.vertice) {

                }
            }
        }
        GRAFO subgrafo = GRAFO(tg,sub_vertices,sub_arcos,demandas);
        subgrafos.push_back(subgrafo);
    }

    vector<VERTICE> visited = {};
    stack<VERTICE> finished_stack = {};
    set<VERTICE> finished_set = {};
    vector<VERTICE> assigned = {};
    map<VERTICE, vector<VERTICE>> components = {};

    void visit(VERTICE &u){
        if (find(visited.begin(), visited.end(), u) == visited.end() ) {
            visited.push_back(u);
            for(string v: dest_de(u.vertice)){
                visit(vertices_completo_dic[v]);
                if(find(finished_set.begin(), finished_set.end(),vertices_completo_dic[v]) == finished_set.end()){
                    finished_set.insert(u);
                    finished_stack.push(u);
                }
            }
        }
    }

    void assign(VERTICE &v, VERTICE &root){
        if(find(assigned.begin(), assigned.end(), v) == assigned.end()){
            assigned.push_back(v);
            const VERTICE key = root;
            vector<VERTICE> list_components;
            //regra para atualizar a lista de componentes
            if(root!= components.find(key)->first){ //check if root is empty
                list_components = {v};
            }else{
                list_components = components.find(root)->second;
                list_components.push_back(v);
            }
            const vector<VERTICE> value = list_components;
            components.erase(key);
            components.insert(make_pair(key,value));
            for(string u: orgs_de(v.vertice)) {
                assign(vertices_completo_dic[u], root);
            }
        }
    }

    void find_components() {
//    For each vertex u of the graph, mark u as unvisited. Let L be empty.
//            For each vertex u of the graph do Visit(u), where Visit(u) is the recursive subroutine:
//    If u is unvisited then:
//    Mark u as visited.
//            For each out-neighbour v of u, do Visit(v).
//                Prepend u to L.
//            Otherwise do nothing.
//    For each element u of L in order, do Assign(u,u) where Assign(u,root) is the recursive subroutine:
//      If u has not been assigned to a component then:
//          Assign u as belonging to the component whose root is root.
//            For each in-neighbour v of u, do Assign(v,root).
//                Otherwise do nothing.
        for (VERTICE u: vertices_completo){
            visit(u);
        }
        while(!finished_stack.empty()){
            assign(finished_stack.top(),finished_stack.top());
            finished_stack.pop();
        }

        //cria uma lista de subgrafos a partir dos componentes
        for(pair<const VERTICE, vector<VERTICE>> c: components) {
            if (c.second.size() > 1) {
                vector<VERTICE> sub_vertices = c.second;
                vector<ARCO> sub_arcos;
                vector<DEMANDA> sub_demanda;
                for (ARCO a: GRAFO::completo) {
                    for (VERTICE u: sub_vertices) {
                        for (VERTICE v: sub_vertices) {
                            if (a.i == u.vertice and a.i == v.vertice) {
                                sub_arcos.push_back(a);
                            }
                        }
                    }
                }
                for (VERTICE u: sub_vertices) {
                    for (DEMANDA d: sub_demanda) {
                        if (u.vertice == d.vertice) {

                        }
                    }
                }
                add_subgrafo(tipo_grafo::componente, sub_vertices);
            }
        }
    }

    // inicia algo do cyclos.
    deque<VERTICE> cycle_candidate = {};
    map<string,bool> blocked = {};
    map<string,vector<VERTICE>> blocked_map = {};
    VERTICE s;

    void print_vectors(string msg = "") {
        if(msg != "") cout << "** " << msg << " **" << endl;
        //print do cycle_candidate
        cout << "Candidato: \t[" ;
        for(VERTICE vc: cycle_candidate) cout << vc.vertice << ", ";
        cout << "]" << endl;
        // print blocked
        cout << "Blocked: \t[";
        for(pair<string,bool> p: blocked) {
            if(p.second) cout << p.first << ", ";
        }
        cout << "]" << endl;
        // print blocked_map
        cout << "Block_map: \t[";
        for(const pair<const string,vector<VERTICE>>& p: blocked_map){
//            if(!p.second.empty()){
            if(true){
                cout << "(" << p.first << ": ";
                for(VERTICE sec: p.second){
                    cout << sec.vertice << ", ";
                }
                cout << ") ";
            }
        }
        cout << "]" << endl;
        cout << endl;
    }

    static void print_subg(GRAFO &subg) {//print subgrafo
        cout << "SUBGRAFO: \t[";
        for(VERTICE v: subg.vertices_completo) cout << v.vertice << ",";
        cout << "]" << endl;
    }

    void unblock(VERTICE &v){
        blocked[v.vertice] = false;
        for(VERTICE w: blocked_map[v.vertice]){
           blocked_map.erase(w.vertice);
           if(blocked[w.vertice] == true) unblock(w);
        }
        print_vectors("unblock");
    }

    bool cycle(VERTICE &v){
        bool f = false;
        cycle_candidate.push_back(v);
        blocked[v.vertice] = true;
        print_vectors("novo vertice no candidato");
        // L1
        for(VERTICE &w: dest_de(v)){
            if(w == s){ // s: inicio do caminho. Se w == s => ciclo encontrado.
                vector<VERTICE> vertices_do_ciclo;
                for(VERTICE i: cycle_candidate) vertices_do_ciclo.push_back(i);
                add_subgrafo(tipo_grafo::ciclo, vertices_do_ciclo);
            }else if(!blocked[w.vertice]){
                if(cycle(w)) f = true;
            }
        }
        // L2
        if(f){
            unblock(v);
            print_vectors("unblock");
        }else{
            for(VERTICE w: dest_de(v)){
                for(VERTICE u: blocked_map[w.vertice]){
                    if(u == w) blocked_map[w.vertice].push_back(v);
                    print_vectors("add blocked map");
                }
            }
        }
        cycle_candidate.pop_back();
        print_vectors();
        return f;
    }



    void find_cycles(){
        //todo: retirar ciclos iguais mas que iniciam em vertices diferentes.
        find_components();
        //seleciona apenas os subgrafos do tipo componente.
        vector<GRAFO> componentes;
        for(GRAFO subg: subgrafos) {
            if (subg.tipo == tipo_grafo::componente) componentes.push_back(subg);
        }

        //main loop (while do no artigo)
        for(GRAFO comp: componentes){
            if(comp.tipo != tipo_grafo::componente) return;
            cycle_candidate = {}; //limpar stack.
            s = {};
            for(const VERTICE& vertice: comp.vertices_completo) {
                print_subg(comp);
                s = vertice;
                //print vertice origem s.
                cout << "Inicio: \t" << s.vertice << endl;
                //verifica se o vertice s pertence ao grafo comp, se não pertencer, encerra o main loop
//                bool sg = false;
//                for (VERTICE v: comp.vertices_completo) {
//                    if (s == v) {
//                        sg = true;
//                        break;
//                    }
//                }
//                if(!sg) return;

                for(VERTICE i: comp.vertices_completo){
                    // limpa os vetores antes de iniciar outro circuito
                    blocked[i.vertice] = false;
                    blocked_map[i.vertice] = {};
                }
                print_vectors();
                bool dummy = cycle(s);
                if(dummy){}
            }
        }

    }
};


template<typename T>
void debug(vector<T> &messages){
    if(DEBUG){
        for(T message: messages){
            cout << message << endl;
        }
    }
}

template<typename T>
void debug(T &&message){
    if(DEBUG) {
        cout << message << endl;
    }
}

template<typename Head, typename... Tail>
void debug(Head &&head, Tail&&... tail) {
    if (DEBUG){
        std::cout << head << " ";
        debug(std::forward<Tail>(tail)...);
    }
}


bool encontrar_elemento(string elem, vector<string> lista){
    for (int i = 0; i < (int) lista.size(); ++i) {
        if(elem == lista[i]){
            return true;
        }
    }
    return false;
}

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

//IloInt nbElements, nbRaw, nbScrap, nbIngot;
//IloNum alloy;
//IloNumArray nm, nr, ns, ni, pp, P;
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
//    pp = IloNumArray(env, nbElements, 0.05, 0.30, 0.60);
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
//
//}

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
//            e[j] = IloNumVar(env, pp[j] * alloy, P[j] * alloy);
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

GRAFO criar_grafo(  vector<VERTICE> vertices_completo = {}, \
                    vector<ARCO> grafo_completo = {}, \
                    vector<DEMANDA> demandas = {}) {
    GRAFO grafo;
//    vector<ARCO> grafo_completo;
//    vector<VERTICE> vertices_completo;
//    vector<DEMANDA> demandas;
    std::map<vector<string>, ARCO> grafo_completo_dic;
    std::map<vector<string>, unsigned int> grafo_completo_idx;
    vector<ARCO_SIMPLES> grafo_simples;
    std::map<vector<string>, ARCO_SIMPLES> grafo_simples_dic;
    std::map<vector<string>, unsigned int> grafo_simples_idx;
    std::map<string,VERTICE> vertices_completo_dic;
    vector<string> vertices;
    std::map<vector<string>,DEMANDA> demandas_dic;

    int qnt_arcos_localidade = 0;
    int qnt_arcos_transporte = 0;
    // criar grafo_completo a partir do arquivo de dados
    if(vertices_completo.empty() and grafo_completo.empty() and demandas.empty()){
        vertices_completo = ler_csv_vertices(caminho + "vertices_completo.csv");
        grafo_completo = ler_csv_grafo(caminho + "arcos_consolidados.csv");
        demandas = ler_csv_demandas(caminho + "demandas_fornecimento.csv");
//    vertices_completo = ler_csv_vertices(caminho + "dados_comp_vertices.csv");
//    grafo_completo = ler_csv_grafo(caminho + "dados_comp_arcos.csv");
//    demandas = ler_csv_demandas(caminho + "dados_comp_dem_forn.csv");
    }

    // tranforma o vertices_completo em um dicionário para consulta por nós e salva uma lista com os vértices
    for (VERTICE &vertice : vertices_completo) {
        string key = vertice.vertice;
        vertices_completo_dic[key] = vertice;
        vertices.push_back(vertice.vertice);

    }

    // tranforma "demandas" em um dicionário para consulta por "nó, sku" e salva uma lista com os vértices
    for (int i = 0; i < int(demandas.size()); ++i) {
        DEMANDA demanda = demandas[i];
        vector<string> key = {demanda.vertice, demanda.s};
        demandas_dic[key] = demanda;
//        vertices.push_back(demanda.demanda);

    }
    // tranforma o grafo_completo em um dicionário para consulta por par de nós e sku
    for (unsigned int i = 0; i < grafo_completo.size(); ++i) {
        vector<string> key = {grafo_completo[i].i, grafo_completo[i].j, grafo_completo[i].s};
        grafo_completo_dic[key] = grafo_completo[i];
        grafo_completo_idx[key] = i;
    }

    // quantidades de cada tipo de arco
    for (int arco = 0; arco < (int) grafo_completo.size(); arco++){
        if (grafo_completo[arco].tipo_de_arco == "localidade"){
           ++qnt_arcos_localidade;
        }else if (grafo_completo[arco].tipo_de_arco == "transporte"){
           ++qnt_arcos_transporte;
        }
    }

    // grafo simples, sem considerar o sku
    int cont_arco_s = 0; // contagem de arcos simples é menor porque precisa nao considera sku
    for (int arco = 0; arco < (int) grafo_completo.size(); arco++){
        if (arco == 0){
            ARCO_SIMPLES arco_simples;
            arco_simples.index = cont_arco_s;
            arco_simples.tipo_de_arco = grafo_completo[arco].tipo_de_arco;
            arco_simples.i = grafo_completo[arco].i ;
            arco_simples.j = grafo_completo[arco].j ;
            arco_simples.a = grafo_completo[arco].a ;
            arco_simples.c = grafo_completo[arco].c ;
            grafo_simples.push_back(arco_simples);
            ++cont_arco_s;
        }else if (!(grafo_completo[arco].i == grafo_completo[arco - 1].i and grafo_completo[arco].j == grafo_completo[arco - 1].j)) {
            ARCO_SIMPLES arco_simples;
            arco_simples.index = cont_arco_s;
            arco_simples.tipo_de_arco = grafo_completo[arco].tipo_de_arco;
            arco_simples.i = grafo_completo[arco].i ;
            arco_simples.j = grafo_completo[arco].j ;
            arco_simples.a = grafo_completo[arco].a ;
            arco_simples.c = grafo_completo[arco].c ;
            grafo_simples.push_back(arco_simples);
            ++cont_arco_s;
        }
    }

    // tranforma o grafo_simples em um dicionário para consulta por par de nós
    for (unsigned int i = 0; i < grafo_simples.size(); ++i) {
        vector<string> key = {grafo_simples[i].i, grafo_simples[i].j};
        grafo_simples_dic[key] = grafo_simples[i];
        grafo_simples_idx[key] = i;
    }

    // encontra quais os produtos "s" presentes no grafo
    vector<string> produtos;
    int qnt_produtos = 0;
    for (int arco = 0; arco < (int) grafo_completo.size() ; ++arco) {
        string produto = grafo_completo[arco].s;
        if (produto == "")continue;
        if(!encontrar_elemento(produto,produtos)){
            produtos.push_back(produto);
            qnt_produtos++;
        }
    }

    // quantidade de localidades
    int qnt_localidades = 0;
    for (ARCO_SIMPLES arco: grafo_simples){
       if (arco.tipo_de_arco == "localidade") {
           ++qnt_localidades;
       }
    }


    // atribuindo valores ao struct do grafo
    grafo.completo = grafo_completo;
    grafo.completo_dic = grafo_completo_dic;
    grafo.completo_idx = grafo_completo_idx;
    grafo.simples = grafo_simples;
    grafo.simples_dic = grafo_simples_dic;
    grafo.simples_idx = grafo_simples_idx;
    grafo.vertices_completo = vertices_completo;
    grafo.vertices_completo_dic = vertices_completo_dic;
    grafo.vertices = vertices;
    grafo.produtos = produtos;
    grafo.qnt_arcos_transporte = qnt_arcos_transporte;
    grafo.qnt_arcos_localidade = qnt_arcos_localidade;
    grafo.qnt_arcos = (int) grafo.completo.size();
    grafo.qnt_localidades = qnt_localidades;
    grafo.qnt_arcos_simples = (int) grafo.simples.size();
    grafo.qnt_vertices = (int) grafo.vertices_completo.size();
    grafo.qnt_produtos = qnt_produtos;
    grafo.demandas = demandas;
    grafo.demandas_dic = demandas_dic;
    return grafo;
}

IloNumArray a, b, c, m, n;
IloNumArray p;
vector<string> UFs;
void definir_constantes(IloEnv env, GRAFO grafo) {
    UFs = {"RO", "AC", "AM", "RR", "PA", "AP", "TO", "MA", "PI", "CE", "RN", "PB", "PE", "AL", "SE", \
    "BA", "MG", "ES", "RJ", "SP", "PR", "SC", "RS", "MS", "MT", "GO", "DF"};
    a = IloNumArray(env);
    b = IloNumArray(env);
    c = IloNumArray(env);
    m = IloNumArray(env);
    n = IloNumArray(env);
    for (IloInt arco = 0; arco < (IloInt) grafo.qnt_arcos; arco++) {
        b.add((IloNum) grafo.completo[arco].b);
        m.add((IloNum) grafo.completo[arco].m);
        n.add((IloNum) grafo.completo[arco].n);
    }
    for (int arco = 0; arco < (IloInt) grafo.qnt_arcos_simples; ++arco) {
        if (grafo.simples[arco].tipo_de_arco == "localidade"){
            a.add((IloNum) grafo.simples[arco].a);
            c.add((IloNum) grafo.simples[arco].c);
        }
    }

    //DEBUG
//    IloNumArray lista = n;
//    cout << lista << endl;
//    cout << "tamanho da lista: " << (int) lista.getSize() << endl;
//    cout << "quantidade de arcos: " << (int) qnt_arcos << endl;

}

IloBoolVarArray x;
IloNumVarArray y, z;
map<string,string> definir_variaveis(IloEnv env, GRAFO grafo) {
    x = IloBoolVarArray(env,grafo.qnt_localidades);
    y = IloNumVarArray(env, grafo.qnt_arcos, 0, IloInfinity);
    z = IloNumVarArray(env, 27, 0, IloInfinity);

    //Constroi dicionário para encontrar o nome das vaviráveis no modelo.
    map<string,string> dic_var;
    size_t qnt_var = grafo.qnt_localidades + grafo.qnt_arcos + 27;
    size_t qnt_loc = size_t(grafo.qnt_localidades);
    size_t qnt_arcos = size_t(grafo.qnt_arcos);
    string key;
    string value;
    size_t idx_loc = 0;
    size_t idx_arc = 0;
    size_t idx_uf = 0;
    for (size_t i = 0; i < qnt_var; ++i) {
        if(i < qnt_loc){
            idx_loc = i;
            string vertice = grafo.vertices[idx_loc];
            key = "IloBoolVar(" + to_string(i) + ")";
//            value = "x_(" + to_string(i) + ")";
            value = "x_(" + vertice + ")";
        }else if(i < qnt_arcos + qnt_loc){
            idx_arc = i - qnt_loc;
            string arco = grafo.completo[idx_arc].i;
            arco += ", " + grafo.completo[idx_arc].j;
            arco += ", " + grafo.completo[idx_arc].s;
            key = "IloNumVar(" + to_string(i) + ")";
            value = "y_(" + arco + ")";
        }else{
            idx_uf = i - qnt_loc - qnt_arcos;
//            string uf = grafo.completo[idx_uf].i;
            string uf = UFs[idx_uf];
            key = "IloNumVar(" + to_string(i) + ")";
            value = "z_(" + uf + ")";
        }
//        string key = "IloNumVar(" + to_string(i) + ")";
//        string value = "fluxo(" + to_string(i) + ")";
        dic_var.insert(pair<string,string>(key, value));
    }
    return dic_var;
}

vector<string> split(string str, string separator, vector<string> splited_str = {}){
    int sep_size = separator.length();
    size_t pos = str.find(separator);
    string part;

    if(pos == string::npos){
        splited_str.push_back(str);
        return splited_str;
    }

    for(size_t i = 0; i < pos ; i++){
        part.push_back(str[i]);
    }

    splited_str.push_back(part);
    str.erase(0,pos + sep_size);
    splited_str = split(str,separator,splited_str); //recorrencia para continuar a busca
    return splited_str;
}

string replace_vars(string str, map<string,string> dic_var = {}){
    string replaced_vars;
    string sep_start = "IloNumVar";
    string sep_end= ")";
    size_t pos_start = str.find(sep_start);
    size_t pos_end = str.find(sep_end);
    while ((pos_start != string::npos) & (pos_end != string::npos)){
        size_t word_len = pos_end - pos_start + 1;
        string word = str.substr(pos_start,word_len);
        string nome_var = dic_var[word];
        if (nome_var != "") str.replace(pos_start, word_len, nome_var);
        //muda para a próxima palavra
        pos_start = str.find(sep_start,pos_end);
        pos_end = str.find(sep_end,pos_end+1);
    }
        return str;
}

string refiner(IloEnv &env, IloModel model, IloCplex cplex,
               bool print_problem = false, bool print_conflict = false){
    stringstream conflict_ss; // retorno do conflito para ser tratado fora da funcao
    try {
//        if (argc != 2)
//            usage(argv[0]);

        // Create the model and CPLEX objects.
//        IloModel model(env);
//        IloCplex cplex(env);

        // Read model from file 1ith name args[0] into cplex optimizer object.
//        cplex.importModel(model, argv[1]);
//        cplex.extract(model);

        // A list of constraints to be considered by the conflict refiner.
        IloConstraintArray constraints(env);

        // Loop over all objects in the model and gather constraints.
        for (IloModel::Iterator it(model); it.ok(); ++it) {
            IloExtractable ext = *it;
            if (ext.isVariable()) {
                IloNumVar v = ext.asVariable();
                // Add variable bounds to the constraints array.
                constraints.add(IloBound(v, IloBound::Lower));
                constraints.add(IloBound(v, IloBound::Upper));
            }
            else if (ext.isConstraint()) {
                IloConstraint c = ext.asConstraint();
                constraints.add(c);
            }
        }

        // Define preferences for the constraints. Here, we give all
        // constraints a preference of 1.0, so they will be treated
        // equally.
        IloNumArray prefs(env, constraints.getSize());
        for (int i = 0; i < prefs.getSize(); ++i)
            prefs[i] = 1.0;

        // Run the conflict refiner. As opposed to letting the conflict
        // refiner run to completion (as is done here), the user can set
        // a resource limit (e.g., a time limit, an iteration limit, or
        // node limit) and still potentially get a "possible" conflict.
        if (cplex.refineConflict(constraints, prefs)) {
            // Display the solution status.
            IloCplex::CplexStatus status = cplex.getCplexStatus();
            cout << "Solution status = " << status << " (" <<
                 static_cast<int>(status) << ")" << endl;

            // Get the conflict status for the constraints that were specified.
            IloCplex::ConflictStatusArray conflict = cplex.getConflict(constraints);

            // Print constraints that participate in the conflict.
            cout << "Conflict:" << endl;

            for (int i = 0; i < constraints.getSize(); ++i) {
                if (conflict[i] == IloCplex::ConflictMember ||
                    conflict[i] == IloCplex::ConflictPossibleMember) {
                    if(print_conflict){
                        cout << "  " << constraints[i] << endl;
                    }
                    conflict_ss << constraints[i] << endl;
                }
            }

            cout << endl;

            // Write the identified conflict in the LP format.
            const char *confFile = "iloconflictex1.lp";
            cout << "Writing conflict file to '" << confFile << "'...." << endl << endl;
            cplex.writeConflict(confFile);


            // Display the entire conflict subproblem.
            if(print_problem){
                string line;
                ifstream file(confFile);
                if (file.is_open()) {
                    while (getline(file, line)) {
                        cout << line << endl;
                    }
                }
                file.close();
            }
        }
        else {
            cout << "A conflict was not identified." << endl;
            cout << "Exiting...." << endl;
            return "-1"; // sem conflito encontrado.
        }

    }
    catch(IloException& e) {
        cerr << "Concert exception caught" << endl;
        throw;
    }
    catch(...) {
        cerr << "Unknown exception caught" << endl;
        throw;
    }
    return conflict_ss.str();
//    env.end();
}

int flow(){
    IloEnv env;
    GRAFO grafo = GRAFO();
//    grafo.find_components();
    grafo.find_cycles();

    cout << "COMPONENTES" << endl;
    for(GRAFO g: grafo.subgrafos){
        if(g.tipo == tipo_grafo::componente){
            for(string v: g.vertices){
                cout << v << " \t ";
            }
            cout << "" << endl;
        }
    }

    cout << endl << "CICLOS" << endl;
    for(GRAFO g: grafo.subgrafos){
        if(g.tipo == tipo_grafo::ciclo){
            cout << "" << endl;
            for(string v: g.vertices){
                cout << v << " \t ";
            }
        }
    }
    cout << endl ;

    // DEBUG
//    find_components(grafo);
//    for(pair<const VERTICE, vector<VERTICE>> c: components){
//        cout << "" << endl;
//        cout << "ROOT: " << c.first.vertice << endl;
//        for(VERTICE v: c.second){
//            cout << v.vertice << " \t ";
//        }
//    }
//    cout << endl ;

    try {
        definir_constantes(env, grafo);
        map<string,string> dic_var;
        dic_var = definir_variaveis(env, grafo);

        IloModel model(env);
        // constantes para as variáveis auxiliares z
        IloIntArray ones = IloIntArray(env);
        for (int i = 0; i < 27 ; i++){
            ones.add(1);
        }

        //DEBUG
        debug("a: ", to_string(a.getSize()));
        debug("x: ", to_string(x.getSize()));
        debug("b: ", to_string(b.getSize()));
        debug("y: ", to_string(y.getSize()));

        // Objective Function: Minimize Cost
        model.add(IloMinimize(env, IloScalProd(a, x) + IloScalProd(b, y) +
                                   IloScalProd(ones,z) ));


        // retrição de balanço de massa
        for (VERTICE vertice: grafo.vertices_completo) {
            string j = vertice.vertice;
            DEBUG = false;
            debug("");
            debug("VERTICE: ", j);
            debug(" tipo: ", vertice.tipo); //debug do tipo de vertice
            for (string s: grafo.produtos) {
                debug(" PRODUTO: ", s);
                auto y_entradas = IloNumVarArray(env);
                auto y_saidas = IloNumVarArray(env);
                debug("  ENTRADAS");
                for (string i: grafo.orgs_de(j)) {
                    debug("   ",i,s);
                    y_entradas.add(y[grafo.completo_idx[{i, j, s}]]);
                }
                debug("  SAIDAS");
                for (string i: grafo.dest_de(j)) {
                    debug("   ",i,s);
                    y_saidas.add(y[grafo.completo_idx[{j, i, s}]]);
                }

//                debug("entradas -> " ,j ,s ,": ", y_entradas); //debug dos vertices de entrada
//                debug("saidas -> "   ,j ,s ,": ", y_saidas); //debug dos vertices de saida

                if (vertice.tipo == "passagem") {
                    model.add(IloSum(y_entradas) - IloSum(y_saidas) == 0);
                }else if (vertice.tipo == "origem"){
                    model.add(IloSum(y_entradas) - IloSum(y_saidas) == -grafo.demandas_dic[{j,s}].o);
                }else if (vertice.tipo == "demanda"){
                    model.add(IloSum(y_entradas) - IloSum(y_saidas) == grafo.demandas_dic[{j,s}].d);
                }else{
                   throw invalid_argument("vertice não possui um dos 3 tipos permitidos (passagem, origem, demanda)");
                }

            }
        }

        // restrição de capacidade e alocação de custo fixo
        for(ARCO_SIMPLES arco : grafo.simples){
            if(arco.tipo_de_arco == "localidade"){
                string i = arco.i;
                string j = arco.j;
                IloNumVarArray y_sum = IloNumVarArray(env);
                for (string s : grafo.produtos){
                    int index = grafo.completo_idx[{i,j,s}];
                    y_sum.add(y[index]);
                }
                float c = arco.c;
                c = 100000;
                model.add(IloSum(y_sum) <= c*x[arco.index]);
            }
        }

        model.add(IloSum(x) <= 26); // teste de restricao para abertura de cds

        // restrição para o custo do saldo de icms
        for(int u = 0; u < 27 ; u++){
            string uf = UFs[u];
            vector<ARCO> entradas_uf = grafo.entradas_uf(uf);
            vector<ARCO> saidas_uf = grafo.saidas_uf(uf);
            IloExpr deb(env);
            IloExpr cred(env);
            for (ARCO arco : entradas_uf){
                int index = arco.index;
                deb += (y[index]*m[index]);
//                cout << index << endl;

            }
            for (ARCO arco : entradas_uf){
                int index = arco.index;
                cred += (y[index]*n[index]);
            }
            model.add(z[u] >= deb - cred);
        }

//        string uf = UFs[0];
//        cout << grafo.entradas_uf(uf)[0].index << endl;
//        cout << grafo.saidas_uf(uf)[0].index << endl;

        // Optimize
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());
        cplex.setWarning(env.getNullStream());
        cplex.solve();

        if (cplex.getStatus() == IloAlgorithm::Infeasible){
            env.out() << "No Solution" << endl;
            cout << "POSSÍVEL CONFLITO:" << endl;
            string conflict = refiner(env,model,cplex,false,false);
//            cout << conflict << endl;
            cout << replace_vars(conflict,dic_var) << endl;
        }

        env.out() << "Solution status: " << cplex.getStatus() << endl;

        // Print results
        env.out() << "Cost:" << cplex.getObjValue() << endl;

        env.out() << endl << "LOCALIDADES" << endl;
        int indice_localidade = 0;
        for(ARCO_SIMPLES arco : grafo.simples){
            if(arco.tipo_de_arco == "localidade"){
                env.out() << arco.i + " -> " + arco.j + " (cap: " << + arco.c <<  ")" \
                << " (UF: " << grafo.vertices_completo_dic[arco.i].uf << ": "\
                << cplex.getValue(x[indice_localidade]) \
                << endl;
                ++indice_localidade;
            }
        }

        env.out() << endl << "SALDOS DE UF" << endl;
        for(int u = 1; u < 27; u++){
           env.out() << UFs[u] << " -> " << cplex.getValue(z[u]) << endl;
        }

    }
    catch (IloException& ex) {
        cerr << "Error: " << ex << endl;
    }
//    catch (...) {
//        cerr << "Error" << endl;
//    }
    env.end();
    return 0;
}

int main() {
    flow();

//    DEBUG
//    string str = " -1 * IloNumVar(37)[0 .. inf]  == -0 == 10";
//    string separator = "==";
//    vector<string> partes = split(str,separator);
//    cout << "primeira parte na main: " << partes[0] << endl;
//    cout << "segunda parte na main: " << partes[1] << endl;
//    cout << "segunda parte na main: " << partes[2] << endl;

//    blend();

//    int a = 1;
//    string b = "banana";
//    float c = 0.5;
//    string sp = " :  ";
//
//    vector<variant<int, float, string, IloNumArray, IloNumVarArray>> values = {a,b,c};
//    vector<void*> pointers = {&a, &b, &c};
//    variant<string, int, float> mutante = b;
//    cout << get<int>(mutante) << endl;

//    cout << a << sp << &a << sp << (long) &a << sp << pointers[0] << sp << values[0] << endl;
//    cout << b << sp << &b << sp << (long) &b << sp << pointers[1] << sp << values[1] << endl;
//    cout << c << sp << &c << sp << (long) &c << sp << pointers[2] << sp << values[2] << endl;


    return 0;
}