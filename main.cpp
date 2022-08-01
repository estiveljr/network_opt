#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <sstream>
#include <limits>
#include <map>
#include <ilcplex/ilocplex.h>
#include <ilcplex/ilocplexi.h>
#include <variant>
#include <algorithm>
#include <stack>
#include <set>
#include <cmath>
#include <filesystem>
#include <chrono>

using namespace std;

//caminho sin
//static string caminho = "F:\\OneDrive\\_each\\_Quali\\Artigo\\modelocpp\\";

//caminho wsl
//static string caminho = "/mnt/f/OneDrive/_each/_Quali/artigo/";
static string caminho = filesystem::current_path().string() + "/";

bool DEBUG = false;

auto start_time = chrono::high_resolution_clock::now();
auto last_time = start_time;
void print_running_time(string msg = ""){
    auto now= chrono::high_resolution_clock::now();
    auto running = chrono::duration_cast<chrono::seconds>(now - last_time);
    auto running_m = running.count()/60;
    auto running_s = running.count()%60;
    auto total = chrono::duration_cast<chrono::seconds>(now - start_time);
    auto total_m = total.count()/60;
    auto total_s = total.count()%60;

    cout << endl;
    if (msg != "") cout << msg << endl;
    cout << "Running time: \t" << running_m << "m" << running_s << "s\t";
    cout << "Total time: \t" << total_m << "m" << total_s << "s\t" << endl;
    cout << "---------------------------------------------" << endl;
    last_time = now;
}

enum tipo_grafo{
    original,
    ciclo,
    componente,
    dummy,
};

struct ARCO {
    int index;
    string tipo_de_arco;
    string i; //entrada da location ou origem da rota
    string j; //saida da location ou destino da rota
    string s; // sku
    float a = 0; // CF
    float b = 0 ; // CVL + icmsst + custos_fonecimento
    float c = 0 ; // capacidade do arco
    float m = 0 ; // icms - cred_pres + difal
    float n = 0 ; // icms * (1 - anulacao)
    float v = 0 ; // volume do fluxo (usado no baseline)
    IloBoolVar* w_ptr = nullptr; // initialize pointer for w_ptr model var
    IloNumVar* y_ptr = nullptr; // initialize pointer for y_ptr model var
    //nome do arco
    string name(){
       return "(" + tipo_de_arco.substr(0,5)+ ": "  + this->i + "->" + j + "," + s + ")";
    }
};

struct ARCO_SIMPLES{
    int index = 0;
    string tipo_de_arco;
    string i; //entrada da location ou origem da rota
    string j; //saida da location ou destino da rota
    float a = 0;
    float c = 0;
    IloBoolVar *x_ptr = nullptr; //initializer null pointer for model var.
//    IloBoolVar &x = *x_ptr; // cant be used because the reference.
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
    double d;
    double o;
};


float stringtofloat(std::string s){
    if (s.length() == 0) {
        return std::numeric_limits<float>::quiet_NaN();
    } else {
        if(s.find(",") < 1e10) s.replace(s.find(","),1,".");
        return stof(s);
    }
}

