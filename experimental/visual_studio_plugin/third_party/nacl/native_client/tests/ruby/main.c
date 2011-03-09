/* Copyright 2009 The Native Client Authors.  All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file. */

#include <stdio.h>
#include <string.h>
#include <nacl/nacl_srpc.h>
#include "ruby-1.8.6-p368/ruby.h"

int initialized = 0;

char* INIT_RUBY_SCRIPT =
    "$stdout_default = $stdout \n"
    "class Logger \n"
    "  def initialize() \n"
    "    @strings = [] \n"
    "  end \n"
    "\n"
    "  def write(s) \n"
    "    @strings.push(s) \n"
    "    $stdout_default.write s \n"
    "  end \n"
    "\n"
    "  def get_string() \n"
    "    current = @strings \n"
    "    @strings = [] \n"
    "    return current.join("") \n"
    "  end \n"
    "end \n"
    "\n"
    "$stdout_logger = Logger.new \n"
    "$stdout = $stdout_logger \n";

/*
 *  Evals Ruby expression and returns stdout updates.
 */
NaClSrpcError RubyEval(NaClSrpcChannel *channel,
    NaClSrpcArg **in_args, NaClSrpcArg **out_args) {

  /* TODO(krasin): MAKE IT THREAD-SAFE */
  if (!initialized) {
    ruby_init();
    rb_eval_string(INIT_RUBY_SCRIPT);
    initialized = 1;
  }
  rb_eval_string(in_args[0]->u.sval);
  VALUE result = rb_eval_string("$stdout_logger.get_string()");

  /*
   * Strdup must be used because the SRPC layer frees the string passed to it.
   */
  out_args[0]->u.sval = strdup(StringValuePtr(result));

  return NACL_SRPC_RESULT_OK;
}

/*
 * Export the method as taking no arguments and returning one integer.
 */
NACL_SRPC_METHOD("rubyEval:s:s", RubyEval);
