/**
 * smal definition of an variable length Array
 * with an max size and an custum content
 *
 * Its just like an Generic ArrayList from Java
 * exclusively the maximum size
 */
#ifndef ARRAY
#define ARRAY
#include <time.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Generates an Long 64 Bit random number
 */
#define LONG_RAND\
  ((((((((((u_int64_t) rand()) << 15)\
  + (u_int64_t) rand()) << 15)\
  + (u_int64_t) rand()) << 15)\
  + (u_int64_t) rand()) << 15)\
  + (u_int64_t) rand() + 1) 

/**
 * Generates an random index for the given Array
 */
#define RAND_INDEX(ARY) LONG_RAND % ARY.length

/**
 * defines an new Array struct with a given type as content
 * and a given Name
 */
#define DEFINE_ARRAY(TYPE, NAME)\
typedef struct {\
  TYPE *ptr;\
  u_int64_t length;\
  u_int64_t max_len;\
} NAME;

/**
 * Initialize an given Array with an given maximal length
 * also initialize rand
 */
#define ARY_INIT(TYPE, ARY, MAX_LEN)\
  ARY.ptr = (TYPE *) malloc(sizeof(TYPE) * MAX_LEN);\
  ARY.length = 0;\
  ARY.max_len = MAX_LEN;\
  srand(time(NULL) * rand());

/**
 * Insert an given element at the end of a given Array if possible
 * NOTE the Arry content type should be the same as th type of 
 * the given element
 */
#define ARY_ADD(ARY, E)\
  if (ARY.length < ARY.max_len) {\
    ARY.ptr[ARY.length] = E;\
    ARY.length++;\
  }

/**
 * Insert an given element at the end of a given Array
 * NOTE the TYPE should be the same as th type of 
 * the given element
 *
 * Note it my reallocates space if array is to smal
 */
#define ARY_PUSH(TYPE, ARY, E)\
  if (ARY.length >= ARY.max_len) {\
    ARY.ptr = (TYPE *) realloc(ARY.ptr, sizeof(TYPE) * ARY.max_len * 2);\
    ARY.max_len *= 2;\
  }\
  ARY.ptr[ARY.length] = E;\
  ARY.length++;
  
/**
 * Removes and saves the last element in E
 */
#define ARY_PULL(ARY, E)\
  if (ARY.length > 0) {\
    ARY.length--;\
    E = ARY.ptr[ARY.length];\
  }

/**
 * Extract a random element from a given array
 * and saves it into a given value if possible
 *
 * note it changes the order of the given array
 * NOTE the Arry content type should be the same as th type of 
 * the given value
 */
#define ARY_EXTRACT(ARY, VALUE)\
  if (ARY.length > 0) {\
    u_int64_t ary_ext_i = RAND_INDEX(ARY);\
    VALUE = ARY.ptr[ary_ext_i];\
    ARY.ptr[ary_ext_i] = ARY.ptr[ARY.length - 1];\
    ARY.length--;\
  }


/**
 * Deletes al instaces of the given Element from th given Array
 * NOTE the Arry content type should be the same as th type of 
 * the given element
 */
#define ARY_DELETE(ARY, E)\
{\
  u_int64_t adi, new_len;\
  for (adi = new_len = 0; adi < ARY.length; adi++) {\
    ARY.ptr[new_len] = ARY.ptr[adi];\
    if (ARY.ptr[adi] != E) { new_len++; }\
  }\
  ARY.length = new_len;\
}

/**
 * Returns wheter the given Array is empty or not
 */
#define EMPTY(ARY) ARY.length == 0
#define NOTEMPTY(ARY) ARY.length != 0

#endif // end of Array.h
