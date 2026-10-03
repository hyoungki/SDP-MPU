
#ifndef _MY_LIB_H_
#define  _MY_LIB_H_

#ifdef _LIB_MYLIB_C_
#define TK_EXTERN 
#else
#define TK_EXTERN extern
#endif 


#define MY_LITTLE_ENDIAN   1
#define MY_BIG_ENDIAN      2

#define ROAD_LITTLE_ENDIAN   1
#define ROAD_BIG_ENDIAN      2


typedef unsigned int   uint32_t ;
// b0³­ LSB ...b3°¡ MSG
#define BITS_TO_FLOAT(b0, b1, b2, b3) ({ \
    uint32_t _u = ((uint32_t)(b0)) | ((uint32_t)(b1) << 8) | \
                  ((uint32_t)(b2) << 16) | ((uint32_t)(b3) << 24); \
    float _f; \
    memcpy(&_f, &_u, 4); \
    _f; \
})




TK_EXTERN  int check_endian(void) ;
TK_EXTERN  void ntoh_timeval_safe(struct timeval *out, const void *in_src) ;


TK_EXTERN  void print_timeval(struct timeval tv) ;

TK_EXTERN unsigned long init_base_address_safe(void) ;


TK_EXTERN int check_already_running(const char *app_name) ;

#endif 