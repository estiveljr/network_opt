# Um modelo de programação inteira para a otimização de um projeto de rede de cadeia de suprimentos com tributação.

## Resumo

Um projeto de rede de cadeia de suprimentos é responsável por definir a estrutura logística da cadeia de suprimentos de uma empresa através da escolha de quais localidades serão usadas para estabelecer as instalações e quais são os caminhos que os produtos devem percorrer para chegar ao cliente final. Essas escolhas são de longo de prazo e possuem um custo elevado para a implementação das instalações e apresentam certa flexibilidade para os fluxos de produtos. As decisões tomadas podem ser a favor de menor custo, da redução do tempo de atendimento ao cliente, ou uma combinação entre os dois.

A tributação constitui um fator de grande relevância para o projeto de rede de cadeia de suprimentos quando o foco é redução de custos, pois um projeto de rede ótimo quando considerados apenas os custos logísticos, pode deixar de ser ótimo quando considerados os custos tributários, uma vez que os custos tributários mudam conforme a movimentação de produtos entre os estados.

Este estudo propõe um modelo de programação inteira para a otimização de custos de um projeto de rede de cadeia de suprimentos com tributação, com ênfase no imposto que mais impacta nas decisões da rede, que é o ICMS e seus derivados, como benefícios fiscais, substituição tributária e o diferencial de alíquotas.

## Implementação

Programa feito em c++ com a utilização do solver IBM Cplex&reg;

Dados disponíveis na pasta build/dados. 

Ambiente linux através de wsl. Scripts *.bat* disponíveis junto com os dados.

Dados artificiais criados a partir do script python, em criar_instancia/criar_instatancia.py
