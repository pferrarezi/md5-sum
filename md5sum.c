#include <windows.h>
#include <bcrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>

#ifdef _MSC_VER
#pragma comment(lib, "bcrypt.lib")
#endif

#define MD5_HASH_LEN 16
#define READ_CHUNK   (64 * 1024)

#ifndef NT_SUCCESS
#define NT_SUCCESS(s) (((NTSTATUS)(s)) >= 0)
#endif

static int hash_file(BCRYPT_ALG_HANDLE hAlg, const char *path) {
    FILE *fp;
    if (strcmp(path, "-") == 0) {
        fp = stdin;
        _setmode(_fileno(stdin), _O_BINARY);
    } else {
        fp = fopen(path, "rb");
        if (!fp) {
            fprintf(stderr, "md5sum: %s: %s\n", path, strerror(errno));
            return 1;
        }
    }

    BCRYPT_HASH_HANDLE hHash = NULL;
    NTSTATUS st = BCryptCreateHash(hAlg, &hHash, NULL, 0, NULL, 0, 0);
    if (!NT_SUCCESS(st)) {
        fprintf(stderr, "md5sum: BCryptCreateHash failed (0x%08lx)\n", (unsigned long)st);
        if (fp != stdin) fclose(fp);
        return 1;
    }

    unsigned char *buf = (unsigned char *)malloc(READ_CHUNK);
    if (!buf) {
        BCryptDestroyHash(hHash);
        if (fp != stdin) fclose(fp);
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
        fprintf(stderr, "md5sum: %s: read error\n", path);
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
            printf("  %s\n", path);
        }
    }

    free(buf);
    BCryptDestroyHash(hHash);
    if (fp != stdin) fclose(fp);
    return rc;
}

int main(int argc, char **argv) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    NTSTATUS st = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_MD5_ALGORITHM, NULL,
                                              BCRYPT_HASH_REUSABLE_FLAG);
    if (!NT_SUCCESS(st)) {
        fprintf(stderr, "md5sum: BCryptOpenAlgorithmProvider failed (0x%08lx)\n",
                (unsigned long)st);
        return 1;
    }

    int rc = 0;
    if (argc < 2) {
        rc = hash_file(hAlg, "-");
    } else {
        for (int i = 1; i < argc; i++) {
            if (hash_file(hAlg, argv[i]) != 0) rc = 1;
        }
    }

    BCryptCloseAlgorithmProvider(hAlg, 0);
    return rc;
}
