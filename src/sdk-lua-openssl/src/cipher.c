/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2012 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
/*=========================================================================*\
* cipher routines
* lua-openssl toolkit
*
* This product includes PHP software, freely available from <http://www.php.net/software/>
* Author:  george zhao <zhaozg(at)gmail.com>
\*=========================================================================*/
#include "openssl.h"

/* cipher module for the Lua/OpenSSL binding.
 *
 * The functions in this module can be used to load, parse, export, verify... functions.
 * get_cipher()
 * cipher_info()
 */

/*  openssl.get_cipher([null|bool aliases=true]|string alg|int alg_id|openssl.asn1_obj|alg_obj) -> table|openssl.evp_cipher|null{{{1
	openssl.get_cipher([bool aliases = true]) will return all ciphers methods default with alias
	other will return a cipher method
*/
LUA_FUNCTION(openssl_get_cipher) {
    const EVP_CIPHER* cipher = NULL;

    if (lua_isnoneornil(L,1) || lua_isboolean(L,1))
    {
        int aliases = lua_isnoneornil(L,1)?1:lua_toboolean(L,1);

        lua_newtable(L);
        OBJ_NAME_do_all_sorted(OBJ_NAME_TYPE_CIPHER_METH, aliases ? openssl_add_method_or_alias: openssl_add_method, L);
        return 1;
    }
    if (lua_isstring(L,1))
        cipher = EVP_get_cipherbyname(lua_tostring(L,1));
    else if(lua_isnumber(L,1))
        cipher = EVP_get_cipherbynid(lua_tointeger(L,1));
    else if(auxiliar_isclass(L,"openssl.obj",1))
        cipher = EVP_get_cipherbyobj(CHECK_OBJECT(1,ASN1_OBJECT,"openssl.asn1_object"));
    else {
        luaL_typerror(L,1,"please input correct paramater");
    }

    if(cipher)
        PUSH_OBJECT((void*)cipher,"openssl.evp_cipher");
    else
        lua_pushnil(L);
    return 1;
}
/* }}} */

LUA_FUNCTION(openssl_cipher_info)
{
    EVP_CIPHER *cipher = CHECK_OBJECT(1,EVP_CIPHER, "openssl.evp_cipher");
    lua_newtable(L);
    add_assoc_string(L,"name", EVP_CIPHER_name(cipher));
    add_assoc_int(L,"block_size", EVP_CIPHER_block_size(cipher));
    add_assoc_int(L,"key_length", EVP_CIPHER_key_length(cipher));
    add_assoc_int(L,"iv_length", EVP_CIPHER_iv_length(cipher));
    add_assoc_int(L,"flags", EVP_CIPHER_flags(cipher));
    add_assoc_int(L,"mode", EVP_CIPHER_mode(cipher));
    return 1;
}

LUA_FUNCTION(openssl_cipher_tostring)
{
    EVP_CIPHER *cipher = CHECK_OBJECT(1,EVP_CIPHER, "openssl.evp_cipher");
    lua_pushfstring(L,"openssl.evp_cipher:%p",cipher);
    return 1;
}

LUA_FUNCTION(openssl_evp_BytesToKey)
{
    EVP_CIPHER* c = CHECK_OBJECT(1,EVP_CIPHER, "openssl.evp_cipher");
    EVP_MD* m = CHECK_OBJECT(2,EVP_MD, "openssl.evp_digest");
    const char* salt, *k;  /* PKCS5_SALT_LEN */
    size_t lsalt, lk;

    char key[EVP_MAX_KEY_LENGTH],iv[EVP_MAX_IV_LENGTH];

    salt = luaL_checklstring(L, 3, &lsalt);
    if(lsalt < PKCS5_SALT_LEN)
        luaL_error(L, "salt must not shorter than %d",PKCS5_SALT_LEN);
    k = luaL_checklstring(L, 4, &lk);


    EVP_BytesToKey(c,m,(unsigned char*)salt, (unsigned char*)k, lk,1,(unsigned char*)key,(unsigned char*)iv);
    lua_pushlstring(L, key, EVP_MAX_KEY_LENGTH);
    lua_pushlstring(L, iv, EVP_MAX_IV_LENGTH);
    return 2;
}


/*  openssl.evp_encrypt_init(openssl.evp_cipher cipher[, string key [,string iv [,openssl.engine engimp]]])->openssl.evp_cipher_ctx{{{1
*/

