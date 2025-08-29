#include "runtime/runtime.hpp"
#include <iostream>

using namespace aqua;

int main() {
    std::cout << "🌊 Aqua Language - Hello World!" << std::endl;
    
    try {
        // Inicializar a runtime
        Runtime& runtime = Runtime::get_instance();
        runtime.initialize();
        
        std::cout << "✅ Runtime inicializada com sucesso!" << std::endl;
        
        // Criar um canal simples
        auto channel = runtime.make_channel(5);
        std::cout << "✅ Canal criado com capacidade: " << channel->capacity() << std::endl;
        
        // Enviar uma mensagem
        Value message("Olá, Aqua! 🌊");
        if (channel->send(message)) {
            std::cout << "✅ Mensagem enviada para o canal" << std::endl;
        }
        
        // Receber a mensagem
        auto received = channel->receive();
        if (received) {
            std::cout << "📨 Mensagem recebida: " << received->to_string() << std::endl;
        }
        
        // Fechar o canal
        channel->close();
        std::cout << "🔒 Canal fechado" << std::endl;
        
        // Mostrar estatísticas
        std::cout << "📊 Estatísticas da Runtime:" << std::endl;
        std::cout << "   - Fibras ativas: " << runtime.get_scheduler().active_fibers() << std::endl;
        std::cout << "   - Total de fibras: " << runtime.get_scheduler().total_fibers() << std::endl;
        std::cout << "   - Objetos alocados: " << runtime.get_gc().allocated_objects() << std::endl;
        std::cout << "   - Memória total: " << runtime.get_gc().total_memory() << " bytes" << std::endl;
        
        // Finalizar a runtime
        runtime.shutdown();
        std::cout << "✅ Runtime finalizada" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Erro: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "🎉 Programa executado com sucesso!" << std::endl;
    return 0;
}
