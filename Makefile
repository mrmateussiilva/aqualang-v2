# Makefile para a linguagem de programação Aqua
# Compila a runtime em C++ e o lexer em Python

# Variáveis
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
PYTHON = python3

# Diretórios
SRC_DIR = src
BUILD_DIR = build
RUNTIME_DIR = $(SRC_DIR)/runtime
LEXER_DIR = $(SRC_DIR)/lexer
EXAMPLES_DIR = examples
TESTS_DIR = tests

# Arquivos fonte C++
RUNTIME_SOURCES = $(RUNTIME_DIR)/runtime.cpp
RUNTIME_OBJECTS = $(RUNTIME_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Executáveis
RUNTIME_LIB = $(BUILD_DIR)/libaqua.a
TEST_RUNTIME = $(BUILD_DIR)/test_runtime

# Padrões
.PHONY: all clean test examples runtime lexer docs

# Alvo principal
all: runtime lexer examples

# Criar diretórios de build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/runtime
	mkdir -p $(BUILD_DIR)/lexer
	mkdir -p $(BUILD_DIR)/examples
	mkdir -p $(BUILD_DIR)/tests

# Compilar runtime C++
runtime: $(BUILD_DIR) $(RUNTIME_LIB)

$(RUNTIME_LIB): $(RUNTIME_OBJECTS)
	ar rcs $@ $^

$(BUILD_DIR)/runtime/%.o: $(RUNTIME_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Testar lexer Python
lexer:
	@echo "Testando lexer Python..."
	$(PYTHON) $(LEXER_DIR)/lexer.py

# Compilar e executar exemplos
examples: runtime
	@echo "Compilando exemplos..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $(EXAMPLES_DIR)/hello_world.cpp $(RUNTIME_LIB) -o $(BUILD_DIR)/hello_world
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $(EXAMPLES_DIR)/fibers_channels.cpp $(RUNTIME_LIB) -o $(BUILD_DIR)/fibers_channels
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $(EXAMPLES_DIR)/concurrent_sum.cpp $(RUNTIME_LIB) -o $(BUILD_DIR)/concurrent_sum

# Executar exemplos
run_examples: examples
	@echo "Executando exemplos..."
	@echo "=== Hello World ==="
	$(BUILD_DIR)/hello_world
	@echo "=== Fibras e Canais ==="
	$(BUILD_DIR)/fibers_channels
	@echo "=== Soma Concorrente ==="
	$(BUILD_DIR)/concurrent_sum

# Testes
test: runtime
	@echo "Compilando testes..."
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $(TESTS_DIR)/test_runtime.cpp $(RUNTIME_LIB) -o $(TEST_RUNTIME)
	@echo "Executando testes..."
	$(TEST_RUNTIME)

# Limpar arquivos de build
clean:
	rm -rf $(BUILD_DIR)
	find . -name "*.pyc" -delete
	find . -name "__pycache__" -delete

# Instalar dependências (Ubuntu/Debian)
install_deps:
	sudo apt-get update
	sudo apt-get install -y build-essential g++ python3 python3-pip

# Instalar dependências (Arch Linux)
install_deps_arch:
	sudo pacman -S base-devel gcc python

# Verificar dependências
check_deps:
	@echo "Verificando dependências..."
	@which $(CXX) > /dev/null || (echo "Erro: $(CXX) não encontrado. Instale build-essential." && exit 1)
	@which $(PYTHON) > /dev/null || (echo "Erro: $(PYTHON) não encontrado. Instale python3." && exit 1)
	@echo "Dependências OK!"

# Mostrar ajuda
help:
	@echo "Makefile para Aqua Language"
	@echo ""
	@echo "Alvos disponíveis:"
	@echo "  all          - Compila tudo (runtime, lexer, exemplos)"
	@echo "  runtime      - Compila apenas a runtime C++"
	@echo "  lexer        - Testa o lexer Python"
	@echo "  examples     - Compila os exemplos"
	@echo "  run_examples - Compila e executa os exemplos"
	@echo "  test         - Compila e executa os testes"
	@echo "  clean        - Remove arquivos de build"
	@echo "  install_deps - Instala dependências (Ubuntu/Debian)"
	@echo "  check_deps   - Verifica se as dependências estão instaladas"
	@echo "  help         - Mostra esta ajuda"
	@echo ""
	@echo "Exemplo de uso:"
	@echo "  make all"
	@echo "  make run_examples"

# Alvo padrão
.DEFAULT_GOAL := all
