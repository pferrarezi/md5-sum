#include <windows.h>
#include <bcrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#include <wchar.h>

#ifdef _MSC_VER
#pragma comment(lib, "bcrypt.lib")
#endif

#define MD5_HASH_LEN 16
#define READ_CHUNK   (64 * 1024)

#ifndef NT_SUCCESS
#define NT_SUCCESS(s) (((NTSTATUS)(s)) >= 0)
#endif

/* Converts a UTF-16 string to a freshly allocated UTF-8 buffer.
   Caller frees with free(). Returns NULL on failure. */
static char *wide_to_utf8(const wchar_t *w) {
    int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
    if (n <= 0) return NULL;
    char *s = (char *)malloc((size_t)n);
    if (!s) return NULL;
    if (WideCharToMultiByte(CP_UTF8, 0, w, -1, s, n, NULL, NULL) <= 0) {
        free(s);
        return NULL;
    }
    return s;
}

static int hash_file(BCRYPT_ALG_HANDLE hAlg, const wchar_t *path) {
    FILE *fp;
    int is_stdin = (wcscmp(path, L"-") == 0);
    char *path_utf8 = wide_to_utf8(path);
    if (!path_utf8) {
        fprintf(stderr, "md5sum: path conversion failed\n");
        return 1;
    }

    if (is_stdin) {
        fp = stdin;
        _setmode(_fileno(stdin), _O_BINARY);
    } else {
        fp = _wfopen(path, L"rb");
        if (!fp) {
            fprintf(stderr, "md5sum: %s: %s\n", path_utf8, strerror(errno));
            free(path_utf8);
            return 1;
        }
    }

    BCRYPT_HASH_HANDLE hHash = NULL;
    NTSTATUS st = BCryptCreateHash(hAlg, &hHash, NULL, 0, NULL, 0, 0);
    if (!NT_SUCCESS(st)) {
        fprintf(stderr, "md5sum: BCryptCreateHash failed (0x%08lx)\n", (unsigned long)st);
        if (!is_stdin) fclose(fp);
        free(path_utf8);
        return 1;
    }

    unsigned char *buf = (unsigned char *)malloc(READ_CHUNK);
    if (!buf) {
        BCryptDestroyHash(hHash);
        if (!is_stdin) fclose(fp);
        free(path_utf8);
        fprintf(stderr, "md5sum: out of memory\n");
        return 1;
    }

    size_t n;
    int rc = 0;
    while ((n = fread(buf, 1, READ_CHUNK, fp)) > 0) {
        st = BCryptHashData(hHash, buf, (ULONG)n, 0);
        if (!NT_SUCCESS(st)) {
            fprintf(stderr, "md5sum: BCryptHashData failed (0x%08lx)\n", (unsigned long)st);
            rc = 1;
            break;
        }
    }
    if (!rc && ferror(fp)) {
        fprintf(stderr, "md5sum: %s: read error\n", path_utf8);
        rc = 1;
    }

    if (!rc) {
        unsigned char digest[MD5_HASH_LEN];
        st = BCryptFinishHash(hHash, digest, sizeof(digest), 0);
        if (!NT_SUCCESS(st)) {
            fprintf(stderr, "md5sum: BCryptFinishHash failed (0x%08lx)\n", (unsigned long)st);
            rc = 1;
        } else {
            for (int i = 0; i < MD5_HASH_LEN; i++) printf("%02x", digest[i]);
            printf("  %s\n", path_utf8);
        }
    }

    free(buf);
    BCryptDestroyHash(hHash);
    if (!is_stdin) fclose(fp);
    free(path_utf8);
    return rc;
}

int wmain(int argc, wchar_t **argv) {
    /* Make console interpret our printf output as UTF-8.
       Harmless when stdout is redirected to a file/pipe. */
    SetConsoleOutputCP(CP_UTF8);

    BCRYPT_ALG_HANDLE hAlg = NULL;
    NTSTATUS st = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_MD5_ALGORITHM, NULL, 0);
    if (!NT_SUCCESS(st)) {
        fprintf(stderr, "md5sum: BCryptOpenAlgorithmProvider failed (0x%08lx)\n",
                (unsigned long)st);
        return 1;
    }

    int rc = 0;
    if (argc < 2) {
        rc = hash_file(hAlg, L"-");
    } else {
        for (int i = 1; i < argc; i++) {
            if (hash_file(hAlg, argv[i]) != 0) rc = 1;
        }
    }

    BCryptCloseAlgorithmProvider(hAlg, 0);
    return rc;
}
