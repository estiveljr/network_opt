from typing import Type

import pandas as pd
from random import randrange
from collections import namedtuple


# arco_localidade = namedtuple("arco_localidade", ['i', 'j', 'custo_fixo', 'capacidade'])
# arco_transporte = namedtuple("arco_transporte", ['i', 'j', 's',
#                                                  'custo_variavel',
#                                                  'icms_st',
#                                                  'custos_fornecimento',
#                                                  'icms',
#                                                  'cred_pres',
#                                                  'difal',
#                                                  'anulacao'])


# arco_localidade = namedtuple("arco_localidade", ['i', 'j', 's', 'a', 'b', 'c', 'm', 'n'])
# arco_transporte = namedtuple("arco_transporte", ['i', 'j', 's', 'a', 'b', 'c', 'm', 'n'])
# arco_localidade = namedtuple("arco_localidade", ['i', 'j'])
# arco_transporte = namedtuple("arco_transporte", ['i', 'j', 's'])


arco = namedtuple("arco", ['tipo_de_arco', 'i', 'j', 's', 'a', 'b', 'c', 'm', 'n'])

# CONFIGURAÇÃO PARA A CONSTRUÇÃO DA INSTÂNCIA
# Define tamanho do grafo
n_fornecedores = 5
n_cds = 5
n_clientes = 20
n_produtos = 10
# Define a demanda total
DEMANDA_TOTAL = 1E3


# Define range de custos abertos
def custo_fixo(tipo_de_arco, i, s) -> float:
    if tipo_de_arco == "localidade":
        return randrange(int(10e3), int(100e3))
    else:
        return 0


def capacidade(tipo_de_arco, i, j):
    if tipo_de_arco == "localidade":
        return DEMANDA_TOTAL
        # return randrange(int(1e3), int(10e3))
    else:
        return 0


def custo_variavel(tipo_de_arco, i, j):
    if tipo_de_arco == "localidade":
        return randrange(10, 100)
    else:
        return 0


