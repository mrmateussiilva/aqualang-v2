#include "runtime/runtime.hpp"
#include <iostream>
#include <vector>
#include <numeric>
#include <chrono>

using namespace aqua;

// Função para calcular soma de um intervalo de números
void sum_range_fiber(std::shared_ptr<Channel> result_channel, 
                     const std::vector<int>& numbers, 
                     size_t start, size_t end, 
                     int id) {
    int sum = 0;
    for (size_t i = start; i < end && i < numbers.size(); ++i) {
        sum += numbers[i];
    }
    
    Value result(sum);
    result_channel->send(result);
    
    std::cout << "🧮 Fibra " << id << " calculou soma de " 
              << start << " até " << (end - 1) << ": " << sum << std::endl;
}

// Função para agrupar resultados parciais
void aggregator_fiber(std::shared_ptr<Channel> input_channel, 
                      std::shared_ptr<Channel> output_channel, 
                      int num_fibers) {
    int total_sum = 0;
    int received_count = 0;
    
    while (received_count < num_fibers) {
        auto result = input_channel->receive();
        if (result && result->is<int64_t>()) {
            total_sum += result->get<int64_t>();
            received_count++;
            std::cout << "📊 Agregador recebeu resultado " << received_count 
                      << "/" << num_fibers << std::endl;
        }
    }
    
    Value final_result(total_sum);
    output_channel->send(final_result);
    std::cout << "🎯 Agregador finalizou com soma total: " << total_sum << std::endl;
}

int main() {
    std::cout << "🌊 Aqua Language - Soma Concorrente com Fibras!" << std::endl;
    
    try {
        // Inicializar a runtime
        Runtime& runtime = Runtime::get_instance();
        runtime.initialize();
        
        std::cout << "✅ Runtime inicializada" << std::endl;
        
        // Criar vetor de números para somar
        const int NUM_NUMBERS = 1000000;
        const int NUM_FIBERS = 8;
        const int CHUNK_SIZE = NUM_NUMBERS / NUM_FIBERS;
        
        std::vector<int> numbers(NUM_NUMBERS);
        std::iota(numbers.begin(), numbers.end(), 1); // 1, 2, 3, ..., 1000000
        
        std::cout << "📊 Calculando soma de " << NUM_NUMBERS << " números usando " 
                  << NUM_FIBERS << " fibras" << std::endl;
        
        // Calcular soma sequencial para comparação
        auto start_time = std::chrono::high_resolution_clock::now();
        int sequential_sum = std::accumulate(numbers.begin(), numbers.end(), 0);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto sequential_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        std::cout << "🔢 Soma sequencial: " << sequential_sum 
                  << " (tempo: " << sequential_duration.count() << " μs)" << std::endl;
        
        // Canais para comunicação
        auto partial_results = runtime.make_channel(NUM_FIBERS);
        auto final_result = runtime.make_channel(1);
        
        // Iniciar fibras para calcular somas parciais
        std::cout << "\n🚀 Iniciando fibras de cálculo..." << std::endl;
        for (int i = 0; i < NUM_FIBERS; ++i) {
            size_t start = i * CHUNK_SIZE;
            size_t end = (i == NUM_FIBERS - 1) ? NUM_NUMBERS : (i + 1) * CHUNK_SIZE;
            
            runtime.spawn_fiber([partial_results, &numbers, start, end, i]() {
                sum_range_fiber(partial_results, numbers, start, end, i);
            });
        }
        
        // Iniciar fibra agregadora
        std::cout << "🚀 Iniciando fibra agregadora..." << std::endl;
        runtime.spawn_fiber([partial_results, final_result, NUM_FIBERS]() {
            aggregator_fiber(partial_results, final_result, NUM_FIBERS);
        });
        
        // Aguardar resultado final
        std::cout << "\n⏳ Aguardando resultado final..." << std::endl;
        auto final_result_value = final_result->receive();
        
        if (final_result_value && final_result_value->is<int64_t>()) {
            int concurrent_sum = final_result_value->get<int64_t>();
            
            std::cout << "\n🎯 Resultado da soma concorrente: " << concurrent_sum << std::endl;
            std::cout << "✅ Somas são iguais: " << (sequential_sum == concurrent_sum ? "SIM" : "NÃO") << std::endl;
            
            if (sequential_sum == concurrent_sum) {
                std::cout << "🎉 Cálculo concorrente correto!" << std::endl;
            } else {
                std::cout << "❌ Erro no cálculo concorrente!" << std::endl;
            }
        }
        
        // Fechar canais
        partial_results->close();
        final_result->close();
        
        // Mostrar estatísticas
        std::cout << "\n📊 Estatísticas da execução:" << std::endl;
        std::cout << "   - Fibras criadas: " << runtime.get_scheduler().total_fibers() << std::endl;
        std::cout << "   - Objetos alocados: " << runtime.get_gc().allocated_objects() << std::endl;
        std::cout << "   - Memória total: " << runtime.get_gc().total_memory() << " bytes" << std::endl;
        
        // Executar coleta de lixo
        runtime.get_gc().collect();
        std::cout << "   - Objetos após GC: " << runtime.get_gc().allocated_objects() << std::endl;
        
        // Finalizar runtime
        runtime.shutdown();
        std::cout << "✅ Runtime finalizada" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Erro: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "🎉 Programa de soma concorrente concluído!" << std::endl;
    return 0;
}
