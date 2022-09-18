#include <stdio.h>
#include <string.h>

#include <openssl/rand.h>

#include "crypt_blowfish.h"
#include "nvcrypt.h"

#define BLOWFISH_GEN_SALT_LEN 7+22+1
#define BLOWFISH_CRYPT_LEN 128

char *
blowfish_gen_salt(char *outbuf)
{
	char	*salt;
	char	 rand_bytes[16];
	int	 rounds = 6;

	RAND_bytes((void *)rand_bytes, 16);

	return _crypt_gensalt_blowfish_rn("$2b$", rounds, rand_bytes, 16,
	    outbuf, BLOWFISH_GEN_SALT_LEN);
}


char *
blowfish_crypt(const char *password, const char *salt, char *outbuf)
{
	return _crypt_blowfish_rn(password, salt, outbuf, BLOWFISH_CRYPT_LEN);

}

int
test()
{
	char output_salt[BLOWFISH_GEN_SALT_LEN];
	char output_crypt[BLOWFISH_CRYPT_LEN];
	char *salt = NULL;
	char *crypt = NULL;

	salt = blowfish_gen_salt(output_salt);

	printf("salt[%lu]: %s\n", strlen(salt), salt);

	crypt = blowfish_crypt("hello", salt, output_crypt);

	printf("crypt[%lu]: %s\n\n", strlen(crypt), crypt);
}
