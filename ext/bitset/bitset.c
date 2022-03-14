#include "ruby.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

VALUE cBitset;

typedef struct {
    int len;
    uint64_t * data;
} Bitset;

#define BYTES(_bs) (((_bs->len-1) >> 3) + 1)
#define INTS(_bs) (((_bs->len-1) >> 6) + 1)

Bitset * bitset_new() {
    return (Bitset *) malloc(sizeof(Bitset));
}

void bitset_setup(Bitset * bs, int len) {
    bs->len = len;
    bs->data = (uint64_t *) calloc((((bs->len-1) >> 6) + 1), sizeof(uint64_t)); // 2^6=64
}

void bitset_free(Bitset * bs) {
    if(bs->data)
        free(bs->data);
    free(bs);
}


Bitset * get_bitset(VALUE obj) {
    Bitset * bs;
    Data_Get_Struct(obj, Bitset, bs);
    return bs;
}

static VALUE rb_bitset_alloc(VALUE klass) {
    VALUE obj;
    obj = Data_Wrap_Struct(klass, 0, bitset_free, bitset_new());
    return obj;
}

static VALUE rb_bitset_initialize(VALUE self, VALUE len) {
    Bitset * bs = get_bitset(self);
    bitset_setup(bs, NUM2INT(len));
    return self;
}

static VALUE rb_bitset_size(VALUE self, VALUE len) {
    Bitset * bs = get_bitset(self);
    return INT2NUM(bs->len);
}

void raise_index_error() {
    VALUE rb_eIndexError = rb_const_get(rb_cObject, rb_intern("IndexError"));
    rb_raise(rb_eIndexError, "Index out of bounds");
}

#define _bit_segment(bit) ((bit) >> 6UL)
#define _bit_mask(bit) (((uint64_t) 1) << ((bit) & 0x3f))

void validate_index(Bitset * bs, int idx) {
    if(idx < 0 || idx >= bs->len)
        raise_index_error();
}

uint64_t get_bit(Bitset * bs, int idx) {
    uint64_t segment = bs->data[_bit_segment(idx)];
    return segment & _bit_mask(idx);
}

void set_bit(Bitset * bs, int idx) {
    bs->data[_bit_segment(idx)] |= _bit_mask(idx);
}

void clear_bit(Bitset * bs, int idx) {
    bs->data[_bit_segment(idx)] &= ~_bit_mask(idx);
}

void assign_bit(Bitset * bs, int idx, VALUE value) {
    if(NIL_P(value) || value == Qfalse)
        clear_bit(bs, idx);
    else
        set_bit(bs, idx);
}

static VALUE rb_bitset_aref(VALUE self, VALUE index) {
    Bitset * bs = get_bitset(self);
    int idx = NUM2INT(index);
    validate_index(bs, idx);
    return get_bit(bs, idx) > 0 ? Qtrue : Qfalse;
}

static VALUE rb_bitset_aset(VALUE self, VALUE index, VALUE value) {
    Bitset * bs = get_bitset(self);
    int idx = NUM2INT(index);
    validate_index(bs, idx);
    assign_bit(bs, idx, value);
    return Qtrue;
}

static VALUE rb_bitset_set(int argc, VALUE * argv, VALUE self) {
    int i;
    Bitset * bs = get_bitset(self);
    for(i = 0; i < argc; i++) {
        VALUE index = argv[i];
        int idx = NUM2INT(index);
        validate_index(bs, idx);
        set_bit(bs, idx);
    }
    return Qtrue;
}

static VALUE rb_bitset_clear(int argc, VALUE * argv, VALUE self) {
    int i;
    Bitset * bs = get_bitset(self);
    for(i = 0; i < argc; i++) {
        VALUE index = argv[i];
        int idx = NUM2INT(index);
        validate_index(bs, idx);
        clear_bit(bs, idx);
    }
    return Qtrue;
}

static VALUE rb_bitset_clear_p(int argc, VALUE * argv, VALUE self) {
    int i;
    Bitset * bs = get_bitset(self);
    for(i = 0; i < argc; i++) {
        VALUE index = argv[i];
        int idx = NUM2INT(index);
        validate_index(bs, idx);
        if(get_bit(bs, idx) > 0)
            return Qfalse;
    }
    return Qtrue;
}

