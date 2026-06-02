# md5sum (Zig)

Reimplementação minimalista do utilitário `md5sum` do Linux, escrita em
**Zig**. Usa o MD5 da biblioteca padrão (`std.crypto.hash.Md5`) — sem
dependências externas e multiplataforma (Windows, Linux, macOS).

## Funcionalidades

- Calcula o hash MD5 de um ou mais arquivos.
- Lê de `stdin` quando recebido `-` ou nenhum argumento.
- Saída no formato compatível com o `md5sum` do GNU coreutils:
  `<hash em hex>  <nome do arquivo>`.
- Leitura em chunks de 64 KB — funciona com arquivos grandes sem
  carregar tudo na memória.
- Exit code `1` se qualquer arquivo falhar (idem `md5sum` do Linux).
- Suporte a caminhos Unicode (UTF-16 nativo no Windows via
  `std.process.argsAlloc`); nomes de arquivo são impressos em UTF-8.

## Requisitos

- [Zig](https://ziglang.org/) 0.15 ou superior.

## Build

A partir da raiz do projeto:

```sh
zig build
```

Isso gera `zig-out/bin/md5sum` (`md5sum.exe` no Windows).

Para uma build otimizada de release:

```sh
zig build -Doptimize=ReleaseFast
```

Para rodar direto pelo build system:

```sh
zig build run -- arquivo.bin
```

## Uso

```sh
# Um arquivo
md5sum arquivo.bin

# Vários arquivos
md5sum a.txt b.txt c.txt

# Via stdin
type arquivo.bin | md5sum
md5sum - < arquivo.bin
```

Exemplo de saída:

```
d41d8cd98f00b204e9800998ecf8427e  vazio.txt
5d41402abc4b2a76b9719d911017c592  hello.txt
```

## Erros

Se um arquivo não existir ou não puder ser aberto, a mensagem vai para
`stderr` e o programa continua com os demais:

```
> md5sum existe.txt naoexiste.txt
d41d8cd98f00b204e9800998ecf8427e  existe.txt
md5sum: naoexiste.txt: FileNotFound
```

O exit code final será `1` se ao menos um arquivo falhou.

## Aviso de segurança

MD5 está criptograficamente quebrado (colisões práticas desde 2004). Use
apenas para verificação de integridade não-adversarial (checksums de
download, deduplicação, chaves de cache). Para qualquer uso de
segurança, prefira SHA-256 — basta trocar `std.crypto.hash.Md5` por
`std.crypto.hash.sha2.Sha256`.
