#pragma once

#include <memory>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>
#include <optional>

namespace aqua {

// Forward declarations
class Fiber;
class Channel;
class Scheduler;
class GarbageCollector;
class Value;

// Tipos de valores suportados
using ValueType = std::variant<
    std::nullptr_t,
    bool,
    int64_t,
    double,
    std::string,
    std::shared_ptr<Channel>
>;

// Classe para representar valores na runtime
class Value {
public:
    Value() : data(std::nullptr_t{}) {}
    Value(const ValueType& val) : data(val) {}
    
    template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Value>>>
    Value(T&& val) : data(std::forward<T>(val)) {}
    
    // Construtor de cópia
    Value(const Value& other) : data(other.data) {}
    
    // Construtor de movimento
    Value(Value&& other) noexcept : data(std::move(other.data)) {}
    
    // Operador de atribuição
    Value& operator=(const Value& other) {
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }
    
    // Operador de atribuição de movimento
    Value& operator=(Value&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
        }
        return *this;
    }
    
    template<typename T>
    T get() const {
        return std::get<T>(data);
    }
    
    template<typename T>
    bool is() const {
        return std::holds_alternative<T>(data);
    }
    
    std::string type_name() const;
    std::string to_string() const;
    
private:
    ValueType data;
};

// Classe para representar canais de comunicação
class Channel {
public:
    Channel(size_t buffer_size = 0);
    ~Channel();
    
    // Operações de canal
    bool send(const Value& value);
    std::optional<Value> receive();
    void close();
    bool is_closed() const;
    bool is_empty() const;
    bool is_full() const;
    
    // Getters
    size_t size() const;
    size_t capacity() const;
    
private:
    std::queue<Value> buffer;
    size_t max_size;
    bool closed;
    mutable std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;
};

// Classe para representar uma fibra (coroutine)
class Fiber {
public:
    enum class State {
        READY,
        RUNNING,
        WAITING,
        FINISHED,
        ERROR
    };
    
    Fiber(std::function<void()> func);
    ~Fiber();
    
    // Controle da fibra
    void start();
    void yield();
    void resume();
    void wait();
    void finish();
    
    // Getters
    State get_state() const;
    size_t get_id() const;
    bool is_finished() const;
    
    // Contexto da fibra
    void set_local(const std::string& key, const Value& value);
    std::optional<Value> get_local(const std::string& key) const;
    
private:
    static size_t next_id;
    size_t id;
    State state;
    std::function<void()> function;
    std::unordered_map<std::string, Value> locals;
    
    // Contexto de execução (será implementado com ucontext ou similar)
    void* context;
    void* stack;
    size_t stack_size;
    
    void setup_context();
    void cleanup_context();
};

// Agendador de fibras
class Scheduler {
public:
    Scheduler(size_t num_threads = std::thread::hardware_concurrency());
    ~Scheduler();
    
    // Gerenciamento de fibras
    void spawn(std::function<void()> func);
    void yield();
    void wait_all();
    
    // Controle do agendador
    void start();
    void stop();
    bool is_running() const;
    
    // Estatísticas
    size_t active_fibers() const;
    size_t total_fibers() const;
    
private:
    std::vector<std::thread> workers;
    std::queue<std::shared_ptr<Fiber>> ready_queue;
    std::vector<std::shared_ptr<Fiber>> running_fibers;
    std::vector<std::shared_ptr<Fiber>> waiting_fibers;
    
    mutable std::mutex queue_mutex;
    std::condition_variable worker_condition;
    bool running;
    
    void worker_loop();
    void schedule_fiber(std::shared_ptr<Fiber> fiber);
    std::shared_ptr<Fiber> get_next_fiber();
};

// Coletor de lixo simples
class GarbageCollector {
public:
    GarbageCollector();
    ~GarbageCollector();
    
    // Gerenciamento de memória
    void register_object(void* ptr, size_t size);
    void unregister_object(void* ptr);
    void collect();
    
    // Configuração
    void set_threshold(size_t threshold);
    size_t get_threshold() const;
    
    // Estatísticas
    size_t allocated_objects() const;
    size_t total_memory() const;
    
private:
    struct ObjectInfo {
        void* ptr;
        size_t size;
        bool marked;
    };
    
    std::unordered_map<void*, ObjectInfo> objects;
    size_t memory_threshold;
    size_t total_allocated;
    
    mutable std::mutex gc_mutex;
    
    void mark_and_sweep();
    void mark_reachable_objects();
    void sweep_unreachable_objects();
};

// Runtime principal da linguagem Aqua
class Runtime {
public:
    Runtime();
    ~Runtime();
    
    // Inicialização e finalização
    void initialize();
    void shutdown();
    
    // Acesso aos componentes
    Scheduler& get_scheduler();
    GarbageCollector& get_gc();
    
    // Funções utilitárias
    std::shared_ptr<Channel> make_channel(size_t buffer_size = 0);
    void spawn_fiber(std::function<void()> func);
    void sleep_ms(int milliseconds);
    
    // Variáveis globais
    void set_global(const std::string& name, const Value& value);
    std::optional<Value> get_global(const std::string& name) const;
    
private:
    std::unique_ptr<Scheduler> scheduler;
    std::unique_ptr<GarbageCollector> gc;
    std::unordered_map<std::string, Value> globals;
    mutable std::mutex globals_mutex;
    
    static Runtime* instance;
    
public:
    static Runtime& get_instance();
};

// Funções utilitárias globais
std::shared_ptr<Channel> make_channel(size_t buffer_size = 0);
void spawn(std::function<void()> func);
void sleep(int milliseconds);

} // namespace aqua
