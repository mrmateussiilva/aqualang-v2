from typing import Iterable



def read_file(namefile):
    with open(namefile, "r") as fp:
        return list(filter(lambda l: l != " ",map(lambda l: l.strip("\n"),fp.readlines())))
    
    
    
    
if __name__ == "__main__":
    arquivo = "examples/hello_world.aqua"
    dados = read_file(arquivo)
    print(len(dados))