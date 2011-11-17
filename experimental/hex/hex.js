// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file
 * The JavaScript user interface of the Hex game.  Maintains the
 * basic board state (red,blue,clear) and communicates with the nexe
 * AI program.
 */

// Provide the HexGame namespace
HexGame = {};

HexGame.LEFT_OFFSET = 100;
HexGame.TOP_OFFSET = 50;
HexGame.HEX_SIZE = 50;
HexGame.HALF_HEX_SIZE = HexGame.HEX_SIZE / 2;
HexGame.Cos60 = 0.50;
HexGame.Sin60 = 0.866025403784439;
HexGame.WHITE = 'rgb(255,255,255)';
var X_DELTA = HexGame.HEX_SIZE * HexGame.Cos60;
var Y_DELTA = HexGame.HEX_SIZE * HexGame.Sin60;
var Y_HEX_SIZE = 2 * Y_DELTA;
var X_HEX_SIZE = 2 * X_DELTA + HexGame.HALF_HEX_SIZE;

var canvas;
var canvasBk;
var hexTurnColor;
var ctx;
var ctxBk;
var x = 75;
var y = 50;
var WIDTH = 900;
var HEIGHT = 800;

var maxRows = 8;  // how many 'rows' of hexes
var redColor = 'rgb(255,182,193)';

var whoseTurn = 'R'; // turn in 'B' or 'R'
var gameWinner = ''; // set when we get a winner
var turnNumber = 1;
var lastUserColumn = 0;
var lastUserRow = 0;

// global variable for the board (2D array of hexagons).
// theBoard also maintains state information about what is
// currently happening / expected in the UI.
var theBoard;

function handleMessage(message_event) {
  console.log('handleMessage RECVD[' + message_event.data + ']');
 
  var result = message_event.data;
  if (result == "INVALIDMOVE") {
    alert('The last move ' + lastUserColumn + ':' + lastUserRow + ' was NOT valid');
    hexTurnColor = Board.HexType.RED;
    whoseTurn = 'R';
  } else if (result == "COMPUTERWINS") {
    gameWinner = 'The computer won';
    alert('Sorry, the Computer won the game!');
    updateHtmlField('Turn', gameWinner);
  } else if (result == "USERWINS") {
    gameWinner = 'The user won';
    alert('Congratulations, you won the game!');
    updateHtmlField('Turn', gameWinner);
  } else if (result.indexOf("COMPUTERMOVE:") != -1) {
    console.log('RECIEVED COMPUTER MOVE ' + result + ' whoseTurn=' +
                whoseTurn + ' hexTurnColor=' + hexTurnColor);
    colon_char = result.indexOf(":");
    result = result.substring(colon_char+1);
    var moveArray = result.split(',');
    var col = parseInt(moveArray[0]);
    var row = parseInt(moveArray[1]);

    var colorAtHex = theBoard.getHex(col, row);
    if (colorAtHex == Board.HexType.RED) {
      alert('ERROR: Computer is trying to go to ' + col + ':' + row + ' which is already ' + colorAtHex);
      return;
    }
    if (colorAtHex == Board.HexType.BLUE) {
      // Sometimes happens when computer wins...because final move is
      // sent to make sure that it does get drawn
      return;
    }

    console.log('Setting hex ' + col + ':' + row + ' to ' + hexTurnColor);
    theBoard.setHex(col, row, hexTurnColor);
    console.log('SetHex, whoseTurn was ' + whoseTurn);
    if (hexTurnColor == Board.HexType.RED) {
      hexTurnColor = Board.HexType.BLUE;
      whoseTurn = 'B';
    } else {
      hexTurnColor = Board.HexType.RED;
      whoseTurn = 'R';
      turnNumber++;  //after a blue move, bump turn number
    }
    console.log('whoseTurn now is ' + whoseTurn);
    theBoard.initDraw(ctxBk);
  } else {
    console.log('DID NOT HANDLE ' + result);
  }
}

function clearContext(theContext) {
  theContext.beginPath();
  theContext.clearRect(0, 0, canvas.width, canvas.height);
  theContext.closePath();
}

