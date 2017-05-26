#include "ruby.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>

VALUE cBitset;

typedef struct {
    int len;
    uint64_t * data;
} Bitset;

// Avoid using (len-1), just in case len == 0.
#define BYTES(_bs) (((_bs)->len+7) >> 3)
#define INTS(_bs) (((_bs)->len+63) >> 6)
 // 2^6=64

Bitset * bitset_new() {
    return (Bitset *) calloc(1, sizeof(Bitset));
}

void bitset_setup(Bitset * bs, int len) {
    bs->len = len;
    bs->data = (uint64_t *) calloc(INTS(bs), sizeof(uint64_t));
}

void bitset_free(Bitset * bs) {
    if(bs->data)
        free(bs->data);
    free(bs);
}


static Bitset * get_bitset(VALUE obj) {
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

static void raise_index_error() {
    VALUE rb_eIndexError = rb_const_get(rb_cObject, rb_intern("IndexError"));
    rb_raise(rb_eIndexError, "Index out of bounds");
}

#define _bit_no(bit) ((bit) & 0x3f)
#define _bit_segment(bit) ((bit) >> 6)
#define _bit_mask(bit) (((uint64_t) 1) << _bit_no(bit))
#define _seg_no_to_bit_no(seg_no) ((seg_no) << 6)
#define _get_bit(bs, idx) ((bs)->data[_bit_segment(idx)] & _bit_mask(idx))
#define _set_bit(bs, idx) ((bs)->data[_bit_segment(idx)] |= _bit_mask(idx))
#define _clear_bit(bs, idx) ((bs)->data[_bit_segment(idx)] &= ~_bit_mask(idx))

static void validate_index(Bitset * bs, int idx) {
    if(idx < 0 || idx >= bs->len)
        raise_index_error();
}

static void verify_equal_size(Bitset * bs1, Bitset * bs2) {
   if (bs1->len != bs2->len) {
       VALUE rb_eArgumentError = rb_const_get(rb_cObject, rb_intern("ArgumentError"));
       rb_raise(rb_eArgumentError, "Operands size mismatch");
   }
}

void assign_bit(Bitset * bs, int idx, VALUE value) {
    if(NIL_P(value) || value == Qfalse)
        _clear_bit(bs, idx);
    else
        _set_bit(bs, idx);
}

static VALUE rb_bitset_aref(VALUE self, VALUE index) {
    Bitset * bs = get_bitset(self);
    int idx = NUM2INT(index);
    validate_index(bs, idx);
    return _get_bit(bs, idx) > 0 ? Qtrue : Qfalse;
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

    if (argc == 1 && rb_obj_is_kind_of(argv[0], rb_const_get(rb_cObject, rb_intern("Array")))) {
        for(i = 0; i < RARRAY_LEN(argv[0]); i++) {
            VALUE index = RARRAY_PTR(argv[0])[i];
            int idx = NUM2INT(index);
            validate_index(bs, idx);
            _set_bit(bs, idx);
        }
    } else {
        for(i = 0; i < argc; i++) {
            VALUE index = argv[i];
            int idx = NUM2INT(index);
            validate_index(bs, idx);
            _set_bit(bs, idx);
        }
    }
    return Qtrue;
}

static VALUE rb_bitset_clear(int argc, VALUE * argv, VALUE self) {
    int i;
    Bitset * bs = get_bitset(self);

    if (argc == 1 && rb_obj_is_kind_of(argv[0], rb_const_get(rb_cObject, rb_intern("Array")))) {
        for(i = 0; i < RARRAY_LEN(argv[0]); i++) {
            VALUE index = RARRAY_PTR(argv[0])[i];
            int idx = NUM2INT(index);
            validate_index(bs, idx);
            _clear_bit(bs, idx);
        }
    } else {
        for(i = 0; i < argc; i++) {
            VALUE index = argv[i];
            int idx = NUM2INT(index);
            validate_index(bs, idx);
            _clear_bit(bs, idx);
        }
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
        if(_get_bit(bs, idx) > 0)
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
        if(_get_bit(bs, idx) == 0)
            return Qfalse;
    }
    return Qtrue;
}

static VALUE rb_bitset_cardinality(VALUE self) {
    Bitset * bs = get_bitset(self);
    int i;
    int max = INTS(bs);
    int count = 0;
    for(i = 0; i < max; i++) {
        count += __builtin_popcountll(bs->data[i]);
    }
    return INT2NUM(count);
}

static VALUE rb_bitset_intersect(VALUE self, VALUE other) {
    Bitset * bs = get_bitset(self);
    Bitset * other_bs = get_bitset(other);
    Bitset * new_bs;
    int max = INTS(bs);
    int i;

    verify_equal_size(bs, other_bs);
    new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

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
    Bitset * new_bs;
    int max = INTS(bs);
    int i;

    verify_equal_size(bs, other_bs);
    new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

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
    Bitset * new_bs;
    int max = INTS(bs);
    int i;

    verify_equal_size(bs, other_bs);
    new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

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
    Bitset * new_bs;
    int max = INTS(bs);
    int i;

    verify_equal_size(bs, other_bs);
    new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

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
    int max = INTS(bs);
    int i;

    bitset_setup(new_bs, bs->len);
    for(i = 0; i < max; i++) {
        uint64_t segment = bs->data[i];
        new_bs->data[i] = ~segment;
    }
    if(_bit_no(bs->len) != 0)
        new_bs->data[max-1] &= _bit_mask(bs->len) - 1;

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

static VALUE rb_bitset_to_s(VALUE self) {
    Bitset * bs = get_bitset(self);

    int i;
    char * data = malloc(bs->len + 1);
    for(i = 0; i < bs->len; i++) {
        data[i] = _get_bit(bs, i)  ? '1' : '0';
    }
    data[bs->len] = 0;

    return rb_str_new2(data);
}

static VALUE rb_bitset_from_s(VALUE self, VALUE s) {
    int length = RSTRING_LEN(s);
    char* data = StringValuePtr(s);
    Bitset * new_bs = bitset_new();
    int i;

    bitset_setup(new_bs, length);

    for (i = 0; i < length; i++) {
        if (data[i] == '1') {
            _set_bit(new_bs, i);
        }
    }

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

static VALUE rb_bitset_hamming(VALUE self, VALUE other) {
    Bitset * bs = get_bitset(self);
    Bitset * other_bs = get_bitset(other);

    int max = INTS(bs);
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
        rb_yield(_get_bit(bs, i) ? Qtrue : Qfalse);
    }

    return self;
}

static VALUE rb_bitset_marshall_dump(VALUE self) {
    Bitset * bs = get_bitset(self);
    VALUE hash = rb_hash_new();
    VALUE data = rb_str_new((const char *) bs->data, BYTES(bs));

    rb_hash_aset(hash, ID2SYM(rb_intern("len")), UINT2NUM(bs->len));
    rb_hash_aset(hash, ID2SYM(rb_intern("data")), data);

    return hash;
}

static VALUE rb_bitset_marshall_load(VALUE self, VALUE hash) {
    Bitset * bs = get_bitset(self);
    int len = NUM2INT(rb_hash_aref(hash, ID2SYM(rb_intern("len"))));

    VALUE data = rb_hash_aref(hash, ID2SYM(rb_intern("data")));

    bitset_setup(bs, len);

    bs->data = (uint64_t *) calloc(INTS(bs), sizeof(uint64_t));
    memcpy(bs->data, RSTRING_PTR(data), BYTES(bs));

    return Qnil;
}

static VALUE rb_bitset_to_binary_array(VALUE self) {
    Bitset * bs = get_bitset(self);
    int i;

    VALUE array = rb_ary_new2(bs->len / 2);
    for(i = 0; i < bs->len; i++) {
        rb_ary_push(array, INT2NUM(_get_bit(bs, i) > 0 ? 1 : 0));
    }

    return array;
}

static VALUE rb_bitset_dup(VALUE self) {
    Bitset * bs = get_bitset(self);
    int max = INTS(bs);

    Bitset * new_bs = bitset_new();
    bitset_setup(new_bs, bs->len);

    memcpy(new_bs->data, bs->data, max * sizeof(bs->data[0]));
    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

/* Yield the bit numbers of each set bit in sequence to a block. If
   there is no block, return an array of those numbers. */
static VALUE rb_bitset_each_set(VALUE self) {
    Bitset * bs = get_bitset(self);
    int seg_no;
    int max = INTS(bs);
    uint64_t* seg_ptr = bs->data;
    int block_p = rb_block_given_p();
    VALUE ary = Qnil;

    if (!block_p) {
       ary = rb_ary_new();
    }

    for (seg_no = 0; seg_no < max; ++seg_no, ++seg_ptr) {
       uint64_t segment = *seg_ptr;
       int bit_position = 0;
       while (segment) {
          VALUE v;

          if (!(segment & 1)) {
             int shift = __builtin_ctzll(segment);
             bit_position += shift;
             segment >>= shift;
          }
          v = INT2NUM(_seg_no_to_bit_no(seg_no) + bit_position);
          if (block_p) {
             rb_yield(v);
          } else {
             rb_ary_push(ary, v);
          }
          ++bit_position;
          segment >>= 1;
       }
    }

    return block_p ? self : ary;
}

static VALUE rb_bitset_empty_p(VALUE self) {
    Bitset * bs = get_bitset(self);
    int seg_no;
    int max = INTS(bs);
    uint64_t* seg_ptr = bs->data;

    for (seg_no = 0; seg_no < max; ++seg_no, ++seg_ptr) {
       if (*seg_ptr) {
          return Qfalse;
       }
    }
    return Qtrue;
}

static VALUE rb_bitset_values_at(VALUE self, VALUE index_array) {
    int i;
    Bitset * bs = get_bitset(self);
    int blen = bs->len;
    int alen = RARRAY_LEN(index_array);
    VALUE *ptr = RARRAY_PTR(index_array);
    Bitset * new_bs = bitset_new();
    bitset_setup(new_bs, alen);
    for (i = 0; i < alen; ++i) {
       int idx = NUM2INT(ptr[i]);
       if (idx >= 0 && idx < blen && _get_bit(bs, idx)) {
          _set_bit(new_bs, i);
       }
    }

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

/** This could run a bit faster if you worked at it. */
static VALUE rb_bitset_reverse(VALUE self, VALUE index_array) {
    int i;
    Bitset * bs = get_bitset(self);
    int len = bs->len;
    Bitset * new_bs = bitset_new();
    bitset_setup(new_bs, len);
    for (i = 0; i < len; ++i) {
       if (_get_bit(bs, i)) {
          _set_bit(new_bs, len - i - 1);
       }
    }

    return Data_Wrap_Struct(cBitset, 0, bitset_free, new_bs);
}

static VALUE rb_bitset_equal(VALUE self, VALUE other) {
    int i;
    Bitset * bs = get_bitset(self);
    Bitset * other_bs = get_bitset(other);
    int max = INTS(bs);

    if (bs->len != other_bs->len)
       return Qfalse;
    for(i = 0; i < max; i++) {
       if (bs->data[i] != other_bs->data[i]) {
          return Qfalse;
       }
    }
    return Qtrue;
}

typedef uint64_t (*bitwise_op)(uint64_t, uint64_t);
inline uint64_t and(uint64_t a, uint64_t b) { return a & b; }
inline uint64_t or(uint64_t a, uint64_t b) { return a | b; }
inline uint64_t xor(uint64_t a, uint64_t b) { return a ^ b; }
inline uint64_t difference(uint64_t a, uint64_t b) { return a & ~b; }

static VALUE mutable(VALUE self, VALUE other, bitwise_op operator) {
    Bitset * bs = get_bitset(self);
    Bitset * other_bs = get_bitset(other);
    int max = INTS(bs);
    int i;
    verify_equal_size(bs, other_bs);

    for(i = 0; i < max; i++) {
        uint64_t segment = bs->data[i];
        uint64_t other_segment = other_bs->data[i];
        bs->data[i] = operator(segment, other_segment);
    }

    return self;
}

static VALUE rb_bitset_union_mutable(VALUE self, VALUE other) {
    return mutable(self, other, &or);
}

static VALUE rb_bitset_intersect_mutable(VALUE self, VALUE other) {
    return mutable(self, other, &and);
}

static VALUE rb_bitset_xor_mutable(VALUE self, VALUE other) {
    return mutable(self, other, &xor);
}

static VALUE rb_bitset_difference_mutable(VALUE self, VALUE other) {
    return mutable(self, other, &difference);
}

static VALUE rb_bitset_reset(VALUE self) {
    Bitset * bs = get_bitset(self);
    memset(bs->data, 0, (INTS(bs) * sizeof(uint64_t)) );
    return self;
}

void Init_bitset() {
    cBitset = rb_define_class("Bitset", rb_cObject);
    rb_include_module(cBitset, rb_mEnumerable);
    rb_define_alloc_func(cBitset, rb_bitset_alloc);
    rb_define_method(cBitset, "initialize", rb_bitset_initialize, 1);
    rb_define_method(cBitset, "reset!", rb_bitset_reset, 0);
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
    rb_define_alias(cBitset, "and", "intersect");
    rb_define_method(cBitset, "intersect!", rb_bitset_intersect_mutable, 1);
    rb_define_alias(cBitset, "and!", "intersect!");
    rb_define_method(cBitset, "union", rb_bitset_union, 1);
    rb_define_alias(cBitset, "|", "union");
    rb_define_alias(cBitset, "or", "union");
    rb_define_method(cBitset, "union!", rb_bitset_union_mutable, 1);
    rb_define_alias(cBitset, "or!", "union!");
    rb_define_method(cBitset, "difference", rb_bitset_difference, 1);
    rb_define_alias(cBitset, "-", "difference");
    rb_define_method(cBitset, "difference!", rb_bitset_difference_mutable, 1);
    rb_define_method(cBitset, "xor", rb_bitset_xor, 1);
    rb_define_alias(cBitset, "^", "xor");
    rb_define_alias(cBitset, "symmetric_difference", "xor");
    rb_define_method(cBitset, "xor!", rb_bitset_xor_mutable, 1);
    rb_define_alias(cBitset, "symmetric_difference!", "xor!");
    rb_define_method(cBitset, "not", rb_bitset_not, 0);
    rb_define_alias(cBitset, "~", "not");
    rb_define_method(cBitset, "hamming", rb_bitset_hamming, 1);
    rb_define_method(cBitset, "each", rb_bitset_each, 0);
    rb_define_method(cBitset, "to_s", rb_bitset_to_s, 0);
    rb_define_singleton_method(cBitset, "from_s", rb_bitset_from_s, 1);
    rb_define_method(cBitset, "marshal_dump", rb_bitset_marshall_dump, 0);
    rb_define_method(cBitset, "marshal_load", rb_bitset_marshall_load, 1);
    rb_define_method(cBitset, "to_binary_array", rb_bitset_to_binary_array, 0);
    rb_define_method(cBitset, "dup", rb_bitset_dup, 0);
    rb_define_alias(cBitset, "clone", "dup");
    rb_define_method(cBitset, "each_set", rb_bitset_each_set, 0);
    rb_define_alias(cBitset, "to_a", "each_set");
    /* #each_set allows an optional block, and #to_a normally doesn't.
        But an alias is simpler than having two different functions. */
    rb_define_method(cBitset, "empty?", rb_bitset_empty_p, 0);
    rb_define_method(cBitset, "values_at", rb_bitset_values_at, 1);
    rb_define_alias(cBitset, "select_bits", "values_at");
    rb_define_method(cBitset, "reverse", rb_bitset_reverse, 0);
    rb_define_method(cBitset, "==", rb_bitset_equal, 1);
}
