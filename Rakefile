require 'rake'
require 'rubygems'

require 'rspec/core/rake_task'
RSpec::Core::RakeTask.new(:spec) do |t|
  t.rspec_opts = ["--color"]
  t.fail_on_error = false
end

task :default => :spec

require 'jeweler'
Jeweler::Tasks.new do |gem|
  gem.name = "bitset"
  gem.homepage = "http://github.com/ericboesch/bitset"
  gem.license = "MIT"
  gem.summary = 'Bitset implementation.'
  gem.description = 'A fast C-based Bitset. It supports the standard set operations as well as operations you may expect on bit arrays,such as popcount.'
  gem.email = "eric.boesch@nist.gov"
  gem.authors = ["Tyler McMullen"]
  # Other significant contributions from Eric Boesch, Gabriel Formica, and Brendon McLean.

end
Jeweler::RubygemsDotOrgTasks.new

require 'rdoc/task'
RDoc::Task.new do |rdoc|
  version = File.exist?('VERSION') ? File.read('VERSION') : ""

  rdoc.rdoc_dir = 'rdoc'
  rdoc.title = "bitset #{version}"
  rdoc.rdoc_files.include('README*')
  rdoc.rdoc_files.include('ext/**/*.c')
end
