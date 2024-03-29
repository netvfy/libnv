#include <stdio.h>
#include <string.h>

#include <openssl/rand.h>

#include "crypt.h"
#include "crypt_blowfish.h"

char *
blowfish_gen_salt(char *outbuf)
{
	char	 rand_bytes[16];
	int	 rounds = 6;

	RAND_bytes((void *)rand_bytes, 16);

	return _crypt_gensalt_blowfish_rn("$2a$", rounds, rand_bytes, 16,
	    outbuf, BLOWFISH_GEN_SALT_LEN);
}


char *
blowfish_crypt(const char *password, const char *salt, char *outbuf)
{
	return _crypt_blowfish_rn(password, salt, outbuf, BLOWFISH_CRYPT_LEN);
}
