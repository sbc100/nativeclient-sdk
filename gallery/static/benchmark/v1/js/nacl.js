var benchmarkNaClModule = null;

function setupNaclBenchmark() {
  updateNaclStatus('Loading module...');
  benchmarkNaClModule = document.getElementById('benchmark_nexe');
}

function updateNaclResults(results) {
  resultbox = document.getElementById('nacl_results');
  resultbox.innerHTML += results + '<br>';
}

// Handle a message coming from the NaCl module.
function handleNaclMessage(message_event) {
  var msg = message_event.data;
  //console.log("nexe said: " + msg);
  if (msg.search(":") != -1) {
    if (msg.search("Score") != -1) {
      benchmarkNaClModule.removeEventListener('message', handleNaclMessage, false);
      updateNaclStatus(msg);
      document.getElementById('nacl').className = "run";
      naclBenchmarkFinished();
    } else if (msg.search("Score") == -1) {
      updateNaclResults(msg);
    } else {
      console.log("unknown NaCl message: " + msg);
    }
  } else {
    updateNaclStatus(msg);
  }
}

function clearNaclResults() {
  var results = document.getElementById("nacl_status");
  // Only clear after we have completed a run
  if (results.innerHTML.search("Score:") != -1) {
    document.getElementById("nacl_results").innerHTML = "<br />";
  }
}

function runSmallNaclBenchmarks() {
  clearNaclResults();
  benchmarkNaClModule.postMessage('runBenchmarks small');
}
function runLargeNaclBenchmarks() {
  clearNaclResults();
  benchmarkNaClModule.postMessage('runBenchmarks large');
}

function runNaclBenchmarks() {
  benchmarkNaClModule.addEventListener('message', handleNaclMessage, false);
  document.getElementById('nacl').className = "run running";
  runLargeNaclBenchmarks();
}

function updateNaclStatus(opt_message) {
  if (opt_message)
    statusText = opt_message;
  var NaclStatus = document.getElementById('nacl_status');
  if (NaclStatus) {
    NaclStatus.innerHTML = statusText;
  }
}

function naclModuleDidLoad() {
  naclDoneWaiting();
  runNaclBenchmarks();
}

function naclModuleError() {
  naclDoneWaiting();
  console.log(benchmarkNaClModule.lastError);
  updateNaclStatus("LOAD ERROR");
  naclBenchmarkFinished();
}

function setupNaclListeners(module_wrapper) {
  module_wrapper.addEventListener('load', naclModuleDidLoad, true);
  module_wrapper.addEventListener('error', naclModuleError, true);
}
