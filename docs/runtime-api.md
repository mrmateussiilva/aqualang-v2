# 🌊 Aqua Runtime API

Esta documentação descreve a API da runtime da linguagem de programação Aqua, implementada em C++.

## 📋 Visão Geral

A runtime Aqua fornece:
- **Sistema de Fibras**: Corrotinas leves para concorrência
- **Canais**: Comunicação segura entre fibras
- **Scheduler**: Agendamento automático de fibras
- **Coletor de Lixo**: Gerenciamento automático de memória
- **Sistema de Valores**: Tipos de dados unificados

## 🏗️ Estrutura Principal

### Namespace
```cpp
namespace aqua
```

### Classe Runtime
Classe principal que gerencia todos os componentes da runtime.

```cpp
class Runtime {
public:
    static Runtime& get_instance();
    void initialize();
    void shutdown();
    
    Scheduler& get_scheduler();
    GarbageCollector& get_gc();
    
    std::shared_ptr<Channel> make_channel(size_t buffer_size = 0);
    void spawn_fiber(std::function<void()> func);
    void sleep_ms(int milliseconds);
    
    void set_global(const std::string& name, const Value& value);
    std::optional<Value> get_global(const std::string& name) const;
};
```

## 🔄 Sistema de Fibras

### Classe Fiber
Representa uma fibra (corrotina) na runtime.

```cpp
class Fiber {
public:
    enum class State {
        READY,      // Pronta para execução
        RUNNING,    // Executando
        WAITING,    // Aguardando
        FINISHED,   // Finalizada
        ERROR       // Erro
    };
    
    Fiber(std::function<void()> func);
    ~Fiber();
    
    void start();
    void yield();
    void resume();
    void wait();
    void finish();
    
    State get_state() const;
    size_t get_id() const;
    bool is_finished() const;
    
    void set_local(const std::string& key, const Value& value);
    std::optional<Value> get_local(const std::string& key) const;
};
```

### Classe Scheduler
Gerencia o agendamento e execução de fibras.

```cpp
class Scheduler {
public:
    Scheduler(size_t num_threads = std::thread::hardware_concurrency());
    ~Scheduler();
    
    void spawn(std::function<void()> func);
    void yield();
    void wait_all();
    
    void start();
    void stop();
    bool is_running() const;
    
    size_t active_fibers() const;
    size_t total_fibers() const;
};
```

## 📡 Sistema de Canais

### Classe Channel
Canal de comunicação thread-safe entre fibras.

```cpp
class Channel {
public:
    Channel(size_t buffer_size = 0);
    ~Channel();
    
    bool send(const Value& value);
    std::optional<Value> receive();
    void close();
    
    bool is_closed() const;
    bool is_empty() const;
    bool is_full() const;
    
    size_t size() const;
    size_t capacity() const;
};
```

**Características dos Canais:**
- **Buffer Limitado**: Se `buffer_size > 0`, o canal tem buffer interno
- **Buffer Ilimitado**: Se `buffer_size = 0`, o canal é síncrono
- **Thread-Safe**: Operações são atômicas e thread-safe
- **Bloqueante**: `send()` e `receive()` bloqueiam quando necessário

## 💾 Sistema de Valores

### Classe Value
Representa valores unificados na runtime.

```cpp
class Value {
public:
    Value();
    Value(const ValueType& val);
    
    template<typename T>
    Value(T&& val);
    
    template<typename T>
    T get() const;
    
    template<typename T>
    bool is() const;
    
    std::string type_name() const;
    std::string to_string() const;
};
```

**Tipos Suportados:**
```cpp
using ValueType = std::variant<
    std::nullptr_t,           // null
    bool,                     // boolean
    int64_t,                  // integer
    double,                   // float
    std::string,              // string
    std::shared_ptr<Channel>  // channel
>;
```

## 🗑️ Coletor de Lixo

### Classe GarbageCollector
Gerencia a memória automaticamente.

```cpp
class GarbageCollector {
public:
    GarbageCollector();
    ~GarbageCollector();
    
    void register_object(void* ptr, size_t size);
    void unregister_object(void* ptr);
    void collect();
    
    void set_threshold(size_t threshold);
    size_t get_threshold() const;
    
    size_t allocated_objects() const;
    size_t total_memory() const;
};
```