// Convert an xCoord to a column
function xToColumn(xCoord) {
  // TODO: handle the 'pointy parts' of a hex
  var column = Math.floor((xCoord - HexGame.LEFT_OFFSET) / X_HEX_SIZE);
  return column;
}

// Convert a column, yCoord to a row
function yToRow(column, yCoord) {
  // TODO: handle the 'pointy parts' of a hex
  var columnTop = yCoord - 30;
  if (column % 2 == 1) {
    columnTop -= Y_DELTA;
  }
  var row = Math.floor(columnTop / Y_HEX_SIZE);
  var rowFloat = columnTop / Y_HEX_SIZE;

  var columnWithMaxRows = Math.floor(theBoard.columns_ / 2) + 1; // column with max number of rows
  // var adjusted column for layout
  var columnsFromCenter = Math.floor(Math.abs(columnWithMaxRows - column));
  console.log('columnsFromCenter='+columnsFromCenter);
  var numRows = Math.abs(columnWithMaxRows - columnsFromCenter);
  var firstRow = Math.floor((theBoard.rows_ - numRows) / 2);
  if (column%2==1) { firstRow--; }

  var adjusted_row = row - firstRow - 1;
  return adjusted_row;
}

// note: Uses ctx.fillColor to fill
function drawHex(ctx, column, row, logicalColumn, logicalRow, isLastRow, color) {

  ctx.fillStyle = color;

  var x = HexGame.LEFT_OFFSET + X_HEX_SIZE * column;
  var y = HexGame.TOP_OFFSET + Y_HEX_SIZE * row;
  if (column % 2 == 1) y += Y_DELTA;
  y += HexGame.HALF_HEX_SIZE;
  ctx.beginPath();
  ctx.moveTo(x, y);
  var lineThickness = ctx.lineThickness;
  var lineWidth = ctx.lineWidth;
  x += X_DELTA;
  y += Y_DELTA;
  ctx.lineTo(x, y);
  x += HexGame.HEX_SIZE;
  ctx.lineTo(x, y);
  x += X_DELTA;
  y -= Y_DELTA;
  ctx.lineTo(x, y);
  x -= X_DELTA;
  y -= Y_DELTA;
  ctx.lineTo(x, y);
  x -= HexGame.HEX_SIZE;
  ctx.lineTo(x, y);
  x -= X_DELTA;
  y += Y_DELTA;
  ctx.lineTo(x, y);
  ctx.stroke();
  ctx.fill(); // fill hex with the color set before calling HexGame (red,blue)
  ctx.closePath();

/*** Experimental code to draw circles instead of a filled hex -- use this if
     you comment out the ctx.fill() call a few lines above.

  if (color != HexGame.WHITE) {
    var midx = HexGame.LEFT_OFFSET + X_HEX_SIZE * column + X_HEX_SIZE/1.4;
    var midy = HexGame.TOP_OFFSET + Y_HEX_SIZE * row + Y_HEX_SIZE/3;
    if (column % 2 == 1) midy += Y_DELTA;
    ctx.beginPath();
    ctx.arc( midx, midy, X_HEX_SIZE/3, Y_HEX_SIZE/3, true);
    ctx.closePath();
    ctx.fill();
  }
***/

  ctx.lineThickness = lineThickness;
 
  var showAlerts = false;
  // if this is an edge...add an extra line   topLeft,bottomRight = blue; topRight,bottomLeft = red
  // FIXME -- hardcoded to 11 being middle, etc.
  var strokeStyle = ctx.strokeStyle;
  if (logicalRow==1 && logicalColumn >= 1 && logicalColumn < 11 ) {
    if(showAlerts) {alert('top left');}
    // then the two 'top left' segments should be blue
    var x = HexGame.LEFT_OFFSET + X_HEX_SIZE * column;
    var y = HexGame.TOP_OFFSET + Y_HEX_SIZE * row;
    if (column % 2 == 1) y += Y_DELTA;
    y += HexGame.HALF_HEX_SIZE;
    ctx.strokeStyle = 'rgb(0,0,255)';
    ctx.lineWidth = 12;
    ctx.beginPath();
    ctx.moveTo(x, y);
    x += X_DELTA;
    y -= Y_DELTA;
    ctx.lineTo(x, y);
    x += HexGame.HEX_SIZE;
    ctx.lineTo(x, y);
    ctx.stroke();
    ctx.closePath();
    if(showAlerts) {alert('done TL');}
  }
  if (logicalRow==1 && logicalColumn >= 11 && logicalColumn <= 21) {
    // then the two 'top right' segments should be red
    if(showAlerts) {alert('top right');}
    var x = HexGame.LEFT_OFFSET + X_HEX_SIZE * column + X_DELTA;
    var y = HexGame.TOP_OFFSET + Y_HEX_SIZE * row - Y_DELTA;
    if (column % 2 == 1) y += Y_DELTA;
    y += HexGame.HALF_HEX_SIZE;
    ctx.strokeStyle = 'rgb(255,0,0)';
    ctx.lineWidth = 12;
    ctx.beginPath();
    ctx.moveTo(x, y);
    x += HexGame.HEX_SIZE;
    ctx.lineTo(x, y);
    x += X_DELTA;
    y += Y_DELTA
    ctx.lineTo(x, y);
    ctx.stroke();
    ctx.closePath();
    if(showAlerts) {alert('done TR');}
  }
  if (logicalRow == logicalColumn) {
    // then it is a BOTTOM hex
      //then the two 'bottom left' hexes need to be RED
      if(showAlerts) {alert('bottom left');}
      var x = HexGame.LEFT_OFFSET + X_HEX_SIZE * column;
      var y = HexGame.TOP_OFFSET + Y_HEX_SIZE * row;
      if (column % 2 == 1) y += Y_DELTA;
      y += HexGame.HALF_HEX_SIZE;
      ctx.strokeStyle = 'rgb(255,0,0)';
      ctx.lineWidth = 12;
      ctx.beginPath();
      ctx.moveTo(x, y);
      x += X_DELTA;
      y += Y_DELTA;
      ctx.lineTo(x, y);
      x += HexGame.HEX_SIZE;
      ctx.lineTo(x, y);
      ctx.stroke();
      ctx.closePath();
    if(showAlerts) {alert('done BL');}
  }
  if (logicalRow + logicalColumn == 22 && logicalColumn > 11 && isLastRow) {
      // then the two 'bottom right' segments should be BLUE
      if(showAlerts) {alert('bottom right' + ' isLastRow=' + isLastRow + ' logicalRow=' + logicalRow + ' logicalColumn=' + logicalColumn);}
      var x = HexGame.LEFT_OFFSET + X_HEX_SIZE * (column-1) + X_DELTA * 2 + HexGame.HEX_SIZE;
      var y = HexGame.TOP_OFFSET + Y_HEX_SIZE * row + Y_DELTA; // + HexGame.Y_HEX_SIZE;
      if (column % 2 == 1) y += Y_DELTA;
      y += HexGame.HALF_HEX_SIZE;
      ctx.strokeStyle = 'rgb(0,0,255)';
      ctx.lineWidth = 12; // WHY 16 and not 12
      ctx.beginPath();
      ctx.moveTo(x, y);
      x += HexGame.HEX_SIZE;
      ctx.lineTo(x, y);
      x += X_DELTA;
      y -= Y_DELTA
      ctx.lineTo(x, y);
      ctx.stroke();
      ctx.closePath();
      if(showAlerts) {alert('done bottom right');}
  }
  if (logicalColumn == 11 && logicalRow == 1) {
    // ONE more little segment on top
    if(showAlerts) {alert('top small');}
    var x = HexGame.LEFT_OFFSET + X_HEX_SIZE * column;
    var y = HexGame.TOP_OFFSET + Y_HEX_SIZE * row;
    if (column % 2 == 1) y += Y_DELTA;
    y += HexGame.HALF_HEX_SIZE;
    ctx.strokeStyle = 'rgb(0,0,255)';
    ctx.lineWidth = 12;
    ctx.beginPath();
    ctx.moveTo(x, y);
    x += X_DELTA;
    y -= Y_DELTA;
    ctx.lineTo(x, y);
    ctx.stroke();
    ctx.closePath();
    if(showAlerts) {alert('done top small');}
  }
  if (logicalColumn == 11 && logicalRow == 11) {
    if(showAlerts) {alert('11 && 11');}
    // ONE more little segment bottomright
    var x = HexGame.LEFT_OFFSET + X_HEX_SIZE * column + X_HEX_SIZE;
    var y = HexGame.TOP_OFFSET + Y_HEX_SIZE * (row) + Y_DELTA * 2;
    y += HexGame.HALF_HEX_SIZE;
    ctx.strokeStyle = 'rgb(0,0,255)';
    ctx.lineWidth = 12;
    ctx.beginPath();
    ctx.moveTo(x, y);
    x += X_DELTA;
    y -= Y_DELTA;
    ctx.lineTo(x, y);
    ctx.stroke();
    ctx.closePath();
    if(showAlerts) {alert('done 11x11');}
  }
  ctx.lineThickness = lineThickness;
  ctx.lineWidth = lineWidth;
  ctx.strokeStyle = strokeStyle;
}

