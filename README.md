# 🌊 Aqua - Linguagem de Programação Expressiva e Concorrente

> **Aqua** é uma linguagem de programação expressiva, simples e robusta, criada para quem busca concorrência eficiente e uma sintaxe fluída, sem abrir mão da performance. 🚀💡

## ✨ Características Principais

- **🔹 Concorrência Simples**: Baseada em canais e fibras, no estilo do Go, sem complicação!
- **🔹 Sintaxe Clean**: Inspirada no Python, sem chaves `{}` – apenas código claro e direto
- **🔹 Coletor de Lixo**: Gerenciamento de memória automático, sem estresse!
- **🔹 Futuro Promissor**: Aqua será interpretada inicialmente, com um transpilador para Zig em desenvolvimento

## 🚀 Status do Projeto

- [x] Design da linguagem
- [x] Estrutura do projeto
- [ ] Lexer (analisador léxico)
- [ ] Parser (analisador sintático)
- [ ] Runtime com fibras e canais
- [ ] Interpretador básico
- [ ] Transpilador para Zig
- [ ] Coletor de lixo

## 📁 Estrutura do Projeto

```
aquaLang-2/
├── src/                    # Código fonte principal
│   ├── lexer/             # Analisador léxico
│   ├── parser/            # Analisador sintático
│   ├── runtime/           # Runtime com fibras e canais
│   └── interpreter/       # Interpretador
├── examples/              # Exemplos de código Aqua
├── tests/                 # Testes unitários
├── docs/                  # Documentação
└── tools/                 # Ferramentas auxiliares
```

## 💻 Exemplo de Sintaxe

```aqua
# Programa Aqua - Hello World com concorrência
import std.io

func main()
    # Criando um canal para comunicação
    let channel = make_channel<string>()
    
    # Iniciando uma fibra para imprimir mensagens
    spawn print_messages(channel)
    
    # Enviando mensagens para o canal
    channel.send("Olá, Aqua!")
    channel.send("Concorrência é simples assim!")
    
    # Aguardando um pouco
    sleep(100ms)
    
    # Fechando o canal
    channel.close()

func print_messages(channel: Channel<string>)
    loop
        match channel.receive()
            case Some(msg) => io.println("Recebido: " + msg)
            case None => break

# Executando o programa
main()
```

## 🛠️ Como Executar

```bash
# Clonar o repositório
git clone https://github.com/seu-usuario/aquaLang-2.git
cd aquaLang-2

# Instalar dependências (quando implementado)
# npm install  # ou cargo install, etc.

# Executar exemplos (quando implementado)
# aqua examples/hello.aqua
```

## 🧪 Testes

```bash
# Executar testes (quando implementado)
# npm test
# cargo test
```

## 📚 Documentação

- [Especificação da Linguagem](docs/specification.md)
- [Guia de Uso](docs/usage.md)
- [Exemplos](examples/)
- [API de Runtime](docs/runtime-api.md)

## 🤝 Contribuindo

Aqua está em desenvolvimento ativo! Contribuições são bem-vindas:

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## 🌟 Roadmap

- **Fase 1**: Lexer e Parser básicos
- **Fase 2**: Runtime com fibras e canais
- **Fase 3**: Interpretador funcional
- **Fase 4**: Transpilador para Zig
- **Fase 5**: Otimizações e coletor de lixo
- **Fase 6**: Biblioteca padrão robusta

---

**Aqua está chegando para transformar a forma como pensamos em programação. Prepare-se para a revolução! 🌊💻✨**
