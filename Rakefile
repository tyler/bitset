require "bundler/gem_tasks"
require 'rspec/core/rake_task'
require 'rake/extensiontask'

gemspec = Gem::Specification.load('bitset.gemspec')
Rake::ExtensionTask.new do |ext|
  ext.name = 'bitset'
  ext.source_pattern = "*.{c}"
  ext.ext_dir = 'ext/bitset'
  ext.gem_spec = gemspec
end

desc "Run specs"
RSpec::Core::RakeTask.new do |t|
  t.pattern = "./spec/**/*_spec.rb" # don't need this, it's default.
  # Put spec opts in a file named .rspec in root
end

task :default => [:compile, :spec]
