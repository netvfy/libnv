#ifndef NV_CRYPTO_H
#define NV_CRYPTO_H
char	*blowfish_gen_salt(char *);
char	*blowfish_crypt(const char *, const char *, char *);
#endif
