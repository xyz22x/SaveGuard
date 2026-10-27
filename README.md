# SaveGuard

SaveGuard é um aplicativo de desktop para recuperar espaço em disco: ele encontra arquivos duplicados por conteúdo (SHA-256), lista os maiores arquivos de uma pasta e envia arquivos para a Lixeira do sistema após uma central de revisão. Nenhuma exclusão é permanente e nenhuma exclusão automática acontece: toda remoção passa por seleção manual, revisão e revalidação do arquivo.

## Funcionalidades

- **Painel**: uso real das unidades (QStorageInfo), resumo da última análise (arquivos analisados, grupos de duplicatas, espaço recuperável calculado uma única vez por grupo) e atalhos para as outras páginas.
- **Duplicatas**: análise recursiva de múltiplas pastas com agrupamento por tamanho e confirmação por hash em duas etapas (primeiros 64 KiB e, quando necessário, hash completo do arquivo). Cada grupo mostra caminho, tamanho, data de modificação e espaço recuperável. É possível abrir o arquivo, revelar o local no gerenciador de arquivos e marcar cópias para remoção (ao menos uma cópia por grupo é mantida).
- **Arquivos grandes**: limites pré-definidos (100 MB, 500 MB, 1 GB) ou personalizado, filtro por extensão, busca por nome, ordenação por qualquer coluna e seleção múltipla.
- **Limpeza segura**: nunca exclui automaticamente. A central de revisão mostra quantidade, caminhos e bytes totais; a remoção usa apenas `QFile::moveToTrash`; cada arquivo é revalidado (tamanho, data e conteúdo) no momento da remoção; caminhos protegidos do sistema são recusados; links simbólicos nunca são seguidos nem removidos; ações destrutivas ficam bloqueadas durante uma análise; toda operação é registrada no histórico.
- **Histórico**: registros locais em JSON (data, duração, pastas, contagem de arquivos, espaço recuperável, itens enviados à Lixeira, erros), com botão para apagar todo o histórico. Nenhum dado pessoal é armazenado além dos caminhos que você mesmo analisou.
- **Configurações**: pastas ignoradas, extensões ignoradas, tamanho mínimo por arquivo, limite de tarefas simultâneas de hash e idioma da interface (português do Brasil ou inglês).

Diretórios críticos do sistema (`Windows`, `Program Files`, `/usr`, `/etc` e similares) nunca são analisados nem removidos. Links simbólicos e junctions são ignorados durante o percurso, o que também elimina ciclos de diretórios.

## Requisitos

- CMake 3.24 ou superior
- Compilador C++20: MSVC 2022 (Windows) ou GCC 11+/Clang 13+ (Linux)
- Qt 6.5 ou superior (módulos Core, Gui, Widgets, Test)
- Ninja (opcional; qualquer gerador serve)

## Compilação no Windows

No prompt de comando do desenvolvedor (x64 Native Tools Command Prompt) ou chamando `vcvars64.bat`:

```bat
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.8.2/msvc2022_64
cmake --build build
```

Ajuste `CMAKE_PREFIX_PATH` para o diretório do seu kit Qt (msvc).

## Compilação no Linux

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/caminho/do/Qt/6.8.2/gcc_64
cmake --build build
```

Em muitas distribuições o Qt instalado pelo gerenciador de pacotes é detectado sem `CMAKE_PREFIX_PATH`:

```bash
sudo apt install cmake ninja-build qt6-base-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Executar e testar

```bash
./build/SaveGuardTests       # suíte Qt Test (15 casos)
ctest --test-dir build       # mesma suíte pelo CTest
./build/SaveGuard            # aplicativo
```

No Windows, se as DLLs do Qt não forem encontradas, adicione o diretório `bin` do kit ao `PATH` antes de executar os executáveis (ou use `windeployqt` como abaixo).

## Empacotar para distribuição

Windows (gera uma pasta com o aplicativo e as DLLs necessárias):

```bat
windeployqt build\SaveGuard.exe
```

Linux:

```bash
linuxdeployqt build/SaveGuard -appimage
```

## Limitações conhecidas

- A remoção depende da Lixeira do sistema. Em volumes sem suporte (por exemplo, unidades removíveis sem Lixeira ou ambientes sem sessão gráfica no Linux), `QFile::moveToTrash` falha e o SaveGuard informa a falha mantendo o arquivo intacto.
- No Windows, restaurar itens da Lixeira é feito pelo próprio sistema; o SaveGuard não garante restauração.
- A suíte de testes cobre o núcleo (scanner, deduplicação, limpeza, histórico). A interface gráfica não tem testes automatizados e foi verificada manualmente.
- A build documentada e testada foi no Windows 11 com Qt 6.8.2 (MSVC 2022). As instruções para Linux seguem o padrão do Qt, mas não foram executadas nesta máquina.