/**
 *  Constructor for the Board: columns and rows of hexagons.
 *  hexColumn_ is a 2d array of the type of hexagon at each location.
 */
function Board(columns, rows) {
  console.log('Creating Board ' + columns + ' x ' + rows);
  this.columns_ = columns;
  this.rows_ = rows;
  this.hexColumn_ = new Array();
  for (var i = 1; i <= columns; ++i) {
    this.hexColumn_[i] = new Array();
    for (var j = 1; j <= rows; ++j) {
      this.hexColumn_[i][j] = Board.HexType.CLEAR;
    }
  }
  this.turnState_ = 0;
}

/**
 * The values used for Board status to hex types.
 */
Board.HexType = {
  CLEAR: 0,
  RED: 1,
  BLUE: 2
};

Board.TurnState = {
  MOVING: 0,
  ATTACKING: 1,
};


Board.prototype.setHex = function(column, row, hexType) {
  console.log('setHex column=' + column + ' row=' + row + ' hexType=' + hexType);
  this.hexColumn_[column][row] = hexType;
};
Board.prototype.getHex = function(column, row) {
  if (this.hexColumn_[column] == undefined ||
      this.hexColumn_[column][row] == undefined) {
    return Board.HexType.CLEAR;
  }
  var value = this.hexColumn_[column][row];
  if (value == undefined || value == null)
    return Board.HexType.CLEAR;
  return this.hexColumn_[column][row];
};
Board.prototype.postMsg = function() {
  if (!naclModule) {
    console.log('No NaCl module');
    return;
  }
  var stringData = 'POST_BOARD ' + this.columns_ +
                   ' ' + this.rows_ + ' ';
  for (i = 0; i < this.columns_; ++i) {
    for (j = 0; j < this.rows_; ++j) {
      var hexType = this.hexColumn_[i][j];
      stringData = stringData + hexType + ' ';
    }
  }
  console.log('naclModule=' + naclModule + ' stringData: ' + stringData);
  var result = naclModule.postMessage(stringData);
  console.log('result = ' + result);
};