**Algoritmo:**
- **Mark and Sweep**: Algoritmo de coleta de lixo implementado
- **Threshold**: Coleta automática quando o limite de memória é atingido
- **Thread-Safe**: Operações são thread-safe

## 🚀 Uso Básico

### Inicialização
```cpp
#include "runtime/runtime.hpp"

int main() {
    // Obter instância da runtime
    Runtime& runtime = Runtime::get_instance();
    
    // Inicializar
    runtime.initialize();
    
    // ... seu código aqui ...
    
    // Finalizar
    runtime.shutdown();
    return 0;
}
```

### Criando Fibras
```cpp
// Spawn de uma fibra
runtime.spawn_fiber([]() {
    std::cout << "Executando em uma fibra!" << std::endl;
});

// Aguardar todas as fibras terminarem
runtime.get_scheduler().wait_all();
```

### Usando Canais
```cpp
// Criar canal
auto channel = runtime.make_channel(10);

// Spawn de produtor
runtime.spawn_fiber([channel]() {
    Value msg("Olá do produtor!");
    channel->send(msg);
});

// Spawn de consumidor
runtime.spawn_fiber([channel]() {
    auto received = channel->receive();
    if (received) {
        std::cout << "Recebido: " << received->to_string() << std::endl;
    }
});

// Fechar canal
channel->close();
```

## 🔧 Configuração e Otimização

### Flags de Compilação
```bash
# Compilação com otimizações
g++ -std=c++17 -O3 -DNDEBUG runtime.cpp

# Compilação com debug
g++ -std=c++17 -g -O0 runtime.cpp
```

### Configurações do Scheduler
```cpp
// Scheduler com 4 threads
Scheduler scheduler(4);

// Scheduler com número automático de threads
Scheduler scheduler(std::thread::hardware_concurrency());
```

### Configurações do GC
```cpp
// Definir threshold de memória (1MB)
gc.set_threshold(1024 * 1024);

// Forçar coleta manual
gc.collect();
```

## 📊 Monitoramento e Debug

### Estatísticas do Scheduler
```cpp
auto& scheduler = runtime.get_scheduler();
std::cout << "Fibras ativas: " << scheduler.active_fibers() << std::endl;
std::cout << "Total de fibras: " << scheduler.total_fibers() << std::endl;
```

### Estatísticas do GC
```cpp
auto& gc = runtime.get_gc();
std::cout << "Objetos alocados: " << gc.allocated_objects() << std::endl;
std::cout << "Memória total: " << gc.total_memory() << " bytes" << std::endl;
```

### Estado das Fibras
```cpp
// Verificar estado de uma fibra
if (fiber->get_state() == Fiber::State::RUNNING) {
    std::cout << "Fibra está executando" << std::endl;
}
```

## ⚠️ Considerações de Performance

### Fibras vs Threads
- **Fibras**: Leves, troca de contexto rápida, menor overhead
- **Threads**: Mais pesadas, mas verdadeira concorrência

### Canais
- **Buffer Limitado**: Melhor para controle de fluxo
- **Buffer Ilimitado**: Pode causar vazamentos de memória

### Coletor de Lixo
- **Threshold Baixo**: Coleta mais frequente, menor uso de memória
- **Threshold Alto**: Coleta menos frequente, melhor performance

## 🐛 Troubleshooting

### Problemas Comuns

1. **Fibras não executam**
   - Verificar se o scheduler foi iniciado
   - Verificar se há threads disponíveis

2. **Deadlocks em canais**
   - Verificar se todos os produtores enviam dados
   - Verificar se todos os consumidores recebem dados

3. **Vazamentos de memória**
   - Verificar se objetos são desregistrados do GC
   - Ajustar threshold do GC

### Debug
```cpp
// Habilitar debug
#define AQUA_DEBUG 1

// Logs de debug aparecerão no console
```

## 🔮 Roadmap

### Próximas Versões
- [ ] Suporte a Windows (fibras nativas)
- [ ] Otimizações de performance
- [ ] Mais tipos de dados
- [ ] Sistema de módulos
- [ ] Transpilador para Zig

---

**Para mais informações, consulte o README.md e os exemplos no diretório `examples/`.**
