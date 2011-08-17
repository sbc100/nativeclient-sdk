// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * The stamp editor object.  This manages a table whose cells represent cells
 * of the stamp.  Clicking on a table cell changes the state of the
 * corresponding stamp cell.  The table can be resized.
 */


goog.provide('stamp');
goog.provide('stamp.Editor');

goog.require('goog.Disposable');
goog.require('goog.dom');
goog.require('goog.editor.Table');
goog.require('goog.events');
goog.require('goog.style');

/**
 * Manages the data and interface for the stamp editor.
 * @param {!Element} noteContainer The element under which DOM nodes for
 *     the stamp editor should be added.
 * @constructor
 * @extends {goog.Disposable}
 */
stamp.Editor = function() {
  goog.Disposable.call(this);
};
goog.inherits(stamp.Editor, goog.events.EventTarget);

/**
 * The table that represents the stamp.
 * @type {goog.editor.Table}
 * @private
 */
stamp.Editor.prototype.stampEditorTable_ = null;

/**
 * The minimum number of rows and columns in the stamp editor table.
 */
stamp.Editor.prototype.MIN_ROW_COUNT = 3;
stamp.Editor.prototype.MIN_COLUMN_COUNT = 3;

/**
 * Attributes added to cells to cache certain parameters like aliveState.
 * @enum {string}
 * @private
 */
