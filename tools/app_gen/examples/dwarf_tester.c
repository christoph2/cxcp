/* dwarf_test.c
 * Kompiliere mit: gcc -g -O0 -std=c11 dwarf_test.c -o dwarf_test
 *
 * Ziel: viele verschiedene Typkonstrukte erzeugen (typedefs, structs,
 * unions, enums, arrays, function pointers, bitfields, const/volatile).
 */

#include <stdint.h>
#include <stddef.h>

/* Grundtypen und Aliase */
typedef int32_t i32;
typedef const char *cstr;
typedef volatile unsigned long vulong;

/* Pointer- und Array-Aliase */
typedef i32 *pi32;
typedef pi32 pi32_array[4];
typedef char (*str_array_t)[64]; /* pointer to array of 64 chars */

/* Funktionszeiger */
typedef int (*cmp_fn)(const void *, const void *);
typedef void (*callback_t)(int, void*);

/* Verschachtelte typedef-Ketten */
typedef pi32 *ppi32;
typedef ppi32 *pppi32;

/* Struktur mit vielen Feldern, Qualifiern und Bitfeldern */
typedef struct ComplexStruct {
    cstr name;
    i32 id;
    vulong flags;
    union {
        double d;
        long l;
        void *ptr;
    } u;
    struct {
        unsigned int a:5;
        unsigned int b:11;
        signed int c:16;
    } bits;
    pi32_array arr; /* array of pointers */
    str_array_t names; /* pointer to 64-char arrays */
} ComplexStruct;

/* Typedef für Pointer auf ComplexStruct const volatile */
typedef ComplexStruct * const volatile cvpComplexStruct;

/* Typedef für Array von Struct-Pointern */
typedef ComplexStruct *ComplexPtrArray[3];

/* Enum und typedef */
typedef enum Color {
    COLOR_RED = 1,
    COLOR_GREEN = 2,
    COLOR_BLUE = 4
} Color;

/* Typedef für Funktion mit komplexer Signatur */
typedef int (*complex_fn_t)(const ComplexStruct *, Color, callback_t);

/* Flexible-Array-Struct */
typedef struct Flex {
    size_t len;
    int data[]; /* flexible array member */
} Flex;

/* Globale Variablen mit verschiedenen Typen */
i32 global_int = 42;
static const cstr global_name = "dwarf_test";
ComplexStruct global_cs = {
    .name = "global",
    .id = 7,
    .flags = 0xdeadbeef,
    .u.ptr = NULL,
    .bits = { .a = 3, .b = 1023, .c = -1 },
};
ComplexPtrArray global_ptrs = { &global_cs, NULL, NULL };
cvpComplexStruct global_cvptr = (cvpComplexStruct)&global_cs;

/* Extern declaration to create DW_TAG_variable with declaration attribute */
extern int extern_var;

/* Function using many local typedefs and complex types */
int process(const ComplexStruct *cs, Color col, callback_t cb) {
    typedef const ComplexStruct *cpc;
    cpc local_cs = cs;
    i32 local_vals[3] = {1,2,3};
    pi32 p_local = local_vals;
    ppi32 chain = NULL;
    Flex *f = (Flex*)malloc(sizeof(Flex) + 10 * sizeof(int));
    if (!f) return -1;
    f->len = 10;
    for (size_t i = 0; i < f->len; ++i) f->data[i] = (int)i;
    if (cb) cb((int)f->len, (void*)local_cs);
    free(f);
    (void)col;
    (void)chain;
    (void)p_local;
    return local_cs ? local_cs->id : 0;
}

/* Small wrapper to create more DIEs: inline, static, volatile locals */
static inline void helper(void) {
    volatile int v = 5;
    register int r = v + global_int;
    (void)r;
}

int main(void) {
    callback_t cb = NULL;
    helper();
    process(&global_cs, COLOR_GREEN, cb);
    return 0;
}
