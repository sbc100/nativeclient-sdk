// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * The stamp panel object.  This object manages the UI connections for various
 * elements of the stamp editor panel, and makes the dynamic parts of the
 * panel's DOM.
 */


goog.provide('stamp');
goog.provide('stamp.StampPanel');

goog.require('goog.Disposable');
goog.require('goog.dom');
goog.require('goog.editor.Table');
goog.require('goog.events');
goog.require('goog.style');
goog.require('goog.ui.Zippy');

goog.require('stamp.Editor');

/**
 * Manages the data and interface for the stamp editor.
 * @param {!Element} noteContainer The element under which DOM nodes for
 *     the stamp editor should be added.
 * @constructor
 * @extends {goog.events.EventTarget}
 */
stamp.StampPanel = function(editorContainer) {
  goog.events.EventTarget.call(this);
  this.parent_ = editorContainer;
};
goog.inherits(stamp.StampPanel, goog.events.EventTarget);

/**
 * A dictionary of all the DOM elements used by the stamp editor.
 * @type {Object<Element>}
 * @private
 */
stamp.StampPanel.prototype.domElements_ = {};

/**
 * The minimum number of rows and columns in the stamp editor table.
 */
stamp.StampPanel.prototype.MIN_ROW_COUNT = 3;
stamp.StampPanel.prototype.MIN_COLUMN_COUNT = 3;

/**
 * The ids used for elements in the DOM.  The stamp editor expects these
 * elements to exist.
 * @enum {string}
 */
stamp.StampPanel.DomIds = {
  ADD_COLUMN_BUTTON: 'add_column_button',
  ADD_ROW_BUTTON: 'add_row_button',
  CANCEL_BUTTON: 'cancel_button',
  OK_BUTTON: 'ok_button',
  REMOVE_COLUMN_BUTTON: 'remove_column_button',
  REMOVE_ROW_BUTTON: 'remove_row_button',
  STAMP_EDITOR_BUTTON: 'stamp_editor_button',
  STAMP_EDITOR_CONTAINER: 'stamp_editor_container',
  STAMP_EDITOR_PANEL: 'stamp_editor_panel'
};

/**
 * Panel events.  These are dispatched on the stamp editor container object
 * (usually a button that opens the panel), and indicate things like when the
 * panel is about to open, or when it has been closed using specific buttons.
 * @enum {string}
 */
stamp.StampPanel.Events = {
  PANEL_DID_CANCEL: 'panel_did_cancel',  // The Cancel button was clicked.
  PANEL_DID_SAVE: 'panel_did_save',  // The OK button was clicked.
  // Sent right before the editor panel will collapse.  Note that it is possible
  // for the event to be sent, but the panel will not actually close.  This can
  // happen, for example, if the panel open button gets a mouse-down, but does
  // not get a complete click event (no corresponding mouse-up).
  PANEL_WILL_CLOSE: 'panel_will_close',
  // Sent right before the editor panel will expand.  Note that it is possible
  // for the event to be sent, but the panel will not actually open.  This can
  // happen, for example, if the panel open button gets a mouse-down, but does
  // not get a complete click event (no corresponding mouse-up).
  PANEL_WILL_OPEN: 'panel_will_open'
};

/**
 * Attributes added to cells to cache certain parameters like aliveState.
 * @enum {string}
 * @private
 */
stamp.StampPanel.CellAttributes_ = {
  ENABLED_OPACITY: '1',
  DISABLED_OPACITY: '0.5'
};

/**
 * Characters used to encode the stamp as a string.
 * @enum {string}
 * @private
 */
stamp.Editor.StringEncoding_ = {
  DEAD_CELL: '.',
  END_OF_ROW: '\n',
  LIVE_CELL: '*'
};

/**
 * Override of disposeInternal() to dispose of retained objects and unhook all
 * events.
 * @override
 */
stamp.StampPanel.prototype.disposeInternal = function() {
  for (elt in this.domElements_) {
    goog.events.removeAll(elt);
  }
  goog.events.removeAll(this.parent_);
  this.panel_ = null;
  this.parent_ = null;
  this.stampEditor_ = null;
  stamp.StampPanel.superClass_.disposeInternal.call(this);
}

/**
 * Respond to a MOUSEDOWN event on the parent element.  This dispatches a
 * a PANEL_WILL* event based on the expanded state of the panel.
 * @param {!goog.events.Event} mousedownEvent The MOUSEDOWN event that
 *     triggered this handler.
 */