LUA_FUNCTION(openssl_evp_encrypt_init)
{
    EVP_CIPHER* c = CHECK_OBJECT(1,EVP_CIPHER, "openssl.evp_cipher");
    const char* k = luaL_optstring(L,2,NULL);
    const char* iv = luaL_optstring(L,3,NULL);
    ENGINE*     e = lua_gettop(L)>3?CHECK_OBJECT(4,ENGINE,"openssl.engine"):NULL;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    PUSH_OBJECT(ctx,"openssl.evp_cipher_ctx");
    EVP_CIPHER_CTX_init(ctx);

    if (!EVP_EncryptInit_ex(ctx,c,e, (const byte*)k, (const byte*)iv)) {
        luaL_error(L,"EVP_EncryptInit failed");
    }
    return 1;
}
/* }}} */

/*  openssl.evp_encrypt_init(openssl.evp_cipher_ctx ctx, string data)->string{{{1
*/
LUA_FUNCTION(openssl_evp_encrypt_update)
{
    EVP_CIPHER_CTX* c = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    size_t inl;
    const char* in= luaL_checklstring(L,2,&inl);
    int outl = inl+EVP_MAX_BLOCK_LENGTH;
    unsigned char* out = malloc(outl);

    if(EVP_EncryptUpdate(c,out,&outl,(const byte*)in,inl) && outl)
    {
        lua_pushlstring(L,(const char*)out,outl);
    }
    free(out);
    return outl?1:0;
}
/* }}} */

/*  openssl.evp_encrypt_final(openssl.evp_cipher_ctx ctx)->string{{{1
*/
LUA_FUNCTION(openssl_evp_encrypt_final)
{
    EVP_CIPHER_CTX* c = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    int outl = EVP_MAX_BLOCK_LENGTH;
    unsigned char out[EVP_MAX_BLOCK_LENGTH];

    if(EVP_EncryptFinal_ex(c,out,&outl) && outl)
    {
        lua_pushlstring(L,(const char*)out,outl);
        return 1;
    }
    return 0;
}
/* }}} */



/*  openssl.evp_decrypt_init(openssl.evp_cipher cipher[, string key [,string iv [,openssl.engine engimp]]])->openssl.evp_cipher_ctx{{{1
*/

LUA_FUNCTION(openssl_evp_decrypt_init)
{
    EVP_CIPHER* c = CHECK_OBJECT(1,EVP_CIPHER, "openssl.evp_cipher");
    const char* k = luaL_optstring(L,2,NULL);
    const char* iv = luaL_optstring(L,3,NULL);
    ENGINE*     e = lua_gettop(L)>3?CHECK_OBJECT(4,ENGINE,"openssl.engine"):NULL;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    PUSH_OBJECT(ctx,"openssl.evp_cipher_ctx");
    EVP_CIPHER_CTX_init(ctx);

    if (!EVP_DecryptInit_ex(ctx,c,e, (const byte*)k, (const byte*)iv)) {
        luaL_error(L,"EVP_DecryptInit_ex failed");
    }
    return 1;
}
/* }}} */

/*  openssl.evp_decrypt_update(openssl.evp_cipher_ctx ctx, string data)->string{{{1
*/
LUA_FUNCTION(openssl_evp_decrypt_update)
{
    EVP_CIPHER_CTX* c = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    size_t inl;
    const char* in= luaL_checklstring(L,2,&inl);
    int outl = inl+EVP_MAX_BLOCK_LENGTH;
    char* out = malloc(outl);

    if(EVP_DecryptUpdate(c,(byte*)out,&outl,(const byte*)in,inl) && outl)
    {
        lua_pushlstring(L,out,outl);
    }
    free(out);
    return outl?1:0;
}
/* }}} */

/*  openssl.evp_decrypt_final(openssl.evp_cipher_ctx ctx)->string{{{1
*/
LUA_FUNCTION(openssl_evp_decrypt_final)
{
    EVP_CIPHER_CTX* c = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    int outl = EVP_MAX_BLOCK_LENGTH;
    char out[EVP_MAX_BLOCK_LENGTH];

    if(EVP_DecryptFinal_ex(c,(byte*)out,&outl) && outl)
    {
        lua_pushlstring(L,out,outl);
        return 1;
    }
    return 0;
}
/* }}} */



/*  openssl.evp_cipher_init(openssl.evp_cipher cipher, bool enc, [, string key [,string iv [,openssl.engine engimp]]])->openssl.evp_cipher_ctx{{{1
*/

