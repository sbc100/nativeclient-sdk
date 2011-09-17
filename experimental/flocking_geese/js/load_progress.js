// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * A load progress bar.  It has some title text, a dynamic progress bar that
 * shows load progress based on the values of the 'progress' event, and some
 * status test that shows load progress textually.
 */

goog.provide('LoadProgress');

goog.require('goog.Disposable');
goog.require('goog.dom');

/**
 * Constructor for the LoadProgress class.  Use the run() method to populate
 * the object with controllers and wire up the events.
 * @constructor
 * @extends {goog.Disposable}
 */
LoadProgress = function() {
  goog.Disposable.call(this);
}
goog.inherits(LoadProgress, goog.Disposable);

/**
 * The ids used for elements in the DOM.
 * @enum {string}
 */
LoadProgress.DomIds = {
  // The <DIV> containing all the load progress elements: title text, a
  // progress bar and some status text.
  PROGRESS: 'progress',
  // The <DIV> containing the load progress bar.
  PROGRESS_BAR: 'progress_bar',
  // The element containing the progress text.
  PROGRESS_BAR_TEXT: 'progress_bar_text',
  // The progress bar track, the width of this element tracks progress.
  PROGRESS_TRACK: 'progress_track',
};

/**
 * Label values.
 * @enum {string}
 */
LoadProgress.Labels = {
  TITLE_TEXT: 'Loading NaCl Module&hellip;',
  PROGRESS_TEXT: 'Computing size&hellip;'
}

/**
 * Override of disposeInternal() to dispose of retained objects.
 * @override
 */
LoadProgress.prototype.disposeInternal = function() {
  LoadProgress.superClass_.disposeInternal.call(this);
}

/**
 * Set the visiblity of the load progress bar.
 * @param {boolean} isVisible Whether the load progress bar is visible or not.
 */
LoadProgress.prototype.setVisible = function(isVisible) {
  var progress = document.getElementById(LoadProgress.DomIds.PROGRESS);
  progress.style.visibility = isVisible ? 'visible' : 'hidden';
}

/**
 * Handle a progress event by the NaCl module loader.  |progressEvent| contains
 * a couple of interesting properties that are used in this example:
 *     total The size of the NaCl module in bytes.  Note that this value
 *         is 0 until |lengthComputable| is true.  In particular, this
 *         value is 0 for the first 'progress' event.
 *     loaded The number of bytes loaded so far.
 *     lengthComputable A boolean indicating that the |total| field
 *         represents a valid length.
 * @param {Event} progressEvent The ProgressEvent that triggered this handler.
 */
LoadProgress.prototype.handleProgressEvent = function(progressEvent) {
  var loadPercent = 0.0;
  var loadPercentString;
  if (event.lengthComputable && event.total > 0) {
    loadPercent = event.loaded / event.total;
    loadPercentString = (loadPercent * 100.0).toFixed() + '%';
  } else {
    // The total length is not yet known.
    loadPercent = -1.0;
    loadPercentString = 'Computing size&hellip;';
  }
  var progressBarText = document.getElementById(
      LoadProgress.DomIds.PROGRESS_BAR_TEXT);
  progressBarText.innerHTML = loadPercentString +
    ' (' + event.loaded + ' of ' + event.total + ' bytes)';
  var progressTrack = document.getElementById(
      LoadProgress.DomIds.PROGRESS_TRACK);
  if (loadPercent >= 0.0) {
    var progressBar =
        document.getElementById(LoadProgress.DomIds.PROGRESS_BAR);
    var paddingBox = goog.style.getPaddingBox(progressBar);
    var maxTrackWidth = progressBar.clientWidth -
                        (paddingBox.left + paddingBox.right);
    progressTrack.style.width = (loadPercent * maxTrackWidth).toFixed() + 'px';
  } else {
    progressTrack.style.width = '0px';
  }
}

/**
 * Create the DOM elements for the progress bar.  The progress element is a
 * <DIV> that contains all the other progress bar elements.  The DOM layout
 * looks like this:
 *   <div id=LoadProgress.DomIds.PROGRESS class="progress">
 *     <p class="progressstatus">Loading NaCl Module&hellip;</p>
 *       <div id=LoadProgress.DomIds.PROGRESS_BAR class="progressbar tall">
 *         <div id=LoadProgress.DomIds.PROGRESS_TRACK class="progresstrack">
 *         </div>
 *       </div>
 *     <p id=LoadProgress.DomIds.PROGRESS_BAR_TEXT class="progresstext">
 *       Computing size&hellip;
 *     </p>
 *   </div>
 * @return {Element} The parent DOM element that contains the progress bar.
 */
LoadProgress.prototype.createDom = function() {
  var titleText = goog.dom.createDom('p', {'class': 'progressstatus'});
  titleText.innerHTML = LoadProgress.Labels.TITLE_TEXT;
  // The progress bar div contains a div representing the progress bar track.
  var progressBar = goog.dom.createDom(
      'div', {
          'id': LoadProgress.DomIds.PROGRESS_BAR,
          'class': 'progressbar tall'
      },
      goog.dom.createDom(
          'div', {
              'id': LoadProgress.DomIds.PROGRESS_TRACK,
              'class': 'progresstrack'
          }
      )
  );
  var progressBarText = goog.dom.createDom(
      'p', {
          'id': LoadProgress.DomIds.PROGRESS_BAR_TEXT,
          'class': 'progresstext'
      }
  );
  progressBarText.innerHTML = LoadProgress.Labels.PROGRESS_TEXT;
  // Combine all of the above into a single <DIV>.
  var progress = goog.dom.createDom(
      'div', {
          'id': LoadProgress.DomIds.PROGRESS,
          'class': 'progress'
      },
      titleText,
      progressBar,
      progressBarText
  )
  return progress;
}

