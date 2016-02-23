class Bitset

  # Return a string that represents this bitset packed into 8-bit
  # characters. The first 3 bits represent the number of padding bits
  # in the final byte of the string.

  # You could make a good case that this is redundant with
  # Marshal.dump and Marshal.load, but it does save a few bytes.
  def pack
    # Number of bits of zero padding in this representation.
    padding_bits = (size+3) & 7
    padding_bits = (padding_bits == 0) ? 0 : (8 - padding_bits)
    [("%03b" % padding_bits) + self.to_s].pack("b*")
  end

  # Convert a string created using the pack method back into a bitset.
  def self.unpack str
    bits = str.unpack("b*")[0]
    padding_bits = bits[0...3].to_i(2)
    from_s(bits[3 .. -1 - padding_bits])
  end

end