def icms_st(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transporte':
        return randrange(10, 100)
    else:
        return 0


def custos_fornecimento(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transporte':
        return randrange(10, 100)
    else:
        return 0


def icms(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transporte':
        return randrange(10, 100)
    else:
        return 0


def cred_pres(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transporte':
        return int(randrange(0, 10))
    else:
        return 0


def difal(tipo_de_arco, i, j, s):
    if tipo_de_arco == 'transporte':
        return int(randrange(0, 10))
    else:
        return 0


def anulacao(tipo_de_arco, j, s):
    if tipo_de_arco == 'transporte':
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

# CRIA ARCOS DE LOCALIDADE
tipo_de_arco = 'localidade'
arcos_fornecedores = []
for forn in range(n_fornecedores):
    a = gerar_a(tipo_de_arco, forn, forn)
    c = gerar_c(tipo_de_arco, forn, forn)
    arcos_fornecedores.extend([arco(tipo_de_arco, f'F_{forn}_entrada', f"F_{forn}_saida", str(prod),
                                    a,
                                    gerar_b(tipo_de_arco, forn, forn, prod),
                                    c,
                                    gerar_m(tipo_de_arco, forn, forn, prod),
                                    gerar_n(tipo_de_arco, forn, forn, prod))
                               for prod in range(n_produtos)])


arcos_cds = []
for cd in range(n_cds):
    a = gerar_a(tipo_de_arco, forn, forn)
    c = gerar_c(tipo_de_arco, forn, forn)
    arcos_cds.extend([arco(tipo_de_arco, f"CD_{cd}_entrada", f"CD_{cd}_saida", str(prod),
                  a,
                  gerar_b(tipo_de_arco, cd, cd, prod),
                  c,
                  gerar_m(tipo_de_arco, cd, cd, prod),
                  gerar_n(tipo_de_arco, cd, cd, prod))
             for prod in range(n_produtos)])

arcos_clientes = []
for cliente in range(n_clientes):
    a = gerar_a(tipo_de_arco, forn, forn)
    c = gerar_c(tipo_de_arco, forn, forn)
    arcos_clientes.extend([arco(tipo_de_arco, f"C_{cliente}_entrada", f"C_{cliente}_saida", str(prod),
                       a,
                       gerar_b(tipo_de_arco, cliente, cliente,prod),
                       c,
                       gerar_m(tipo_de_arco, cliente, cliente, prod),
                       gerar_n(tipo_de_arco, cliente, cliente, prod))
             for prod in range(n_produtos)])

# CRIA ARCOS DE TRANSPORTE
tipo_de_arco = 'transporte'
arcos_inbound = [arco(tipo_de_arco, f"F_{forn}_saida", f"CD_{cd}_entrada", str(prod),
                      gerar_a(tipo_de_arco, forn, cd),
                      gerar_b(tipo_de_arco, forn, cd, prod),
                      gerar_c(tipo_de_arco, forn, cd),
                      gerar_m(tipo_de_arco, forn, cd, prod),
                      gerar_n(tipo_de_arco, forn, cd, prod))
                 for forn in range(n_fornecedores)
                 for cd in range(n_cds)
                 for prod in range(n_produtos)]

arcos_transferencia = [arco(tipo_de_arco, f"CD_{cdi}_saida", f"CD_{cdj}_entrada", str(prod),
                            gerar_a(tipo_de_arco, cdi, cdj),
                            gerar_b(tipo_de_arco, cdi, cdj, prod),
                            gerar_c(tipo_de_arco, cdi, cdj),
                            gerar_m(tipo_de_arco, cdi, cdj, prod),
                            gerar_n(tipo_de_arco, cdi, cdj, prod))
                       for cdi in range(n_cds)
                       for cdj in range(n_cds)
                       for prod in range(n_produtos) if cdi != cdj]

arcos_outbound = [arco(tipo_de_arco, f"CD_{cd}_saida", f"C_{cli}_entrada", str(prod),
                       gerar_a(tipo_de_arco, cd, cli),
                       gerar_b(tipo_de_arco, cd, cli, prod),
                       gerar_c(tipo_de_arco, cd, cli),
                       gerar_m(tipo_de_arco, cd, cli, prod),
                       gerar_n(tipo_de_arco, cd, cli, prod))
                  for cd in range(n_cds)
                  for cli in range(n_clientes)
                  for prod in range(n_produtos)]

# CRIAR DF COM ARCOS DE LOCALIDADES
arcos_localidades = [arcos_fornecedores, arcos_cds, arcos_clientes]
df_arcos_localidades = pd.DataFrame()
for arcos in arcos_localidades:
    df_arcos_localidades = df_arcos_localidades.append(pd.DataFrame(arcos))

# CRIAR DF COM ARCOS DE TRANSPORTE
arcos_transporte = [arcos_inbound, arcos_transferencia, arcos_outbound]
df_arcos_transporte = pd.DataFrame()
for arcos in arcos_transporte:
    df_arcos_transporte = df_arcos_transporte.append(pd.DataFrame(arcos))

# AGRUPAR TODOS OS ARCOS
df_arcos_consolidados = df_arcos_localidades.append(df_arcos_transporte)

# DEFINIR VERTICES
#atribui uma UF para cada vértice
colunas_uf = ["vertice", "UF"]
UFs = ["RO", "AC", "AM", "RR", "PA", "AP", "TO", "MA", "PI", "CE", "RN", "PB", "PE", "AL", "SE", "BA", "MG", "ES", "RJ",
       "SP", "PR", "SC", "RS", "MS", "MT", "GO", "DF"]
localidades = df_arcos_localidades[["i","j"]].copy().drop_duplicates()
localidades["UF"] = None
localidades["UF"] = localidades["UF"].apply(lambda x: UFs[randrange(0, len(UFs))])

#forma df dos vértices
df_vertices = pd.concat([localidades[["i", "UF"]].rename(columns={"i": "vertice"}),
                         localidades[["j","UF"]].rename(columns={"j":"vertice"})]).drop_duplicates()

#vertices de demanda e origem
df_vertices_origem = pd.DataFrame({"vertice":[arco.i for arco in arcos_fornecedores],"tipo":"origem"}).drop_duplicates()
df_vertices_demanda = pd.DataFrame({"vertice":[arco.j for arco in arcos_clientes],"tipo":"demanda"}).drop_duplicates()
df_vertices = df_vertices.merge(pd.concat([df_vertices_origem, df_vertices_demanda]), how="left")
df_vertices = df_vertices.fillna("passagem")

# definição de demanda
df_demandas_fornecimento = df_vertices.copy()
org_skus = df_arcos_localidades[["i", "s"]].rename(columns={"i": "vertice"})
dst_skus = df_arcos_localidades[["j", "s"]].rename(columns={"j": "vertice"})
vertices = pd.concat([org_skus, dst_skus]).drop_duplicates()
df_demandas_fornecimento = df_demandas_fornecimento.merge(vertices, on="vertice")
qnt_skus = df_arcos_localidades['s'].drop_duplicates().shape[0]
qnt_clientes_sku = df_vertices_demanda.shape[0] * qnt_skus
qnt_fornecedores_sku = df_vertices_origem.shape[0] * qnt_skus
df_demandas_fornecimento["d"] = 0
df_demandas_fornecimento["o"] = 0
df_demandas_fornecimento.loc[df_demandas_fornecimento["tipo"] == "origem", "o"] = DEMANDA_TOTAL / qnt_fornecedores_sku
df_demandas_fornecimento.loc[df_demandas_fornecimento["tipo"] == "demanda", "d"] = DEMANDA_TOTAL / qnt_clientes_sku
df_demandas_fornecimento = df_demandas_fornecimento[["vertice", "s", "UF", "tipo", "d", "o"]]


pasta_destino = "F:/OneDrive/_each/_Quali/Artigo/modelocpp/"
try:
    with pd.ExcelWriter('dados.xlsx') as writer:
        df_arcos_consolidados.to_excel(writer, sheet_name='arcos_consolidados', index=False)
        df_vertices.to_excel(writer, sheet_name='vertices', index=False)
        df_demandas_fornecimento.to_excel(writer, sheet_name='demandas_fornecimento', index=False)
        print("Excel salvo com sucesso.")
except:
    print("Base de dados muito grande. Apenas o csv sará salvo.")

try:
    df_arcos_consolidados.to_csv(pasta_destino + "arcos_consolidados.csv", index=False)
    print("Csvs salvos com sucesso.")
except:
    print("Erro em salvar os csvs.")

try:
    df_vertices.to_csv(pasta_destino + "vertices_completo.csv", index=False)
    print("Csv de UF por vertice salvo com sucesso.")
except:
    print("Erro em salvar os csv de UF por vertice.")

try:
    df_demandas_fornecimento.to_csv(pasta_destino + "demandas_fornecimento.csv", index=False)
    print("Csv de demandas e fornecimento salvo com sucesso.")
except:
    print("Erro em salvar os csv de demandas e fornecimento.")




