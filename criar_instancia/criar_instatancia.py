from collections import namedtuple
from random import randrange

import pandas as pd

arco = namedtuple("arco", ['tipo_de_arco', 'i', 'j', 's', 'a', 'b', 'c', 'm', 'n'])

# CONFIGURAÇÃO PARA A CONSTRUÇÃO DA INSTÂNCIA
# DEFINE SE O GRAFO SERÁ GERADO POR N ELEMENTOS OU POR LISTAS DE VÉRTICES

# INPUT_DATA = True #read data from file
# OUTPUT_FOLDER = "F:/OneDrive/_each/_Quali/artigo/dados/dados_oficiais/"
# DATA_NAME = "sbpo"
# SEPARATOR = ";"

INPUT_DATA = False #read from file
OUTPUT_FOLDER = "../build/dados/"
DATA_NAME = "artificial_"
SEPARATOR = ";"

# Read node data from csv file
if INPUT_DATA:
    df_nodes_input = pd.read_excel("input.xlsx", sheet_name='nodes')
    suppliers = df_nodes_input['node name'][df_nodes_input['node type'] == 'supplier']
    factories = df_nodes_input['node name'][df_nodes_input['node type'] == 'factory']
    facilyties = df_nodes_input['node name'][df_nodes_input['node type'] == 'facility']
    clients = df_nodes_input['node name'][df_nodes_input['node type'] == 'client']
    goods = pd.read_excel("input.xlsx", sheet_name='goods')['good']

# Define data from set size of each level (if not read from file)
n_suppliers = 10
n_factories = 10
n_facilyties = 10
n_clients = 100
n_goods = 50

if not INPUT_DATA:
    DATA_NAME = DATA_NAME + "s{}_f{}_f{}_c{}_g{}".format(
        n_suppliers,
        n_factories,
        n_facilyties,
        n_clients,
        n_goods
    )

# Define a demanda total
FULL_DEMAND = 1E6

# Define iteratarators to create the arcs
if INPUT_DATA:
    sups_iterator = suppliers
    flt_iterator = factories
    fac_iterator = facilyties
    clt_iterator = clients
    goods_iterator = goods
else:
    sups_iterator = range(n_suppliers)
    flt_iterator = range(n_factories)
    fac_iterator = range(n_facilyties)
    clt_iterator = range(n_clients)
    goods_iterator = range(n_goods)

# Define range of costs
def custo_fixo(tipo_de_arco, i, s) -> float:
    if tipo_de_arco == "location":
        return randrange(int(10e3), int(100e3))
    else:
        return 0


def capacidade(tipo_de_arco, i, j):
    if tipo_de_arco == "location":
        if INPUT_DATA:
            return 0
        else:
            return FULL_DEMAND
        # return randrange(int(1e3), int(10e3))
    else:
        return 0


def custo_variavel(tipo_de_arco, i, j):
    if tipo_de_arco == "location":
        return randrange(10, 100)
    else:
        return 0