vector<ARCO> ler_csv_grafo(const std::string &nome_arquivo, const std::string delimiter = ";") {
    ifstream infile;
    string dados;
    vector<ARCO> grafo; // cria um array com elemente do tipo ARCO, formando um grafo_pai orientado.

    infile.open(nome_arquivo);
    if (!infile) {
        cerr << "Erro ao abrir o arquivo";
        exit(1);
    }
    std::string line;
    int numlinha = 0;
    while (infile >> line) {
        size_t pos = 0;
        std::string token;

        ARCO arco;
        grafo.push_back(arco);
        string dados_arco[10];

        //read columns
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
            grafo[numlinha - 1].index = numlinha - 1; //indice do arco no grafo_pai
            grafo[numlinha - 1].tipo_de_arco = dados_arco[0];
            grafo[numlinha - 1].i = dados_arco[1];
            grafo[numlinha - 1].j = dados_arco[2];
            grafo[numlinha - 1].s = dados_arco[3];
            grafo[numlinha - 1].a = stringtofloat(dados_arco[4]);
            grafo[numlinha - 1].b = stringtofloat(dados_arco[5]);
            grafo[numlinha - 1].c = stringtofloat(dados_arco[6]);
            grafo[numlinha - 1].m = stringtofloat(dados_arco[7]);
            grafo[numlinha - 1].n = stringtofloat(dados_arco[8]);
            grafo[numlinha - 1].v = stringtofloat(dados_arco[9]);
        }
        numlinha++;
    }
    infile.close();
    grafo.resize(grafo.size() - 1 );

    return grafo;
}
vector<VERTICE> ler_csv_vertices(const std::string &nome_arquivo, std::string delimiter = ";") {
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
vector<DEMANDA> ler_csv_demandas(const std::string &nome_arquivo, std::string delimiter = ";") {
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
    private:
    void find_components(map<int,IloNum> arcs_vol);

    public:
    void find_cycles(map<int,IloNum> arcs, bool print_cycles, bool alocated_arcs);

    string nome;
    tipo_grafo tipo;
    vector<ARCO> completo; // arco ijs
    map<vector<string>, ARCO> completo_dic; // retorna struct ARCO a partir da entrada ijs
    map<vector<string>, unsigned int> completo_idx; // retorna indice do arco completo ijs
    vector<ARCO_SIMPLES> simples; //arco ij
    vector<ARCO_SIMPLES> simples_loc; //arco ij para arcos do tipo locadalide
    map<vector<string>, ARCO_SIMPLES> simples_dic; // retorna struct ARCO_SIMPLES a partir da entrada ij
    map<vector<string>, unsigned int> simples_idx; // retorna indice do arco simpes ij
    vector<VERTICE> vertices_completo;
    map<string,VERTICE> vertices_completo_dic;
    vector<string> vertices;
    vector<DEMANDA> demandas;
    std::map<vector<string>,DEMANDA> demandas_dic;
    vector<string> produtos;
    vector<GRAFO> subgrafos;
    float bigM = 1e12;
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

    // retorna todos os arcos de transportation que possuem destino na UF especificada,
    vector<ARCO> entradas_uf(string &uf){
        vector<ARCO> entradas;
        for(VERTICE& vertice: vertices_completo){
            if (vertice.uf == uf){
                for (ARCO &arco: completo){
                    if (vertice.vertice == arco.j and arco.tipo_de_arco == "transportation"){
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


    explicit GRAFO( tipo_grafo tg = tipo_grafo::original,
                    const vector<VERTICE>& vertices_comp = {},
                    const vector<ARCO>& grafo_comp= {},
                    const vector<DEMANDA>& demandas_comp = {});
    explicit GRAFO( const string& data_name,
                    tipo_grafo tg = tipo_grafo::original,
                    const vector<VERTICE>& vertices_comp = {},
                    const vector<ARCO>& grafo_comp= {},
                    const vector<DEMANDA>& demandas_comp = {});// end of struct GRAFO

    // retona todos os arcos de transportation que possuem origem na UF especificada,
    vector<ARCO> saidas_uf(string &uf){
        vector<ARCO> saidas;
        for(VERTICE& vertice : vertices_completo){
            if (vertice.uf == uf){
                for (ARCO &arco: completo){
                    if (vertice.vertice == arco.i and arco.tipo_de_arco == "transportation"){
                        saidas.push_back(arco);
                    }
                }
            }
        }
        return saidas;
    }

    //adicionar um subgrafo ao grafo_pai.
    void add_subgrafo(tipo_grafo tg, vector<VERTICE> &sub_vertices){
        // verifica se o subgrafo já existe.
        for(auto sub : this->subgrafos){
            if(tg == sub.tipo){
                for(auto va : sub.vertices) {
                    bool vaInB = false;
                    for (auto vb: sub_vertices) {
                        if (va == vb.vertice) {
                            vaInB = true;
                        }
                    }
                    if (vaInB) {
                        return;
                    }
                }
            }
        }
        vector<ARCO> sub_arcos;
        vector<DEMANDA> sub_demandas;
        for (ARCO a: GRAFO::completo) {
            for (VERTICE u: sub_vertices) {
                for (VERTICE v: sub_vertices) {
                    if (a.i == u.vertice and a.j == v.vertice) {
                        sub_arcos.push_back(a);
                    }
                }
            }
        }
        for (VERTICE u: sub_vertices) {
            for (DEMANDA d: GRAFO::demandas) {
                if (u.vertice == d.vertice) {
                    sub_demandas.push_back(d);
                }
            }
        }
        GRAFO subgrafo = GRAFO("subgrafo",tg, sub_vertices, sub_arcos, sub_demandas);
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
                if(find(finished_set.begin(),
                        finished_set.end(),
                        vertices_completo_dic[v]) == finished_set.end()){
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

    // inicia algo de ciclos.
    deque<VERTICE> cycle_candidate = {};
    map<string,bool> blocked = {};
    map<string,vector<VERTICE>> blocked_map = {};
    VERTICE s;

    void print_vectors(string msg = "") {
        return;
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
        return;
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
            if(w == s){ // s: inicio do caminho. Se w_ptr == s => ciclo encontrado.
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

};

void GRAFO::find_cycles(map<int,IloNum> arcs, bool print_cycles, bool alocated_arcs) {
    //todo: retirar ciclos iguais mas que iniciam em vertices diferentes.
    //seleciona apenas os subgrafos do tipo componente.

    this->find_components(arcs);
    vector<GRAFO> componentes;
    for(GRAFO& subg: this->subgrafos) {
        if (subg.tipo == tipo_grafo::componente) componentes.push_back(subg);
    }

    //main loop (while...do no artigo)
    for(GRAFO comp: componentes){
        if(comp.tipo != tipo_grafo::componente) return;
        cycle_candidate = {}; //limpar stack.
        s = {};
        for(const VERTICE& vertice: comp.vertices_completo) {
            print_subg(comp);
            s = vertice;
            //print vertice origem s.
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
    if(print_cycles){
        cout << endl << "COMPONENTES" << endl;
        for (GRAFO subg: this->subgrafos) {
            if (subg.tipo == tipo_grafo::componente) {
                for (string v: subg.vertices) {
                    cout << v << " \t ";
                }
                cout << "" << endl;
            }
        }
        cout << endl << "CICLOS" << endl;
        for (GRAFO g: this->subgrafos) {
            if (g.tipo == tipo_grafo::ciclo) {
                for (string v: g.vertices) {
                    cout << v << " \t ";
                }
                cout << endl ;
            }
        }
    } // end print cycles

}

void GRAFO::find_components(map<int,IloNum> arcs_vol) {
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

    //Filtra arcos e vértices com volume com volume
    vector<ARCO> arcos_com_volume;
    std::set<VERTICE> vertices_com_volume;
    for (ARCO& a: completo){
        if(arcs_vol[a.index] > 0){ //define o limitar do volume para considerar um ciclo
            arcos_com_volume.push_back(a);
            for (VERTICE& u: vertices_completo){
                if(u.vertice == a.i or u.vertice == a.j) {
                    vertices_com_volume.insert(u);
                }
            }
        }
    }

    // iniciar algo para encontrar componentes
    for (VERTICE u: vertices_com_volume){
        visit(u);
    }

    // adicionar componentes ao grafo
    while(!finished_stack.empty()){
        assign(finished_stack.top(),finished_stack.top());
        finished_stack.pop();
    }

    //remove componentes duplicados
    for(auto comp_a : this->components){
        for(auto comp_b : this->components) {
//            auto mapPosA = this->components.find(comp_a.first);
//            auto mapPosB = this->components.find(comp_b.first);
            if(comp_a.first == comp_b.first) break;
            for (VERTICE a: comp_a.second) {
                bool AinB = false;
                for (VERTICE b: comp_b.second) {
                    if (a.vertice == b.vertice) {
                        AinB = true;
                        break;
                    }
                }
                if(!AinB){
                    break;
                }
            }
            auto k = find(this->components.begin(), this->components.end(),comp_b);
            this->components.erase(k);
        }
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

GRAFO::GRAFO(tipo_grafo tg,
             const vector<VERTICE> &vertices_comp,
             const vector<ARCO> &grafo_comp,
             const vector<DEMANDA> &demandas_comp) {
    string data_name;
    GRAFO(data_name, tg, vertices_comp, grafo_comp, demandas_comp);
}

GRAFO::GRAFO(const string &data_name,
             tipo_grafo tg,
             const vector<VERTICE> &vertices_comp,
             const vector<ARCO> &grafo_comp,
             const vector<DEMANDA> &demandas_comp) {
    nome = data_name;
    tipo = tg;
    if (tg == tipo_grafo::dummy) {
        return;
    }

    // defines the file name, if it's an original graph
    string d_name;
    if (tg == tipo_grafo::original){
        d_name = data_name;
    }else{
        d_name = data_name;
    }
    vertices_completo = vertices_comp;
    completo = grafo_comp;
    demandas = demandas_comp;
    std::map<vector<string>, ARCO> grafo_completo_dic;
    std::map<vector<string>, unsigned int> grafo_completo_idx;
    vector<ARCO_SIMPLES> grafo_simples; // i: products not consider
    vector<ARCO_SIMPLES> grafo_simples_loc; // ij apenas para location
    std::map<vector<string>, ARCO_SIMPLES> grafo_simples_dic;
    std::map<vector<string>, unsigned int> grafo_simples_idx;


    //define caminhos de leitura
    if(tg == tipo_grafo::original){
        string data_folder = caminho +  "dados/" + d_name;
        string path_arcos = data_folder + "arcos.csv";
        string path_vertices = data_folder + "vertices.csv";
        string path_dem_forn = data_folder + "dem_forn.csv";
        // verifica se todos os arquivos estão acessíveis
        bool files_exists = filesystem::exists(path_arcos) and
                            filesystem::exists(path_vertices) and
                            filesystem::exists(path_dem_forn);
        if(!files_exists) {
            cerr << "Bases de dados não encontradas" << endl;
            throw exception();
        }
        // cria grafo_completo a partir do arquivo de dados
        if(vertices_completo.empty() and completo.empty() and demandas.empty()){
            // ler dados da pasta de build caso não tenha sido passado nenhum parâmetro
            completo = ler_csv_grafo(path_arcos);
            vertices_completo = ler_csv_vertices(path_vertices);
            demandas = ler_csv_demandas(path_dem_forn);
        }
    }

    //calcula bigM em função da demanda total do grafo_pai
    bigM = 0;
    if(!demandas.empty()){
        for(DEMANDA & demanda : demandas){
            bigM += demanda.d;
        }
        bigM = bigM * 1.1;
    }else{
        bigM = pow(10,10);
    }

    // tranforma o vertices_completo em um dicionário para consulta por nós e salva uma lista com os vértices
    for (VERTICE &vertice : vertices_completo) {
        string key = vertice.vertice;
        vertices_completo_dic[key] = vertice;
        vertices.push_back(vertice.vertice);

    }

    // tranforma "demandas" em um dicionário para consulta por "nó, sku" e salva uma lista com os vértices
    for (const auto& demanda : demandas) {
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
    for (auto & arco : completo){
        if (arco.tipo_de_arco == "location"){
            ++qnt_arcos_localidade;
        }else if (arco.tipo_de_arco == "transportation"){
            ++qnt_arcos_transporte;
        }
    }

    // grafo_pai simples, sem considerar o sku
    // e grafo_pai simples loc (apenas de localidades)
    int cont_arco_s = 0; // contagem de arcos simples é menor porque precisa nao considerar sku
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
            if (arco_simples.tipo_de_arco == "location")
                grafo_simples_loc.push_back(arco_simples);
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
            if (arco_simples.tipo_de_arco == "location")
                grafo_simples_loc.push_back(arco_simples);
            ++cont_arco_s;
        }
    }

    // tranforma o grafo_simples em um dicionário para consulta por par de nós
    for (unsigned int i = 0; i < grafo_simples.size(); ++i) {
        vector<string> key = {grafo_simples[i].i, grafo_simples[i].j};
        grafo_simples_dic[key] = grafo_simples[i];
        grafo_simples_idx[key] = i;
    }

    // encontra quais os produtos "s" presentes no grafo_pai
    for (auto & arco : completo) {
        string produto = arco.s;
        if (produto.empty())continue;
        if(!encontrar_elemento(produto,produtos)){
            produtos.push_back(produto);
            qnt_produtos++;
        }
    }

    // quantidade de localidades
    for (ARCO_SIMPLES & arco: grafo_simples){
        if (arco.tipo_de_arco == "location") {
            ++qnt_localidades;
        }
    }


    // atribuindo valores ao struct do grafo_pai
    completo_dic = grafo_completo_dic;
    completo_idx = grafo_completo_idx;
    simples = grafo_simples;
    simples_loc = grafo_simples_loc;
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
        if (grafo.simples[arco].tipo_de_arco == "location"){
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

IloBoolVarArray x,w;
IloNumVarArray y, z;
auto definir_variaveis(IloEnv env, GRAFO *grafo) {
    // The order matters for the variable declaration in cplex.
    // The index var is by the order that variables are created.
    x = IloBoolVarArray(env,grafo->qnt_localidades);
    w = IloBoolVarArray(env,grafo->qnt_arcos);
    y = IloNumVarArray(env, grafo->qnt_arcos, 0, IloInfinity);
    z = IloNumVarArray(env, 27, 0, IloInfinity);

    // Defines names that will appear in model.lp file
    x.setNames("X");
    w.setNames("W");
    y.setNames("Y");
    z.setNames("Z");


    //Constroi dicionário para encontrar o nome das variáveis no modelo.
    map<string,string> dic_var;
    map<string,string> dic_var_modelo;
    struct ret_values{
        map<string,string> dic_var, dic_var_modelo;
    };
    size_t qnt_var = grafo->qnt_localidades + (grafo->qnt_arcos * 2) + 27;
    size_t qnt_loc = size_t(grafo->qnt_localidades);
    size_t qnt_arcos = size_t(grafo->qnt_arcos);
    string key;
    string value;
    long idx_x= 0;
    long idx_w = 0;
    long idx_y = 0;
    long idx_z = 0;
    for (size_t i = 0; i < qnt_var; ++i) {
        if(i < qnt_loc){
            string loc_simples = grafo->simples_loc[idx_x].i;
            loc_simples += ", " + grafo->simples_loc[idx_x].j;
            key = "IloBoolVar(" + to_string(i) + ")";
            value = "x_(" + loc_simples + ")";
            grafo->simples_loc[idx_x].x_ptr = &x[idx_x]; //set model var index
            grafo->simples[grafo->simples_loc[idx_x].index].x_ptr = &x[idx_x]; //set model var index
            idx_x++;
        }else if(i < qnt_arcos + qnt_loc){
            string arco = grafo->completo[idx_w].i;
            arco += ", " + grafo->completo[idx_w].j;
            arco += ", " + grafo->completo[idx_w].s;
            key = "IloBoolVar(" + to_string(i) + ")";
            value = "w_(" + arco + ")";
            grafo->completo[idx_w].w_ptr = &w[idx_w];
            idx_w++;
        }else if(i < (qnt_arcos * 2) + qnt_loc){
            string arco = grafo->completo[idx_y].i;
            arco += ", " + grafo->completo[idx_y].j;
            arco += ", " + grafo->completo[idx_y].s;
            key = "IloNumVar(" + to_string(i) + ")";
            value = "y_(" + arco + ")";
            grafo->completo[idx_y].y_ptr = &y[idx_y];
            idx_y++;
        }else{
            string uf = UFs[idx_z];
            key = "IloNumVar(" + to_string(i) + ")";
            value = "z_(" + uf + ")";
            idx_z++;
        }
        dic_var.insert(pair<string,string>(key, value));
        dic_var_modelo.insert(pair<string,string>("x" + to_string(i + 1), value));
    }

    // salva de/para com o nome das variáveis em arquivo.
    ofstream de_para_vars("dados/debug/de_para_vars.txt");
    // bool vars
    for (size_t idx = 0; idx < qnt_loc + qnt_arcos; idx++){
        string vkey = "IloBoolVar(" + to_string(idx) + ")";
//        cout << vkey << " -> " << dic_var[vkey] <<endl;
        if(de_para_vars) de_para_vars << vkey << " -> " << dic_var[vkey] <<endl;
    }
    // num vars
    for (size_t idx = qnt_loc + qnt_arcos; idx < dic_var.size(); idx++){
        string vkey = "IloNumVar(" + to_string(idx) + ")";
//        cout << vkey << " -> " << dic_var[vkey] <<endl;
        if(de_para_vars) de_para_vars << vkey << " -> " << dic_var[vkey] <<endl;
    }
    de_para_vars.close();

    return ret_values{dic_var, dic_var_modelo};
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

GRAFO grafo_pai = GRAFO(tipo_grafo::dummy);


ILOLAZYCONSTRAINTCALLBACK1(CycleElimitation,GRAFO&, grafo){
    // restição de ciclos
    /*
     * CC: conjunto de ciclos
     * C: conjunto de vertices que representa um ciclo
     * |C|: cardinalidade de C
     * w_ijs = {0,1 | ij is in ARCOS, s is in Produtos }
     * w_ijs = binário que indica o se um determinado arco está sendo usado
     *
     * FO: y_ijs <= M * w_ijs  // liga o binário caso o fluxo y_ijs seja utilizado
     * s.a. : sum[w_ijs in C](w_ijs) < |C| forall(C in CC)
     *
    */

    // Encontra ciclos no modelo

    cout << "\n ** REMOVENDO CIRCUITOS **" << endl;
    map<int,IloNum> y_values;
    for(ARCO& arco : grafo.completo){
        y_values.insert(pair<int,IloNum>(arco.index,getValue(*arco.y_ptr)));
    }
    grafo.find_cycles(y_values,true,true);

    // ADICIONA RESTRICAO PARA ELIMINACAO DE CICLOS
    for(GRAFO& subgrafo: grafo.subgrafos){
        if(subgrafo.tipo == tipo_grafo::ciclo){
            IloBoolVarArray w_sum = IloBoolVarArray(getEnv());
            for(ARCO& arco: subgrafo.completo){
                w_sum.add(*arco.w_ptr);
            }
//            cout << w_sum << " < " << subgrafo.qnt_vertices << endl; // debug
            add(IloSum(w_sum) < subgrafo.qnt_vertices);
        }
    }


};

int flow(bool baseline = false,
         bool find_cycles = false,
         float time_limit_sec = 20*60,
         string data_name = ""){

    // Print of program configurations
    print_running_time("Starting program");
    cout << "SETUP" << endl;
    cout << "baseline: \t" << to_string(baseline) << endl;
    cout << "find cycles: \t" << to_string(find_cycles) << endl;
    cout << "debug: \t\t" << DEBUG << endl;
    cout << endl;

    IloEnv env;
    print_running_time("Starting data reading");
//    cout << "Starting data reading" << endl;
    grafo_pai = GRAFO(data_name);
    // Print of graphs totals
    cout << "Data name: " << data_name << endl;
    cout << "Nodes: \t\t" << to_string(grafo_pai.qnt_vertices) << endl;
    cout << "Arcs: \t\t" << to_string(grafo_pai.qnt_arcos) << endl;
    cout << "Location arcs: \t" << to_string(grafo_pai.qnt_arcos_localidade) << endl;
    cout << "Transp arcs: \t" << to_string(grafo_pai.qnt_arcos_transporte) << endl;
    cout << "Locations: \t" << to_string(grafo_pai.qnt_localidades) << endl;
    cout << "Products: \t" << to_string(grafo_pai.qnt_produtos) << endl;

    print_running_time("Data reading end");
    cout << endl << "Model construction start" << endl;


    try {
        //definir constantes e variáveis.
        definir_constantes(env, grafo_pai);
        map<string,string> dic_var;
        map<string,string> dic_var_modelo;
        auto dics_var = definir_variaveis(env, &grafo_pai);
        dic_var = dics_var.dic_var;
        dic_var_modelo = dics_var.dic_var_modelo;


        IloModel model(env);
        // constantes para as variáveis auxiliares z
        IloIntArray ones = IloIntArray(env);
        for (int i = 0; i < 27 ; i++){
            ones.add(1);
        }

        debug("TOTAL DE CONSTANTES");
        debug("a: ", to_string(a.getSize()));
        debug("b: ", to_string(b.getSize()));
        debug("c: ", to_string(c.getSize()));
        debug("TOTAL DE VARIÁVEIS");
        debug("x: ", to_string(x.getSize()));
        debug("w: ", to_string(w.getSize()));
        debug("y: ", to_string(y.getSize()));
        debug("z: ", to_string(z.getSize()));

        // Objective Function: Minimize Cost
        model.add(IloMinimize(env,
                              IloScalProd(a, x) +
                              IloSum(w) +
                              IloScalProd(b, y) +
                              IloScalProd(ones, z)));


        // restrição de balanço de massa
        for (VERTICE vertice: grafo_pai.vertices_completo) {
            string j = vertice.vertice;
            DEBUG = false;
            debug("");
            debug("VERTICE: ", j);
            debug(" tipo: ", vertice.tipo); //debug do tipo de vértice
            for (string s: grafo_pai.produtos) {
                debug(" PRODUTO: ", s);
                auto y_entradas = IloNumVarArray(env);
                auto y_saidas = IloNumVarArray(env);
                debug("  ENTRADAS");
                for (string i: grafo_pai.orgs_de(j)) {
                    debug("   ",i,s);
                    y_entradas.add(y[grafo_pai.completo_idx[{i, j, s}]]);
                }
                debug("  SAIDAS");
                for (string i: grafo_pai.dest_de(j)) {
                    debug("   ",i,s);
                    y_saidas.add(y[grafo_pai.completo_idx[{j, i, s}]]);
                }

//                debug("entradas -> " ,j ,s ,": ", y_entradas); //debug dos vertices de entrada
//                debug("saidas -> "   ,j ,s ,": ", y_saidas); //debug dos vertices de saida

                if (vertice.tipo == "passagem") {
                    model.add(IloSum(y_entradas) - IloSum(y_saidas) == 0);
                }else if (vertice.tipo == "origem"){
                    model.add(IloSum(y_entradas) - IloSum(y_saidas) == -grafo_pai.demandas_dic[{j, s}].o);
                }else if (vertice.tipo == "demanda"){
                    model.add(IloSum(y_entradas) - IloSum(y_saidas) == grafo_pai.demandas_dic[{j, s}].d);
                }else{
                   throw invalid_argument("vertice não possui um dos 3 tipos permitidos (passagem, origem, demanda)");
                }

            }
        }

        // restrição de capacidade e alocação de custo fixo
        int x_index = 0;
        for(const ARCO_SIMPLES& arco : grafo_pai.simples){
            if(arco.tipo_de_arco == "location"){
                string i = arco.i;
                string j = arco.j;
                float cap;
                auto y_sum = IloNumVarArray(env);
                for (const string& s : grafo_pai.produtos){
                    auto index = grafo_pai.completo_idx[{i, j, s}];
//                    cout << "DEBUG y_ptr index: " << index << endl;
                    y_sum.add(y[index]);
//                    y_sum.add(grafo_pai.completo[index].y);
                }
                if(isnan(arco.c)){
                    cap = grafo_pai.bigM;
                }else{
                    cap = arco.c;
                }
                model.add(IloSum(y_sum) <= cap * *arco.x_ptr);
                x_index++;
            }
        }


        // RESTRICAO PARA LIMITAR A QUANTIDADE DE LOCALIDADES ABERTAS
//        model.add(IloSum(x) <= 10); // teste de restricao para abertura de cds

        // restrição para o custo do saldo de icms
        for(int u = 0; u < 27 ; u++){
            string uf = UFs[u];
            vector<ARCO> entradas_uf = grafo_pai.entradas_uf(uf);
            vector<ARCO> saidas_uf = grafo_pai.saidas_uf(uf);
            IloExpr deb(env);
            IloExpr cred(env);
            for (ARCO arco : saidas_uf){
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



        //restrição de baseline INCOMPLETA
        if(baseline){
            float buffer = 1;
            for(ARCO arco: grafo_pai.completo){
                int index = arco.index;
                model.add(y[index] + buffer >= arco.v);
                model.add(y[index] - buffer <= arco.v);
            }
        }

        //RESTRIÇAO DE CICLOS lazy constraint
        /*
         * sum[w_ijs in C'](w_ijs) < |c| forall(C' in C)
         * y_ijs <= M * w_ijs  // liga o binário caso o fluxo y_ijs seja utilizado
         *
         * onde:
         * C = {C' | C' é um ciclo}
         * C'= {w_ptr}
         * w_ijs = {0,1 | ij isin ARCOS, s isin Prod }
         * w_ijs = binário que indica o se um determinado arco está sendo usado
        */

        // restrição para ligar os binários W que serão usados no corte.
        for(ARCO& arco: grafo_pai.completo){
            model.add(*arco.w_ptr * grafo_pai.bigM >= *arco.y_ptr); // W entra na FO
        }

        print_running_time("Model construction end");

        // Optimize
        cout << endl << "Optimization start" << endl;
        IloCplex cplex(model);

        if(find_cycles) {
            // todo: mudar find_cycles parar receber parametro para encontrar cyclos apenas em rotas com volume alocado
            // chama a lazy constraint para o modelo
            cplex.use(CycleElimitation(env,grafo_pai));
//            grafo_pai.find_cycles(env,true, true);
        }
        cout << endl ;

        // PARAMETROS CPLEX
        // parametros de exempo de cplex iloadmipex8
//        cplex.setParam(IloCplex::Param::Preprocessing::Presolve, 0);
//        cplex.setParam(IloCplex::Param::MIP::Strategy::HeuristicFreq, -1);
//        cplex.setParam(IloCplex::Param::MIP::Cuts::MIRCut, -1);
//        cplex.setParam(IloCplex::Param::MIP::Cuts::Implied, -1);
//        cplex.setParam(IloCplex::Param::MIP::Cuts::Gomory, -1);
//        cplex.setParam(IloCplex::Param::MIP::Cuts::FlowCovers, -1);
//        cplex.setParam(IloCplex::Param::MIP::Cuts::PathCut, -1);
//        cplex.setParam(IloCplex::Param::MIP::Cuts::LiftProj, -1);
//        cplex.setParam(IloCplex::Param::MIP::Cuts::ZeroHalfCut, -1);
//        cplex.setParam(IloCplex::Param::MIP::Cuts::Cliques, -1);
//        cplex.setParam(IloCplex::Param::MIP::Cuts::Covers, -1);
        // parametros originais
        cplex.setParam(IloCplex::Param::TimeLimit, time_limit_sec);
        cplex.setParam(IloCplex::Param::Threads, 8);
//        cplex.setOut(env.getNullStream());
        cplex.setWarning(env.getNullStream());
        //dumping model and fixing variable names in model file.
        cplex.exportModel("dados/debug/modelo.lp");
        fstream model_file;
        string model_file_str;
        string word;
        model_file.open("dados/debug/modelo.lp");
        string init;
        string tmp_str_word;
        string tmp_str_whitespace;
        unsigned long line_size = 0;
        unsigned int line_limit;
        unsigned int word_count;
        unsigned int word_limit = 100;
        char line;
        if(model_file.is_open()){
            while(true){
                line_limit = 100;
                init = "";
                tmp_str_whitespace = char(32);
                model_file >> word;
                model_file.get(line);
                if(word == "End") break; //end of model
                if(word == "Bounds") word_limit = 5;
                if(word == "Binaries") word_limit = 100;
                // replace variable x if its in dic
                if(word.at(0) == 'x'){
                    if(!dic_var_modelo[word].empty()){
                        tmp_str_word = dic_var_modelo[word];
                    }else{
                        tmp_str_word = "ERROR";
                    }
                }else{
                    tmp_str_word = word;
                }
                //limit line by words and chars
                word_count = 0;
                if(line_size <= line_limit and word_count <= word_limit){
                    line_size += tmp_str_word.length() + tmp_str_whitespace.length();
                    word_count++;
                }else{
                    tmp_str_whitespace = "\n";
                    line_size = 0;
                    word_count = 0;
                }
                //key words breakline
                if((word.front() == 'c' and word.back() == ':') or //constraints
                    word == "IloCplex" or
                    word == "Minimize" or
                    word == "Binaries" or
                    word == "Bounds"
                        ){
                    init = "\n";
                    tmp_str_whitespace = "\n";
                    line_size = 0;
                    word_count = 0;
                }
                //key words whitespace breakline
                if( word == "To" or
                    word == "1"
                        ){
                    tmp_str_whitespace = "\n";
                    word_count = 0;
                    line_size = 0;
                }
                model_file_str += init; //breakline for keywords
                model_file_str += tmp_str_word;
                model_file_str += tmp_str_whitespace;
            }
            model_file.close();
            model_file.open("dados/debug/modelo_convertido.lp");
            model_file << model_file_str;
            model_file.close();
        }
        cplex.solve();
        print_running_time("Optimization end");

        if (cplex.getStatus() == IloAlgorithm::Infeasible){
            env.out() << "No Solution Exists" << endl;
            cout << "Salvando modelo" << endl;
            cout << "POSSÍVEL CONFLITO:" << endl;
            string conflict = refiner(env,model,cplex,false,false);
//            cout << conflict << endl;
            cout << replace_vars(conflict,dic_var) << endl;
        }

        // Print and save results
        if(baseline) cout << "##### BASELINE #####" << endl;
        env.out() << "Solution status: " << cplex.getStatus() << endl;
        env.out() << "Cost:" << cplex.getObjValue() << endl;
        cout << "Relative GAP: " << cplex.getMIPRelativeGap() << endl;
        env.out() << endl << "LOCALIDADES" << endl;

        // define arquivo a ser salvo conforme se otimizado ou baseline
        string nome_arquivo_localidades;
        if(baseline) {
            nome_arquivo_localidades = "dados/resultados/" + data_name + "baseline_localidades.csv";
        }else{
            nome_arquivo_localidades = "dados/resultados/" + data_name + "localidades.csv";
        }
        ofstream csv_localidades (caminho + nome_arquivo_localidades);

        int indice_localidade = 0;
        for(ARCO_SIMPLES arco : grafo_pai.simples){
            if(arco.tipo_de_arco == "location"){
                //convert to portuguese number delimiter.
                string vol_loc = to_string(abs(cplex.getValue(x[indice_localidade])));
                if(vol_loc.find(".") < 1e20) vol_loc.replace(vol_loc.find("."),1,",");
                vol_loc = vol_loc[0];
                //print terminal
                env.out() << arco.i + " -> " + arco.j + " (cap: " << + arco.c << ")"
                          << " (" << grafo_pai.vertices_completo_dic[arco.i].uf << "):  "
                          << vol_loc;
                if(vol_loc == string("1")) cout << "\t <<<";
                cout << endl;

                //saving into csv
                if (indice_localidade == 0){
                    csv_localidades << "org;dest;capacidade max;uf;uso" << endl;
                }
                csv_localidades << arco.i + ";" + arco.j + ";" << + arco.c \
 << ";" << grafo_pai.vertices_completo_dic[arco.i].uf \
 << ";" << cplex.getValue(x[indice_localidade]) \
                << endl;

                ++indice_localidade;
            }
        }
        csv_localidades.close();

        env.out() << endl << "SALDOS DE UF" << endl;

        // define arquivo a ser salvo conforme se otimizado ou baseline
        string nome_arquivo_saldos;
        if(baseline) {
            nome_arquivo_saldos = "dados/resultados/" + data_name + "baseline_saldos.csv";
        }else{
            nome_arquivo_saldos = "dados/resultados/" + data_name + "saldos.csv";
        }
        ofstream csv_saldos (caminho + nome_arquivo_saldos);
        csv_saldos << "UF;Saldo" << endl;
        int valor_saldo_num = 0;
        int total_icms = 0;
        string valor_saldo;
        for(int u = 0; u < 27; u++){
            valor_saldo_num = cplex.getValue(z[u]);
            total_icms += valor_saldo_num;
            string valor_saldo = to_string(valor_saldo_num);
            env.out() << UFs[u] << " -> " << valor_saldo << endl;
            csv_saldos << UFs[u] << ";" << valor_saldo << endl;
        }
        cout << "Total ICMS: " << total_icms << endl;
        csv_saldos.close();

        cout << "Salvando fluxos" << endl;
        // define arquivo a ser salvo conforme se otimizado ou baseline
        string nome_arquivo_fluxos;
        if(baseline) {
            nome_arquivo_fluxos = "dados/resultados/" + data_name + "baseline_fluxos.csv";
        }else{
            nome_arquivo_fluxos = "dados/resultados/" + data_name + "fluxos.csv";
        }
        ofstream csv_fluxos (caminho + nome_arquivo_fluxos);

        for(int i = 0; i < grafo_pai.qnt_arcos; i++){
            string flow_vol = to_string(cplex.getValue(y[i]));
            if(flow_vol.find(".") < 1e20) flow_vol.replace(flow_vol.find("."),1,",");
            if(i==0) csv_fluxos << "org;dest;sku;volume" << endl;
           csv_fluxos << grafo_pai.completo[i].i << ";";
            csv_fluxos << grafo_pai.completo[i].j << ";";
            csv_fluxos << grafo_pai.completo[i].s << ";";
           csv_fluxos << flow_vol << endl;
        }
        csv_fluxos.close();
        // print final model
        cplex.exportModel("dados/debug/modelo_final.lp");
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

int main(int argc, char* argv[]) {
    string loop_input;
    // read arguments and define scenario.
    string data_name;
    data_name = "teste_";
    float time_limit_sec = 10*60;
//    data_name = "sem_prodepe_";
//    data_name = "com_prodepe_";
//    data_name = "merc10_";
    if (argc > 0){
        for(int i = 0; i < argc; i++){
            cout << i << ": ";
            cout << argv[i]<< endl;
            if(argv[i] == string("-c")){
                data_name = argv[i + 1];
            }else if(argv[i] == string("-t")){
                time_limit_sec = stof(argv[i+1]);
            }
        }
    }

    while(true){
        flow(false,true, time_limit_sec,data_name);
        cout << "Press \"s\" to run again" << endl;
        cin >> loop_input;
        if(loop_input != "s"){
            cout << "Exiting Program" << endl;
            break;
        }
    }

//    DEBUG
//    string str = " -1 * IloNumVar(37)[0 .. inf]  == -0 == 10";
//    string separator = "==";
//    vector<string> partes = split(str,separator);
//    cout << "primeira parte na main: " << partes[0] << endl;
//    cout << "segunda parte na main: " << partes[1] << endl;
//    cout << "segunda parte na main: " << partes[2] << endl;

    return 0;
}