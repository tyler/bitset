Bitset
======

A fast Bitset implementation for Ruby. Available as the 'bitset' gem.


Installation
------------

Usually you want to do this:

    gem install bitset

But if you want the latest patches or want to work on it yourself, you may want
to do this:

    git clone git://github.com/ericboesch/bitset.git
    cd bitset
    rake build
    gem install pkg/bitset-<version>.gem


Usage
-----

You create a bitset like this:

    >> Bitset.new(8)
    => 00000000

Here we created an 8-bit bitset. All bits are initialized to 0.

We can also create a bitset based on a string of ones and zeros.

    >> Bitset.from_s('00010001')
    => 00010001

or from an array. Falsey values (false and nil) are converted to
zeroes; all other values, including 0 and "", are converted to ones.

    >> Bitset.new [false, nil, 3, 0]
    => 0011

To input an array of ones and zeroes:

    >> Bitset.new([0,1,1,0].map(&:positive?))
    => 0110

Obviously you can also set and clear bits...

    >> bitset = Bitset.new(8)
    => 00000000

    >> bitset[3] = true
    => 00010000

    >> bitset[3] = false
    => 00000000

    >> bitset.set(1, 3, 5, 7)
    => 01010101

    >> bitset.clear(1, 5)
    => 00010001

Arrays of Integers can also be passed to #clear and #set (c/o brendon9x).

The point of a bitset is to be, effectively, an array of single bits. It should
support basic set and bitwise operations. So, let's look at a few of those.

    >> a = Bitset.from_s('00001111')
    => 00001111

    >> b = Bitset.from_s('01010101')
    => 01010101

    >> a & b
    => 00000101

    >> a | b
    => 01011111

    >> b - a
    => 01010000

    >> a ^ b
    => 01011010

    >> ~a
    => 11110000

    >> a.hamming(b)
    => 4

    >> a.cardinality
    => 4

    >> a.reverse
    => 11110000

    # Tell whether all of the given bit numbers are set
    >> a.set? 6
    => true

    # Return a new Bitset composed of bits #1, #3, #5, #4, and #1
    # again. Unlike Array#values_at, this function currently only
    # accepts an array of Fixnums as its argument.
    >> a.values_at [1,3,5,4,1]
    => 00110

    # Tell whether all of the given bit numbers are clear
    >> a.clear? 1,3,5
    => false

    # Tell whether all bits are clear
    >> a.empty?
    => false

    # Pass all bits to the block
    >> b.each { |v| puts v }
    => false
       true
       false
       ...

    # Pass the positions of all set bits to the block
    >> b.each_set { |bit| puts bit }
    => 1
       3
       5
       7

    # Return an array of the positions of all set bits
    >> b.each_set      # AKA b.to_a
    => [1, 3, 5, 7]

    # b.each_set(index) == b.each_set[index], but faster.
    >> b.each_set(-3) # Negative index wraps around.
    => 3

    # b.each_set(index, len) == b.each_set[index, len], but faster.
    >> b.each_set(2,2) # Block is also allowed
    => [5,7]


    # The following methods modify a Bitset in place very quickly:
    >> a.intersect!(b)      #  like a &= b
    >> a.union!(b)          #  like a |= b
    >> a.difference!(b)     #  like a -= b
    >> a.xor!(b)            #  like a ^= b
    >> a.reset!             # Zeroes all bits

    # Above, "like" does not mean "identical to." a |= b creates a new
    # Bitset object. a.union!(b) changes an existing object which
    # affects all variables that point to the same object.

    # Attempting to apply bitwise binary operators or their in-place
    # equivalents between bitsets of different sizes will raise an
    # ArgumentError.

    >> b.to_binary_array
    => [0, 1, 0, 1, 0, 1, 0, 1]

    # b.dup and b.clone are also available.

    # Marshal.dump and Marshal.load are also supported. If you want to
    # save a few bytes and don't need Marshal.load to work, you can
    # use #pack and Bitset.unpack instead.

Contributing
------------

The best way to contribute is to fork the project on GitHub, make your changes,
and send a pull request. This is always much appreciated. If you want to mess
around with the version numbers, gemspec, or anything like that feel free... But
do it in separate commits so I can easily ignore them.


License
-------

See LICENSE.txt.


### Thanks for using Bitset!
