#include "runtime.hpp"
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>

#ifdef __linux__
#include <ucontext.h>
#elif defined(__APPLE__)
#include <ucontext.h>
#else
// Para Windows, usar fibers nativas ou implementação alternativa
#endif

namespace aqua {

// Implementação da classe Value
std::string Value::type_name() const {
    if (is<std::nullptr_t>()) return "null";
    if (is<bool>()) return "bool";
    if (is<int64_t>()) return "int";
    if (is<double>()) return "float";
    if (is<std::string>()) return "string";
    if (is<std::shared_ptr<Channel>>()) return "channel";
    return "unknown";
}

std::string Value::to_string() const {
    if (is<std::nullptr_t>()) return "null";
    if (is<bool>()) return get<bool>() ? "true" : "false";
    if (is<int64_t>()) return std::to_string(get<int64_t>());
    if (is<double>()) return std::to_string(get<double>());
    if (is<std::string>()) return get<std::string>();
    if (is<std::shared_ptr<Channel>>()) return "channel";
    return "unknown";
}

// Implementação da classe Channel
Channel::Channel(size_t buffer_size) 
    : max_size(buffer_size), closed(false) {
}

Channel::~Channel() = default;

bool Channel::send(const Value& value) {
    std::unique_lock<std::mutex> lock(mutex);
    
    if (closed) {
        return false;
    }
    
    // Se o canal tem buffer limitado, aguardar espaço
    if (max_size > 0 && buffer.size() >= max_size) {
        not_full.wait(lock, [this] { return buffer.size() < max_size || closed; });
        if (closed) return false;
    }
    
    buffer.push(value);
    not_empty.notify_one();
    return true;
}

std::optional<Value> Channel::receive() {
    std::unique_lock<std::mutex> lock(mutex);
    
    // Aguardar até que haja dados ou o canal seja fechado
    not_empty.wait(lock, [this] { return !buffer.empty() || closed; });
    
    if (buffer.empty() && closed) {
        return std::nullopt;
    }
    
    Value value(buffer.front());
    buffer.pop();
    
    if (max_size > 0) {
        not_full.notify_one();
    }
    
    return value;
}

void Channel::close() {
    std::unique_lock<std::mutex> lock(mutex);
    closed = true;
    not_empty.notify_all();
    not_full.notify_all();
}

bool Channel::is_closed() const {
    std::unique_lock<std::mutex> lock(mutex);
    return closed;
}

bool Channel::is_empty() const {
    std::unique_lock<std::mutex> lock(mutex);
    return buffer.empty();
}

bool Channel::is_full() const {
    if (max_size == 0) return false;
    std::unique_lock<std::mutex> lock(mutex);
    return buffer.size() >= max_size;
}

size_t Channel::size() const {
    std::unique_lock<std::mutex> lock(mutex);
    return buffer.size();
}

size_t Channel::capacity() const {
    return max_size;
}

// Implementação da classe Fiber
size_t Fiber::next_id = 1;

Fiber::Fiber(std::function<void()> func) 
    : id(next_id++), state(State::READY), function(std::move(func)), 
      context(nullptr), stack(nullptr), stack_size(8192) {
    setup_context();
}

Fiber::~Fiber() {
    cleanup_context();
}

void Fiber::setup_context() {
#ifdef __linux__
    stack = malloc(stack_size);
    if (stack) {
        ucontext_t* ctx = new ucontext_t();
        getcontext(ctx);
        ctx->uc_stack.ss_sp = stack;
        ctx->uc_stack.ss_size = stack_size;
        ctx->uc_link = nullptr;
        
        // Por enquanto, não usamos makecontext devido à complexidade
        // com lambdas. Em uma implementação real, seria necessário
        // um sistema mais sofisticado de gerenciamento de contexto.
        context = ctx;
    }
#endif
}

void Fiber::cleanup_context() {
#ifdef __linux__
    if (context) {
        delete static_cast<ucontext_t*>(context);
        context = nullptr;
    }
    if (stack) {
        free(stack);
        stack = nullptr;
    }
#endif
}

void Fiber::start() {
    if (state == State::READY) {
        state = State::RUNNING;
        // Aqui seria feita a troca de contexto real
        function();
        state = State::FINISHED;
    }
}

void Fiber::yield() {
    if (state == State::RUNNING) {
        state = State::READY;
        // Troca de contexto para o scheduler
    }
}

void Fiber::resume() {
    if (state == State::READY) {
        state = State::RUNNING;
        // Troca de contexto para esta fibra
    }
}

void Fiber::wait() {
    if (state == State::RUNNING) {
        state = State::WAITING;
        // Troca de contexto para o scheduler
    }
}

void Fiber::finish() {
    state = State::FINISHED;
}

Fiber::State Fiber::get_state() const {
    return state;
}

size_t Fiber::get_id() const {
    return id;
}

bool Fiber::is_finished() const {
    return state == State::FINISHED || state == State::ERROR;
}

void Fiber::set_local(const std::string& key, const Value& value) {
    locals[key] = value;
}

std::optional<Value> Fiber::get_local(const std::string& key) const {
    auto it = locals.find(key);
    if (it != locals.end()) {
        return it->second;
    }
    return std::nullopt;
}

// Implementação da classe Scheduler
Scheduler::Scheduler(size_t num_threads) : running(false) {
    workers.reserve(num_threads);
}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    if (running) return;
    
    running = true;
    
    for (size_t i = 0; i < workers.capacity(); ++i) {
        workers.emplace_back(&Scheduler::worker_loop, this);
    }
}

void Scheduler::stop() {
    if (!running) return;
    
    running = false;
    worker_condition.notify_all();
    
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers.clear();
}