LUA_FUNCTION(openssl_evp_cipher_init)
{
    EVP_CIPHER* c = CHECK_OBJECT(1,EVP_CIPHER, "openssl.evp_cipher");
    int enc = auxiliar_checkboolean(L,2);

    const char* k = luaL_optstring(L,3,NULL);
    const char* iv = luaL_optstring(L,4,NULL);
    ENGINE*     e = lua_gettop(L)>4? CHECK_OBJECT(5,ENGINE,"openssl.engine") :NULL;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    PUSH_OBJECT(ctx,"openssl.evp_cipher_ctx");
    EVP_CIPHER_CTX_init(ctx);

    if (!EVP_CipherInit_ex(ctx,c,e, (const byte*)k, (const byte*)iv,enc)) {
        luaL_error(L,"EVP_DecryptInit_ex failed");
    }
    return 1;
}
/* }}} */

/*  openssl.evp_cipher_update(openssl.evp_cipher_ctx ctx, string data)->string{{{1
*/
LUA_FUNCTION(openssl_evp_cipher_update)
{
    EVP_CIPHER_CTX* c = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    size_t inl;
    const char* in= luaL_checklstring(L,2,&inl);
    int outl = inl + EVP_MAX_BLOCK_LENGTH;
    char* out = malloc(outl);


    if(EVP_CipherUpdate(c,(byte*)out,&outl,(const byte*)in,inl) && outl)
    {
        lua_pushlstring(L,out,outl);
    }
    free(out);

    return outl?1:0;
}
/* }}} */

/*  openssl.evp_decrypt_final(openssl.evp_cipher_ctx ctx)->string{{{1
*/
LUA_FUNCTION(openssl_evp_cipher_final)
{
    EVP_CIPHER_CTX* c = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    int outl = EVP_MAX_BLOCK_LENGTH;
    char out[EVP_MAX_BLOCK_LENGTH];

    if(EVP_CipherFinal_ex(c,(byte*)out,&outl) && outl)
    {
        lua_pushlstring(L,out,outl);
        return 1;
    }
    return 0;
}
/* }}} */

LUA_FUNCTION(openssl_cipher_ctx_info)
{
    EVP_CIPHER_CTX *ctx = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    lua_newtable(L);
    add_assoc_int(L,"block_size", EVP_CIPHER_CTX_block_size(ctx));
    add_assoc_int(L,"key_length", EVP_CIPHER_CTX_key_length(ctx));
    add_assoc_int(L,"iv_length", EVP_CIPHER_CTX_iv_length(ctx));
    add_assoc_int(L,"flags", EVP_CIPHER_CTX_flags(ctx));
    add_assoc_int(L,"nid", EVP_CIPHER_CTX_nid(ctx));
    add_assoc_int(L,"type", EVP_CIPHER_CTX_mode(ctx));
    add_assoc_int(L,"mode", EVP_CIPHER_CTX_type(ctx));
    PUSH_OBJECT((void*)EVP_CIPHER_CTX_cipher(ctx),"openssl.evp_cipher");
    lua_setfield(L,-2,"cipher");
    return 1;
}

LUA_FUNCTION(openssl_cipher_ctx_tostring) {
    EVP_CIPHER_CTX *ctx = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    lua_pushfstring(L,"openssl.evp_cipher_ctx:%p",ctx);
    return 1;
}