stamp.Editor.CellAttributes_ = {
  IS_ALIVE: 'isalive'  // Whether the cell is alive or dead.
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
stamp.Editor.prototype.disposeInternal = function() {
  var tableCells =
      this.stampEditorTable_.element.getElementsByTagName(goog.dom.TagName.TD);
  for (var i = 0; i < tableCells.length; ++i) {
    goog.events.removeAll(tableCells[i]);
  }
  this.stampEditorTable_ = null;
  stamp.Editor.superClass_.disposeInternal.call(this);
}

/**
 * Fills out the TABLE structure for the stamp editor.  The stamp editor
 * can be resized, and handles clicks in its cells by toggling their state.
 * The resulting TABLE element will have the minumum number of rows and
 * columns, and be filled in with a default stamp that creates a glider.
 * @param {!Element<TABLE>} stampEditorTableElement The TABLE element that gets
 *     filled out with the editable cells.
 * @private
 */
stamp.Editor.prototype.makeStampEditorDom = function(stampEditorTableElement) {
  var domTable = goog.editor.Table.createDomTable(
      document,
      this.MIN_COLUMN_COUNT,
      this.MIN_ROW_COUNT,
      { 'borderWidth': 1, 'borderColor': 'white' });
  var tableStyle = {
    'borderCollpase': 'collapse',
    'borderSpacing': '0px',
    'borderStyle': 'solid'
  };

  goog.style.setStyle(domTable, tableStyle);
  var tableCells =
      domTable.getElementsByTagName(goog.dom.TagName.TD);
  this.initCells_(tableCells);
  goog.dom.appendChild(stampEditorTableElement, domTable);
  this.stampEditorTable_ = new goog.editor.Table(domTable);
}

/**
 * Initialize a list of cells to the "alive" state: sets the is-alive
 * attribute and the enclosed image element.  Fix up the various attributes
 * that goog.editor.Table sets on cells.
 * @param {!Array<Element>} cells The array of cells to initialize.
 * @private
 */
stamp.Editor.prototype.initCells_ = function(cells) {
  var cellStyle = {
    'padding': '0px'
  };
  for (var i = 0; i < cells.length; ++i) {
    var cell = cells[i];
    // The goog.editor.Table functions set the cell widths to 60px.
    cell.style.removeProperty('width');
    goog.style.setStyle(cell, cellStyle);
    this.setCellIsAlive(cell, false);
    goog.events.listen(cell, goog.events.EventType.CLICK,
        this.toggleCellState_, false, this);
  }
}

/**
 * Inspect the encoded stamp string and make sure it's valid.  Add things like
 * newline characters when necessary.  |stampStringIn| is assumed to have
 * length > 0.
 * @param {!string} stampStringIn The input stamp string.  Must have length > 0.
 * @return {!string} The santized version of the input string.
 * @private
 */
stamp.Editor.prototype.sanitizeStampString_ = function(stampStringIn) {
  var stampString = stampStringIn;
  if (stampString[stampString.length - 1] !=
      stamp.Editor.StringEncoding_.END_OF_ROW) {
    stampString += stamp.Editor.StringEncoding_.END_OF_ROW;
  }
  return stampString;
}

/**
 * Compute a stamp size from an encoded stamp string.  Stamps are assumed to be
 * rectangular.  The width is the length in characters of the first line in
 * the stamp string.  The height is the number of lines.
 * @param {!string} stampString The encoded stamp string.  Must have length > 0.
 * @return {!Object} An object containing width and height.
 * @private
 */
stamp.Editor.prototype.getSizeFromStampString_ = function(stampString) {
  var size = {width: 0, height: 0};
  var eorPos = stampString.indexOf(stamp.Editor.StringEncoding_.END_OF_ROW);
  if (eorPos == -1) {
    // The stamp string is a single row.
    size.width = stampString.length;
    size.height = 1;
  } else {
    size.width = eorPos;
    // Count up the number of rows in the encoded string.
    var rowCount = 0;
    do {
      ++rowCount;
      eorPos = stampString.indexOf(stamp.Editor.StringEncoding_.END_OF_ROW,
                                   eorPos + 1);
    } while (eorPos != -1);
    size.height = rowCount;
  }
  return size;
}

/**
 * Return the current stamp expressed as a string.  The format loosely follows
 * the .LIF 1.05 "spec", where rows are delineated by a \n, a live cell is
 * represented by a '*' and a dead one by a '.'.
 */
stamp.Editor.prototype.getStampAsString = function() {
  var stampString = '';
  var rowCount = this.rowCount();
  for (var rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
    var row = this.stampEditorTable_.rows[rowIndex];
    for (var colIndex = 0; colIndex < row.columns.length; ++colIndex) {
      var cell = row.columns[colIndex];
      if (this.cellIsAlive(cell.element)) {
        stampString += stamp.Editor.StringEncoding_.LIVE_CELL;
      } else {
        stampString += stamp.Editor.StringEncoding_.DEAD_CELL;
      }
    }
    stampString += stamp.Editor.StringEncoding_.END_OF_ROW;
  }
  return stampString;
}

/**
 * Sets the current stamp baes on the stamp encoding in |stampString|.  The
 * format loosely follows the .LIF 1.05 "spec", where rows are delineated by a
 * \n, a live cell is represented by a '*' and a dead one by a '.'.
 * @param {!string} stampString The encoded stamp string.
 */
stamp.Editor.prototype.setStampFromString = function(stampStringIn) {
  if (stampStringIn.length == 0)
    return;  // Error?
  var stampString = this.sanitizeStampString_(stampStringIn);
  var newSize = this.getSizeFromStampString_(stampString);
  this.resize(newSize.width, newSize.height);

  // Set all the cells to "dead".
  var tableCells =
      this.stampEditorTable_.element.getElementsByTagName(goog.dom.TagName.TD);
  this.initCells_(tableCells);

  // Parse the stamp string and set cell states.
  var rowIndex = 0;
  var columnIndex = 0;
  for (var i = 0; i < stampString.length; ++i) {
    var cell = this.domCellAt(rowIndex, columnIndex);
    switch (stampString.charAt(i)) {
    case stamp.Editor.StringEncoding_.DEAD_CELL:
      this.setCellIsAlive(cell, false);
      ++columnIndex;
      break;
    case stamp.Editor.StringEncoding_.LIVE_CELL:
      this.setCellIsAlive(cell, true);
      ++columnIndex;
      break;
    case stamp.Editor.StringEncoding_.END_OF_ROW:
      ++rowIndex;
      columnIndex = 0;
      break;
    default:
      // Invalid character, set the cell to "dead".
      this.setCellIsAlive(cell, false);
      ++columnIndex;
      break;
    }
  }
}

/**
 * Return the first TABLE cell element that contains |target|.  Return null
 * if there is no such enclosing element.
 * @return {?Element} The DOM element (a TD) that contains |target|.
 */
stamp.Editor.prototype.enclosingTargetForElement = function(target) {
  // The cell is the enclosing TD element.
  var domCell = goog.dom.getAncestor(target, function(node) {
      return node.tagName && node.tagName.toUpperCase() == goog.dom.TagName.TD;
    });
  return domCell;
}

/**
 * Respond to a CLICK event in a table cell by toggling its state.
 * @param {!goog.events.Event} clickEvent The CLICK event that triggered this
 *     handler.
 * @private
 */
stamp.Editor.prototype.toggleCellState_ = function(clickEvent) {
  var cell = this.enclosingTargetForElement(clickEvent.target);
  if (!cell)
    return;
  // TODO(dspringer): throw an error or assert if no enclosing TD element is
  // found.
  this.setCellIsAlive(cell, !this.cellIsAlive(cell));
}

/**
 * Return the DOM element for the cell at location (|row|, |column|) (this is
 * usually a TD element).
 * @param {number} rowIndex The row index.  This is 0-based.
 * @param {number} columnIndex The column index.  This is 0-based.
 * @return {?Element} The TD element representing the cell.  If no cell exists
 *     then return null.
 */
stamp.Editor.prototype.domCellAt = function(rowIndex, columnIndex) {
  if (rowIndex < 0 || rowIndex >= this.stampEditorTable_.rows.length)
    return null;
  var row = this.stampEditorTable_.rows[rowIndex];
  if (columnIndex < 0 || columnIndex >= row.columns.length)
    return null;
  return row.columns[columnIndex].element;
}

/**
 * Resize the table of cells to contain |width| columns and |height| rows. A
 * 0 value for either dimension leaves that dimension unchanged.  Both
 * dimensions are clamped to the minumum and maximum row/column counts.
 * @param {!number} width The new width, must be >= 0.
 * @param {!number} height The new height, must be >= 0.
 */
stamp.Editor.prototype.resize = function(width, height) {
  if (width < this.MIN_COLUMN_COUNT) {
    width = this.MIN_COLUMN_COUNT;
  }
  if (height < this.MIN_ROW_COUNT) {
    height = this.MIN_ROW_COUNT;
  }
  var currentWidth = this.columnCount();
  if (width > 0 && width != currentWidth) {
    if (currentWidth < width) {
      for (var col = 0; col < width - currentWidth; ++col) {
        this.appendColumn();
      }
    } else {
      for (var col = 0; col < currentWidth - width; ++col) {
        this.removeLastColumn();
      }
    }
  }
  var currentHeight = this.rowCount();
  if (height > 0 && height != currentHeight) {
    if (currentHeight < height) {
      for (var row = 0; row < height - currentHeight; ++row) {
        this.appendRow();
      }
    } else {
      for (var row = 0; row < currentHeight - height; ++row) {
        this.removeLastRow();
      }
    }
  }
}

/**
 * Add a column at the right-end of the editor table.
 */
stamp.Editor.prototype.appendColumn = function() {
  var newCells = this.stampEditorTable_.insertColumn();
  this.initCells_(newCells);
}

/**
 * Remove the last column of editor table cells.  If the number of columns is
 * already at the minumum, do nothing.
 */
stamp.Editor.prototype.removeLastColumn = function() {
  var columnCount = this.columnCount();
  if (columnCount <= this.MIN_COLUMN_COUNT) {
    return;
  }
  // Unhook all the listeners that have been attached to the cells in the
  // last column, then remove the column.
  for (var i = 0; i < this.stampEditorTable_.rows.length; ++i) {
    var row = this.stampEditorTable_.rows[i];
    var cell = row.columns[columnCount - 1];
    goog.events.removeAll(cell);
  }
  this.stampEditorTable_.removeColumn(columnCount - 1);
}

/**
 * Return the number of columns in the stamp editor table.  This assumes that
 * there are no merged cells in row[0], and that the number of cells in all
 * rows is the same as the length of row[0].
 * @return {int} The number of columns.
 */
stamp.Editor.prototype.columnCount = function() {
  if (!this.stampEditorTable_)
    return 0;
  if (!this.stampEditorTable_.rows)
    return 0;
  if (!this.stampEditorTable_.rows[0].columns)
    return 0;
  return this.stampEditorTable_.rows[0].columns.length;
}

/**
 * Add a row at the bottom of the editor table.
 */
stamp.Editor.prototype.appendRow = function() {
  var newTableRow = this.stampEditorTable_.insertRow();
  this.initCells_(goog.editor.Table.getChildCellElements(newTableRow));
}

/**
 * Remove the last row of editor table cells.  If the number of rows is already
 * at the minumum, do nothing.
 */
stamp.Editor.prototype.removeLastRow = function() {
  var rowCount = this.rowCount();
  if (rowCount <= this.MIN_ROW_COUNT) {
    return;
  }
  // Unhook all the listeners that have been attached to the cells in the
  // last row, then remove the row.
  var lastRow = this.stampEditorTable_.rows[rowCount - 1];
  for (var i = 0; i < lastRow.columns.length; ++i) {
    var cell = lastRow.columns[i];
    goog.events.removeAll(cell);
  }
  this.stampEditorTable_.removeRow(rowCount - 1);
}

/**
 * Return the number of rows in the stamp editor table.  Assumes that there are
 * no merged cells in any columns.
 * @return {int} The number of rows.
 */
stamp.Editor.prototype.rowCount = function() {
  if (!this.stampEditorTable_)
    return 0;
  if (!this.stampEditorTable_.rows)
    return 0;
  return this.stampEditorTable_.rows.length;
}

/**
 * Accessor for the is-alive state of a cell.
 * @param {!Element} domCell The DOM element representing the target cell.
 * @return {bool} The is-alive state of |cell|.
 */
stamp.Editor.prototype.cellIsAlive = function(domCell) {
  isAlive = domCell.getAttribute(stamp.Editor.CellAttributes_.IS_ALIVE);
  return isAlive != 'false';
}

/**
 * Change the is-alive state of a cell to |isAlive|.  The appearance of the cell
 * is also changed to match the new state.
 * @param {!Element} domCell The DOM element representing the target cell.
 * @param {bool} isAlive The new is-alive state of the cell.
 */
stamp.Editor.prototype.setCellIsAlive = function(domCell, isAlive) {
  domCell.setAttribute(stamp.Editor.CellAttributes_.IS_ALIVE, isAlive);
  var cellImg = isAlive ? 'img/live_cell.png' : 'img/dead_cell.png';
  domCell.innerHTML = '<img src="' +
                      cellImg +
                      '" alt="Click to change state." />';
}