stamp.StampPanel.prototype.handleParentMousedown_ = function(mousedownEvent) {
  mousedownEvent.stopPropagation();
  var eventType;
  if (this.panel_.isExpanded()) {
    // About to close the panel.
    eventType = stamp.StampPanel.Events.PANEL_WILL_CLOSE;
  } else {
    eventType = stamp.StampPanel.Events.PANEL_WILL_OPEN;
  }
  this.dispatchEvent(new goog.events.Event(eventType));
}

/**
 * Return the current stamp expressed as a string.
 * @return {!string} The stamp currently in the editor.
 */
stamp.StampPanel.prototype.getStampAsString = function() {
  return this.stampEditor_.getStampAsString();
}

/**
 * Sets the current stamp based on the stamp encoding in |stampString|.
 * @param {!string} stampString The encoded stamp string.
 */
stamp.StampPanel.prototype.setStampFromString = function(stampString) {
  this.stampEditor_.setStampFromString(stampString);
  this.refreshUI_();
}

/**
 * Creates the DOM structure for the stamp editor panel and adds it to the
 * document.  The panel is built into the mainPanel DOM element.  The expected
 * DOM elements are keys in the |stampEditorElements| object:
 *   mainPanel The main panel element.
 *   editorContainer: A container for a TABLE element that impelements the
 *       stamp editor.
 *   addColumnButton: An element that becomes the button used to add a column to
 *       the stamp editor.  An onclick listener is attached to this element.
 *   removeColumnButton:  An element that becomes the button used to remove a
 *       column from the stamp editor.  An onclick listener is attached to this
 *       element.
 *   addRowButton:  An element that becomes the button used to add a row to
 *       the stamp editor.  An onclick listener is attached to this element.
 *   removeRowButton: An element that becomes the button used to remove a
 *       row from the stamp editor.  An onclick listener is attached to this
 *       element.
 *   cancelButton: An element that becomes the button used to close the panel
 *       without changing the current stamp.  An onclick listener is attached
 *       to this element.  Dispatches the "panel did close" event.
 *   okButton:  An element that becomes the button used to close the panel
 *       and update the current stamp.  An onclick listener is attached
 *       to this element.  Dispatches the "panel did close" event.
 * @param {!Object<Element>} stampEditorElements A dictionary of DOM elements
 *     required by the stamp editor panel.
 * @return {!goog.ui.Zippy} The Zippy element representing the entire stamp
 *     panel with enclosed editor.
 */
stamp.StampPanel.prototype.makeStampEditorPanel =
    function(stampEditorElements) {
  if (!stampEditorElements) {
    return null;
  }
  for (elt in stampEditorElements) {
    this.domElements_[elt] = stampEditorElements[elt];
  }
  // Create DOM structure to represent the stamp editor panel.  This panel
  // contains all the UI elements of the stamp editor: the stamp editor
  // itself, add/remove column and row buttons, a legend and a title string.
  // The layout of the panel is described in the markup; this code wires up
  // the button and editor behaviours.
  this.domElements_.panelHeader = goog.dom.createDom('div',
      {'style': 'background-color:#EEE',
       'class': 'panel-container'}, 'Stamp Editor...');
  this.domElements_.panelContainer = stampEditorElements.mainPanel;
  goog.style.setPosition(this.domElements_.panelContainer, 0,
                         goog.style.getSize(this.parent_).height);
  this.domElements_.panelContainer.style.display = 'block';
  var newEditor = goog.dom.createDom('div', null,
      this.domElements_.panelHeader, this.domElements_.panelContainer);

  // Create the editable stamp representation within the editor panel.
  this.stampEditor_ = new stamp.Editor();
  this.stampEditor_.makeStampEditorDom(stampEditorElements.editorContainer);

  goog.events.listen(this.parent_, goog.events.EventType.MOUSEDOWN,
                     this.handleParentMousedown_, false, this);

  // Wire up the add/remove column and row buttons.
  goog.events.listen(stampEditorElements.addColumnButton,
                     goog.events.EventType.CLICK,
                     this.addColumn_, false, this);
  goog.events.listen(stampEditorElements.removeColumnButton,
                     goog.events.EventType.CLICK,
                     this.removeColumn_, false, this);
  goog.events.listen(stampEditorElements.addRowButton,
                     goog.events.EventType.CLICK,
                     this.addRow_, false, this);
  goog.events.listen(stampEditorElements.removeRowButton,
                     goog.events.EventType.CLICK,
                     this.removeRow_, false, this);
  goog.events.listen(stampEditorElements.cancelButton,
                     goog.events.EventType.CLICK,
                     this.closePanelAndCancel_, false, this);
  goog.events.listen(stampEditorElements.okButton,
                     goog.events.EventType.CLICK,
                     this.closePanelAndSave_, false, this);

  // Add the panel's DOM structure to the document.
  goog.dom.appendChild(this.parent_, newEditor);
  this.panel_ = new goog.ui.Zippy(this.domElements_.panelHeader,
                                  this.domElements_.panelContainer);
  return this.panel_;
};

