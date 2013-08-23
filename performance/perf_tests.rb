require 'bitset'
require 'benchmark'
require 'debugger'

bitset = Bitset.new(10_000)
(0...10_000).to_a.sample(5000).each { |i| bitset.set i }

puts Benchmark.measure {
  (1..13_000).each { bitset.to_a }
}