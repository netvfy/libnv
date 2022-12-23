#ifndef CRYPT_H
#define CRYPT_H

#define BLOWFISH_GEN_SALT_LEN 7+22+1
#define BLOWFISH_CRYPT_LEN 128

char	*blowfish_gen_salt(char *outbuf);
char	*blowfish_crypt(const char *, const char *salt, char *);

#endif
