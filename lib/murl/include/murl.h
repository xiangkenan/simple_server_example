#ifndef _MURL_H
#define _MURL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 remember to add curl and cares and so many others libs 
 when using murl

LDFLAG = -L$(THIRD64)/curl/lib -lcurl -lcares\
		 -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err \
		 -lidn -lssl -lcrypto -lkrb5support -lkeyutils \
		 -lselinux -lsepol -lldap -lrt \
*/

#define MURLS_OK 0
#define MURLS_NONEXIST 1
#define MURLS_TOPERFORM 2

#define MURLE_OK 0
#define MURLE_TIMEOUT -1
#define MURLE_OVERFLOW -2
#define MURLE_INVALID_URL -3
#define MURLE_TOOMUCH_URL -4
#define MURLE_MULTI -5
#define MURLE_NULL -6

struct murl_t;

struct murl_t *murl_malloc(int size);
int murl_free(struct murl_t *murl);

int murl_clean(struct murl_t *murl);

// buf的前32位作为返回数据的大小保存,真正的数据从32位之后开始
int murl_add_url(struct murl_t *murl, const char *url, char *buf, int size, long timeout_ms,
        const char *cookie, const char *token);
int murl_remove_url(struct murl_t *murl, const char *url);
int murl_url_status(struct murl_t *murl, const char *url, char *error, int size);

// for long connection
struct murl_t *murl_malloc_long_connection(int size);
int murl_clean_long_connection(struct murl_t *murl);
int murl_add_url_long_connection(struct murl_t *murl, const char *url, char *buf, int size, long timeout_ms,
        const char *cookie, const char *token);
int murl_remove_url_long_connection(struct murl_t *murl, const char *url);

// 一次性返回的,不能重复调用
int murl_get_contents(struct murl_t *murl, long timeout_ms);

// 返回剩余url数
// 可以重复调用
int murl_perform(struct murl_t *murl, long timeout_ms);

// easy tool
// cookie可为空
// buf的前32位作为返回数据的大小保存,真正的数据从32位之后开始
int murl_get_url(const char *url, char *buf, int size, long timeout_ms,
        const char *cookie, const char *token, const char *args);

#ifdef __cplusplus
}
#endif

#endif
