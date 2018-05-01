require 'bitset'

describe Bitset do
  it 'can be initialized by size' do
    Bitset.new(64)
  end

  it 'can be initialized by array' do
    Bitset.new([false, nil, 3, 0]).to_s == "0011"
  end

  it 'raises ArgumentError wihen initialized with no argument' do
    expect { Bitset.new }.to raise_error(ArgumentError)
  end

  describe :size do
    it 'returns the correct size' do
      expect(Bitset.new(64).size).to eq(64)
      expect(Bitset.new(73).size).to eq(73)
    end
  end

  describe :[] do
    it 'returns True for set bits' do
      bs = Bitset.new(8)
      bs[0] = true
      expect(bs[0]).to be true
    end

    it 'returns False for unset bits' do
      bs = Bitset.new(8)
      expect(bs[0]).to be false
    end

    it 'raises an error when accessing out of bound indexes' do
      bs = Bitset.new(8)
      expect { bs[8] }.to raise_error(IndexError)
      expect { bs[-1] }.to raise_error(IndexError)
    end
  end

  describe :[]= do
    it 'sets True for truthy values' do
      bs = Bitset.new(8)

      bs[0] = true
      expect(bs[0]).to be true

      bs[1] = 123
      expect(bs[1]).to be true

      bs[2] = "woo"
      expect(bs[2]).to be true
    end

    it 'sets False for falsey values' do
      bs = Bitset.new(8)

      bs[0] = false
      expect(bs[0]).to be false

      bs[1] = nil
      expect(bs[1]).to be false
    end

    it 'raises an error when setting out of bound indexes' do
      bs = Bitset.new(8)
      expect { bs[8] = true }.to raise_error(IndexError)
    end
  end

  describe :set do
    it 'sets True for all given indexes' do
      bs = Bitset.new(8)
      bs.set 1,2,3

      expect(bs[1]).to be true
      expect(bs[0]).to be false
      expect(bs[4]).to be false
      expect(bs[3]).to be true
    end
  end

  describe :clear do
    it 'sets False for all given indexes' do
      bs = Bitset.new(8)
      bs.set 1,2,3
      bs.clear 1,3

      expect(bs[1]).to be false
      expect(bs[2]).to be true
      expect(bs[3]).to be false
    end
  end

  describe :set? do
    it 'returns True if all bits indexed are set' do
      bs = Bitset.new(8)
      bs.set 1, 4, 5
      expect(bs.set?(1,4,5)).to be true
    end

    it 'returns False if not all bits indexed are set' do
      bs = Bitset.new(8)
      bs.set 1, 4
      expect(bs.set?(1,4,5)).to be false
    end
  end

  describe :clear? do
    it 'returns True if all bits indexed are clear' do
      bs = Bitset.new(8)
      bs.set 1, 4, 5
      expect(bs.clear?(0,2,3,6)).to be true
    end

    it 'works with the full range of 64 bit values'  do
      bs = Bitset.new(68)
      bs.set 0, 2, 66
      expect(bs.clear?(32, 33, 34)).to be true
    end

    it 'returns false if not all bits indexed are clear' do
      bs = Bitset.new(8)
      bs.set 1, 4
      expect(bs.clear?(1,2,6)).to be false
    end
  end

  describe :cardinality do
    it 'returns the number of bits set' do
      bs = Bitset.new(64)
      expect(bs.cardinality).to eq(0)

      bs[0] = true
      expect(bs.cardinality).to eq(1)

      bs[1] = true
      expect(bs.cardinality).to eq(2)

      bs[2] = true
      expect(bs.cardinality).to eq(3)

      expect(bs.not.cardinality).to eq(bs.size - bs.cardinality)
    end

    it '... even for large numbers of bits' do
      bs = Bitset.new(10_000)
      size = 5000
      bs.set((0...size).to_a)
      expect(bs.cardinality).to eq(size)

      bs = Bitset.from_s "01001101000000000000000000000011000010100100000000000000010000101000000000000000100000000100000000000010100100010000000010000100000100000001001000110000000000100010000000010100000000000000110000000000000000000000000100000000100010010000000000000000000001000000000000000000000000000001000000000000000000000000000100000000010010000000000000000000100100000000000000001000000010000001000000000000001000001100010001000000000000001000001000001000000000000001100010000010010001000000010000100000000000110000"
      expect(bs.cardinality).to eq(63)
      expect(bs.not.cardinality).to eq(bs.size - bs.cardinality)
    end
  end

  describe :& do
    it 'returns a new Bitset which is the intersection of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      bs3 = bs1 & bs2
      expect(bs3.set?(1,4)).to be true
      expect(bs3.clear?(0,2,3,5,6,7)).to be true
    end
  end

  describe :| do
    it 'returns a new Bitset which is the union of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      bs3 = bs1 | bs2
      expect(bs3.set?(1,2,4,6,7)).to be true
      expect(bs3.clear?(0,3,5)).to be true
    end
    it 'throws if size mismatch' do
      expect { Bitset.new(3) | Bitset.new(7) }.to raise_error(ArgumentError)
    end
  end

  describe :- do
    it 'returns a new Bitset which is the difference of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      bs3 = bs1 - bs2
      expect(bs3.set?(7)).to be true
      expect(bs3.clear?(0,1,2,3,4,5,6)).to be true
    end
    it 'throws if size mismatch' do
      expect { Bitset.new(3) - Bitset.new(7) }.to raise_error(ArgumentError)
    end
  end

  describe :^ do
    it 'returns a new Bitset which is the xor of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      bs3 = bs1 ^ bs2
      expect(bs3.set?(2,6,7)).to be true
      expect(bs3.clear?(0,1,3,4,5)).to be true
    end

    it 'throws if size mismatch' do
      expect { Bitset.new(3) ^ Bitset.new(7) }.to raise_error(ArgumentError)
    end
  end

  describe :not do
    it "returns a new Bitset which is the not of one Bitset" do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = bs1.not
      expect(bs2.set?(0, 2, 3, 5, 6)).to be true
      expect(bs2.clear?(1, 4, 7)).to be true
    end
  end

  describe :hamming do
    it 'returns the hamming distance of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      expect(bs1.hamming(bs2)).to eq(3)
    end
  end

  describe :each do
    it 'iterates over the bits in the Bitset' do
      bs = Bitset.new(4)
      bs.set 0, 2

      i = 0
      bs.each do |bit|
        expect(bit).to be bs[i]
        i += 1
      end
      expect(i).to eq(4)
    end
  end

  describe :each_set do
    it 'iterates over each set bit in the Bitset' do
      bs = Bitset.new(4)
      sets = [0,3]
      bs.set sets
      sets2 = []
      bs.each_set { |bit| sets2 << bit }
      expect(sets2).to eq(sets)
    end

    it 'without a block, it returns an array of set bits' do
      bs = Bitset.new(4)
      sets = [0,3]
      bs.set(*sets)
      expect(bs.each_set).to eq(sets)
    end

    it 'behaves properly with arguments' do
      bs = Bitset.from_s "110110011"
      expect { bs.each_set 1, 2, 3 }.to raise_error(ArgumentError)
      expect(bs.each_set 2).to eq(3)
      expect(bs.each_set -3, 2).to eq([4,7])
    end
  end

  describe :empty? do
    it 'returns empty only if all zeroes' do
      expect(Bitset.new(225).tap { |bs| bs[133] = true }.empty?).to be false
      expect(Bitset.new(0).empty?).to be true
      expect(Bitset.new(225).empty?).to be true
    end
  end

  describe :dup do
    it "returns a duplicate" do
      bs = Bitset.from_s("11011")
      expect(bs.dup.tap { |bs| bs.clear 1,3 }.to_s).to eq("10001")
      expect(bs.to_s).to eq("11011")
    end
  end

  describe :clone do
    it "works" do
      expect(Bitset.new(0).clone.to_s).to eq("")
    end
  end

  describe :to_s do
    it 'correctly prints out a binary string' do
      bs = Bitset.new(4)
      bs.set 0, 2
      expect(bs.to_s).to eq("1010")

      bs = Bitset.new(68)
      bs.set 0, 2, 66
      expect(bs.to_s).to eq("101" + ("0" * 63) + "10")
    end
  end

  describe :from_s do
    it 'correctly creates a bitmap from a binary string' do
      bs = Bitset.from_s("10101")
      expect(bs.set?(0, 2, 4)).to be true
    end
  end

  describe :marshalling do
    it 'can marshal and load' do
      bs = Bitset.new(68)
      bs.set 1, 65

      serialized = Marshal.load(Marshal.dump(bs))
      expect(serialized.set?(1, 65)).to be true
      expect(serialized.cardinality).to eq(2)
    end
  end

  describe :union! do
    it 'acts like |=' do
      bs = Bitset.from_s "11011"
      bs2 = Bitset.from_s "01111"
      bs3 = bs.dup
      bs.union!(bs2)
      expect(bs).to eq(bs3 | bs2)
    end

    it 'throws if size mismatch' do
      expect { Bitset.new(3).union!(Bitset.new(7)) }.to raise_error(ArgumentError)
    end
  end

  describe :intersect! do
    it 'acts like &=' do
      bs = Bitset.from_s "11011"
      bs2 = Bitset.from_s "01111"
      bs3 = bs.dup
      bs.intersect!(bs2)
      expect(bs).to eq(bs3 & bs2)
    end
  end

  describe :xor! do
    it 'acts like ^=' do
      bs = Bitset.from_s "11011"
      bs2 = Bitset.from_s "01111"
      bs3 = bs.dup
      bs.xor!(bs2)
      expect(bs).to eq(bs3 ^ bs2)
    end

    it 'throws if size mismatch' do
      expect { Bitset.new(3).xor!(Bitset.new(7)) }.to raise_error(ArgumentError)
    end
  end

  describe :difference! do
    it 'acts like -=' do
      bs = Bitset.from_s "11011"
      bs2 = Bitset.from_s "01111"
      bs3 = bs.dup
      bs.difference!(bs2)
      expect(bs).to eq(bs3 - bs2)
    end
  end

  describe :reset! do
    it 'causes empty? to become true' do
      bs = Bitset.from_s "11011"
      bs.reset!
      expect(bs.empty?).to be true
    end
  end

  describe :to_a do
    it "can convert to an array of set positions" do
      bs = Bitset.new(68)
      bs.set 1, 64, 65

      expect(bs.to_a).to eq([1, 64, 65])
    end
  end

  describe :to_binary_array do
    it "can convert to an array of 1s and 0s" do
      bs = Bitset.new(68)
      bs.set 1, 64, 65

      expect(bs.to_binary_array.values_at(1, 64, 65, 66)).to eq([1, 1, 1, 0])
    end
  end

  describe :pack do
    it 'round trips with #unpack' do
      %w{110 00010 101010011101011100000110011010110011010001}.each do |bits|
        bitset = Bitset.from_s bits
        packed = bitset.pack
        unpacked = Bitset.unpack(packed)
        expect(bitset).to eq(unpacked)
      end
    end
  end
  
  describe :inspect do
    it "returns expected output" do
      expect(Bitset.from_s("1011").inspect).to eq("Bitset:1011")
    end
  end
end
