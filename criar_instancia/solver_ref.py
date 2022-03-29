from collections import namedtuple
from pulp import *
Item = namedtuple("Item", ['index', 'value', 'weight'])

DEBUG = 2

def solve_it(input_data):
    # parse the input
    lines = input_data.split('\n')

    firstLine = lines[0].split()
    item_count = int(firstLine[0])
    capacity = int(firstLine[1])
    conflict_count = int(firstLine[2])

    items = []
    conflicts = []

    for i in range(1, item_count+1):
        line = lines[i]
        parts = line.split()
        items.append(Item(i-1, int(parts[0]), int(parts[1])))

    for i in range(1, conflict_count+1):
        line = lines[item_count + i]
        parts = line.split()
        conflicts.append((int(parts[0]), int(parts[1])))

    return knapsackNaive(item_count, items, capacity, conflict_count, conflicts)


def knapsackNaive(num_items, items, capacity, num_conflicts, conflicts):

    if DEBUG >= 1:
        print(f"numero de itens = {num_items}")
        print(f"capacidade da mochila = {capacity}")
        print(f"numero de conflitos = {num_conflicts}")

    if DEBUG >= 2:
        print("Itens na ordem em que foram lidos")
        for item in items:
            print(item)
        print()

    if DEBUG >= 2:
        print("Conflitos na ordem em que foram lidos")
        for conflict in conflicts:
            print(conflict)
        print()

    # Modify this code to run your optimization algorithm
    
    prob = LpProblem("Problema_da_mochila",LpMaximize)
    bin = LpVariable.dicts('items_Selecionados',[item.index for item in items],cat=LpBinary)
    
    # define função objetivo
    prob += lpSum([item.value*bin[item.index] for item in items])

    # apenas um entre dois itens conflitantes
    for conflict in conflicts:
        prob += bin[conflict[0]] + bin[conflict[1]] <= 1

    # respeitar a capacidade
    prob += lpSum([item.weight*bin[item.index] for item in items]) <= capacity
    prob.solve()

    # prepare the solution in the specified output format
    print('Capacidade de peso na mochila: ' + str(capacity))
    print("Peso atual na mochila: " + str(int(lpSum([item.weight*bin[item.index] for item in items]).value())))
    resultado = int(lpSum([item.value*bin[item.index].value() for item in items]).value())
    output_data = str(resultado) + '\n'
    output_data += ' '.join(map(str,[int(v.value()) for v in bin.values()]))
    # output_data += ' '.join(map(str, [item.value() for item in itemsselecionados]))

    # solution = [0]*num_items
    # solution_value = 0
    # solution_weight = 0

    # for item in items:
    #     if solution_weight + item.weight <= capacity:
    #         solution[item.index] = 1
    #         solution_value += item.value
    #         solution_weight += item.weight

    # # prepare the solution in the specified output format
    # output_data = str(solution_value) + '\n'
    # output_data += ' '.join(map(str, solution))

    return output_data


if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        file_location = sys.argv[1].strip()
        with open(file_location, 'r') as input_data_file:
            input_data = input_data_file.read()
        output_data = solve_it(input_data)
        print(output_data)
        solution_file = open(file_location + ".sol", "w")
        solution_file.write(output_data)
        solution_file.close()
    else:
        print('This test requires an input file.  Please select one from the data directory. (i.e. python solver.py ./data/ks_4_0)')