def icms_st(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transportation':
        return randrange(10, 100)
    else:
        return 0


def custos_fornecimento(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transportation':
        return randrange(10, 100)
    else:
        return 0


def icms(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transportation':
        return randrange(10, 100)
    else:
        return 0


def cred_pres(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transportation':
        return int(randrange(0, 10))
    else:
        return 0


def difal(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transportation':
        return int(randrange(0, 10))
    else:
        return 0


def anulacao(tipo_de_arco, j, s):
    if tipo_de_arco == 'transportation':
        return 0
    else:
        return 0


def icmsca(u):
    return 0


# CUSTOS AGRUPADOS
def gerar_a(tipo_de_arco, i, j) -> float:
    return custo_fixo(tipo_de_arco, i, j)


def gerar_b(tipo_de_arco, i, j, s):
    return custo_variavel(tipo_de_arco, i, j) + icms_st(tipo_de_arco, i, j, s) + custos_fornecimento(tipo_de_arco, i, j,
                                                                                                     s)


def gerar_c(tipo_de_arco, i, j):
    return capacidade(tipo_de_arco, i, j)


def gerar_m(tipo_de_arco, i, j, s):
    return icms(tipo_de_arco, i, j, s) + cred_pres(tipo_de_arco, i, j, s) + difal(tipo_de_arco, i, j, s)


def gerar_n(tipo_de_arco, i, j, s):
    return icms(tipo_de_arco, i, j, s) * (1 - anulacao(tipo_de_arco, j, s))


p = lambda u: icmsca(u)

# CRIA ARCOS DE location
tipo_de_arco = 'location'
arcos_fornecedores = []
for sup in sups_iterator:
    a = gerar_a(tipo_de_arco, sup, sup)
    if INPUT_DATA: a = 0 #factories fixed cost
    c = gerar_c(tipo_de_arco, sup, sup)
    arcos_fornecedores.extend([arco(tipo_de_arco, f'SUP_{sup}_in', f"SUP_{sup}_out", str(good),
                                    a,
                                    gerar_b(tipo_de_arco, sup, sup, good),
                                    c,
                                    gerar_m(tipo_de_arco, sup, sup, good),
                                    gerar_n(tipo_de_arco, sup, sup, good))
                               for good in goods_iterator])

tipo_de_arco = 'location'
arcos_fac = []
for fac in fac_iterator:
    a = gerar_a(tipo_de_arco, fac, fac)
    if INPUT_DATA: a = 3464620 #factories fixed cost
    c = gerar_c(tipo_de_arco, fac, fac)
    arcos_fac.extend([arco(tipo_de_arco, f"FAC_{fac}_in", f"FAC_{fac}_out", str(good),
                           a,
                           gerar_b(tipo_de_arco, fac, fac, good),
                           c,
                           gerar_m(tipo_de_arco, fac, fac, good),
                           gerar_n(tipo_de_arco, fac, fac, good))
                      for good in goods_iterator])

tipo_de_arco = 'location'
arcos_cds = []
for flt in flt_iterator:
    a = gerar_a(tipo_de_arco, flt, flt)
    if INPUT_DATA: a = 2713488 #facility fixed cost
    c = gerar_c(tipo_de_arco, flt, flt)
    arcos_cds.extend([arco(tipo_de_arco, f"FLT_{flt}_in", f"FLT_{flt}_out", str(good),
                           a,
                           gerar_b(tipo_de_arco, flt, flt, good),
                           c,
                           gerar_m(tipo_de_arco, flt, flt, good),
                           gerar_n(tipo_de_arco, flt, flt, good))
                      for good in goods_iterator])


tipo_de_arco = 'location'
arcos_clientes = []
for clt in clt_iterator:
    a = gerar_a(tipo_de_arco, clt, clt)
    if INPUT_DATA: a = 0 #factories fixed cost
    c = gerar_c(tipo_de_arco, clt, clt)
    arcos_clientes.extend([arco(tipo_de_arco, f"CLT_{clt}_in", f"CLT_{clt}_out", str(good),
                                a,
                                gerar_b(tipo_de_arco, clt, clt, good),
                                c,
                                gerar_m(tipo_de_arco, clt, clt, good),
                                gerar_n(tipo_de_arco, clt, clt, good))
                           for good in goods_iterator])

# CRIA ARCOS DE transportation
tipo_de_arco = 'transportation'
arcos_inbound = [arco(tipo_de_arco, f"SUP_{sup}_out", f"FAC_{fac}_in", str(good),
                      gerar_a(tipo_de_arco, sup, fac),
                      gerar_b(tipo_de_arco, sup, fac, good),
                      gerar_c(tipo_de_arco, sup, fac),
                      gerar_m(tipo_de_arco, sup, fac, good),
                      gerar_n(tipo_de_arco, sup, fac, good))
                 for sup in sups_iterator
                 for fac in fac_iterator
                 for good in goods_iterator]

arcos_transferencia_fac_flt = [arco(tipo_de_arco, f"FAC_{fac}_out", f"FLT_{flt}_in", str(good),
                            gerar_a(tipo_de_arco, fac, flt),
                            gerar_b(tipo_de_arco, fac, flt, good),
                            gerar_c(tipo_de_arco, fac, flt),
                            gerar_m(tipo_de_arco, fac, flt, good),
                            gerar_n(tipo_de_arco, fac, flt, good))
                       for fac in fac_iterator
                       for flt in flt_iterator
                       for good in goods_iterator]

arcos_transferencia = [arco(tipo_de_arco, f"FLT_{flt_i}_out", f"FLT_{flt_j}_in", str(good),
                            gerar_a(tipo_de_arco, flt_i, flt_j),
                            gerar_b(tipo_de_arco, flt_i, flt_j, good),
                            gerar_c(tipo_de_arco, flt_i, flt_j),
                            gerar_m(tipo_de_arco, flt_i, flt_j, good),
                            gerar_n(tipo_de_arco, flt_i, flt_j, good))
                       for flt_i in flt_iterator
                       for flt_j in flt_iterator
                       for good in goods_iterator if flt_i != flt_j]

arcos_outbound = [arco(tipo_de_arco, f"FLT_{flt}_out", f"CLT_{clt_i}_in", str(good),
                       gerar_a(tipo_de_arco, flt, clt_i),
                       gerar_b(tipo_de_arco, flt, clt_i, good),
                       gerar_c(tipo_de_arco, flt, clt_i),
                       gerar_m(tipo_de_arco, flt, clt_i, good),
                       gerar_n(tipo_de_arco, flt, clt_i, good))
                  for flt in flt_iterator
                  for clt_i in clt_iterator
                  for good in goods_iterator]

# CRIAR DF COM ARCOS DE locationS
arcos_locations = [arcos_fornecedores, arcos_fac, arcos_cds, arcos_clientes]
df_arcos_locations = pd.DataFrame()
for arcos in arcos_locations:
    df_arcos_locations = df_arcos_locations.append(pd.DataFrame(arcos))

# CRIAR DF COM ARCOS DE transportation
arcos_transportation = [arcos_inbound, arcos_transferencia_fac_flt, arcos_transferencia, arcos_outbound]
df_arcos_transportation = pd.DataFrame()
for arcos in arcos_transportation:
    df_arcos_transportation = df_arcos_transportation.append(pd.DataFrame(arcos))

# AGRUPAR TODOS OS ARCOS
df_arcos_consolidados = df_arcos_locations.append(df_arcos_transportation)

# DEFINIR VERTICES
#atribui uma UF para cada vértice
colunas_uf = ["vertice", "UF"]
UFs = ["RO", "AC", "AM", "RR", "PA", "AP", "TO", "MA", "PI", "CE", "RN", "PB", "PE", "AL", "SE", "BA", "MG", "ES", "RJ",
       "SP", "PR", "SC", "RS", "MS", "MT", "GO", "DF"]
locations = df_arcos_locations[["i","j"]].copy().drop_duplicates()
locations["UF"] = None
if not INPUT_DATA: locations["UF"] = locations["UF"].apply(lambda x: UFs[randrange(0, len(UFs))])

#forma df dos vértices
df_vertices = pd.concat([locations[["i", "UF"]].rename(columns={"i": "vertice"}),
                         locations[["j", "UF"]].rename(columns={"j":"vertice"})]).drop_duplicates()

#vertices de demanda e origem
df_vertices_origem = pd.DataFrame({"vertice":[arco.i for arco in arcos_fornecedores],"tipo":"origem"}).drop_duplicates()
df_vertices_demanda = pd.DataFrame({"vertice":[arco.j for arco in arcos_clientes],"tipo":"demanda"}).drop_duplicates()
df_vertices = df_vertices.merge(pd.concat([df_vertices_origem, df_vertices_demanda]), how="left")
df_vertices = df_vertices.fillna("passagem")

# definição de demanda
df_demandas_fornecimento = df_vertices.copy()
org_skus = df_arcos_locations[["i", "s"]].rename(columns={"i": "vertice"})
dst_skus = df_arcos_locations[["j", "s"]].rename(columns={"j": "vertice"})
vertices = pd.concat([org_skus, dst_skus]).drop_duplicates()
df_demandas_fornecimento = df_demandas_fornecimento.merge(vertices, on="vertice")
qnt_skus = df_arcos_locations['s'].drop_duplicates().shape[0]
qnt_clientes_sku = df_vertices_demanda.shape[0] * qnt_skus
qnt_fornecedores_sku = df_vertices_origem.shape[0] * qnt_skus
df_demandas_fornecimento["d"] = 0
df_demandas_fornecimento["o"] = 0
df_demandas_fornecimento.loc[df_demandas_fornecimento["tipo"] == "origem", "o"] = FULL_DEMAND / qnt_fornecedores_sku
df_demandas_fornecimento.loc[df_demandas_fornecimento["tipo"] == "demanda", "d"] = FULL_DEMAND / qnt_clientes_sku
df_demandas_fornecimento = df_demandas_fornecimento[["vertice", "s", "UF", "tipo", "d", "o"]]

def create_wsl_bat(sup,fac,fcl,clt,gds):

    bat = "C:\Windows\system32\wsl.exe --distribution Debian --exec /bin/bash -c " \
          "\"cd /mnt/d/OneDrive/_each/_Quali/Artigo/build " \
          "&& /mnt/d/OneDrive/_each/_Quali/Artigo/build/modelocpp " \
          "-c artificial_s{}_f{}_f{}_c{}_g{}_\"" \
          "\n %pause".format(sup,fac,fcl,clt,gds)
    try:
        with open(OUTPUT_FOLDER + "rodar_build_s{}_f{}_f{}_c{}_g{}.bat".format(sup,fac,flt,clt,gds), "w") as file:
            file.write(bat)

        print(".bat salvo com sucesso")
    except:
        print("Não foi possível salvar o arquivo .bat")


try:
    with pd.ExcelWriter(OUTPUT_FOLDER + DATA_NAME + '_malha.xlsx') as writer:
        df_arcos_consolidados.to_excel(writer, sheet_name='arcos_consolidados', index=False)
        df_vertices.to_excel(writer, sheet_name='vertices', index=False)
        df_demandas_fornecimento.to_excel(writer, sheet_name='demandas_fornecimento', index=False)
    print("Excel salvo com sucesso.")
except:
    print("Base de dados muito grande. Apenas o csv sará salvo.")

try:
    df_arcos_consolidados.to_csv(OUTPUT_FOLDER + DATA_NAME + "_arcos.csv", index=False, sep=SEPARATOR)
    print("Csvs salvos com sucesso.")
    create_wsl_bat(n_suppliers,n_factories,n_facilyties,n_clients,n_goods)
except:
    print("Erro em salvar os csvs.")

try:
    df_vertices.to_csv(OUTPUT_FOLDER + DATA_NAME + "_vertices.csv", index=False, sep=SEPARATOR)
    print("Csv de UF por vertice salvo com sucesso.")
except:
    print("Erro em salvar os csv de UF por vertice.")

try:
    df_demandas_fornecimento.to_csv(OUTPUT_FOLDER + DATA_NAME + "_dem_forn.csv", index=False, sep=SEPARATOR)
    print("Csv de demandas e fornecimento salvo com sucesso.")
except:
    print("Erro em salvar os csv de demandas e fornecimento.")




