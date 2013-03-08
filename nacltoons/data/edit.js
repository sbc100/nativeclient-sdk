// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Called by the common.js module.
function attachListeners() {
  document.getElementById('reloadGame').addEventListener('click',
      ReloadGame);
  document.getElementById('reloadLevel').addEventListener('click',
      ReloadLevel);
}

function ReloadGame() {
  text = document.getElementById('gameSrc').innerHTML;
  common.naclModule.postMessage('GAME:' + text);
}

function ReloadLevel() {
  text = document.getElementById('levelSrc').innerHTML;
  common.naclModule.postMessage('LEVL:' + text);
}
