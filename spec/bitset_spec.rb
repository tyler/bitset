require 'bitset'

describe Bitset do
  it 'can be initialized' do
    Bitset.new(64)
  end

  describe :size do
    it 'returns the correct size' do
      Bitset.new(64).size.should == 64
      Bitset.new(73).size.should == 73
    end
  end

  describe :[] do
    it 'returns True for set bits' do
      bs = Bitset.new(8)
      bs[0] = true
      bs[0].should == true
    end

    it 'returns False for unset bits' do
      bs = Bitset.new(8)
      bs[0].should == false
    end

    it 'raises an error when accessing out of bound indexes' do
      bs = Bitset.new(8)
      expect { bs[8] }.to raise_error(IndexError)
    end
  end

  describe :[]= do
    it 'sets True for truthy values' do
      bs = Bitset.new(8)

      bs[0] = true
      bs[0].should == true

      bs[1] = 123
      bs[1].should == true

      bs[2] = "woo"
      bs[2].should == true
    end

    it 'sets False for falsey values' do
      bs = Bitset.new(8)

      bs[0] = false
      bs[0].should == false

      bs[1] = nil
      bs[1].should == false
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

      bs[1].should == true
      bs[2].should == true
      bs[3].should == true
    end
  end

  describe :clear do
    it 'sets False for all given indexes' do
      bs = Bitset.new(8)
      bs.set 1,2,3
      bs.clear 1,3

      bs[1].should == false
      bs[2].should == true
      bs[3].should == false
    end
  end

  describe :set? do
    it 'returns True if all bits indexed are set' do
      bs = Bitset.new(8)
      bs.set 1, 4, 5
      bs.set?(1,4,5).should == true
    end

    it 'returns False if not all bits indexed are set' do
      bs = Bitset.new(8)
      bs.set 1, 4
      bs.set?(1,4,5).should == false
    end
  end

  describe :clear? do
    it 'returns True if all bits indexed are clear' do
      bs = Bitset.new(8)
      bs.set 1, 4, 5
      bs.clear?(0,2,3,6).should == true
    end

    it 'returns works with the full range of 64 bit values'  do
      bs = Bitset.new(68)
      bs.set 0, 2, 66
      bs.clear?(32, 33, 34).should == true
    end

    it 'returns False if not all bits indexed are clear' do
      bs = Bitset.new(8)
      bs.set 1, 4
      bs.clear?(1,2,6).should == false
    end
  end

  describe :cardinality do
    it 'returns the number of bits set' do
      bs = Bitset.new(8)
      bs.cardinality.should == 0

      bs[0] = true
      bs.cardinality.should == 1

      bs[1] = true
      bs.cardinality.should == 2

      bs[2] = true
      bs.cardinality.should == 3

      bs = Bitset.new(64)
      bs[0] = 1

      bs.cardinality.should == 1
    end

    it '... even for large numbers of bits' do
      bs = Bitset.new(10_000)
      bs.set(*(0...5000).to_a)
      bs.cardinality.should == 5000

      bs = Bitset.from_s "01001101000000000000000000000011000010100100000000000000010000101000000000000000100000000100000000000010100100010000000010000100000100000001001000110000000000100010000000010100000000000000110000000000000000000000000100000000100010010000000000000000000001000000000000000000000000000001000000000000000000000000000100000000010010000000000000000000100100000000000000001000000010000001000000000000001000001100010001000000000000001000001000001000000000000001100010000010010001000000010000100000000000110000"
      bs.cardinality.should == 63
    end
  end

  describe :& do
    it 'returns a new Bitset which is the intersection of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      bs3 = bs1 & bs2
      bs3.set?(1,4).should == true
      bs3.clear?(0,2,3,5,6,7).should == true
    end
  end

  describe :| do
    it 'returns a new Bitset which is the union of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      bs3 = bs1 | bs2
      bs3.set?(1,2,4,6,7).should == true
      bs3.clear?(0,3,5).should == true
    end
  end

  describe :- do
    it 'returns a new Bitset which is the difference of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      bs3 = bs1 - bs2
      bs3.set?(7).should == true
      bs3.clear?(0,1,2,3,4,5,6).should == true
    end
  end

  describe :^ do
    it 'returns a new Bitset which is the xor of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      bs3 = bs1 ^ bs2
      bs3.set?(2,6,7).should == true
      bs3.clear?(0,1,3,4,5).should == true
    end
  end

  describe :not do
    it "returns a new Bitset with is the not of one Bitset" do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = bs1.not
      bs2.set?(0, 2, 3, 5, 6).should == true
      bs2.clear?(1, 4, 7).should == true
    end
  end

  describe :hamming do
    it 'returns the hamming distance of two Bitsets' do
      bs1 = Bitset.new(8)
      bs1.set 1, 4, 7

      bs2 = Bitset.new(8)
      bs2.set 1, 2, 4, 6

      bs1.hamming(bs2).should == 3
    end
  end

  describe :each do
    it 'iterates over the bits in the Bitset' do
      bs = Bitset.new(4)
      bs.set 0, 2

      i = 0
      bs.each do |bit|
        bit.should == bs[i]
        i += 1
      end
      i.should == 4
    end
  end

  describe :to_s do
    it 'correctly prints out a binary string' do
      bs = Bitset.new(4)
      bs.set 0, 2
      bs.to_s.should == "1010"

      bs = Bitset.new(68)
      bs.set 0, 2, 66
      bs.to_s.should == "101" + ("0" * 63) + "10"
    end
  end

  describe :from_s do
    it 'correctly creates a bitmap from a binary string' do
      bs = Bitset.from_s("10101")
      bs.set?(0, 2, 4).should == true
    end
  end

  describe :marshalling do
    it 'can marshal and load' do
      bs = Bitset.new(68)
      bs.set 1, 65

      serialized = Marshal.load(Marshal.dump(bs))
      serialized.set?(1, 65).should == true
      serialized.cardinality.should == 2
    end
  end
end
