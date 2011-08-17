// This file must be loaded *after* all the benchmark script files 

// Instead of V8-style registration of the benchmarks in their own files,
// do it here all together, to make changing parameters easier.
// Code implementing the benchmark framework goes in base.js, stuff
// specific to the benchmarks (e.g. parameters or normalization) goes here.

var normalization_constants = Object(); 
normalization_constants["Fannkuchredux"] = 64052288;
normalization_constants["Nbody"] = 73000000;
normalization_constants["Spectralnorm"] = 150020779;
normalization_constants["Fasta"] = 51667385;
normalization_constants["Revcomp"] = 23542857;
normalization_constants["Binarytrees"] = 383306452;
normalization_constants["Knucleotide"] = 433893130;
normalization_constants["Pidigits"] = 406976744;


function SetupBenchmark(name, entrypoint, param) {
  var benchmark = new BenchmarkSuite(name, normalization_constants[name], [
    new Benchmark(name, function () { entrypoint(param) } )
  ]);
}

function ClearBenchmarks() {
  BenchmarkSuite.suites = [];
}

function SetRunModel(model) {
  BenchmarkSuite.run_model = model;
}

function SetupSmallBenchmarks() {
  SetupBenchmark("Fannkuchredux", FannkuchBenchmark, 10);
  SetupBenchmark("Nbody", NbodyBenchmark, 1000000);
  SetupBenchmark("Spectralnorm", SpectralnormBenchmark, 350);
  SetupBenchmark("Fasta", FastaBenchmark, 10000);
  SetupBenchmark("Revcomp", RevcompBenchmark, 0);
  SetupBenchmark("Binarytrees", BinarytreesBenchmark, 15);
  SetupBenchmark("Knucleotide", KnucleotideBenchmark, 0);
  SetupBenchmark("Pidigits", PidigitsBenchmark, 1000);
  benchmarks = BenchmarkSuite.CountBenchmarks();
  SetRunModel("repeated");
}

function SetupLargeBenchmarks() {
  SetupBenchmark("Fannkuchredux", FannkuchBenchmark, 11);
  SetupBenchmark("Nbody", NbodyBenchmark, 10000000);
  SetupBenchmark("Spectralnorm", SpectralnormBenchmark, 5500);
  SetupBenchmark("Fasta", FastaBenchmark, 3000000);
  SetupBenchmark("Revcomp", RevcompBenchmark, 0);
  SetupBenchmark("Binarytrees", BinarytreesBenchmark, 18);
  SetupBenchmark("Knucleotide", KnucleotideBenchmark, 0);
  SetupBenchmark("Pidigits", PidigitsBenchmark, 5000);
  benchmarks = BenchmarkSuite.CountBenchmarks();
  SetRunModel("once");
}