//
// If rows = 21, then max is 11 (21/2+1).
//  For column = 0, firstRow is 5, lastRow is 5
//  For column = 1, firstRow is 4, lastRow is 5
//  For column = 2, firstRow is 4, lastRow is 6
//  For column = 11, firstRow is 0, lastRow is 11
//  For column = 12, firstRow is 0, lastRow is 10
//
Board.prototype.initDraw = function(ctx) {
  this.turnState = Board.TurnState.MOVING;
  console.log('this.columns_: ' + this.columns_ + ' this.rows_:' + this.rows_);
  var drawThisHex;
  var fillColor;

  ctx.globalCompositeOperation = 'source-over';
  var columnWithMaxRows = Math.floor(this.columns_ / 2) + 1; // column with max number of rows
  for (i = 0; i <= this.columns_; ++i) {

    var columnsFromCenter = Math.floor(Math.abs(columnWithMaxRows - i));
    var numRows = Math.abs(columnWithMaxRows - columnsFromCenter);
    var firstRow = Math.floor((this.rows_ - numRows) / 2);
    if (i%2 == 0) {
      firstRow++;
    }
    var lastRow = firstRow + numRows;

    for (j = 0; j <= this.rows_; ++j) {
      fillColor = 'rgb(255,255,255)';
      drawThisHex = false;
      if (j > firstRow && j <= lastRow) {
        drawThisHex = true;
      }
      if (drawThisHex == false) {
        fillColor = 'rgb(255,0,0)';
      } else {
        var hexRow = j - firstRow;
        var hexColumn = i + 1;
        var adjCol = i;
        var adjRow = hexRow;
        // adjCol and adjRow are 1-based column-row from the game/user POV
        var colorAtHex = this.getHex(adjCol, adjRow);
        if (colorAtHex == Board.HexType.RED) {
          ctx.strokeStyle = 'rgb(255,0,0)';
          fillColor = 'rgb(255,0,0)';
        } else if (colorAtHex == Board.HexType.BLUE) {
          ctx.strokeStyle = 'rgb(0,0,255)';   // FIXME, use a constant for EACH color
          fillColor = 'rgb(0,0,255)';
        } else {
          // if no one has the hex, then it's clear
          ctx.strokeStyle = 'rgb(0,0,0)';
        }
      }
      // ctx.fillStyle = fillColor;
      if(drawThisHex) {
        drawHex(ctx, i, j, adjCol, adjRow, j == lastRow, fillColor);
      }
    }
  }

  console.log('WhoseTurn=' + whoseTurn + ' turnNumber=' + turnNumber);
  if (gameWinner != '') {
    updateHtmlField('Turn', gameWinner);
  } else {
    updateHtmlField('Turn', 'TURN: ' + whoseTurn);
  }
  updateHtmlField('TurnNumber', 'Turn: Number: ' + turnNumber);
  updateHtmlField('status_field', 'UPDATED');
};