static VALUE rb_bitset_set_p(int argc, VALUE * argv, VALUE self) {
    int i;
    Bitset * bs = get_bitset(self);
    for(i = 0; i < argc; i++) {
        VALUE index = argv[i];
        int idx = NUM2INT(index);
        validate_index(bs, idx);
        if(get_bit(bs, idx) == 0)
            return Qfalse;
    }
    return Qtrue;
}

static VALUE rb_bitset_cardinality(VALUE self) {
    Bitset * bs = get_bitset(self);
    int i;
    int max = ((bs->len-1) >> 6) + 1;
    int count = 0;
    for(i = 0; i < max; i++) {
        uint64_t segment = bs->data[i];
        if(i+1 == max)
            segment &= ((((uint64_t) 1) << (bs->len & 0x3F)) - 1);
        count += __builtin_popcountll(segment);
    }
    return INT2NUM(count);
}

static VALUE rb_bitset_intersect(VALUE self, VALUE other) {
    Bitset * bs = get_bitset(self);
    Bitset * other_bs = get_bitset(other);

    Bitset * new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

    int max = ((bs->len-1) >> 6) + 1;
    int i;
    for(i = 0; i < max; i++) {
        uint64_t segment = bs->data[i];
        uint64_t other_segment = other_bs->data[i];
        new_bs->data[i] = segment & other_segment;
    }

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

static VALUE rb_bitset_union(VALUE self, VALUE other) {
    Bitset * bs = get_bitset(self);
    Bitset * other_bs = get_bitset(other);

    Bitset * new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

    int max = ((bs->len-1) >> 6) + 1;
    int i;
    for(i = 0; i < max; i++) {
        uint64_t segment = bs->data[i];
        uint64_t other_segment = other_bs->data[i];
        new_bs->data[i] = segment | other_segment;
    }

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

static VALUE rb_bitset_difference(VALUE self, VALUE other) {
    Bitset * bs = get_bitset(self);
    Bitset * other_bs = get_bitset(other);

    Bitset * new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

    int max = ((bs->len-1) >> 6) + 1;
    int i;
    for(i = 0; i < max; i++) {
        uint64_t segment = bs->data[i];
        uint64_t other_segment = other_bs->data[i];
        new_bs->data[i] = segment & ~other_segment;
    }

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

static VALUE rb_bitset_xor(VALUE self, VALUE other) {
    Bitset * bs = get_bitset(self);
    Bitset * other_bs = get_bitset(other);

    Bitset * new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

    int max = ((bs->len-1) >> 6) + 1;
    int i;
    for(i = 0; i < max; i++) {
        uint64_t segment = bs->data[i];
        uint64_t other_segment = other_bs->data[i];
        new_bs->data[i] = segment ^ other_segment;
    }

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

static VALUE rb_bitset_not(VALUE self) {
    Bitset * bs = get_bitset(self);

    Bitset * new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

    int max = ((bs->len-1) >> 6) + 1;
    int i;
    for(i = 0; i < max; i++) {
        uint64_t segment = bs->data[i];
        new_bs->data[i] = ~segment;
    }

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

static VALUE rb_bitset_to_s(VALUE self) {
    Bitset * bs = get_bitset(self);

    int i;
    char * data = malloc(bs->len + 1);
    for(i = 0; i < bs->len; i++) {
        data[i] = get_bit(bs, i)  ? '1' : '0';
    }
    data[bs->len] = 0;

    return rb_str_new2(data);
}

static VALUE rb_bitset_from_s(VALUE self, VALUE s) {
    int length = RSTRING_LEN(s);
    char* data = StringValuePtr(s);

    Bitset * new_bs = bitset_new();
    bitset_setup(new_bs, length);

    int i;
    for (i = 0; i < length; i++) {
        if (data[i] == '1') {
            set_bit(new_bs, i);
        }
    }

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

static VALUE rb_bitset_hamming(VALUE self, VALUE other) {
    Bitset * bs = get_bitset(self);
    Bitset * other_bs = get_bitset(other);

    int max = ((bs->len-1) >> 6) + 1;
    int count = 0;
    int i;
    for(i = 0; i < max; i++) {
        uint64_t segment = bs->data[i];
        uint64_t other_segment = other_bs->data[i];
        count += __builtin_popcountll(segment ^ other_segment);
    }

    return INT2NUM(count);
}

static VALUE rb_bitset_each(VALUE self) {
    Bitset * bs = get_bitset(self);
    int i;

    for(i = 0; i < bs->len; i++) {
        rb_yield(get_bit(bs, i) > 0 ? Qtrue : Qfalse);
    }

    return self;
}

static VALUE rb_bitset_each_with_index(VALUE self) {
    Bitset * bs = get_bitset(self);
    int i;

    for(i = 0; i < bs->len; i++) {
        rb_yield_values(2, get_bit(bs, i) > 0 ? Qtrue : Qfalse, INT2NUM(i));
    }

    return self;
}

static VALUE rb_bitset_each_set_index(VALUE self) {
    Bitset * bs = get_bitset(self);
    int i, n = INTS(bs);

    for(i = 0; i < n; i++) {
        uint64_t bits = bs->data[i];
        int j = 0;
        while (bits) {
            if (bits & 1) {
                rb_yield(INT2NUM((i << 6) + j));
            }
            bits >>= 1;
            j++;
        }
    }

    return self;
}

static VALUE rb_bitset_marshal_dump(VALUE self) {
    Bitset * bs = get_bitset(self);
    VALUE hash = rb_hash_new();
    VALUE data = rb_str_new(bs->data, BYTES(bs));

    rb_hash_aset(hash, ID2SYM(rb_intern("len")), UINT2NUM(bs->len));
    rb_hash_aset(hash, ID2SYM(rb_intern("data")), data);

    return hash;
}

static VALUE rb_bitset_marshal_load(VALUE self, VALUE hash) {
    Bitset * bs = get_bitset(self);
    int len = NUM2INT(rb_hash_aref(hash, ID2SYM(rb_intern("len"))));

    VALUE data = rb_hash_aref(hash, ID2SYM(rb_intern("data")));

    bitset_setup(bs, len);

    bs->data = (uint64_t *) calloc(INTS(bs), sizeof(uint64_t));
    memcpy(bs->data, RSTRING_PTR(data), BYTES(bs));

    return Qnil;
}

void Init_bitset() {
    cBitset = rb_define_class("Bitset", rb_cObject);
    rb_include_module(cBitset, rb_mEnumerable);
    rb_define_alloc_func(cBitset, rb_bitset_alloc);
    rb_define_method(cBitset, "initialize", rb_bitset_initialize, 1);
    rb_define_method(cBitset, "size", rb_bitset_size, 0);
    rb_define_method(cBitset, "[]", rb_bitset_aref, 1);
    rb_define_method(cBitset, "[]=", rb_bitset_aset, 2);
    rb_define_method(cBitset, "set", rb_bitset_set, -1);
    rb_define_method(cBitset, "clear", rb_bitset_clear, -1);
    rb_define_method(cBitset, "set?", rb_bitset_set_p, -1);
    rb_define_method(cBitset, "clear?", rb_bitset_clear_p, -1);
    rb_define_method(cBitset, "cardinality", rb_bitset_cardinality, 0);
    rb_define_method(cBitset, "intersect", rb_bitset_intersect, 1);
    rb_define_alias(cBitset, "&", "intersect");
    rb_define_method(cBitset, "union", rb_bitset_union, 1);
    rb_define_alias(cBitset, "|", "union");
    rb_define_method(cBitset, "difference", rb_bitset_difference, 1);
    rb_define_alias(cBitset, "-", "difference");
    rb_define_method(cBitset, "xor", rb_bitset_xor, 1);
    rb_define_alias(cBitset, "^", "xor");
    rb_define_alias(cBitset, "symmetric_difference", "xor");
    rb_define_method(cBitset, "not", rb_bitset_not, 0);
    rb_define_alias(cBitset, "~", "not");
    rb_define_method(cBitset, "hamming", rb_bitset_hamming, 1);
    rb_define_method(cBitset, "each", rb_bitset_each, 0);
    rb_define_method(cBitset, "each_with_index", rb_bitset_each_with_index, 0);
    rb_define_method(cBitset, "each_set_index", rb_bitset_each_set_index, 0);
    rb_define_method(cBitset, "to_s", rb_bitset_to_s, 0);
    rb_define_singleton_method(cBitset, "from_s", rb_bitset_from_s, 1);
    rb_define_method(cBitset, "marshal_dump", rb_bitset_marshal_dump, 0);
    rb_define_method(cBitset, "marshal_load", rb_bitset_marshal_load, 1);
}
