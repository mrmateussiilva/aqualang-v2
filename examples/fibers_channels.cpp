#include "runtime/runtime.hpp"
#include <iostream>
#include <chrono>
#include <thread>

using namespace aqua;

// Função executada por uma fibra
void producer_fiber(std::shared_ptr<Channel> channel, int id) {
    for (int i = 0; i < 5; ++i) {
        std::string message = "Mensagem " + std::to_string(i) + " da fibra " + std::to_string(id);
        Value msg(message);
        
        if (channel->send(msg)) {
            std::cout << "📤 Produtor " << id << " enviou: " << message << std::endl;
        }
        
        // Simular trabalho
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "🏁 Produtor " << id << " finalizou" << std::endl;
}

// Função consumidora
void consumer_fiber(std::shared_ptr<Channel> channel, int id) {
    int count = 0;
    
    while (count < 10) { // Consumir 10 mensagens no total
        auto received = channel->receive();
        if (received) {
            std::cout << "📥 Consumidor " << id << " recebeu: " << received->to_string() << std::endl;
            count++;
        } else {
            // Canal fechado
            break;
        }
        
        // Simular processamento
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << "🏁 Consumidor " << id << " finalizou (processou " << count << " mensagens)" << std::endl;
}

int main() {
    std::cout << "🌊 Aqua Language - Demonstração de Fibras e Canais!" << std::endl;
    
    try {
        // Inicializar a runtime
        Runtime& runtime = Runtime::get_instance();
        runtime.initialize();
        
        std::cout << "✅ Runtime inicializada" << std::endl;
        
        // Criar um canal com buffer de 10 mensagens
        auto channel = runtime.make_channel(10);
        std::cout << "✅ Canal criado com capacidade: " << channel->capacity() << std::endl;
        
        // Criar múltiplas fibras produtoras
        std::cout << "\n🚀 Iniciando fibras produtoras..." << std::endl;
        runtime.spawn_fiber([channel]() { producer_fiber(channel, 1); });
        runtime.spawn_fiber([channel]() { producer_fiber(channel, 2); });
        
        // Aguardar um pouco para as produtoras começarem
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Criar fibras consumidoras
        std::cout << "\n🚀 Iniciando fibras consumidoras..." << std::endl;
        runtime.spawn_fiber([channel]() { consumer_fiber(channel, 1); });
        runtime.spawn_fiber([channel]() { consumer_fiber(channel, 2); });
        
        // Aguardar todas as fibras terminarem
        std::cout << "\n⏳ Aguardando fibras terminarem..." << std::endl;
        runtime.get_scheduler().wait_all();
        
        // Fechar o canal
        channel->close();
        std::cout << "🔒 Canal fechado" << std::endl;
        
        // Mostrar estatísticas finais
        std::cout << "\n📊 Estatísticas finais:" << std::endl;
        std::cout << "   - Fibras ativas: " << runtime.get_scheduler().active_fibers() << std::endl;
        std::cout << "   - Total de fibras: " << runtime.get_scheduler().total_fibers() << std::endl;
        std::cout << "   - Objetos alocados: " << runtime.get_gc().allocated_objects() << std::endl;
        std::cout << "   - Memória total: " << runtime.get_gc().total_memory() << " bytes" << std::endl;
        
        // Forçar coleta de lixo
        std::cout << "\n🧹 Executando coleta de lixo..." << std::endl;
        runtime.get_gc().collect();
        
        std::cout << "   - Objetos após GC: " << runtime.get_gc().allocated_objects() << std::endl;
        std::cout << "   - Memória após GC: " << runtime.get_gc().total_memory() << " bytes" << std::endl;
        
        // Finalizar a runtime
        runtime.shutdown();
        std::cout << "✅ Runtime finalizada" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Erro: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "🎉 Demonstração concluída com sucesso!" << std::endl;
    return 0;
}