LUA_FUNCTION(openssl_cipher_ctx_free) {
    EVP_CIPHER_CTX *ctx = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

LUA_FUNCTION(openssl_cipher_ctx_cleanup) {
    EVP_CIPHER_CTX *ctx = CHECK_OBJECT(1,EVP_CIPHER_CTX, "openssl.evp_cipher_ctx");
    lua_pushboolean(L,EVP_CIPHER_CTX_cleanup(ctx));
    return 1;
}

LUA_FUNCTION(openssl_evp_encrypt) {
    EVP_CIPHER* cipher = CHECK_OBJECT(1,EVP_CIPHER, "openssl.evp_cipher");
    size_t input_len = 0;
    const char *input = luaL_checklstring(L, 2, &input_len);
    size_t key_len = 0;
    const char *key = luaL_optlstring(L, 3, NULL, &key_len); /* can be NULL */
    size_t iv_len = 0;
    const char *iv = luaL_optlstring(L, 4, NULL, &iv_len); /* can be NULL */
    ENGINE *e = lua_isnoneornil(L,5) ? NULL : CHECK_OBJECT(5, ENGINE, "openssl.engine");
    EVP_CIPHER_CTX c;

    int output_len = 0;
    int len = 0;
    char *buffer = NULL;
    char evp_key[EVP_MAX_KEY_LENGTH] = {0};
    char evp_iv[EVP_MAX_IV_LENGTH] = {0};

    if (key)
    {
        key_len = EVP_MAX_KEY_LENGTH>key_len?key_len:EVP_MAX_KEY_LENGTH;
        memcpy(evp_key, key, key_len);
    }
    if (iv)
    {
        iv_len = EVP_MAX_IV_LENGTH>iv_len?iv_len:EVP_MAX_IV_LENGTH;
        memcpy(evp_iv, iv, iv_len);
    }

    EVP_CIPHER_CTX_init(&c);

    if(!EVP_EncryptInit_ex(&c, cipher, e, key?(const byte*)evp_key:NULL, iv?(const byte*)evp_iv:NULL ))
    {
        luaL_error(L, "EVP_DecryptInit_ex failed, please check openssl error");
    }
    buffer = malloc(input_len + EVP_CIPHER_CTX_block_size(&c));
    EVP_EncryptUpdate(&c,(byte*) buffer, &len, (const byte*)input, input_len);
    output_len += len;
    EVP_EncryptFinal(&c, (byte*)buffer+len, &len);
    output_len += len;
    lua_pushlstring(L, (char*) buffer, output_len);
    EVP_CIPHER_CTX_cleanup(&c);
    free(buffer);
    return 1;
}

LUA_FUNCTION(openssl_evp_decrypt) {
    EVP_CIPHER* cipher = CHECK_OBJECT(1,EVP_CIPHER, "openssl.evp_cipher");
    size_t input_len = 0;
    const char *input = luaL_checklstring(L, 2, &input_len);
    size_t key_len = 0;
    const char *key = luaL_checklstring(L, 3, &key_len);
    size_t iv_len = 0;
    const char *iv = luaL_optlstring(L, 4, NULL, &iv_len); /* can be NULL */
    ENGINE *e = lua_isnoneornil(L,5) ? NULL : CHECK_OBJECT(5, ENGINE, "openssl.engine");
    EVP_CIPHER_CTX c;

    int output_len = 0;
    int len = 0;
    char *buffer = NULL;
    char evp_key[EVP_MAX_KEY_LENGTH] = {0};
    char evp_iv[EVP_MAX_IV_LENGTH] = {0};
    if (key)
    {
        key_len = EVP_MAX_KEY_LENGTH>key_len?key_len:EVP_MAX_KEY_LENGTH;
        memcpy(evp_key, key, key_len);
    }
    if (iv)
    {
        iv_len = EVP_MAX_IV_LENGTH>iv_len?iv_len:EVP_MAX_IV_LENGTH;
        memcpy(evp_iv, iv, iv_len);
    }

    EVP_CIPHER_CTX_init(&c);

    if(!EVP_DecryptInit_ex(&c, cipher, e, key?(const byte*)evp_key:NULL, iv?(const byte*)evp_iv:NULL ))
    {
        luaL_error(L, "EVP_DecryptInit_ex failed, please check openssl error");
    }

    buffer = malloc(input_len);
    EVP_DecryptUpdate(&c, (byte*)buffer, &len, (const byte*)input, input_len);
    output_len += len;
    EVP_DecryptFinal(&c, (byte*)buffer+len, &len);
    output_len += len;
    lua_pushlstring(L, (char*) buffer, output_len);
    EVP_CIPHER_CTX_cleanup(&c);
    free(buffer);
    return 1;
}

static luaL_Reg cipher_funs[] = {
    {"info",			openssl_cipher_info},
    {"encrypt_init",	openssl_evp_encrypt_init},
    {"decrypt_init",	openssl_evp_decrypt_init},
    {"init",			openssl_evp_cipher_init},
    {"BytesToKey",		openssl_evp_BytesToKey},
    {"encrypt",			openssl_evp_encrypt },
    {"decrypt",			openssl_evp_decrypt },

    {"__tostring",		openssl_cipher_tostring},

    {NULL, NULL}
};

static luaL_Reg cipher_ctx_funs[] = {
    {"encrypt_update",	openssl_evp_encrypt_update},
    {"encrypt_final",	openssl_evp_encrypt_final},

    {"decrypt_update",	openssl_evp_decrypt_update},
    {"decrypt_final",	openssl_evp_decrypt_final},
    {"update",			openssl_evp_cipher_update},
    {"final",			openssl_evp_cipher_final},

    {"info",		openssl_cipher_ctx_info},
    {"cleanup",		openssl_cipher_ctx_cleanup},
    {"__gc",		openssl_cipher_ctx_free},
    {"__tostring",		openssl_cipher_ctx_tostring},
    {NULL, NULL}
};

int openssl_register_cipher(lua_State* L)
{
    auxiliar_newclass(L,"openssl.evp_cipher",		cipher_funs);
    auxiliar_newclass(L,"openssl.evp_cipher_ctx",	cipher_ctx_funs);
    return 0;
}