void Scheduler::spawn(std::function<void()> func) {
    auto fiber = std::make_shared<Fiber>(std::move(func));
    
    std::lock_guard<std::mutex> lock(queue_mutex);
    ready_queue.push(fiber);
    worker_condition.notify_one();
}

void Scheduler::yield() {
    // Implementação simplificada - em uma implementação real,
    // isso seria chamado pelo sistema de fibras
}

void Scheduler::wait_all() {
    while (true) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (ready_queue.empty() && running_fibers.empty() && waiting_fibers.empty()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool Scheduler::is_running() const {
    return running;
}

size_t Scheduler::active_fibers() const {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return running_fibers.size();
}

size_t Scheduler::total_fibers() const {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return ready_queue.size() + running_fibers.size() + waiting_fibers.size();
}

void Scheduler::worker_loop() {
    while (running) {
        auto fiber = get_next_fiber();
        if (fiber) {
            if (fiber->get_state() == Fiber::State::READY) {
                fiber->start();
            }
        } else {
            std::unique_lock<std::mutex> lock(queue_mutex);
            worker_condition.wait_for(lock, std::chrono::milliseconds(10));
        }
    }
}

void Scheduler::schedule_fiber(std::shared_ptr<Fiber> fiber) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    ready_queue.push(fiber);
}

std::shared_ptr<Fiber> Scheduler::get_next_fiber() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (ready_queue.empty()) {
        return nullptr;
    }
    
    auto fiber = ready_queue.front();
    ready_queue.pop();
    
    if (fiber->get_state() == Fiber::State::READY) {
        running_fibers.push_back(fiber);
    }
    
    return fiber;
}

// Implementação da classe GarbageCollector
GarbageCollector::GarbageCollector() 
    : memory_threshold(1024 * 1024), total_allocated(0) {
}

GarbageCollector::~GarbageCollector() = default;

void GarbageCollector::register_object(void* ptr, size_t size) {
    std::lock_guard<std::mutex> lock(gc_mutex);
    objects[ptr] = {ptr, size, false};
    total_allocated += size;
    
    if (total_allocated > memory_threshold) {
        collect();
    }
}

void GarbageCollector::unregister_object(void* ptr) {
    std::lock_guard<std::mutex> lock(gc_mutex);
    auto it = objects.find(ptr);
    if (it != objects.end()) {
        total_allocated -= it->second.size;
        objects.erase(it);
    }
}

void GarbageCollector::collect() {
    std::lock_guard<std::mutex> lock(gc_mutex);
    mark_and_sweep();
}

void GarbageCollector::set_threshold(size_t threshold) {
    memory_threshold = threshold;
}

size_t GarbageCollector::get_threshold() const {
    return memory_threshold;
}

size_t GarbageCollector::allocated_objects() const {
    std::lock_guard<std::mutex> lock(gc_mutex);
    return objects.size();
}

size_t GarbageCollector::total_memory() const {
    std::lock_guard<std::mutex> lock(gc_mutex);
    return total_allocated;
}

void GarbageCollector::mark_and_sweep() {
    // Resetar marcação
    for (auto& [ptr, info] : objects) {
        info.marked = false;
    }
    
    // Marcar objetos alcançáveis (simplificado)
    mark_reachable_objects();
    
    // Remover objetos não marcados
    sweep_unreachable_objects();
}

void GarbageCollector::mark_reachable_objects() {
    // Implementação simplificada - em uma implementação real,
    // isso rastrearia referências ativas na runtime
    for (auto& [ptr, info] : objects) {
        // Por enquanto, marcar todos como alcançáveis
        // Em uma implementação real, isso seria baseado no grafo de referências
        info.marked = true;
    }
}

void GarbageCollector::sweep_unreachable_objects() {
    auto it = objects.begin();
    while (it != objects.end()) {
        if (!it->second.marked) {
            total_allocated -= it->second.size;
            it = objects.erase(it);
        } else {
            ++it;
        }
    }
}

// Implementação da classe Runtime
Runtime* Runtime::instance = nullptr;

Runtime::Runtime() {
    scheduler = std::make_unique<Scheduler>();
    gc = std::make_unique<GarbageCollector>();
    instance = this;
}

Runtime::~Runtime() {
    shutdown();
    instance = nullptr;
}

void Runtime::initialize() {
    scheduler->start();
}

void Runtime::shutdown() {
    if (scheduler) {
        scheduler->stop();
    }
}

Scheduler& Runtime::get_scheduler() {
    return *scheduler;
}

GarbageCollector& Runtime::get_gc() {
    return *gc;
}

std::shared_ptr<Channel> Runtime::make_channel(size_t buffer_size) {
    return std::make_shared<Channel>(buffer_size);
}

void Runtime::spawn_fiber(std::function<void()> func) {
    scheduler->spawn(std::move(func));
}

void Runtime::sleep_ms(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Runtime::set_global(const std::string& name, const Value& value) {
    std::lock_guard<std::mutex> lock(globals_mutex);
    globals[name] = value;
}

std::optional<Value> Runtime::get_global(const std::string& name) const {
    std::lock_guard<std::mutex> lock(globals_mutex);
    auto it = globals.find(name);
    if (it != globals.end()) {
        return it->second;
    }
    return std::nullopt;
}

Runtime& Runtime::get_instance() {
    if (!instance) {
        // Criar instância automaticamente se não existir
        instance = new Runtime();
    }
    return *instance;
}

// Funções utilitárias globais
std::shared_ptr<Channel> make_channel(size_t buffer_size) {
    return Runtime::get_instance().make_channel(buffer_size);
}

void spawn(std::function<void()> func) {
    Runtime::get_instance().spawn_fiber(std::move(func));
}

void sleep(int milliseconds) {
    Runtime::get_instance().sleep_ms(milliseconds);
}

} // namespace aqua
