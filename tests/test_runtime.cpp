#include "runtime/runtime.hpp"
#include <iostream>
#include <cassert>
#include <string>

using namespace aqua;

// Função para executar testes
void run_tests() {
    std::cout << "🧪 Executando testes da runtime Aqua..." << std::endl;
    
    int tests_passed = 0;
    int total_tests = 0;
    
    // Teste 1: Criação de valores
    {
        total_tests++;
        std::cout << "  Teste 1: Criação de valores... ";
        
        Value null_val;
        Value bool_val(true);
        Value int_val(42);
        Value float_val(3.14);
        Value string_val("teste");
        
        assert(null_val.is<std::nullptr_t>());
        assert(bool_val.is<bool>());
        assert(int_val.is<int64_t>());
        assert(float_val.is<double>());
        assert(string_val.is<std::string>());
        
        assert(bool_val.get<bool>() == true);
        assert(int_val.get<int64_t>() == 42);
        assert(float_val.get<double>() == 3.14);
        assert(string_val.get<std::string>() == "teste");
        
        std::cout << "✅ PASSOU" << std::endl;
        tests_passed++;
    }
    
    // Teste 2: Canais
    {
        total_tests++;
        std::cout << "  Teste 2: Operações de canal... ";
        
        Channel channel(2);
        
        // Testar envio e recebimento
        Value msg1("mensagem 1");
        Value msg2("mensagem 2");
        
        assert(channel.send(msg1));
        assert(channel.send(msg2));
        
        auto received1 = channel.receive();
        auto received2 = channel.receive();
        
        assert(received1 && received1->get<std::string>() == "mensagem 1");
        assert(received2 && received2->get<std::string>() == "mensagem 2");
        
        // Testar fechamento
        channel.close();
        assert(channel.is_closed());
        
        std::cout << "✅ PASSOU" << std::endl;
        tests_passed++;
    }
    
    // Teste 3: Runtime básica
    {
        total_tests++;
        std::cout << "  Teste 3: Runtime básica... ";
        
        Runtime& runtime = Runtime::get_instance();
        runtime.initialize();
        
        // Testar criação de canal
        auto channel = runtime.make_channel(5);
        assert(channel != nullptr);
        assert(channel->capacity() == 5);
        
        // Testar variáveis globais
        runtime.set_global("test_var", Value(123));
        auto global_val = runtime.get_global("test_var");
        assert(global_val && global_val->get<int64_t>() == 123);
        
        runtime.shutdown();
        
        std::cout << "✅ PASSOU" << std::endl;
        tests_passed++;
    }
    
    // Teste 4: Scheduler de fibras
    {
        total_tests++;
        std::cout << "  Teste 4: Scheduler de fibras... ";
        
        Runtime& runtime = Runtime::get_instance();
        runtime.initialize();
        
        bool fiber_executed = false;
        
        // Spawn de uma fibra simples
        runtime.spawn_fiber([&fiber_executed]() {
            fiber_executed = true;
        });
        
        // Aguardar um pouco para a fibra executar
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Aguardar todas as fibras terminarem
        runtime.get_scheduler().wait_all();
        
        assert(fiber_executed);
        
        runtime.shutdown();
        
        std::cout << "✅ PASSOU" << std::endl;
        tests_passed++;
    }
    
    // Teste 5: Coletor de lixo
    {
        total_tests++;
        std::cout << "  Teste 2: Coletor de lixo... ";
        
        Runtime& runtime = Runtime::get_instance();
        runtime.initialize();
        
        GarbageCollector& gc = runtime.get_gc();
        
        // Simular alocação de objetos
        void* test_ptr1 = malloc(100);
        void* test_ptr2 = malloc(200);
        
        gc.register_object(test_ptr1, 100);
        gc.register_object(test_ptr2, 200);
        
        assert(gc.allocated_objects() == 2);
        assert(gc.total_memory() == 300);
        
        // Executar coleta
        gc.collect();
        
        // Limpar objetos de teste
        gc.unregister_object(test_ptr1);
        gc.unregister_object(test_ptr2);
        free(test_ptr1);
        free(test_ptr2);
        
        runtime.shutdown();
        
        std::cout << "✅ PASSOU" << std::endl;
        tests_passed++;
    }
    
    // Teste 6: Comunicação entre fibras via canais
    {
        total_tests++;
        std::cout << "  Teste 6: Comunicação entre fibras... ";
        
        Runtime& runtime = Runtime::get_instance();
        runtime.initialize();
        
        auto channel = runtime.make_channel(10);
        std::string received_message;
        bool message_received = false;
        
        // Fibra produtora
        runtime.spawn_fiber([channel]() {
            Value msg("Olá da fibra!");
            channel->send(msg);
        });
        
        // Fibra consumidora
        runtime.spawn_fiber([channel, &received_message, &message_received]() {
            auto msg = channel->receive();
            if (msg && msg->is<std::string>()) {
                received_message = msg->get<std::string>();
                message_received = true;
            }
        });
        
        // Aguardar comunicação
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        runtime.get_scheduler().wait_all();
        
        assert(message_received);
        assert(received_message == "Olá da fibra!");
        
        channel->close();
        runtime.shutdown();
        
        std::cout << "✅ PASSOU" << std::endl;
        tests_passed++;
    }
    
    // Resumo dos testes
    std::cout << "\n📊 Resumo dos testes:" << std::endl;
    std::cout << "   - Testes passaram: " << tests_passed << "/" << total_tests << std::endl;
    
    if (tests_passed == total_tests) {
        std::cout << "🎉 Todos os testes passaram!" << std::endl;
    } else {
        std::cout << "❌ " << (total_tests - tests_passed) << " teste(s) falharam!" << std::endl;
    }
}

int main() {
    std::cout << "🌊 Aqua Language - Testes da Runtime" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    try {
        run_tests();
        std::cout << "\n✅ Todos os testes executados com sucesso!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Erro durante os testes: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n❌ Erro desconhecido durante os testes!" << std::endl;
        return 1;
    }
}
