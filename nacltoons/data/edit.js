// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

var current_file = null;
var current_fs = null;

function EscapeString(htmlString) {
  var replaceMap = {
    '&': '&amp;',
    '<': '&amp;',
    '>': '&gt;',
    '"': '&quot;',
    '\'': '&#039;'
  };

  for (var key in replaceMap) {
    htmlString = htmlString.replace(key, replaceMap[key])
  }

  console.log('Got: ' + htmlString)
  return htmlString
}


function ResourceError(e) {
  var msg = '';

  switch (e.code) {
    case FileError.QUOTA_EXCEEDED_ERR:
      msg = 'QUOTA_EXCEEDED_ERR';
      break;
    case FileError.NOT_FOUND_ERR:
      msg = 'NOT_FOUND_ERR';
      break;
    case FileError.SECURITY_ERR:
      msg = 'SECURITY_ERR';
      break;
    case FileError.INVALID_MODIFICATION_ERR:
      msg = 'INVALID_MODIFICATION_ERR';
      break;
    case FileError.INVALID_STATE_ERR:
      msg = 'INVALID_STATE_ERR';
      break;
    default:
      msg = 'Unknown Error';
      break;
  };

  console.log('Error: ' + msg);
}

// Remove a file from the temp file system
function ClearResource(path, callback) {
  var filename = 'Resources/' + path
  current_fs.root.getFile(filename, {create: false},
    function(fileEntry) {
      console.log('Openned ' + filename + ' from temp FS.\n');
      fileEntry.remove(function() {
          console.log('Removed file ' + path + ' from temp FS.\n')
          callback();
      })
    },
    function() {
      console.log('Failed to remove ' + filename + ' from temp FS.\n');
      callback();
    });
}


// Load a Resource first from TEMP, then via URL if not found
function LoadResource(path, callback) {
  // Tried to read the file from the file system first
  var filename = 'Resources/' + path
  current_fs.root.getFile(filename, {create: false},
    function(fileEntry) {
      console.log('Openned ' + filename + ' from temp FS.\n');
      fileEntry.file(function(file) {
        var reader = new FileReader();
        console.log('Created file reader.\n');
        reader.onload = function(e) {
          console.log('Read file ' + path + ' from temp FS.\n')
          callback(this.result);
        }
        reader.onerror = function(e) {
          console.log('Failed to read ' + path +
              'from TEMP.\n  ' + e.toString());
        }
        reader.readAsText(file);
      }, ResourceError);
    },
    function(e) {
      // If it fails to find it locally, read it from the server directly
      if (e.code == FileError.NOT_FOUND_ERR) {
        var fetch = new XMLHttpRequest();
        fetch.open('GET', filename);
        fetch.onload = function(e) {
          console.log('Read file ' + filename + ' from URL.\n');
          callback(fetch.responseText);
        }
        fetch.onerror = function(e) {
          console.log('Failed to load ' + filename + '.\n  ' + e.toString());
        }
        fetch.send();
      } else {
        // Every other failure is a REAL failure
        ResourceError(e);
      }
    });
}

// Save a file to the TEMPFS then push it back to the server
// TBD(noelallen) add Push
function SaveResource(path, data) {
  filename = 'Resources/' + path;
  current_fs.root.getFile(filename, {create: true, exclusive: false},
    function(fileEntry) {
      fileEntry.createWriter(function(fileWriter) {
        fileWriter.onwriteend = function(e) {
          console.log('Wrote ' + filename + ' to temp FS.\n');
        };
        fileWriter.onerror = function(e) {
          console.log('Failed to write ' + filename + '.\n  ' + e.toString());
        };
        var blob = new Blob([data], {type: 'text/plain'});
        fileWriter.write(blob);
      });
    },
    function(e) {
      console.log('Failed to write ' + filename + '.\n  ' + e.toString());
    });
}

function ClearSources(sourceList) {
  var srcs = sourceList.split('\n');
  var counter = 0;

  for (var i=0; i < srcs.length;i++) {
    var filename = srcs[i];
    if (filename) {
      ClearResource(filename, function() {
        counter = counter + 1;
        console.log('Called delete ' + filename + ' ' + counter.toString());
        if (counter == srcs.length) {
          console.log('Calling populate.\n')
          PopulateSources(sourceList);
        }
      });
    } else {
      counter = counter + 1;
    }
  }
}

// Build a ComboBox of sources
function PopulateSources(sourceList) {
  var items  = sourceList.split('\n');
  var listbox = document.getElementById('sources');
  var out = '';

  for (var i=0; i < items.length;i++) {
    if (items[i]) {
      out += '<option>' + EscapeString(items[i]) + '</option>';
    }
  }

  listbox.innerHTML = out;
  ChangedSource(items[i]);
}

// Update the TextArea containing the source.
function PopulateEdit(data) {
  var textbox = document.getElementById('text');
  textbox.value = data;
}

// When Save is clicked, save the Resource file.
function SaveFile() {
  var textbox = document.getElementById('text');
  SaveResource(current_file, textbox.value);
}

// When Sources ComboBox changes, update the edit area.
function ChangedSource(path) {
  if (path) {
    LoadResource(path, function(textdata) {
      PopulateEdit(textdata);
    });
  } else {
    PopulateEdit('');
  }
  current_file = path
}

// Send RESTART to NaCl
function Restart() {
  text = document.getElementById('restart').innerHTML;
  common.naclModule.postMessage('RESTART');
}

// Called by common.js when document loads
function attachListeners() {
  document.getElementById('save').addEventListener('click',
      SaveFile);
  document.getElementById('restart').addEventListener('click',
      Restart);
  document.getElementById('sources').addEventListener('change',
      function(data) { ChangedSource(data.target.value); });

  // Open TEMP FileSystem and load asset list
  window.webkitRequestFileSystem(window.TEMPORARY, 512*1024,
      function(fs) {
        console.log('Opened temporary file system ' + fs.name + '.\n');
        current_fs = fs;
        fs.root.getDirectory('Resources', {create: true}, function(dirEntry) {
          console.log('Created Resources cache.\n');
        }, ResourceError);
        LoadResource('assets.def', ClearSources);
      },
      function(e) { alert('Failed to access space'); });
}
