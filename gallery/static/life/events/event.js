// Copyright 2011 (c) The Native Client Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @fileoverview Life events.  These are goog.events.Event objects with
 * an added ID property, that identifies which UI element triggered the event
 * in the first place.
 */

goog.provide('uikit.events.Event');

goog.require('goog.events.Event');

/**
 * Constructor for the Event class.  This provides a default ID ('gs_event')
 * if |opt_eventId| is null or undefined.
 * @param {string} type The type of this event.
 * @param {?Object} opt_target The target that triggered this event.
 * @param {string} opt_eventId The id for this event.  If null or undefined,
 *     the new event instance gets the default id 'gs_event'.
 * @constructor
 * @extends {goog.events.Event}
 */
uikit.events.Event = function(type, opt_target, opt_eventId) {
  goog.events.Event.call(this, type, opt_target);

  /**
   * The event id.  Initialized to be |opt_eventId| or the default 'gs_event'.
   * @type {string}
   * @private
   */
  this.eventId_ = opt_eventId || uikit.events.Event.DEFAULT_EVENT_ID;
};
goog.inherits(uikit.events.Event, goog.events.Event);

/**
 * Override of disposeInternal() to dispose of retained objects.
 * @override
 */
uikit.events.Event.prototype.disposeInternal = function() {
  uikit.events.Event.superClass_.disposeInternal.call(this);
  this.eventId_ = null;
};

/**
 * Accessor for the |eventId_| property.
 * @return {string} The id of this event instance, guaranteed not to be null.
 */
uikit.events.Event.prototype.eventId = function() {
  return this.eventId_;
};

/**
 * Mutator for the |eventId_| property.
 * @param {string} eventId The new event id.  If null or undefined, this method
 *     has no effect and silently returns.
 */
uikit.events.Event.prototype.setEventId = function(eventId) {
  if (!eventId)
    return;
  this.eventId_ = eventId;
};

/**
 * The default event id.  This is the id for events created using the default
 * constructor parameter.
 * @type {string}
 */
uikit.events.Event.DEFAULT_EVENT_ID = 'gs_event';