/**
 * Respond to a CLICK event on the "add column" button.  Update the display
 * to reflect whether columns can be removed or not.
 * @param {!goog.events.Event} clickEvent The CLICK event that triggered this
 *     handler.
 */
stamp.StampPanel.prototype.addColumn_ = function(clickEvent) {
  clickEvent.stopPropagation();
  this.stampEditor_.appendColumn();
  this.refreshUI_();
}

/**
 * Respond to a CLICK event on the "remove column" button.
 * @param {!goog.events.Event} clickEvent The CLICK event that triggered this
 *     handler.
 */
stamp.StampPanel.prototype.removeColumn_ = function(clickEvent) {
  clickEvent.stopPropagation();
  this.stampEditor_.removeLastColumn();
  this.refreshUI_();
}

/**
 * Respond to a CLICK event on the "add row" button.
 * @param {!goog.events.Event} clickEvent The CLICK event that triggered this
 *     handler.
 */
stamp.StampPanel.prototype.addRow_ = function(clickEvent) {
  clickEvent.stopPropagation();
  this.stampEditor_.appendRow();
  this.refreshUI_();
}

/**
 * Respond to a CLICK event on the "remove row" button.
 * @param {!goog.events.Event} clickEvent The CLICK event that triggered this
 *     handler.
 * @private
 */
stamp.StampPanel.prototype.removeRow_ = function(clickEvent) {
  clickEvent.stopPropagation();
  this.stampEditor_.removeLastRow();
  this.refreshUI_();
}

/**
 * Update the UI to reflect thing like "remove column" button to be enabled if
 * there are column that can be removed.
 * @private
 */
stamp.StampPanel.prototype.refreshUI_ = function() {
  if (this.stampEditor_.rowCount() > this.MIN_ROW_COUNT) {
    goog.style.setStyle(this.domElements_.removeRowButton,
        { 'opacity': stamp.StampPanel.CellAttributes_.ENABLED_OPACITY });
  } else {
    goog.style.setStyle(this.domElements_.removeRowButton,
        { 'opacity': stamp.StampPanel.CellAttributes_.DISABLED_OPACITY });
  }
  if (this.stampEditor_.columnCount() > this.MIN_COLUMN_COUNT) {
    goog.style.setStyle(this.domElements_.removeColumnButton,
        { 'opacity': stamp.StampPanel.CellAttributes_.ENABLED_OPACITY });
  } else {
    goog.style.setStyle(this.domElements_.removeColumnButton,
        { 'opacity': stamp.StampPanel.CellAttributes_.DISABLED_OPACITY });
  }
}

/**
 * Respond to a CLICK event on the "cancel" button.
 * @param {!goog.events.Event} clickEvent The CLICK event that triggered this
 *     handler.
 * @private
 */
stamp.StampPanel.prototype.closePanelAndCancel_ = function(clickEvent) {
  if (this.panel_.isExpanded())
    this.panel_.collapse();
  // Dispatch two events: PANEL_WILL_CLOSE and PANEL_DID_CANCEL.
  this.dispatchEvent(
      new goog.events.Event(stamp.StampPanel.Events.PANEL_WILL_CLOSE));
  this.dispatchEvent(
      new goog.events.Event(stamp.StampPanel.Events.PANEL_DID_CANCEL));
}

/**
 * Respond to a CLICK event on the "ok" button.
 * @param {!goog.events.Event} clickEvent The CLICK event that triggered this
 *     handler.
 * @private
 */
stamp.StampPanel.prototype.closePanelAndSave_ = function(clickEvent) {
  if (this.panel_.isExpanded())
    this.panel_.collapse();
  // Dispatch two events: PANEL_WILL_CLOSE and PANEL_DID_SAVE.
  this.dispatchEvent(
      new goog.events.Event(stamp.StampPanel.Events.PANEL_WILL_CLOSE));
  this.dispatchEvent(
      new goog.events.Event(stamp.StampPanel.Events.PANEL_DID_SAVE));
}
