require "bitset/bitset"

class Bitset
  # @return [String] This bitset packed into bytes.
  #
  # The first 3 bits represent the number of padding bits in the final
  # byte of the string.
  #
  # This is somewhat redundant with Marshal.dump and Marshal.load, but
  # it does save a few bytes.
  def pack
    # Number of bits of zero padding in this representation.
    padding_bits = (size+3) & 7
    padding_bits = (padding_bits == 0) ? 0 : (8 - padding_bits)
    [("%03b" % padding_bits) + self.to_s].pack("b*")
  end

  def inspect
    "#{self.class.name}:#{to_s}"
  end

  # @param [String] str Output from {#pack}
  #
  # @return [Bitset] A duplicate of the input to {#pack}
  def self.unpack str
    bits = str.unpack("b*").first
    padding_bits = bits[0...3].to_i(2)
    from_s(bits[3 .. -1 - padding_bits])
  end
end