function InitHexGame() {
  canvas = document.getElementById('canvas');
  canvasBk = document.getElementById('canvasBk');

  ctx = canvas.getContext('2d');
  ctxBk = canvasBk.getContext('2d');

  theBoard = new Board(21, 11);
  theBoard.initDraw(ctxBk);

  hexTurnColor = Board.HexType.RED;
}

function hexMouseDownHandler(e) {
  var x = e.offsetX;
  var y = e.offsetY;
  var columnClicked = xToColumn(x);
  var rowClicked = yToRow(columnClicked, y);
  console.log(' hexMouseDownHandler: ' + columnClicked + ':' + rowClicked);
  if (columnClicked <= 0 || rowClicked <= 0) {
    return;
  }
  if (gameWinner != '') {
    alert('Sorry, the game is over -- ' + gameWinner);
    return;
  }
  if (whoseTurn == 'B') {
    alert("Sorry, it's the computer's turn -- be patient!");
    return;
  }

  // see if the hex has a color
  var colorAtHex = theBoard.getHex(columnClicked, rowClicked);
  console.log('colorAtHex = ' + colorAtHex + ' RED=' + Board.HexType.RED + ' CLEAR=' + Board.HexType.CLEAR);
  if (colorAtHex == Board.HexType.CLEAR) {
    // then set it to the new color.
    theBoard.setHex(columnClicked, rowClicked, hexTurnColor);
    var stringData = 'USERMOVE ' + columnClicked + ':' + rowClicked;
    console.log('setting hex ' + columnClicked + ':' + rowClicked + ' to ' + hexTurnColor);
    console.log(' In hexMouseDownHandler, hexTurnColor=' + hexTurnColor + ' whoseTurn=' + whoseTurn);
    if (hexTurnColor == Board.HexType.RED) {
      hexTurnColor = Board.HexType.BLUE;
      whoseTurn = 'B';
    } else {
      hexTurnColor = Board.HexType.RED;
      turnNumber++;  //after a blue move, bump turn number
      whoseTurn = 'R';
    }
    theBoard.initDraw(ctxBk);

    lastUserColumn = columnClicked;
    lastUserRow = rowClicked;
    // BEFORE we call postMessage, we need to set our own state!
    naclModule.postMessage(stringData);

  } else {
    alert('The hex was NOT clear');
  }
}

// Remove |element| from |array|
function removeElementFromArray(element, array) {
  var index = array.indexOf(element);
  if (index != -1) {
    array.splice(index, 1); // Remove it if we found it
  }
}

InitHexGame();
canvas.onmousedown = hexMouseDownHandler;
