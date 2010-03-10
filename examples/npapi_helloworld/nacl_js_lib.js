// Copyright 2010 Google, Inc.
/** @fileoverview Contains a class to check for the successful loading of a nexe
 * module.
 */


/**
 * Class to check the successful loading of a nexe module.
 * @param {node} module The nexe module dom element.
 * @param {node} statusfield The element to display the nexe module load status.
 * @param {number} The number of attempts to load the nexe module.
 */
function NaclLib(module, statusfield, num_retries) {
   this.modules_ = module;
   this.statusfield_ = statusfield;
   this.status_ = "WAIT";
   this.message_ = "";
   this.handler_ = null;
   this.retries_ = num_retries;
};

/**
 * Returns the status of the nexe model loading.
 * @return {string} The status of module loading.
 */
NaclLib.prototype.getStatus = function() {
  return this.status_;
};

/**
 * Returns the message describing the result of nexe module loading.
 * @return {string} The message string.
 */
NaclLib.prototype.getMessage = function() {
  return this.message_;
};

/**
 * Clears the internal handler of the nexe module loading attempts.
 */
NaclLib.prototype.cleanUp = function() {
  if (this.handler_) {
     clearInterval(this._handler);
     this.handler_ = null;
  }
};

/**
 * Displays the status and descriptive message of the nexe module loading
 * result.
 */
NaclLib.prototype.setStatus = function() {
  this.statusfield_.innerHTML =
      this.status_ + ": " + this.message_;
};

/**
 * Sets the displayed status as wait status.
 */
NaclLib.prototype.setStatusWait = function(message) {
  this.status_ = "WAIT";
  this.message_ = "" + this.retries_ + " " + message;
  this.setStatus()
};

/**
 * Sets the displayed status as error status.
 */
NaclLib.prototype.setStatusError = function(message) {
  this.status_ = "ERROR";
  this.message_ = message;
  this.setStatus()
};

/**
 * Sets the displayed status as success status.
 */
NaclLib.prototype.setStatusSuccess = function(message) {
  this.status_ = "SUCCESS";
  this.message_ = message;
  this.setStatus()
};

/**
 * Retrieve the number of nexe modules that are ready.
 * @param {array} modules The array of modules to check if ready.
 * @return {number} The number of modules that are ready.
 */
NaclLib.prototype.numModulesReady = function(modules) {
  var count = 0;
  for (var i = 0; i < modules.length; i++) {
    if (modules[i].__moduleReady == 1) {
      count += 1;
    }
  }
  return count;
};

/**
 * Indicates whether there are problems with the nexe modules.
 * @param {array} modules The nexe modules to check for problems.
 * @return {number} Returns 0 if there are no problems.
 */
NaclLib.prototype.areTherePluginProblems = function(modules) {
  for (var i = 0; i < modules.length; i++) {
    if (modules[i].__moduleReady == undefined) return 1;
  }
  return 0;
};

/**
 * Attempts to load the nexe modules and report the status of the load
 * operations.
 */
NaclLib.prototype.checkModuleReadiness = function() {
  // work around bug that does not disable the handler
  if (!this.handler_)
    return;

  if (this.retries_ == 0) {
    this.cleanUp();
    this.setStatusError("The Native Client modules are loading too slowly");
    return;
  }

  this.retries_ -= 1;
  var num_ready = this.numModulesReady(this.modules_);

  if (this.modules_.length == num_ready) {
    if (this.wait) {
      var result = this.wait();
      if (result) {
        this.setStatusWait(result);
        return;
      }
    }

    this.cleanUp();

    var result;
    try {
      result = this.test();
    } catch(e) {
      this.setStatusError(e);
      return;
    }

    if (result == "") {
      this.setStatusSuccess("");
    } else {
      this.setStatusError(result);
    }

    return;
  }

  this.setStatusWait("Loaded " + num_ready + "/" + this.modules_.length + " modules");

  if (this.areTherePluginProblems(this.modules_)) {
    this.cleanUp();
    this.setStatusError("The Native Client plugin was unable to load");
    return;
  }
};


/**
 * Workaround for JS inconsistent scoping behavior.
 */
function wrapperForCheckModuleReadiness(that) {
  that.checkModuleReadiness();
}

/**
 * Attempts to load the nexe modules and run tests.
 */
NaclLib.prototype.waitForModulesAndRunTests = function() {
  // Avoid regsitering two handlers
  if (!this.handler_) {
    this.handler_ = setInterval(wrapperForCheckModuleReadiness, 100, this);
  }
};
