# md5sum (Windows / CNG)

Reimplementação minimalista do utilitário `md5sum` do Linux, escrita em C
puro para Windows usando a API **CNG** (`bcrypt.dll`). Sem dependências
externas — só o que já vem no SO desde o Windows Vista.

## Funcionalidades

- Calcula o hash MD5 de um ou mais arquivos.
- Lê de `stdin` quando recebido `-` ou nenhum argumento.
- Saída no formato compatível com o `md5sum` do GNU coreutils:
  `<hash em hex>  <nome do arquivo>`.
- Leitura em chunks de 64 KB — funciona com arquivos grandes sem
  carregar tudo na memória.
- Exit code `1` se qualquer arquivo falhar (idem `md5sum` do Linux).

## Requisitos

- Windows Vista ou superior.
- [MinGW-w64](https://www.mingw-w64.org/) (`gcc`) + `mingw32-make`
  (ou `make` do MSYS2).

No MSYS2 basta:

```sh
pacman -S mingw-w64-x86_64-gcc make
```

## Build

A partir da raiz do projeto:

```sh
mingw32-make
```

ou simplesmente `make` se estiver no shell do MSYS2. Isso gera
`md5sum.exe` no diretório atual.

Para compilar manualmente sem o Makefile:

```sh
gcc -O2 -Wall -Wextra -std=c11 -o md5sum.exe md5sum.c -lbcrypt
```

Para limpar:

```sh
mingw32-make clean
```

## Uso

```sh
# Um arquivo
md5sum.exe arquivo.bin

# Vários arquivos
md5sum.exe a.txt b.txt c.txt

# Via stdin
type arquivo.bin | md5sum.exe
md5sum.exe - < arquivo.bin
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
> md5sum.exe existe.txt naoexiste.txt
d41d8cd98f00b204e9800998ecf8427e  existe.txt
md5sum: naoexiste.txt: No such file or directory
```

O exit code final será `1` se ao menos um arquivo falhou.

## Aviso de segurança

MD5 está criptograficamente quebrado (colisões práticas desde 2004). Use
apenas para verificação de integridade não-adversarial (checksums de
download, deduplicação, chaves de cache). Para qualquer uso de
segurança, prefira SHA-256 — basta trocar `BCRYPT_MD5_ALGORITHM` por
`BCRYPT_SHA256_ALGORITHM` e ajustar o tamanho do buffer de digest para
32 bytes.
