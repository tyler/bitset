Bitset
======

A fast Bitset implementation for Ruby. Available as the 'bitset' gem.


Installation
------------

Usually you want to do this:

    gem install bitset

But if you want the latest patches or want to work on it yourself, you may want
to do this:

    git clone git://github.com/tyler/bitset.git
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

    # Return a new Bitset composed of bits #1, #3, #5, #4, and #1 again
    >> a.select_bits [1,3,5,4,1]
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
    >> b.each_set
    => [1, 3, 5, 7]

    # b.dup and b.clone also work

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
