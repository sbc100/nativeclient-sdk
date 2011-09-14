var COMPILED = !0, goog = goog || {};
goog.global = this;
goog.DEBUG = !0;
goog.LOCALE = "en";
goog.provide = function(a) {
  if(!COMPILED) {
    if(goog.isProvided_(a)) {
      throw Error('Namespace "' + a + '" already declared.');
    }
    delete goog.implicitNamespaces_[a];
    for(var b = a;b = b.substring(0, b.lastIndexOf("."));) {
      if(goog.getObjectByName(b)) {
        break
      }
      goog.implicitNamespaces_[b] = !0
    }
  }
  goog.exportPath_(a)
};
goog.setTestOnly = function(a) {
  if(COMPILED && !goog.DEBUG) {
    throw a = a || "", Error("Importing test-only code into non-debug environment" + a ? ": " + a : ".");
  }
};
if(!COMPILED) {
  goog.isProvided_ = function(a) {
    return!goog.implicitNamespaces_[a] && !!goog.getObjectByName(a)
  }, goog.implicitNamespaces_ = {}
}
goog.exportPath_ = function(a, b, c) {
  a = a.split(".");
  c = c || goog.global;
  !(a[0] in c) && c.execScript && c.execScript("var " + a[0]);
  for(var d;a.length && (d = a.shift());) {
    !a.length && goog.isDef(b) ? c[d] = b : c = c[d] ? c[d] : c[d] = {}
  }
};
goog.getObjectByName = function(a, b) {
  for(var c = a.split("."), d = b || goog.global, e;e = c.shift();) {
    if(goog.isDefAndNotNull(d[e])) {
      d = d[e]
    }else {
      return null
    }
  }
  return d
};
goog.globalize = function(a, b) {
  var c = b || goog.global, d;
  for(d in a) {
    c[d] = a[d]
  }
};
goog.addDependency = function(a, b, c) {
  if(!COMPILED) {
    for(var d, a = a.replace(/\\/g, "/"), e = goog.dependencies_, f = 0;d = b[f];f++) {
      e.nameToPath[d] = a, a in e.pathToNames || (e.pathToNames[a] = {}), e.pathToNames[a][d] = !0
    }
    for(d = 0;b = c[d];d++) {
      a in e.requires || (e.requires[a] = {}), e.requires[a][b] = !0
    }
  }
};
goog.ENABLE_DEBUG_LOADER = !0;
goog.require = function(a) {
  if(!COMPILED && !goog.isProvided_(a)) {
    if(goog.ENABLE_DEBUG_LOADER) {
      var b = goog.getPathFromDeps_(a);
      if(b) {
        goog.included_[b] = !0;
        goog.writeScripts_();
        return
      }
    }
    a = "goog.require could not find: " + a;
    goog.global.console && goog.global.console.error(a);
    throw Error(a);
  }
};
goog.basePath = "";
goog.nullFunction = function() {
};
goog.identityFunction = function(a) {
  return a
};
goog.abstractMethod = function() {
  throw Error("unimplemented abstract method");
};
goog.addSingletonGetter = function(a) {
  a.getInstance = function() {
    return a.instance_ || (a.instance_ = new a)
  }
};
if(!COMPILED && goog.ENABLE_DEBUG_LOADER) {
  goog.included_ = {}, goog.dependencies_ = {pathToNames:{}, nameToPath:{}, requires:{}, visited:{}, written:{}}, goog.inHtmlDocument_ = function() {
    var a = goog.global.document;
    return typeof a != "undefined" && "write" in a
  }, goog.findBasePath_ = function() {
    if(goog.global.CLOSURE_BASE_PATH) {
      goog.basePath = goog.global.CLOSURE_BASE_PATH
    }else {
      if(goog.inHtmlDocument_()) {
        for(var a = goog.global.document.getElementsByTagName("script"), b = a.length - 1;b >= 0;--b) {
          var c = a[b].src, d = c.lastIndexOf("?"), d = d == -1 ? c.length : d;
          if(c.substr(d - 7, 7) == "base.js") {
            goog.basePath = c.substr(0, d - 7);
            break
          }
        }
      }
    }
  }, goog.importScript_ = function(a) {
    var b = goog.global.CLOSURE_IMPORT_SCRIPT || goog.writeScriptTag_;
    !goog.dependencies_.written[a] && b(a) && (goog.dependencies_.written[a] = !0)
  }, goog.writeScriptTag_ = function(a) {
    return goog.inHtmlDocument_() ? (goog.global.document.write('<script type="text/javascript" src="' + a + '"><\/script>'), !0) : !1
  }, goog.writeScripts_ = function() {
    function a(e) {
      if(!(e in d.written)) {
        if(!(e in d.visited) && (d.visited[e] = !0, e in d.requires)) {
          for(var g in d.requires[e]) {
            if(!goog.isProvided_(g)) {
              if(g in d.nameToPath) {
                a(d.nameToPath[g])
              }else {
                throw Error("Undefined nameToPath for " + g);
              }
            }
          }
        }
        e in c || (c[e] = !0, b.push(e))
      }
    }
    var b = [], c = {}, d = goog.dependencies_, e;
    for(e in goog.included_) {
      d.written[e] || a(e)
    }
    for(e = 0;e < b.length;e++) {
      if(b[e]) {
        goog.importScript_(goog.basePath + b[e])
      }else {
        throw Error("Undefined script input");
      }
    }
  }, goog.getPathFromDeps_ = function(a) {
    return a in goog.dependencies_.nameToPath ? goog.dependencies_.nameToPath[a] : null
  }, goog.findBasePath_(), goog.global.CLOSURE_NO_DEPS || goog.importScript_(goog.basePath + "deps.js")
}
goog.typeOf = function(a) {
  var b = typeof a;
  if(b == "object") {
    if(a) {
      if(a instanceof Array) {
        return"array"
      }else {
        if(a instanceof Object) {
          return b
        }
      }
      var c = Object.prototype.toString.call(a);
      if(c == "[object Window]") {
        return"object"
      }
      if(c == "[object Array]" || typeof a.length == "number" && typeof a.splice != "undefined" && typeof a.propertyIsEnumerable != "undefined" && !a.propertyIsEnumerable("splice")) {
        return"array"
      }
      if(c == "[object Function]" || typeof a.call != "undefined" && typeof a.propertyIsEnumerable != "undefined" && !a.propertyIsEnumerable("call")) {
        return"function"
      }
    }else {
      return"null"
    }
  }else {
    if(b == "function" && typeof a.call == "undefined") {
      return"object"
    }
  }
  return b
};
goog.propertyIsEnumerableCustom_ = function(a, b) {
  if(b in a) {
    for(var c in a) {
      if(c == b && Object.prototype.hasOwnProperty.call(a, b)) {
        return!0
      }
    }
  }
  return!1
};
goog.propertyIsEnumerable_ = function(a, b) {
  return a instanceof Object ? Object.prototype.propertyIsEnumerable.call(a, b) : goog.propertyIsEnumerableCustom_(a, b)
};
goog.isDef = function(a) {
  return a !== void 0
};
goog.isNull = function(a) {
  return a === null
};
goog.isDefAndNotNull = function(a) {
  return a != null
};
goog.isArray = function(a) {
  return goog.typeOf(a) == "array"
};
goog.isArrayLike = function(a) {
  var b = goog.typeOf(a);
  return b == "array" || b == "object" && typeof a.length == "number"
};
goog.isDateLike = function(a) {
  return goog.isObject(a) && typeof a.getFullYear == "function"
};
goog.isString = function(a) {
  return typeof a == "string"
};
goog.isBoolean = function(a) {
  return typeof a == "boolean"
};
goog.isNumber = function(a) {
  return typeof a == "number"
};
goog.isFunction = function(a) {
  return goog.typeOf(a) == "function"
};
goog.isObject = function(a) {
  a = goog.typeOf(a);
  return a == "object" || a == "array" || a == "function"
};
goog.getUid = function(a) {
  return a[goog.UID_PROPERTY_] || (a[goog.UID_PROPERTY_] = ++goog.uidCounter_)
};
goog.removeUid = function(a) {
  "removeAttribute" in a && a.removeAttribute(goog.UID_PROPERTY_);
  try {
    delete a[goog.UID_PROPERTY_]
  }catch(b) {
  }
};
goog.UID_PROPERTY_ = "closure_uid_" + Math.floor(Math.random() * 2147483648).toString(36);
goog.uidCounter_ = 0;
goog.getHashCode = goog.getUid;
goog.removeHashCode = goog.removeUid;
goog.cloneObject = function(a) {
  var b = goog.typeOf(a);
  if(b == "object" || b == "array") {
    if(a.clone) {
      return a.clone()
    }
    var b = b == "array" ? [] : {}, c;
    for(c in a) {
      b[c] = goog.cloneObject(a[c])
    }
    return b
  }
  return a
};
goog.bindNative_ = function(a, b, c) {
  return a.call.apply(a.bind, arguments)
};
goog.bindJs_ = function(a, b, c) {
  if(!a) {
    throw Error();
  }
  if(arguments.length > 2) {
    var d = Array.prototype.slice.call(arguments, 2);
    return function() {
      var c = Array.prototype.slice.call(arguments);
      Array.prototype.unshift.apply(c, d);
      return a.apply(b, c)
    }
  }else {
    return function() {
      return a.apply(b, arguments)
    }
  }
};
goog.bind = function(a, b, c) {
  goog.bind = Function.prototype.bind && Function.prototype.bind.toString().indexOf("native code") != -1 ? goog.bindNative_ : goog.bindJs_;
  return goog.bind.apply(null, arguments)
};
goog.partial = function(a, b) {
  var c = Array.prototype.slice.call(arguments, 1);
  return function() {
    var b = Array.prototype.slice.call(arguments);
    b.unshift.apply(b, c);
    return a.apply(this, b)
  }
};
goog.mixin = function(a, b) {
  for(var c in b) {
    a[c] = b[c]
  }
};
goog.now = Date.now || function() {
  return+new Date
};
goog.globalEval = function(a) {
  if(goog.global.execScript) {
    goog.global.execScript(a, "JavaScript")
  }else {
    if(goog.global.eval) {
      if(goog.evalWorksForGlobals_ == null) {
        goog.global.eval("var _et_ = 1;"), typeof goog.global._et_ != "undefined" ? (delete goog.global._et_, goog.evalWorksForGlobals_ = !0) : goog.evalWorksForGlobals_ = !1
      }
      if(goog.evalWorksForGlobals_) {
        goog.global.eval(a)
      }else {
        var b = goog.global.document, c = b.createElement("script");
        c.type = "text/javascript";
        c.defer = !1;
        c.appendChild(b.createTextNode(a));
        b.body.appendChild(c);
        b.body.removeChild(c)
      }
    }else {
      throw Error("goog.globalEval not available");
    }
  }
};
goog.evalWorksForGlobals_ = null;
goog.getCssName = function(a, b) {
  var c = function(a) {
    return goog.cssNameMapping_[a] || a
  }, d;
  d = goog.cssNameMapping_ ? goog.cssNameMappingStyle_ == "BY_WHOLE" ? c : function(a) {
    for(var a = a.split("-"), b = [], d = 0;d < a.length;d++) {
      b.push(c(a[d]))
    }
    return b.join("-")
  } : function(a) {
    return a
  };
  return b ? a + "-" + d(b) : d(a)
};
goog.setCssNameMapping = function(a, b) {
  goog.cssNameMapping_ = a;
  goog.cssNameMappingStyle_ = b
};
goog.getMsg = function(a, b) {
  var c = b || {}, d;
  for(d in c) {
    var e = ("" + c[d]).replace(/\$/g, "$$$$"), a = a.replace(RegExp("\\{\\$" + d + "\\}", "gi"), e)
  }
  return a
};
goog.exportSymbol = function(a, b, c) {
  goog.exportPath_(a, b, c)
};
goog.exportProperty = function(a, b, c) {
  a[b] = c
};
goog.inherits = function(a, b) {
  function c() {
  }
  c.prototype = b.prototype;
  a.superClass_ = b.prototype;
  a.prototype = new c;
  a.prototype.constructor = a
};
goog.base = function(a, b, c) {
  var d = arguments.callee.caller;
  if(d.superClass_) {
    return d.superClass_.constructor.apply(a, Array.prototype.slice.call(arguments, 1))
  }
  for(var e = Array.prototype.slice.call(arguments, 2), f = !1, g = a.constructor;g;g = g.superClass_ && g.superClass_.constructor) {
    if(g.prototype[b] === d) {
      f = !0
    }else {
      if(f) {
        return g.prototype[b].apply(a, e)
      }
    }
  }
  if(a[b] === d) {
    return a.constructor.prototype[b].apply(a, e)
  }else {
    throw Error("goog.base called from a method of one name to a method of a different name");
  }
};
goog.scope = function(a) {
  a.call(goog.global)
};
goog.debug = {};
goog.debug.Error = function(a) {
  this.stack = Error().stack || "";
  if(a) {
    this.message = String(a)
  }
};
goog.inherits(goog.debug.Error, Error);
goog.debug.Error.prototype.name = "CustomError";
goog.string = {};
goog.string.Unicode = {NBSP:"\u00a0"};
goog.string.startsWith = function(a, b) {
  return a.lastIndexOf(b, 0) == 0
};
goog.string.endsWith = function(a, b) {
  var c = a.length - b.length;
  return c >= 0 && a.indexOf(b, c) == c
};
goog.string.caseInsensitiveStartsWith = function(a, b) {
  return goog.string.caseInsensitiveCompare(b, a.substr(0, b.length)) == 0
};
goog.string.caseInsensitiveEndsWith = function(a, b) {
  return goog.string.caseInsensitiveCompare(b, a.substr(a.length - b.length, b.length)) == 0
};
goog.string.subs = function(a, b) {
  for(var c = 1;c < arguments.length;c++) {
    var d = String(arguments[c]).replace(/\$/g, "$$$$"), a = a.replace(/\%s/, d)
  }
  return a
};
goog.string.collapseWhitespace = function(a) {
  return a.replace(/[\s\xa0]+/g, " ").replace(/^\s+|\s+$/g, "")
};
goog.string.isEmpty = function(a) {
  return/^[\s\xa0]*$/.test(a)
};
goog.string.isEmptySafe = function(a) {
  return goog.string.isEmpty(goog.string.makeSafe(a))
};
goog.string.isBreakingWhitespace = function(a) {
  return!/[^\t\n\r ]/.test(a)
};
goog.string.isAlpha = function(a) {
  return!/[^a-zA-Z]/.test(a)
};
goog.string.isNumeric = function(a) {
  return!/[^0-9]/.test(a)
};
goog.string.isAlphaNumeric = function(a) {
  return!/[^a-zA-Z0-9]/.test(a)
};
goog.string.isSpace = function(a) {
  return a == " "
};
goog.string.isUnicodeChar = function(a) {
  return a.length == 1 && a >= " " && a <= "~" || a >= "\u0080" && a <= "\ufffd"
};
goog.string.stripNewlines = function(a) {
  return a.replace(/(\r\n|\r|\n)+/g, " ")
};
goog.string.canonicalizeNewlines = function(a) {
  return a.replace(/(\r\n|\r|\n)/g, "\n")
};
goog.string.normalizeWhitespace = function(a) {
  return a.replace(/\xa0|\s/g, " ")
};
goog.string.normalizeSpaces = function(a) {
  return a.replace(/\xa0|[ \t]+/g, " ")
};
goog.string.collapseBreakingSpaces = function(a) {
  return a.replace(/[\t\r\n ]+/g, " ").replace(/^[\t\r\n ]+|[\t\r\n ]+$/g, "")
};
goog.string.trim = function(a) {
  return a.replace(/^[\s\xa0]+|[\s\xa0]+$/g, "")
};
goog.string.trimLeft = function(a) {
  return a.replace(/^[\s\xa0]+/, "")
};
goog.string.trimRight = function(a) {
  return a.replace(/[\s\xa0]+$/, "")
};
goog.string.caseInsensitiveCompare = function(a, b) {
  var c = String(a).toLowerCase(), d = String(b).toLowerCase();
  return c < d ? -1 : c == d ? 0 : 1
};
goog.string.numerateCompareRegExp_ = /(\.\d+)|(\d+)|(\D+)/g;
goog.string.numerateCompare = function(a, b) {
  if(a == b) {
    return 0
  }
  if(!a) {
    return-1
  }
  if(!b) {
    return 1
  }
  for(var c = a.toLowerCase().match(goog.string.numerateCompareRegExp_), d = b.toLowerCase().match(goog.string.numerateCompareRegExp_), e = Math.min(c.length, d.length), f = 0;f < e;f++) {
    var g = c[f], h = d[f];
    if(g != h) {
      c = parseInt(g, 10);
      return!isNaN(c) && (d = parseInt(h, 10), !isNaN(d) && c - d) ? c - d : g < h ? -1 : 1
    }
  }
  return c.length != d.length ? c.length - d.length : a < b ? -1 : 1
};
goog.string.encodeUriRegExp_ = /^[a-zA-Z0-9\-_.!~*'()]*$/;
goog.string.urlEncode = function(a) {
  a = String(a);
  return!goog.string.encodeUriRegExp_.test(a) ? encodeURIComponent(a) : a
};
goog.string.urlDecode = function(a) {
  return decodeURIComponent(a.replace(/\+/g, " "))
};
goog.string.newLineToBr = function(a, b) {
  return a.replace(/(\r\n|\r|\n)/g, b ? "<br />" : "<br>")
};
goog.string.htmlEscape = function(a, b) {
  if(b) {
    return a.replace(goog.string.amperRe_, "&amp;").replace(goog.string.ltRe_, "&lt;").replace(goog.string.gtRe_, "&gt;").replace(goog.string.quotRe_, "&quot;")
  }else {
    if(!goog.string.allRe_.test(a)) {
      return a
    }
    a.indexOf("&") != -1 && (a = a.replace(goog.string.amperRe_, "&amp;"));
    a.indexOf("<") != -1 && (a = a.replace(goog.string.ltRe_, "&lt;"));
    a.indexOf(">") != -1 && (a = a.replace(goog.string.gtRe_, "&gt;"));
    a.indexOf('"') != -1 && (a = a.replace(goog.string.quotRe_, "&quot;"));
    return a
  }
};
goog.string.amperRe_ = /&/g;
goog.string.ltRe_ = /</g;
goog.string.gtRe_ = />/g;
goog.string.quotRe_ = /\"/g;
goog.string.allRe_ = /[&<>\"]/;
goog.string.unescapeEntities = function(a) {
  return goog.string.contains(a, "&") ? "document" in goog.global ? goog.string.unescapeEntitiesUsingDom_(a) : goog.string.unescapePureXmlEntities_(a) : a
};
goog.string.unescapeEntitiesUsingDom_ = function(a) {
  var b = {"&amp;":"&", "&lt;":"<", "&gt;":">", "&quot;":'"'}, c = document.createElement("div");
  return a.replace(goog.string.HTML_ENTITY_PATTERN_, function(a, e) {
    var f = b[a];
    if(f) {
      return f
    }
    if(e.charAt(0) == "#") {
      var g = Number("0" + e.substr(1));
      isNaN(g) || (f = String.fromCharCode(g))
    }
    if(!f) {
      c.innerHTML = a + " ", f = c.firstChild.nodeValue.slice(0, -1)
    }
    return b[a] = f
  })
};
goog.string.unescapePureXmlEntities_ = function(a) {
  return a.replace(/&([^;]+);/g, function(a, c) {
    switch(c) {
      case "amp":
        return"&";
      case "lt":
        return"<";
      case "gt":
        return">";
      case "quot":
        return'"';
      default:
        if(c.charAt(0) == "#") {
          var d = Number("0" + c.substr(1));
          if(!isNaN(d)) {
            return String.fromCharCode(d)
          }
        }
        return a
    }
  })
};
goog.string.HTML_ENTITY_PATTERN_ = /&([^;\s<&]+);?/g;
goog.string.whitespaceEscape = function(a, b) {
  return goog.string.newLineToBr(a.replace(/  /g, " &#160;"), b)
};
goog.string.stripQuotes = function(a, b) {
  for(var c = b.length, d = 0;d < c;d++) {
    var e = c == 1 ? b : b.charAt(d);
    if(a.charAt(0) == e && a.charAt(a.length - 1) == e) {
      return a.substring(1, a.length - 1)
    }
  }
  return a
};
goog.string.truncate = function(a, b, c) {
  c && (a = goog.string.unescapeEntities(a));
  a.length > b && (a = a.substring(0, b - 3) + "...");
  c && (a = goog.string.htmlEscape(a));
  return a
};
goog.string.truncateMiddle = function(a, b, c, d) {
  c && (a = goog.string.unescapeEntities(a));
  if(d && a.length > b) {
    d > b && (d = b);
    var e = a.length - d, a = a.substring(0, b - d) + "..." + a.substring(e)
  }else {
    a.length > b && (d = Math.floor(b / 2), e = a.length - d, d += b % 2, a = a.substring(0, d) + "..." + a.substring(e))
  }
  c && (a = goog.string.htmlEscape(a));
  return a
};
goog.string.specialEscapeChars_ = {"\x00":"\\0", "\u0008":"\\b", "\u000c":"\\f", "\n":"\\n", "\r":"\\r", "\t":"\\t", "\u000b":"\\x0B", '"':'\\"', "\\":"\\\\"};
goog.string.jsEscapeCache_ = {"'":"\\'"};
goog.string.quote = function(a) {
  a = String(a);
  if(a.quote) {
    return a.quote()
  }else {
    for(var b = ['"'], c = 0;c < a.length;c++) {
      var d = a.charAt(c), e = d.charCodeAt(0);
      b[c + 1] = goog.string.specialEscapeChars_[d] || (e > 31 && e < 127 ? d : goog.string.escapeChar(d))
    }
    b.push('"');
    return b.join("")
  }
};
goog.string.escapeString = function(a) {
  for(var b = [], c = 0;c < a.length;c++) {
    b[c] = goog.string.escapeChar(a.charAt(c))
  }
  return b.join("")
};
goog.string.escapeChar = function(a) {
  if(a in goog.string.jsEscapeCache_) {
    return goog.string.jsEscapeCache_[a]
  }
  if(a in goog.string.specialEscapeChars_) {
    return goog.string.jsEscapeCache_[a] = goog.string.specialEscapeChars_[a]
  }
  var b = a, c = a.charCodeAt(0);
  if(c > 31 && c < 127) {
    b = a
  }else {
    if(c < 256) {
      if(b = "\\x", c < 16 || c > 256) {
        b += "0"
      }
    }else {
      b = "\\u", c < 4096 && (b += "0")
    }
    b += c.toString(16).toUpperCase()
  }
  return goog.string.jsEscapeCache_[a] = b
};
goog.string.toMap = function(a) {
  for(var b = {}, c = 0;c < a.length;c++) {
    b[a.charAt(c)] = !0
  }
  return b
};
goog.string.contains = function(a, b) {
  return a.indexOf(b) != -1
};
goog.string.removeAt = function(a, b, c) {
  var d = a;
  b >= 0 && b < a.length && c > 0 && (d = a.substr(0, b) + a.substr(b + c, a.length - b - c));
  return d
};
goog.string.remove = function(a, b) {
  var c = RegExp(goog.string.regExpEscape(b), "");
  return a.replace(c, "")
};
goog.string.removeAll = function(a, b) {
  var c = RegExp(goog.string.regExpEscape(b), "g");
  return a.replace(c, "")
};
goog.string.regExpEscape = function(a) {
  return String(a).replace(/([-()\[\]{}+?*.$\^|,:#<!\\])/g, "\\$1").replace(/\x08/g, "\\x08")
};
goog.string.repeat = function(a, b) {
  return Array(b + 1).join(a)
};
goog.string.padNumber = function(a, b, c) {
  a = goog.isDef(c) ? a.toFixed(c) : String(a);
  c = a.indexOf(".");
  if(c == -1) {
    c = a.length
  }
  return goog.string.repeat("0", Math.max(0, b - c)) + a
};
goog.string.makeSafe = function(a) {
  return a == null ? "" : String(a)
};
goog.string.buildString = function(a) {
  return Array.prototype.join.call(arguments, "")
};
goog.string.getRandomString = function() {
  return Math.floor(Math.random() * 2147483648).toString(36) + Math.abs(Math.floor(Math.random() * 2147483648) ^ goog.now()).toString(36)
};
goog.string.compareVersions = function(a, b) {
  for(var c = 0, d = goog.string.trim(String(a)).split("."), e = goog.string.trim(String(b)).split("."), f = Math.max(d.length, e.length), g = 0;c == 0 && g < f;g++) {
    var h = d[g] || "", i = e[g] || "", j = RegExp("(\\d*)(\\D*)", "g"), k = RegExp("(\\d*)(\\D*)", "g");
    do {
      var m = j.exec(h) || ["", "", ""], l = k.exec(i) || ["", "", ""];
      if(m[0].length == 0 && l[0].length == 0) {
        break
      }
      var c = m[1].length == 0 ? 0 : parseInt(m[1], 10), n = l[1].length == 0 ? 0 : parseInt(l[1], 10), c = goog.string.compareElements_(c, n) || goog.string.compareElements_(m[2].length == 0, l[2].length == 0) || goog.string.compareElements_(m[2], l[2])
    }while(c == 0)
  }
  return c
};
goog.string.compareElements_ = function(a, b) {
  if(a < b) {
    return-1
  }else {
    if(a > b) {
      return 1
    }
  }
  return 0
};
goog.string.HASHCODE_MAX_ = 4294967296;
goog.string.hashCode = function(a) {
  for(var b = 0, c = 0;c < a.length;++c) {
    b = 31 * b + a.charCodeAt(c), b %= goog.string.HASHCODE_MAX_
  }
  return b
};
goog.string.uniqueStringCounter_ = Math.random() * 2147483648 | 0;
goog.string.createUniqueString = function() {
  return"goog_" + goog.string.uniqueStringCounter_++
};
goog.string.toNumber = function(a) {
  var b = Number(a);
  return b == 0 && goog.string.isEmpty(a) ? NaN : b
};
goog.string.toCamelCaseCache_ = {};
goog.string.toCamelCase = function(a) {
  return goog.string.toCamelCaseCache_[a] || (goog.string.toCamelCaseCache_[a] = String(a).replace(/\-([a-z])/g, function(a, c) {
    return c.toUpperCase()
  }))
};
goog.string.toSelectorCaseCache_ = {};
goog.string.toSelectorCase = function(a) {
  return goog.string.toSelectorCaseCache_[a] || (goog.string.toSelectorCaseCache_[a] = String(a).replace(/([A-Z])/g, "-$1").toLowerCase())
};
goog.asserts = {};
goog.asserts.ENABLE_ASSERTS = goog.DEBUG;
goog.asserts.AssertionError = function(a, b) {
  b.unshift(a);
  goog.debug.Error.call(this, goog.string.subs.apply(null, b));
  b.shift();
  this.messagePattern = a
};
goog.inherits(goog.asserts.AssertionError, goog.debug.Error);
goog.asserts.AssertionError.prototype.name = "AssertionError";
goog.asserts.doAssertFailure_ = function(a, b, c, d) {
  var e = "Assertion failed";
  if(c) {
    e += ": " + c;
    var f = d
  }else {
    a && (e += ": " + a, f = b)
  }
  throw new goog.asserts.AssertionError("" + e, f || []);
};
goog.asserts.assert = function(a, b, c) {
  goog.asserts.ENABLE_ASSERTS && !a && goog.asserts.doAssertFailure_("", null, b, Array.prototype.slice.call(arguments, 2));
  return a
};
goog.asserts.fail = function(a, b) {
  if(goog.asserts.ENABLE_ASSERTS) {
    throw new goog.asserts.AssertionError("Failure" + (a ? ": " + a : ""), Array.prototype.slice.call(arguments, 1));
  }
};
goog.asserts.assertNumber = function(a, b, c) {
  goog.asserts.ENABLE_ASSERTS && !goog.isNumber(a) && goog.asserts.doAssertFailure_("Expected number but got %s: %s.", [goog.typeOf(a), a], b, Array.prototype.slice.call(arguments, 2));
  return a
};
goog.asserts.assertString = function(a, b, c) {
  goog.asserts.ENABLE_ASSERTS && !goog.isString(a) && goog.asserts.doAssertFailure_("Expected string but got %s: %s.", [goog.typeOf(a), a], b, Array.prototype.slice.call(arguments, 2));
  return a
};
goog.asserts.assertFunction = function(a, b, c) {
  goog.asserts.ENABLE_ASSERTS && !goog.isFunction(a) && goog.asserts.doAssertFailure_("Expected function but got %s: %s.", [goog.typeOf(a), a], b, Array.prototype.slice.call(arguments, 2));
  return a
};
goog.asserts.assertObject = function(a, b, c) {
  goog.asserts.ENABLE_ASSERTS && !goog.isObject(a) && goog.asserts.doAssertFailure_("Expected object but got %s: %s.", [goog.typeOf(a), a], b, Array.prototype.slice.call(arguments, 2));
  return a
};
goog.asserts.assertArray = function(a, b, c) {
  goog.asserts.ENABLE_ASSERTS && !goog.isArray(a) && goog.asserts.doAssertFailure_("Expected array but got %s: %s.", [goog.typeOf(a), a], b, Array.prototype.slice.call(arguments, 2));
  return a
};
goog.asserts.assertBoolean = function(a, b, c) {
  goog.asserts.ENABLE_ASSERTS && !goog.isBoolean(a) && goog.asserts.doAssertFailure_("Expected boolean but got %s: %s.", [goog.typeOf(a), a], b, Array.prototype.slice.call(arguments, 2));
  return a
};
goog.asserts.assertInstanceof = function(a, b, c, d) {
  goog.asserts.ENABLE_ASSERTS && !(a instanceof b) && goog.asserts.doAssertFailure_("instanceof check failed.", null, c, Array.prototype.slice.call(arguments, 3))
};
goog.array = {};
goog.NATIVE_ARRAY_PROTOTYPES = !0;
goog.array.peek = function(a) {
  return a[a.length - 1]
};
goog.array.ARRAY_PROTOTYPE_ = Array.prototype;
goog.array.indexOf = goog.NATIVE_ARRAY_PROTOTYPES && goog.array.ARRAY_PROTOTYPE_.indexOf ? function(a, b, c) {
  goog.asserts.assert(a.length != null);
  return goog.array.ARRAY_PROTOTYPE_.indexOf.call(a, b, c)
} : function(a, b, c) {
  c = c == null ? 0 : c < 0 ? Math.max(0, a.length + c) : c;
  if(goog.isString(a)) {
    return!goog.isString(b) || b.length != 1 ? -1 : a.indexOf(b, c)
  }
  for(;c < a.length;c++) {
    if(c in a && a[c] === b) {
      return c
    }
  }
  return-1
};
goog.array.lastIndexOf = goog.NATIVE_ARRAY_PROTOTYPES && goog.array.ARRAY_PROTOTYPE_.lastIndexOf ? function(a, b, c) {
  goog.asserts.assert(a.length != null);
  return goog.array.ARRAY_PROTOTYPE_.lastIndexOf.call(a, b, c == null ? a.length - 1 : c)
} : function(a, b, c) {
  c = c == null ? a.length - 1 : c;
  c < 0 && (c = Math.max(0, a.length + c));
  if(goog.isString(a)) {
    return!goog.isString(b) || b.length != 1 ? -1 : a.lastIndexOf(b, c)
  }
  for(;c >= 0;c--) {
    if(c in a && a[c] === b) {
      return c
    }
  }
  return-1
};
goog.array.forEach = goog.NATIVE_ARRAY_PROTOTYPES && goog.array.ARRAY_PROTOTYPE_.forEach ? function(a, b, c) {
  goog.asserts.assert(a.length != null);
  goog.array.ARRAY_PROTOTYPE_.forEach.call(a, b, c)
} : function(a, b, c) {
  for(var d = a.length, e = goog.isString(a) ? a.split("") : a, f = 0;f < d;f++) {
    f in e && b.call(c, e[f], f, a)
  }
};
goog.array.forEachRight = function(a, b, c) {
  var d = a.length, e = goog.isString(a) ? a.split("") : a;
  for(d -= 1;d >= 0;--d) {
    d in e && b.call(c, e[d], d, a)
  }
};
goog.array.filter = goog.NATIVE_ARRAY_PROTOTYPES && goog.array.ARRAY_PROTOTYPE_.filter ? function(a, b, c) {
  goog.asserts.assert(a.length != null);
  return goog.array.ARRAY_PROTOTYPE_.filter.call(a, b, c)
} : function(a, b, c) {
  for(var d = a.length, e = [], f = 0, g = goog.isString(a) ? a.split("") : a, h = 0;h < d;h++) {
    if(h in g) {
      var i = g[h];
      b.call(c, i, h, a) && (e[f++] = i)
    }
  }
  return e
};
goog.array.map = goog.NATIVE_ARRAY_PROTOTYPES && goog.array.ARRAY_PROTOTYPE_.map ? function(a, b, c) {
  goog.asserts.assert(a.length != null);
  return goog.array.ARRAY_PROTOTYPE_.map.call(a, b, c)
} : function(a, b, c) {
  for(var d = a.length, e = Array(d), f = goog.isString(a) ? a.split("") : a, g = 0;g < d;g++) {
    g in f && (e[g] = b.call(c, f[g], g, a))
  }
  return e
};
goog.array.reduce = function(a, b, c, d) {
  if(a.reduce) {
    return d ? a.reduce(goog.bind(b, d), c) : a.reduce(b, c)
  }
  var e = c;
  goog.array.forEach(a, function(c, g) {
    e = b.call(d, e, c, g, a)
  });
  return e
};
goog.array.reduceRight = function(a, b, c, d) {
  if(a.reduceRight) {
    return d ? a.reduceRight(goog.bind(b, d), c) : a.reduceRight(b, c)
  }
  var e = c;
  goog.array.forEachRight(a, function(c, g) {
    e = b.call(d, e, c, g, a)
  });
  return e
};
goog.array.some = goog.NATIVE_ARRAY_PROTOTYPES && goog.array.ARRAY_PROTOTYPE_.some ? function(a, b, c) {
  goog.asserts.assert(a.length != null);
  return goog.array.ARRAY_PROTOTYPE_.some.call(a, b, c)
} : function(a, b, c) {
  for(var d = a.length, e = goog.isString(a) ? a.split("") : a, f = 0;f < d;f++) {
    if(f in e && b.call(c, e[f], f, a)) {
      return!0
    }
  }
  return!1
};
goog.array.every = goog.NATIVE_ARRAY_PROTOTYPES && goog.array.ARRAY_PROTOTYPE_.every ? function(a, b, c) {
  goog.asserts.assert(a.length != null);
  return goog.array.ARRAY_PROTOTYPE_.every.call(a, b, c)
} : function(a, b, c) {
  for(var d = a.length, e = goog.isString(a) ? a.split("") : a, f = 0;f < d;f++) {
    if(f in e && !b.call(c, e[f], f, a)) {
      return!1
    }
  }
  return!0
};
goog.array.find = function(a, b, c) {
  b = goog.array.findIndex(a, b, c);
  return b < 0 ? null : goog.isString(a) ? a.charAt(b) : a[b]
};
goog.array.findIndex = function(a, b, c) {
  for(var d = a.length, e = goog.isString(a) ? a.split("") : a, f = 0;f < d;f++) {
    if(f in e && b.call(c, e[f], f, a)) {
      return f
    }
  }
  return-1
};
goog.array.findRight = function(a, b, c) {
  b = goog.array.findIndexRight(a, b, c);
  return b < 0 ? null : goog.isString(a) ? a.charAt(b) : a[b]
};
goog.array.findIndexRight = function(a, b, c) {
  var d = a.length, e = goog.isString(a) ? a.split("") : a;
  for(d -= 1;d >= 0;d--) {
    if(d in e && b.call(c, e[d], d, a)) {
      return d
    }
  }
  return-1
};
goog.array.contains = function(a, b) {
  return goog.array.indexOf(a, b) >= 0
};
goog.array.isEmpty = function(a) {
  return a.length == 0
};
goog.array.clear = function(a) {
  if(!goog.isArray(a)) {
    for(var b = a.length - 1;b >= 0;b--) {
      delete a[b]
    }
  }
  a.length = 0
};
goog.array.insert = function(a, b) {
  goog.array.contains(a, b) || a.push(b)
};
goog.array.insertAt = function(a, b, c) {
  goog.array.splice(a, c, 0, b)
};
goog.array.insertArrayAt = function(a, b, c) {
  goog.partial(goog.array.splice, a, c, 0).apply(null, b)
};
goog.array.insertBefore = function(a, b, c) {
  var d;
  arguments.length == 2 || (d = goog.array.indexOf(a, c)) < 0 ? a.push(b) : goog.array.insertAt(a, b, d)
};
goog.array.remove = function(a, b) {
  var c = goog.array.indexOf(a, b), d;
  (d = c >= 0) && goog.array.removeAt(a, c);
  return d
};
goog.array.removeAt = function(a, b) {
  goog.asserts.assert(a.length != null);
  return goog.array.ARRAY_PROTOTYPE_.splice.call(a, b, 1).length == 1
};
goog.array.removeIf = function(a, b, c) {
  b = goog.array.findIndex(a, b, c);
  return b >= 0 ? (goog.array.removeAt(a, b), !0) : !1
};
goog.array.concat = function(a) {
  return goog.array.ARRAY_PROTOTYPE_.concat.apply(goog.array.ARRAY_PROTOTYPE_, arguments)
};
goog.array.clone = function(a) {
  if(goog.isArray(a)) {
    return goog.array.concat(a)
  }else {
    for(var b = [], c = 0, d = a.length;c < d;c++) {
      b[c] = a[c]
    }
    return b
  }
};
goog.array.toArray = function(a) {
  return goog.isArray(a) ? goog.array.concat(a) : goog.array.clone(a)
};
goog.array.extend = function(a, b) {
  for(var c = 1;c < arguments.length;c++) {
    var d = arguments[c], e;
    if(goog.isArray(d) || (e = goog.isArrayLike(d)) && d.hasOwnProperty("callee")) {
      a.push.apply(a, d)
    }else {
      if(e) {
        for(var f = a.length, g = d.length, h = 0;h < g;h++) {
          a[f + h] = d[h]
        }
      }else {
        a.push(d)
      }
    }
  }
};
goog.array.splice = function(a, b, c, d) {
  goog.asserts.assert(a.length != null);
  return goog.array.ARRAY_PROTOTYPE_.splice.apply(a, goog.array.slice(arguments, 1))
};
goog.array.slice = function(a, b, c) {
  goog.asserts.assert(a.length != null);
  return arguments.length <= 2 ? goog.array.ARRAY_PROTOTYPE_.slice.call(a, b) : goog.array.ARRAY_PROTOTYPE_.slice.call(a, b, c)
};
goog.array.removeDuplicates = function(a, b) {
  for(var c = b || a, d = {}, e = 0, f = 0;f < a.length;) {
    var g = a[f++], h = goog.isObject(g) ? "o" + goog.getUid(g) : (typeof g).charAt(0) + g;
    Object.prototype.hasOwnProperty.call(d, h) || (d[h] = !0, c[e++] = g)
  }
  c.length = e
};
goog.array.binarySearch = function(a, b, c) {
  return goog.array.binarySearch_(a, c || goog.array.defaultCompare, !1, b)
};
goog.array.binarySelect = function(a, b, c) {
  return goog.array.binarySearch_(a, b, !0, void 0, c)
};
goog.array.binarySearch_ = function(a, b, c, d, e) {
  for(var f = 0, g = a.length, h;f < g;) {
    var i = f + g >> 1, j;
    j = c ? b.call(e, a[i], i, a) : b(d, a[i]);
    j > 0 ? f = i + 1 : (g = i, h = !j)
  }
  return h ? f : ~f
};
goog.array.sort = function(a, b) {
  goog.asserts.assert(a.length != null);
  goog.array.ARRAY_PROTOTYPE_.sort.call(a, b || goog.array.defaultCompare)
};
goog.array.stableSort = function(a, b) {
  for(var c = 0;c < a.length;c++) {
    a[c] = {index:c, value:a[c]}
  }
  var d = b || goog.array.defaultCompare;
  goog.array.sort(a, function(a, b) {
    return d(a.value, b.value) || a.index - b.index
  });
  for(c = 0;c < a.length;c++) {
    a[c] = a[c].value
  }
};
goog.array.sortObjectsByKey = function(a, b, c) {
  var d = c || goog.array.defaultCompare;
  goog.array.sort(a, function(a, c) {
    return d(a[b], c[b])
  })
};
goog.array.isSorted = function(a, b, c) {
  for(var b = b || goog.array.defaultCompare, d = 1;d < a.length;d++) {
    var e = b(a[d - 1], a[d]);
    if(e > 0 || e == 0 && c) {
      return!1
    }
  }
  return!0
};
goog.array.equals = function(a, b, c) {
  if(!goog.isArrayLike(a) || !goog.isArrayLike(b) || a.length != b.length) {
    return!1
  }
  for(var d = a.length, c = c || goog.array.defaultCompareEquality, e = 0;e < d;e++) {
    if(!c(a[e], b[e])) {
      return!1
    }
  }
  return!0
};
goog.array.compare = function(a, b, c) {
  return goog.array.equals(a, b, c)
};
goog.array.compare3 = function(a, b, c) {
  for(var c = c || goog.array.defaultCompare, d = Math.min(a.length, b.length), e = 0;e < d;e++) {
    var f = c(a[e], b[e]);
    if(f != 0) {
      return f
    }
  }
  return goog.array.defaultCompare(a.length, b.length)
};
goog.array.defaultCompare = function(a, b) {
  return a > b ? 1 : a < b ? -1 : 0
};
goog.array.defaultCompareEquality = function(a, b) {
  return a === b
};
goog.array.binaryInsert = function(a, b, c) {
  c = goog.array.binarySearch(a, b, c);
  return c < 0 ? (goog.array.insertAt(a, b, -(c + 1)), !0) : !1
};
goog.array.binaryRemove = function(a, b, c) {
  b = goog.array.binarySearch(a, b, c);
  return b >= 0 ? goog.array.removeAt(a, b) : !1
};
goog.array.bucket = function(a, b) {
  for(var c = {}, d = 0;d < a.length;d++) {
    var e = a[d], f = b(e, d, a);
    goog.isDef(f) && (c[f] || (c[f] = [])).push(e)
  }
  return c
};
goog.array.repeat = function(a, b) {
  for(var c = [], d = 0;d < b;d++) {
    c[d] = a
  }
  return c
};
goog.array.flatten = function(a) {
  for(var b = [], c = 0;c < arguments.length;c++) {
    var d = arguments[c];
    goog.isArray(d) ? b.push.apply(b, goog.array.flatten.apply(null, d)) : b.push(d)
  }
  return b
};
goog.array.rotate = function(a, b) {
  goog.asserts.assert(a.length != null);
  a.length && (b %= a.length, b > 0 ? goog.array.ARRAY_PROTOTYPE_.unshift.apply(a, a.splice(-b, b)) : b < 0 && goog.array.ARRAY_PROTOTYPE_.push.apply(a, a.splice(0, -b)));
  return a
};
goog.array.zip = function(a) {
  if(!arguments.length) {
    return[]
  }
  for(var b = [], c = 0;;c++) {
    for(var d = [], e = 0;e < arguments.length;e++) {
      var f = arguments[e];
      if(c >= f.length) {
        return b
      }
      d.push(f[c])
    }
    b.push(d)
  }
};
goog.array.shuffle = function(a, b) {
  for(var c = b || Math.random, d = a.length - 1;d > 0;d--) {
    var e = Math.floor(c() * (d + 1)), f = a[d];
    a[d] = a[e];
    a[e] = f
  }
};
goog.userAgent = {};
goog.userAgent.ASSUME_IE = !1;
goog.userAgent.ASSUME_GECKO = !1;
goog.userAgent.ASSUME_WEBKIT = !1;
goog.userAgent.ASSUME_MOBILE_WEBKIT = !1;
goog.userAgent.ASSUME_OPERA = !1;
goog.userAgent.BROWSER_KNOWN_ = goog.userAgent.ASSUME_IE || goog.userAgent.ASSUME_GECKO || goog.userAgent.ASSUME_MOBILE_WEBKIT || goog.userAgent.ASSUME_WEBKIT || goog.userAgent.ASSUME_OPERA;
goog.userAgent.getUserAgentString = function() {
  return goog.global.navigator ? goog.global.navigator.userAgent : null
};
goog.userAgent.getNavigator = function() {
  return goog.global.navigator
};
goog.userAgent.init_ = function() {
  goog.userAgent.detectedOpera_ = !1;
  goog.userAgent.detectedIe_ = !1;
  goog.userAgent.detectedWebkit_ = !1;
  goog.userAgent.detectedMobile_ = !1;
  goog.userAgent.detectedGecko_ = !1;
  var a;
  if(!goog.userAgent.BROWSER_KNOWN_ && (a = goog.userAgent.getUserAgentString())) {
    var b = goog.userAgent.getNavigator();
    goog.userAgent.detectedOpera_ = a.indexOf("Opera") == 0;
    goog.userAgent.detectedIe_ = !goog.userAgent.detectedOpera_ && a.indexOf("MSIE") != -1;
    goog.userAgent.detectedWebkit_ = !goog.userAgent.detectedOpera_ && a.indexOf("WebKit") != -1;
    goog.userAgent.detectedMobile_ = goog.userAgent.detectedWebkit_ && a.indexOf("Mobile") != -1;
    goog.userAgent.detectedGecko_ = !goog.userAgent.detectedOpera_ && !goog.userAgent.detectedWebkit_ && b.product == "Gecko"
  }
};
goog.userAgent.BROWSER_KNOWN_ || goog.userAgent.init_();
goog.userAgent.OPERA = goog.userAgent.BROWSER_KNOWN_ ? goog.userAgent.ASSUME_OPERA : goog.userAgent.detectedOpera_;
goog.userAgent.IE = goog.userAgent.BROWSER_KNOWN_ ? goog.userAgent.ASSUME_IE : goog.userAgent.detectedIe_;
goog.userAgent.GECKO = goog.userAgent.BROWSER_KNOWN_ ? goog.userAgent.ASSUME_GECKO : goog.userAgent.detectedGecko_;
goog.userAgent.WEBKIT = goog.userAgent.BROWSER_KNOWN_ ? goog.userAgent.ASSUME_WEBKIT || goog.userAgent.ASSUME_MOBILE_WEBKIT : goog.userAgent.detectedWebkit_;
goog.userAgent.MOBILE = goog.userAgent.ASSUME_MOBILE_WEBKIT || goog.userAgent.detectedMobile_;
goog.userAgent.SAFARI = goog.userAgent.WEBKIT;
goog.userAgent.determinePlatform_ = function() {
  var a = goog.userAgent.getNavigator();
  return a && a.platform || ""
};
goog.userAgent.PLATFORM = goog.userAgent.determinePlatform_();
goog.userAgent.ASSUME_MAC = !1;
goog.userAgent.ASSUME_WINDOWS = !1;
goog.userAgent.ASSUME_LINUX = !1;
goog.userAgent.ASSUME_X11 = !1;
goog.userAgent.PLATFORM_KNOWN_ = goog.userAgent.ASSUME_MAC || goog.userAgent.ASSUME_WINDOWS || goog.userAgent.ASSUME_LINUX || goog.userAgent.ASSUME_X11;
goog.userAgent.initPlatform_ = function() {
  goog.userAgent.detectedMac_ = goog.string.contains(goog.userAgent.PLATFORM, "Mac");
  goog.userAgent.detectedWindows_ = goog.string.contains(goog.userAgent.PLATFORM, "Win");
  goog.userAgent.detectedLinux_ = goog.string.contains(goog.userAgent.PLATFORM, "Linux");
  goog.userAgent.detectedX11_ = !!goog.userAgent.getNavigator() && goog.string.contains(goog.userAgent.getNavigator().appVersion || "", "X11")
};
goog.userAgent.PLATFORM_KNOWN_ || goog.userAgent.initPlatform_();
goog.userAgent.MAC = goog.userAgent.PLATFORM_KNOWN_ ? goog.userAgent.ASSUME_MAC : goog.userAgent.detectedMac_;
goog.userAgent.WINDOWS = goog.userAgent.PLATFORM_KNOWN_ ? goog.userAgent.ASSUME_WINDOWS : goog.userAgent.detectedWindows_;
goog.userAgent.LINUX = goog.userAgent.PLATFORM_KNOWN_ ? goog.userAgent.ASSUME_LINUX : goog.userAgent.detectedLinux_;
goog.userAgent.X11 = goog.userAgent.PLATFORM_KNOWN_ ? goog.userAgent.ASSUME_X11 : goog.userAgent.detectedX11_;
goog.userAgent.determineVersion_ = function() {
  var a = "", b;
  goog.userAgent.OPERA && goog.global.opera ? (a = goog.global.opera.version, a = typeof a == "function" ? a() : a) : (goog.userAgent.GECKO ? b = /rv\:([^\);]+)(\)|;)/ : goog.userAgent.IE ? b = /MSIE\s+([^\);]+)(\)|;)/ : goog.userAgent.WEBKIT && (b = /WebKit\/(\S+)/), b && (a = (a = b.exec(goog.userAgent.getUserAgentString())) ? a[1] : ""));
  return goog.userAgent.IE && (b = goog.userAgent.getDocumentMode_(), b > parseFloat(a)) ? String(b) : a
};
goog.userAgent.getDocumentMode_ = function() {
  var a = goog.global.document;
  return a ? a.documentMode : void 0
};
goog.userAgent.VERSION = goog.userAgent.determineVersion_();
goog.userAgent.compare = function(a, b) {
  return goog.string.compareVersions(a, b)
};
goog.userAgent.isVersionCache_ = {};
goog.userAgent.isVersion = function(a) {
  return goog.userAgent.isVersionCache_[a] || (goog.userAgent.isVersionCache_[a] = goog.string.compareVersions(goog.userAgent.VERSION, a) >= 0)
};
goog.userAgent.isDocumentModeCache_ = {};
goog.userAgent.isDocumentMode = function(a) {
  return goog.userAgent.isDocumentModeCache_[a] || (goog.userAgent.isDocumentModeCache_[a] = goog.userAgent.IE && document.documentMode && document.documentMode >= a)
};
goog.dom = {};
goog.dom.BrowserFeature = {CAN_ADD_NAME_OR_TYPE_ATTRIBUTES:!goog.userAgent.IE || goog.userAgent.isDocumentMode(9), CAN_USE_CHILDREN_ATTRIBUTE:!goog.userAgent.GECKO && !goog.userAgent.IE || goog.userAgent.IE && goog.userAgent.isDocumentMode(9) || goog.userAgent.GECKO && goog.userAgent.isVersion("1.9.1"), CAN_USE_INNER_TEXT:goog.userAgent.IE && !goog.userAgent.isVersion("9"), INNER_HTML_NEEDS_SCOPED_ELEMENT:goog.userAgent.IE};
goog.dom.TagName = {A:"A", ABBR:"ABBR", ACRONYM:"ACRONYM", ADDRESS:"ADDRESS", APPLET:"APPLET", AREA:"AREA", B:"B", BASE:"BASE", BASEFONT:"BASEFONT", BDO:"BDO", BIG:"BIG", BLOCKQUOTE:"BLOCKQUOTE", BODY:"BODY", BR:"BR", BUTTON:"BUTTON", CANVAS:"CANVAS", CAPTION:"CAPTION", CENTER:"CENTER", CITE:"CITE", CODE:"CODE", COL:"COL", COLGROUP:"COLGROUP", DD:"DD", DEL:"DEL", DFN:"DFN", DIR:"DIR", DIV:"DIV", DL:"DL", DT:"DT", EM:"EM", FIELDSET:"FIELDSET", FONT:"FONT", FORM:"FORM", FRAME:"FRAME", FRAMESET:"FRAMESET", 
H1:"H1", H2:"H2", H3:"H3", H4:"H4", H5:"H5", H6:"H6", HEAD:"HEAD", HR:"HR", HTML:"HTML", I:"I", IFRAME:"IFRAME", IMG:"IMG", INPUT:"INPUT", INS:"INS", ISINDEX:"ISINDEX", KBD:"KBD", LABEL:"LABEL", LEGEND:"LEGEND", LI:"LI", LINK:"LINK", MAP:"MAP", MENU:"MENU", META:"META", NOFRAMES:"NOFRAMES", NOSCRIPT:"NOSCRIPT", OBJECT:"OBJECT", OL:"OL", OPTGROUP:"OPTGROUP", OPTION:"OPTION", P:"P", PARAM:"PARAM", PRE:"PRE", Q:"Q", S:"S", SAMP:"SAMP", SCRIPT:"SCRIPT", SELECT:"SELECT", SMALL:"SMALL", SPAN:"SPAN", STRIKE:"STRIKE", 
STRONG:"STRONG", STYLE:"STYLE", SUB:"SUB", SUP:"SUP", TABLE:"TABLE", TBODY:"TBODY", TD:"TD", TEXTAREA:"TEXTAREA", TFOOT:"TFOOT", TH:"TH", THEAD:"THEAD", TITLE:"TITLE", TR:"TR", TT:"TT", U:"U", UL:"UL", VAR:"VAR"};
goog.dom.classes = {};
goog.dom.classes.set = function(a, b) {
  a.className = b
};
goog.dom.classes.get = function(a) {
  return(a = a.className) && typeof a.split == "function" ? a.split(/\s+/) : []
};
goog.dom.classes.add = function(a, b) {
  var c = goog.dom.classes.get(a), d = goog.array.slice(arguments, 1), d = goog.dom.classes.add_(c, d);
  a.className = c.join(" ");
  return d
};
goog.dom.classes.remove = function(a, b) {
  var c = goog.dom.classes.get(a), d = goog.array.slice(arguments, 1), d = goog.dom.classes.remove_(c, d);
  a.className = c.join(" ");
  return d
};
goog.dom.classes.add_ = function(a, b) {
  for(var c = 0, d = 0;d < b.length;d++) {
    goog.array.contains(a, b[d]) || (a.push(b[d]), c++)
  }
  return c == b.length
};
goog.dom.classes.remove_ = function(a, b) {
  for(var c = 0, d = 0;d < a.length;d++) {
    goog.array.contains(b, a[d]) && (goog.array.splice(a, d--, 1), c++)
  }
  return c == b.length
};
goog.dom.classes.swap = function(a, b, c) {
  for(var d = goog.dom.classes.get(a), e = !1, f = 0;f < d.length;f++) {
    d[f] == b && (goog.array.splice(d, f--, 1), e = !0)
  }
  if(e) {
    d.push(c), a.className = d.join(" ")
  }
  return e
};
goog.dom.classes.addRemove = function(a, b, c) {
  var d = goog.dom.classes.get(a);
  goog.isString(b) ? goog.array.remove(d, b) : goog.isArray(b) && goog.dom.classes.remove_(d, b);
  goog.isString(c) && !goog.array.contains(d, c) ? d.push(c) : goog.isArray(c) && goog.dom.classes.add_(d, c);
  a.className = d.join(" ")
};
goog.dom.classes.has = function(a, b) {
  return goog.array.contains(goog.dom.classes.get(a), b)
};
goog.dom.classes.enable = function(a, b, c) {
  c ? goog.dom.classes.add(a, b) : goog.dom.classes.remove(a, b)
};
goog.dom.classes.toggle = function(a, b) {
  var c = !goog.dom.classes.has(a, b);
  goog.dom.classes.enable(a, b, c);
  return c
};
goog.math = {};
goog.math.Coordinate = function(a, b) {
  this.x = goog.isDef(a) ? a : 0;
  this.y = goog.isDef(b) ? b : 0
};
goog.math.Coordinate.prototype.clone = function() {
  return new goog.math.Coordinate(this.x, this.y)
};
if(goog.DEBUG) {
  goog.math.Coordinate.prototype.toString = function() {
    return"(" + this.x + ", " + this.y + ")"
  }
}
goog.math.Coordinate.equals = function(a, b) {
  return a == b ? !0 : !a || !b ? !1 : a.x == b.x && a.y == b.y
};
goog.math.Coordinate.distance = function(a, b) {
  var c = a.x - b.x, d = a.y - b.y;
  return Math.sqrt(c * c + d * d)
};
goog.math.Coordinate.squaredDistance = function(a, b) {
  var c = a.x - b.x, d = a.y - b.y;
  return c * c + d * d
};
goog.math.Coordinate.difference = function(a, b) {
  return new goog.math.Coordinate(a.x - b.x, a.y - b.y)
};
goog.math.Coordinate.sum = function(a, b) {
  return new goog.math.Coordinate(a.x + b.x, a.y + b.y)
};
goog.math.Size = function(a, b) {
  this.width = a;
  this.height = b
};
goog.math.Size.equals = function(a, b) {
  return a == b ? !0 : !a || !b ? !1 : a.width == b.width && a.height == b.height
};
goog.math.Size.prototype.clone = function() {
  return new goog.math.Size(this.width, this.height)
};
if(goog.DEBUG) {
  goog.math.Size.prototype.toString = function() {
    return"(" + this.width + " x " + this.height + ")"
  }
}
goog.math.Size.prototype.getLongest = function() {
  return Math.max(this.width, this.height)
};
goog.math.Size.prototype.getShortest = function() {
  return Math.min(this.width, this.height)
};
goog.math.Size.prototype.area = function() {
  return this.width * this.height
};
goog.math.Size.prototype.perimeter = function() {
  return(this.width + this.height) * 2
};
goog.math.Size.prototype.aspectRatio = function() {
  return this.width / this.height
};
goog.math.Size.prototype.isEmpty = function() {
  return!this.area()
};
goog.math.Size.prototype.ceil = function() {
  this.width = Math.ceil(this.width);
  this.height = Math.ceil(this.height);
  return this
};
goog.math.Size.prototype.fitsInside = function(a) {
  return this.width <= a.width && this.height <= a.height
};
goog.math.Size.prototype.floor = function() {
  this.width = Math.floor(this.width);
  this.height = Math.floor(this.height);
  return this
};
goog.math.Size.prototype.round = function() {
  this.width = Math.round(this.width);
  this.height = Math.round(this.height);
  return this
};
goog.math.Size.prototype.scale = function(a) {
  this.width *= a;
  this.height *= a;
  return this
};
goog.math.Size.prototype.scaleToFit = function(a) {
  return this.scale(this.aspectRatio() > a.aspectRatio() ? a.width / this.width : a.height / this.height)
};
goog.object = {};
goog.object.forEach = function(a, b, c) {
  for(var d in a) {
    b.call(c, a[d], d, a)
  }
};
goog.object.filter = function(a, b, c) {
  var d = {}, e;
  for(e in a) {
    b.call(c, a[e], e, a) && (d[e] = a[e])
  }
  return d
};
goog.object.map = function(a, b, c) {
  var d = {}, e;
  for(e in a) {
    d[e] = b.call(c, a[e], e, a)
  }
  return d
};
goog.object.some = function(a, b, c) {
  for(var d in a) {
    if(b.call(c, a[d], d, a)) {
      return!0
    }
  }
  return!1
};
goog.object.every = function(a, b, c) {
  for(var d in a) {
    if(!b.call(c, a[d], d, a)) {
      return!1
    }
  }
  return!0
};
goog.object.getCount = function(a) {
  var b = 0, c;
  for(c in a) {
    b++
  }
  return b
};
goog.object.getAnyKey = function(a) {
  for(var b in a) {
    return b
  }
};
goog.object.getAnyValue = function(a) {
  for(var b in a) {
    return a[b]
  }
};
goog.object.contains = function(a, b) {
  return goog.object.containsValue(a, b)
};
goog.object.getValues = function(a) {
  var b = [], c = 0, d;
  for(d in a) {
    b[c++] = a[d]
  }
  return b
};
goog.object.getKeys = function(a) {
  var b = [], c = 0, d;
  for(d in a) {
    b[c++] = d
  }
  return b
};
goog.object.getValueByKeys = function(a, b) {
  for(var c = goog.isArrayLike(b), d = c ? b : arguments, c = c ? 0 : 1;c < d.length;c++) {
    if(a = a[d[c]], !goog.isDef(a)) {
      break
    }
  }
  return a
};
goog.object.containsKey = function(a, b) {
  return b in a
};
goog.object.containsValue = function(a, b) {
  for(var c in a) {
    if(a[c] == b) {
      return!0
    }
  }
  return!1
};
goog.object.findKey = function(a, b, c) {
  for(var d in a) {
    if(b.call(c, a[d], d, a)) {
      return d
    }
  }
};
goog.object.findValue = function(a, b, c) {
  return(b = goog.object.findKey(a, b, c)) && a[b]
};
goog.object.isEmpty = function(a) {
  for(var b in a) {
    return!1
  }
  return!0
};
goog.object.clear = function(a) {
  for(var b in a) {
    delete a[b]
  }
};
goog.object.remove = function(a, b) {
  var c;
  (c = b in a) && delete a[b];
  return c
};
goog.object.add = function(a, b, c) {
  if(b in a) {
    throw Error('The object already contains the key "' + b + '"');
  }
  goog.object.set(a, b, c)
};
goog.object.get = function(a, b, c) {
  return b in a ? a[b] : c
};
goog.object.set = function(a, b, c) {
  a[b] = c
};
goog.object.setIfUndefined = function(a, b, c) {
  return b in a ? a[b] : a[b] = c
};
goog.object.clone = function(a) {
  var b = {}, c;
  for(c in a) {
    b[c] = a[c]
  }
  return b
};
goog.object.unsafeClone = function(a) {
  var b = goog.typeOf(a);
  if(b == "object" || b == "array") {
    if(a.clone) {
      return a.clone()
    }
    var b = b == "array" ? [] : {}, c;
    for(c in a) {
      b[c] = goog.object.unsafeClone(a[c])
    }
    return b
  }
  return a
};
goog.object.transpose = function(a) {
  var b = {}, c;
  for(c in a) {
    b[a[c]] = c
  }
  return b
};
goog.object.PROTOTYPE_FIELDS_ = "constructor,hasOwnProperty,isPrototypeOf,propertyIsEnumerable,toLocaleString,toString,valueOf".split(",");
goog.object.extend = function(a, b) {
  for(var c, d, e = 1;e < arguments.length;e++) {
    d = arguments[e];
    for(c in d) {
      a[c] = d[c]
    }
    for(var f = 0;f < goog.object.PROTOTYPE_FIELDS_.length;f++) {
      c = goog.object.PROTOTYPE_FIELDS_[f], Object.prototype.hasOwnProperty.call(d, c) && (a[c] = d[c])
    }
  }
};
goog.object.create = function(a) {
  var b = arguments.length;
  if(b == 1 && goog.isArray(arguments[0])) {
    return goog.object.create.apply(null, arguments[0])
  }
  if(b % 2) {
    throw Error("Uneven number of arguments");
  }
  for(var c = {}, d = 0;d < b;d += 2) {
    c[arguments[d]] = arguments[d + 1]
  }
  return c
};
goog.object.createSet = function(a) {
  var b = arguments.length;
  if(b == 1 && goog.isArray(arguments[0])) {
    return goog.object.createSet.apply(null, arguments[0])
  }
  for(var c = {}, d = 0;d < b;d++) {
    c[arguments[d]] = !0
  }
  return c
};
goog.dom.ASSUME_QUIRKS_MODE = !1;
goog.dom.ASSUME_STANDARDS_MODE = !1;
goog.dom.COMPAT_MODE_KNOWN_ = goog.dom.ASSUME_QUIRKS_MODE || goog.dom.ASSUME_STANDARDS_MODE;
goog.dom.NodeType = {ELEMENT:1, ATTRIBUTE:2, TEXT:3, CDATA_SECTION:4, ENTITY_REFERENCE:5, ENTITY:6, PROCESSING_INSTRUCTION:7, COMMENT:8, DOCUMENT:9, DOCUMENT_TYPE:10, DOCUMENT_FRAGMENT:11, NOTATION:12};
goog.dom.getDomHelper = function(a) {
  return a ? new goog.dom.DomHelper(goog.dom.getOwnerDocument(a)) : goog.dom.defaultDomHelper_ || (goog.dom.defaultDomHelper_ = new goog.dom.DomHelper)
};
goog.dom.getDocument = function() {
  return document
};
goog.dom.getElement = function(a) {
  return goog.isString(a) ? document.getElementById(a) : a
};
goog.dom.$ = goog.dom.getElement;
goog.dom.getElementsByTagNameAndClass = function(a, b, c) {
  return goog.dom.getElementsByTagNameAndClass_(document, a, b, c)
};
goog.dom.getElementsByClass = function(a, b) {
  var c = b || document;
  if(goog.dom.canUseQuerySelector_(c)) {
    return c.querySelectorAll("." + a)
  }else {
    if(c.getElementsByClassName) {
      return c.getElementsByClassName(a)
    }
  }
  return goog.dom.getElementsByTagNameAndClass_(document, "*", a, b)
};
goog.dom.getElementByClass = function(a, b) {
  var c = b || document, d = null;
  return(d = goog.dom.canUseQuerySelector_(c) ? c.querySelector("." + a) : goog.dom.getElementsByClass(a, b)[0]) || null
};
goog.dom.canUseQuerySelector_ = function(a) {
  return a.querySelectorAll && a.querySelector && (!goog.userAgent.WEBKIT || goog.dom.isCss1CompatMode_(document) || goog.userAgent.isVersion("528"))
};
goog.dom.getElementsByTagNameAndClass_ = function(a, b, c, d) {
  a = d || a;
  b = b && b != "*" ? b.toUpperCase() : "";
  if(goog.dom.canUseQuerySelector_(a) && (b || c)) {
    return a.querySelectorAll(b + (c ? "." + c : ""))
  }
  if(c && a.getElementsByClassName) {
    if(a = a.getElementsByClassName(c), b) {
      for(var d = {}, e = 0, f = 0, g;g = a[f];f++) {
        b == g.nodeName && (d[e++] = g)
      }
      d.length = e;
      return d
    }else {
      return a
    }
  }
  a = a.getElementsByTagName(b || "*");
  if(c) {
    d = {};
    for(f = e = 0;g = a[f];f++) {
      b = g.className, typeof b.split == "function" && goog.array.contains(b.split(/\s+/), c) && (d[e++] = g)
    }
    d.length = e;
    return d
  }else {
    return a
  }
};
goog.dom.$$ = goog.dom.getElementsByTagNameAndClass;
goog.dom.setProperties = function(a, b) {
  goog.object.forEach(b, function(b, d) {
    d == "style" ? a.style.cssText = b : d == "class" ? a.className = b : d == "for" ? a.htmlFor = b : d in goog.dom.DIRECT_ATTRIBUTE_MAP_ ? a.setAttribute(goog.dom.DIRECT_ATTRIBUTE_MAP_[d], b) : goog.string.startsWith(d, "aria-") ? a.setAttribute(d, b) : a[d] = b
  })
};
goog.dom.DIRECT_ATTRIBUTE_MAP_ = {cellpadding:"cellPadding", cellspacing:"cellSpacing", colspan:"colSpan", rowspan:"rowSpan", valign:"vAlign", height:"height", width:"width", usemap:"useMap", frameborder:"frameBorder", maxlength:"maxLength", type:"type"};
goog.dom.getViewportSize = function(a) {
  return goog.dom.getViewportSize_(a || window)
};
goog.dom.getViewportSize_ = function(a) {
  var b = a.document;
  if(goog.userAgent.WEBKIT && !goog.userAgent.isVersion("500") && !goog.userAgent.MOBILE) {
    typeof a.innerHeight == "undefined" && (a = window);
    var b = a.innerHeight, c = a.document.documentElement.scrollHeight;
    a == a.top && c < b && (b -= 15);
    return new goog.math.Size(a.innerWidth, b)
  }
  a = goog.dom.isCss1CompatMode_(b) ? b.documentElement : b.body;
  return new goog.math.Size(a.clientWidth, a.clientHeight)
};
goog.dom.getDocumentHeight = function() {
  return goog.dom.getDocumentHeight_(window)
};
goog.dom.getDocumentHeight_ = function(a) {
  var b = a.document, c = 0;
  if(b) {
    var a = goog.dom.getViewportSize_(a).height, c = b.body, d = b.documentElement;
    if(goog.dom.isCss1CompatMode_(b) && d.scrollHeight) {
      c = d.scrollHeight != a ? d.scrollHeight : d.offsetHeight
    }else {
      var b = d.scrollHeight, e = d.offsetHeight;
      if(d.clientHeight != e) {
        b = c.scrollHeight, e = c.offsetHeight
      }
      c = b > a ? b > e ? b : e : b < e ? b : e
    }
  }
  return c
};
goog.dom.getPageScroll = function(a) {
  return goog.dom.getDomHelper((a || goog.global || window).document).getDocumentScroll()
};
goog.dom.getDocumentScroll = function() {
  return goog.dom.getDocumentScroll_(document)
};
goog.dom.getDocumentScroll_ = function(a) {
  var b = goog.dom.getDocumentScrollElement_(a), a = goog.dom.getWindow_(a);
  return new goog.math.Coordinate(a.pageXOffset || b.scrollLeft, a.pageYOffset || b.scrollTop)
};
goog.dom.getDocumentScrollElement = function() {
  return goog.dom.getDocumentScrollElement_(document)
};
goog.dom.getDocumentScrollElement_ = function(a) {
  return!goog.userAgent.WEBKIT && goog.dom.isCss1CompatMode_(a) ? a.documentElement : a.body
};
goog.dom.getWindow = function(a) {
  return a ? goog.dom.getWindow_(a) : window
};
goog.dom.getWindow_ = function(a) {
  return a.parentWindow || a.defaultView
};
goog.dom.createDom = function(a, b, c) {
  return goog.dom.createDom_(document, arguments)
};
goog.dom.createDom_ = function(a, b) {
  var c = b[0], d = b[1];
  if(!goog.dom.BrowserFeature.CAN_ADD_NAME_OR_TYPE_ATTRIBUTES && d && (d.name || d.type)) {
    c = ["<", c];
    d.name && c.push(' name="', goog.string.htmlEscape(d.name), '"');
    if(d.type) {
      c.push(' type="', goog.string.htmlEscape(d.type), '"');
      var e = {};
      goog.object.extend(e, d);
      d = e;
      delete d.type
    }
    c.push(">");
    c = c.join("")
  }
  c = a.createElement(c);
  if(d) {
    goog.isString(d) ? c.className = d : goog.isArray(d) ? goog.dom.classes.add.apply(null, [c].concat(d)) : goog.dom.setProperties(c, d)
  }
  b.length > 2 && goog.dom.append_(a, c, b, 2);
  return c
};
goog.dom.append_ = function(a, b, c, d) {
  function e(c) {
    c && b.appendChild(goog.isString(c) ? a.createTextNode(c) : c)
  }
  for(;d < c.length;d++) {
    var f = c[d];
    goog.isArrayLike(f) && !goog.dom.isNodeLike(f) ? goog.array.forEach(goog.dom.isNodeList(f) ? goog.array.clone(f) : f, e) : e(f)
  }
};
goog.dom.$dom = goog.dom.createDom;
goog.dom.createElement = function(a) {
  return document.createElement(a)
};
goog.dom.createTextNode = function(a) {
  return document.createTextNode(a)
};
goog.dom.createTable = function(a, b, c) {
  return goog.dom.createTable_(document, a, b, !!c)
};
goog.dom.createTable_ = function(a, b, c, d) {
  for(var e = ["<tr>"], f = 0;f < c;f++) {
    e.push(d ? "<td>&nbsp;</td>" : "<td></td>")
  }
  e.push("</tr>");
  e = e.join("");
  c = ["<table>"];
  for(f = 0;f < b;f++) {
    c.push(e)
  }
  c.push("</table>");
  a = a.createElement(goog.dom.TagName.DIV);
  a.innerHTML = c.join("");
  return a.removeChild(a.firstChild)
};
goog.dom.htmlToDocumentFragment = function(a) {
  return goog.dom.htmlToDocumentFragment_(document, a)
};
goog.dom.htmlToDocumentFragment_ = function(a, b) {
  var c = a.createElement("div");
  goog.dom.BrowserFeature.INNER_HTML_NEEDS_SCOPED_ELEMENT ? (c.innerHTML = "<br>" + b, c.removeChild(c.firstChild)) : c.innerHTML = b;
  if(c.childNodes.length == 1) {
    return c.removeChild(c.firstChild)
  }else {
    for(var d = a.createDocumentFragment();c.firstChild;) {
      d.appendChild(c.firstChild)
    }
    return d
  }
};
goog.dom.getCompatMode = function() {
  return goog.dom.isCss1CompatMode() ? "CSS1Compat" : "BackCompat"
};
goog.dom.isCss1CompatMode = function() {
  return goog.dom.isCss1CompatMode_(document)
};
goog.dom.isCss1CompatMode_ = function(a) {
  return goog.dom.COMPAT_MODE_KNOWN_ ? goog.dom.ASSUME_STANDARDS_MODE : a.compatMode == "CSS1Compat"
};
goog.dom.canHaveChildren = function(a) {
  if(a.nodeType != goog.dom.NodeType.ELEMENT) {
    return!1
  }
  switch(a.tagName) {
    case goog.dom.TagName.APPLET:
    ;
    case goog.dom.TagName.AREA:
    ;
    case goog.dom.TagName.BASE:
    ;
    case goog.dom.TagName.BR:
    ;
    case goog.dom.TagName.COL:
    ;
    case goog.dom.TagName.FRAME:
    ;
    case goog.dom.TagName.HR:
    ;
    case goog.dom.TagName.IMG:
    ;
    case goog.dom.TagName.INPUT:
    ;
    case goog.dom.TagName.IFRAME:
    ;
    case goog.dom.TagName.ISINDEX:
    ;
    case goog.dom.TagName.LINK:
    ;
    case goog.dom.TagName.NOFRAMES:
    ;
    case goog.dom.TagName.NOSCRIPT:
    ;
    case goog.dom.TagName.META:
    ;
    case goog.dom.TagName.OBJECT:
    ;
    case goog.dom.TagName.PARAM:
    ;
    case goog.dom.TagName.SCRIPT:
    ;
    case goog.dom.TagName.STYLE:
      return!1
  }
  return!0
};
goog.dom.appendChild = function(a, b) {
  a.appendChild(b)
};
goog.dom.append = function(a, b) {
  goog.dom.append_(goog.dom.getOwnerDocument(a), a, arguments, 1)
};
goog.dom.removeChildren = function(a) {
  for(var b;b = a.firstChild;) {
    a.removeChild(b)
  }
};
goog.dom.insertSiblingBefore = function(a, b) {
  b.parentNode && b.parentNode.insertBefore(a, b)
};
goog.dom.insertSiblingAfter = function(a, b) {
  b.parentNode && b.parentNode.insertBefore(a, b.nextSibling)
};
goog.dom.insertChildAt = function(a, b, c) {
  a.insertBefore(b, a.childNodes[c] || null)
};
goog.dom.removeNode = function(a) {
  return a && a.parentNode ? a.parentNode.removeChild(a) : null
};
goog.dom.replaceNode = function(a, b) {
  var c = b.parentNode;
  c && c.replaceChild(a, b)
};
goog.dom.flattenElement = function(a) {
  var b, c = a.parentNode;
  if(c && c.nodeType != goog.dom.NodeType.DOCUMENT_FRAGMENT) {
    if(a.removeNode) {
      return a.removeNode(!1)
    }else {
      for(;b = a.firstChild;) {
        c.insertBefore(b, a)
      }
      return goog.dom.removeNode(a)
    }
  }
};
goog.dom.getChildren = function(a) {
  return goog.dom.BrowserFeature.CAN_USE_CHILDREN_ATTRIBUTE && a.children != void 0 ? a.children : goog.array.filter(a.childNodes, function(a) {
    return a.nodeType == goog.dom.NodeType.ELEMENT
  })
};
goog.dom.getFirstElementChild = function(a) {
  return a.firstElementChild != void 0 ? a.firstElementChild : goog.dom.getNextElementNode_(a.firstChild, !0)
};
goog.dom.getLastElementChild = function(a) {
  return a.lastElementChild != void 0 ? a.lastElementChild : goog.dom.getNextElementNode_(a.lastChild, !1)
};
goog.dom.getNextElementSibling = function(a) {
  return a.nextElementSibling != void 0 ? a.nextElementSibling : goog.dom.getNextElementNode_(a.nextSibling, !0)
};
goog.dom.getPreviousElementSibling = function(a) {
  return a.previousElementSibling != void 0 ? a.previousElementSibling : goog.dom.getNextElementNode_(a.previousSibling, !1)
};
goog.dom.getNextElementNode_ = function(a, b) {
  for(;a && a.nodeType != goog.dom.NodeType.ELEMENT;) {
    a = b ? a.nextSibling : a.previousSibling
  }
  return a
};
goog.dom.getNextNode = function(a) {
  if(!a) {
    return null
  }
  if(a.firstChild) {
    return a.firstChild
  }
  for(;a && !a.nextSibling;) {
    a = a.parentNode
  }
  return a ? a.nextSibling : null
};
goog.dom.getPreviousNode = function(a) {
  if(!a) {
    return null
  }
  if(!a.previousSibling) {
    return a.parentNode
  }
  for(a = a.previousSibling;a && a.lastChild;) {
    a = a.lastChild
  }
  return a
};
goog.dom.isNodeLike = function(a) {
  return goog.isObject(a) && a.nodeType > 0
};
goog.dom.isElement = function(a) {
  return goog.isObject(a) && a.nodeType == goog.dom.NodeType.ELEMENT
};
goog.dom.isWindow = function(a) {
  return goog.isObject(a) && a.window == a
};
goog.dom.contains = function(a, b) {
  if(a.contains && b.nodeType == goog.dom.NodeType.ELEMENT) {
    return a == b || a.contains(b)
  }
  if(typeof a.compareDocumentPosition != "undefined") {
    return a == b || Boolean(a.compareDocumentPosition(b) & 16)
  }
  for(;b && a != b;) {
    b = b.parentNode
  }
  return b == a
};
goog.dom.compareNodeOrder = function(a, b) {
  if(a == b) {
    return 0
  }
  if(a.compareDocumentPosition) {
    return a.compareDocumentPosition(b) & 2 ? 1 : -1
  }
  if("sourceIndex" in a || a.parentNode && "sourceIndex" in a.parentNode) {
    var c = a.nodeType == goog.dom.NodeType.ELEMENT, d = b.nodeType == goog.dom.NodeType.ELEMENT;
    if(c && d) {
      return a.sourceIndex - b.sourceIndex
    }else {
      var e = a.parentNode, f = b.parentNode;
      return e == f ? goog.dom.compareSiblingOrder_(a, b) : !c && goog.dom.contains(e, b) ? -1 * goog.dom.compareParentsDescendantNodeIe_(a, b) : !d && goog.dom.contains(f, a) ? goog.dom.compareParentsDescendantNodeIe_(b, a) : (c ? a.sourceIndex : e.sourceIndex) - (d ? b.sourceIndex : f.sourceIndex)
    }
  }
  d = goog.dom.getOwnerDocument(a);
  c = d.createRange();
  c.selectNode(a);
  c.collapse(!0);
  d = d.createRange();
  d.selectNode(b);
  d.collapse(!0);
  return c.compareBoundaryPoints(goog.global.Range.START_TO_END, d)
};
goog.dom.compareParentsDescendantNodeIe_ = function(a, b) {
  var c = a.parentNode;
  if(c == b) {
    return-1
  }
  for(var d = b;d.parentNode != c;) {
    d = d.parentNode
  }
  return goog.dom.compareSiblingOrder_(d, a)
};
goog.dom.compareSiblingOrder_ = function(a, b) {
  for(var c = b;c = c.previousSibling;) {
    if(c == a) {
      return-1
    }
  }
  return 1
};
goog.dom.findCommonAncestor = function(a) {
  var b, c = arguments.length;
  if(c) {
    if(c == 1) {
      return arguments[0]
    }
  }else {
    return null
  }
  var d = [], e = Infinity;
  for(b = 0;b < c;b++) {
    for(var f = [], g = arguments[b];g;) {
      f.unshift(g), g = g.parentNode
    }
    d.push(f);
    e = Math.min(e, f.length)
  }
  f = null;
  for(b = 0;b < e;b++) {
    for(var g = d[0][b], h = 1;h < c;h++) {
      if(g != d[h][b]) {
        return f
      }
    }
    f = g
  }
  return f
};
goog.dom.getOwnerDocument = function(a) {
  return a.nodeType == goog.dom.NodeType.DOCUMENT ? a : a.ownerDocument || a.document
};
goog.dom.getFrameContentDocument = function(a) {
  return goog.userAgent.WEBKIT ? a.document || a.contentWindow.document : a.contentDocument || a.contentWindow.document
};
goog.dom.getFrameContentWindow = function(a) {
  return a.contentWindow || goog.dom.getWindow_(goog.dom.getFrameContentDocument(a))
};
goog.dom.setTextContent = function(a, b) {
  if("textContent" in a) {
    a.textContent = b
  }else {
    if(a.firstChild && a.firstChild.nodeType == goog.dom.NodeType.TEXT) {
      for(;a.lastChild != a.firstChild;) {
        a.removeChild(a.lastChild)
      }
      a.firstChild.data = b
    }else {
      goog.dom.removeChildren(a);
      var c = goog.dom.getOwnerDocument(a);
      a.appendChild(c.createTextNode(b))
    }
  }
};
goog.dom.getOuterHtml = function(a) {
  if("outerHTML" in a) {
    return a.outerHTML
  }else {
    var b = goog.dom.getOwnerDocument(a).createElement("div");
    b.appendChild(a.cloneNode(!0));
    return b.innerHTML
  }
};
goog.dom.findNode = function(a, b) {
  var c = [];
  return goog.dom.findNodes_(a, b, c, !0) ? c[0] : void 0
};
goog.dom.findNodes = function(a, b) {
  var c = [];
  goog.dom.findNodes_(a, b, c, !1);
  return c
};
goog.dom.findNodes_ = function(a, b, c, d) {
  if(a != null) {
    for(a = a.firstChild;a;) {
      if(b(a) && (c.push(a), d)) {
        return!0
      }
      if(goog.dom.findNodes_(a, b, c, d)) {
        return!0
      }
      a = a.nextSibling
    }
  }
  return!1
};
goog.dom.TAGS_TO_IGNORE_ = {SCRIPT:1, STYLE:1, HEAD:1, IFRAME:1, OBJECT:1};
goog.dom.PREDEFINED_TAG_VALUES_ = {IMG:" ", BR:"\n"};
goog.dom.isFocusableTabIndex = function(a) {
  var b = a.getAttributeNode("tabindex");
  return b && b.specified ? (a = a.tabIndex, goog.isNumber(a) && a >= 0 && a < 32768) : !1
};
goog.dom.setFocusableTabIndex = function(a, b) {
  b ? a.tabIndex = 0 : a.removeAttribute("tabIndex")
};
goog.dom.getTextContent = function(a) {
  if(goog.dom.BrowserFeature.CAN_USE_INNER_TEXT && "innerText" in a) {
    a = goog.string.canonicalizeNewlines(a.innerText)
  }else {
    var b = [];
    goog.dom.getTextContent_(a, b, !0);
    a = b.join("")
  }
  a = a.replace(/ \xAD /g, " ").replace(/\xAD/g, "");
  a = a.replace(/\u200B/g, "");
  goog.dom.BrowserFeature.CAN_USE_INNER_TEXT || (a = a.replace(/ +/g, " "));
  a != " " && (a = a.replace(/^\s*/, ""));
  return a
};
goog.dom.getRawTextContent = function(a) {
  var b = [];
  goog.dom.getTextContent_(a, b, !1);
  return b.join("")
};
goog.dom.getTextContent_ = function(a, b, c) {
  if(!(a.nodeName in goog.dom.TAGS_TO_IGNORE_)) {
    if(a.nodeType == goog.dom.NodeType.TEXT) {
      c ? b.push(String(a.nodeValue).replace(/(\r\n|\r|\n)/g, "")) : b.push(a.nodeValue)
    }else {
      if(a.nodeName in goog.dom.PREDEFINED_TAG_VALUES_) {
        b.push(goog.dom.PREDEFINED_TAG_VALUES_[a.nodeName])
      }else {
        for(a = a.firstChild;a;) {
          goog.dom.getTextContent_(a, b, c), a = a.nextSibling
        }
      }
    }
  }
};
goog.dom.getNodeTextLength = function(a) {
  return goog.dom.getTextContent(a).length
};
goog.dom.getNodeTextOffset = function(a, b) {
  for(var c = b || goog.dom.getOwnerDocument(a).body, d = [];a && a != c;) {
    for(var e = a;e = e.previousSibling;) {
      d.unshift(goog.dom.getTextContent(e))
    }
    a = a.parentNode
  }
  return goog.string.trimLeft(d.join("")).replace(/ +/g, " ").length
};
goog.dom.getNodeAtOffset = function(a, b, c) {
  for(var a = [a], d = 0, e;a.length > 0 && d < b;) {
    if(e = a.pop(), !(e.nodeName in goog.dom.TAGS_TO_IGNORE_)) {
      if(e.nodeType == goog.dom.NodeType.TEXT) {
        var f = e.nodeValue.replace(/(\r\n|\r|\n)/g, "").replace(/ +/g, " ");
        d += f.length
      }else {
        if(e.nodeName in goog.dom.PREDEFINED_TAG_VALUES_) {
          d += goog.dom.PREDEFINED_TAG_VALUES_[e.nodeName].length
        }else {
          for(f = e.childNodes.length - 1;f >= 0;f--) {
            a.push(e.childNodes[f])
          }
        }
      }
    }
  }
  if(goog.isObject(c)) {
    c.remainder = e ? e.nodeValue.length + b - d - 1 : 0, c.node = e
  }
  return e
};
goog.dom.isNodeList = function(a) {
  if(a && typeof a.length == "number") {
    if(goog.isObject(a)) {
      return typeof a.item == "function" || typeof a.item == "string"
    }else {
      if(goog.isFunction(a)) {
        return typeof a.item == "function"
      }
    }
  }
  return!1
};
goog.dom.getAncestorByTagNameAndClass = function(a, b, c) {
  var d = b ? b.toUpperCase() : null;
  return goog.dom.getAncestor(a, function(a) {
    return(!d || a.nodeName == d) && (!c || goog.dom.classes.has(a, c))
  }, !0)
};
goog.dom.getAncestorByClass = function(a, b) {
  return goog.dom.getAncestorByTagNameAndClass(a, null, b)
};
goog.dom.getAncestor = function(a, b, c, d) {
  if(!c) {
    a = a.parentNode
  }
  for(var c = d == null, e = 0;a && (c || e <= d);) {
    if(b(a)) {
      return a
    }
    a = a.parentNode;
    e++
  }
  return null
};
goog.dom.DomHelper = function(a) {
  this.document_ = a || goog.global.document || document
};
goog.dom.DomHelper.prototype.getDomHelper = goog.dom.getDomHelper;
goog.dom.DomHelper.prototype.setDocument = function(a) {
  this.document_ = a
};
goog.dom.DomHelper.prototype.getDocument = function() {
  return this.document_
};
goog.dom.DomHelper.prototype.getElement = function(a) {
  return goog.isString(a) ? this.document_.getElementById(a) : a
};
goog.dom.DomHelper.prototype.$ = goog.dom.DomHelper.prototype.getElement;
goog.dom.DomHelper.prototype.getElementsByTagNameAndClass = function(a, b, c) {
  return goog.dom.getElementsByTagNameAndClass_(this.document_, a, b, c)
};
goog.dom.DomHelper.prototype.getElementsByClass = function(a, b) {
  return goog.dom.getElementsByClass(a, b || this.document_)
};
goog.dom.DomHelper.prototype.getElementByClass = function(a, b) {
  return goog.dom.getElementByClass(a, b || this.document_)
};
goog.dom.DomHelper.prototype.$$ = goog.dom.DomHelper.prototype.getElementsByTagNameAndClass;
goog.dom.DomHelper.prototype.setProperties = goog.dom.setProperties;
goog.dom.DomHelper.prototype.getViewportSize = function(a) {
  return goog.dom.getViewportSize(a || this.getWindow())
};
goog.dom.DomHelper.prototype.getDocumentHeight = function() {
  return goog.dom.getDocumentHeight_(this.getWindow())
};
goog.dom.DomHelper.prototype.createDom = function(a, b, c) {
  return goog.dom.createDom_(this.document_, arguments)
};
goog.dom.DomHelper.prototype.$dom = goog.dom.DomHelper.prototype.createDom;
goog.dom.DomHelper.prototype.createElement = function(a) {
  return this.document_.createElement(a)
};
goog.dom.DomHelper.prototype.createTextNode = function(a) {
  return this.document_.createTextNode(a)
};
goog.dom.DomHelper.prototype.createTable = function(a, b, c) {
  return goog.dom.createTable_(this.document_, a, b, !!c)
};
goog.dom.DomHelper.prototype.htmlToDocumentFragment = function(a) {
  return goog.dom.htmlToDocumentFragment_(this.document_, a)
};
goog.dom.DomHelper.prototype.getCompatMode = function() {
  return this.isCss1CompatMode() ? "CSS1Compat" : "BackCompat"
};
goog.dom.DomHelper.prototype.isCss1CompatMode = function() {
  return goog.dom.isCss1CompatMode_(this.document_)
};
goog.dom.DomHelper.prototype.getWindow = function() {
  return goog.dom.getWindow_(this.document_)
};
goog.dom.DomHelper.prototype.getDocumentScrollElement = function() {
  return goog.dom.getDocumentScrollElement_(this.document_)
};
goog.dom.DomHelper.prototype.getDocumentScroll = function() {
  return goog.dom.getDocumentScroll_(this.document_)
};
goog.dom.DomHelper.prototype.appendChild = goog.dom.appendChild;
goog.dom.DomHelper.prototype.append = goog.dom.append;
goog.dom.DomHelper.prototype.removeChildren = goog.dom.removeChildren;
goog.dom.DomHelper.prototype.insertSiblingBefore = goog.dom.insertSiblingBefore;
goog.dom.DomHelper.prototype.insertSiblingAfter = goog.dom.insertSiblingAfter;
goog.dom.DomHelper.prototype.removeNode = goog.dom.removeNode;
goog.dom.DomHelper.prototype.replaceNode = goog.dom.replaceNode;
goog.dom.DomHelper.prototype.flattenElement = goog.dom.flattenElement;
goog.dom.DomHelper.prototype.getFirstElementChild = goog.dom.getFirstElementChild;
goog.dom.DomHelper.prototype.getLastElementChild = goog.dom.getLastElementChild;
goog.dom.DomHelper.prototype.getNextElementSibling = goog.dom.getNextElementSibling;
goog.dom.DomHelper.prototype.getPreviousElementSibling = goog.dom.getPreviousElementSibling;
goog.dom.DomHelper.prototype.getNextNode = goog.dom.getNextNode;
goog.dom.DomHelper.prototype.getPreviousNode = goog.dom.getPreviousNode;
goog.dom.DomHelper.prototype.isNodeLike = goog.dom.isNodeLike;
goog.dom.DomHelper.prototype.contains = goog.dom.contains;
goog.dom.DomHelper.prototype.getOwnerDocument = goog.dom.getOwnerDocument;
goog.dom.DomHelper.prototype.getFrameContentDocument = goog.dom.getFrameContentDocument;
goog.dom.DomHelper.prototype.getFrameContentWindow = goog.dom.getFrameContentWindow;
goog.dom.DomHelper.prototype.setTextContent = goog.dom.setTextContent;
goog.dom.DomHelper.prototype.findNode = goog.dom.findNode;
goog.dom.DomHelper.prototype.findNodes = goog.dom.findNodes;
goog.dom.DomHelper.prototype.getTextContent = goog.dom.getTextContent;
goog.dom.DomHelper.prototype.getNodeTextLength = goog.dom.getNodeTextLength;
goog.dom.DomHelper.prototype.getNodeTextOffset = goog.dom.getNodeTextOffset;
goog.dom.DomHelper.prototype.getAncestorByTagNameAndClass = goog.dom.getAncestorByTagNameAndClass;
goog.dom.DomHelper.prototype.getAncestorByClass = goog.dom.getAncestorByClass;
goog.dom.DomHelper.prototype.getAncestor = goog.dom.getAncestor;
goog.disposable = {};
goog.disposable.IDisposable = function() {
};
goog.Disposable = function() {
  goog.Disposable.ENABLE_MONITORING && (goog.Disposable.instances_[goog.getUid(this)] = this)
};
goog.Disposable.ENABLE_MONITORING = !1;
goog.Disposable.instances_ = {};
goog.Disposable.getUndisposedObjects = function() {
  var a = [], b;
  for(b in goog.Disposable.instances_) {
    goog.Disposable.instances_.hasOwnProperty(b) && a.push(goog.Disposable.instances_[Number(b)])
  }
  return a
};
goog.Disposable.clearUndisposedObjects = function() {
  goog.Disposable.instances_ = {}
};
goog.Disposable.prototype.disposed_ = !1;
goog.Disposable.prototype.isDisposed = function() {
  return this.disposed_
};
goog.Disposable.prototype.getDisposed = goog.Disposable.prototype.isDisposed;
goog.Disposable.prototype.dispose = function() {
  if(!this.disposed_ && (this.disposed_ = !0, this.disposeInternal(), goog.Disposable.ENABLE_MONITORING)) {
    var a = goog.getUid(this);
    if(!goog.Disposable.instances_.hasOwnProperty(a)) {
      throw Error(this + " did not call the goog.Disposable base constructor or was disposed of after a clearUndisposedObjects call");
    }
    delete goog.Disposable.instances_[a]
  }
};
goog.Disposable.prototype.registerDisposable = function(a) {
  if(!this.dependentDisposables_) {
    this.dependentDisposables_ = []
  }
  this.dependentDisposables_.push(a)
};
goog.Disposable.prototype.disposeInternal = function() {
  this.dependentDisposables_ && goog.disposeAll.apply(null, this.dependentDisposables_)
};
goog.dispose = function(a) {
  a && typeof a.dispose == "function" && a.dispose()
};
goog.disposeAll = function(a) {
  for(var b = 0, c = arguments.length;b < c;++b) {
    var d = arguments[b];
    goog.isArrayLike(d) ? goog.disposeAll.apply(null, d) : goog.dispose(d)
  }
};
goog.debug.entryPointRegistry = {};
goog.debug.EntryPointMonitor = function() {
};
goog.debug.entryPointRegistry.refList_ = [];
goog.debug.entryPointRegistry.register = function(a) {
  goog.debug.entryPointRegistry.refList_[goog.debug.entryPointRegistry.refList_.length] = a
};
goog.debug.entryPointRegistry.monitorAll = function(a) {
  for(var a = goog.bind(a.wrap, a), b = 0;b < goog.debug.entryPointRegistry.refList_.length;b++) {
    goog.debug.entryPointRegistry.refList_[b](a)
  }
};
goog.debug.entryPointRegistry.unmonitorAllIfPossible = function(a) {
  for(var a = goog.bind(a.unwrap, a), b = 0;b < goog.debug.entryPointRegistry.refList_.length;b++) {
    goog.debug.entryPointRegistry.refList_[b](a)
  }
};
goog.debug.errorHandlerWeakDep = {protectEntryPoint:function(a) {
  return a
}};
goog.events = {};
goog.events.BrowserFeature = {HAS_W3C_BUTTON:!goog.userAgent.IE || goog.userAgent.isDocumentMode(9), SET_KEY_CODE_TO_PREVENT_DEFAULT:goog.userAgent.IE && !goog.userAgent.isVersion("8")};
goog.events.Event = function(a, b) {
  goog.Disposable.call(this);
  this.type = a;
  this.currentTarget = this.target = b
};
goog.inherits(goog.events.Event, goog.Disposable);
goog.events.Event.prototype.disposeInternal = function() {
  delete this.type;
  delete this.target;
  delete this.currentTarget
};
goog.events.Event.prototype.propagationStopped_ = !1;
goog.events.Event.prototype.returnValue_ = !0;
goog.events.Event.prototype.stopPropagation = function() {
  this.propagationStopped_ = !0
};
goog.events.Event.prototype.preventDefault = function() {
  this.returnValue_ = !1
};
goog.events.Event.stopPropagation = function(a) {
  a.stopPropagation()
};
goog.events.Event.preventDefault = function(a) {
  a.preventDefault()
};
goog.events.EventType = {CLICK:"click", DBLCLICK:"dblclick", MOUSEDOWN:"mousedown", MOUSEUP:"mouseup", MOUSEOVER:"mouseover", MOUSEOUT:"mouseout", MOUSEMOVE:"mousemove", SELECTSTART:"selectstart", KEYPRESS:"keypress", KEYDOWN:"keydown", KEYUP:"keyup", BLUR:"blur", FOCUS:"focus", DEACTIVATE:"deactivate", FOCUSIN:goog.userAgent.IE ? "focusin" : "DOMFocusIn", FOCUSOUT:goog.userAgent.IE ? "focusout" : "DOMFocusOut", CHANGE:"change", SELECT:"select", SUBMIT:"submit", INPUT:"input", PROPERTYCHANGE:"propertychange", 
DRAGSTART:"dragstart", DRAGENTER:"dragenter", DRAGOVER:"dragover", DRAGLEAVE:"dragleave", DROP:"drop", TOUCHSTART:"touchstart", TOUCHMOVE:"touchmove", TOUCHEND:"touchend", TOUCHCANCEL:"touchcancel", CONTEXTMENU:"contextmenu", ERROR:"error", HELP:"help", LOAD:"load", LOSECAPTURE:"losecapture", READYSTATECHANGE:"readystatechange", RESIZE:"resize", SCROLL:"scroll", UNLOAD:"unload", HASHCHANGE:"hashchange", PAGEHIDE:"pagehide", PAGESHOW:"pageshow", POPSTATE:"popstate", COPY:"copy", PASTE:"paste", CUT:"cut", 
BEFORECOPY:"beforecopy", BEFORECUT:"beforecut", BEFOREPASTE:"beforepaste", MESSAGE:"message", CONNECT:"connect"};
goog.reflect = {};
goog.reflect.object = function(a, b) {
  return b
};
goog.reflect.sinkValue = new Function("a", "return a");
goog.reflect.canAccessProperty = function(a, b) {
  try {
    return goog.reflect.sinkValue(a[b]), !0
  }catch(c) {
  }
  return!1
};
goog.events.BrowserEvent = function(a, b) {
  a && this.init(a, b)
};
goog.inherits(goog.events.BrowserEvent, goog.events.Event);
goog.events.BrowserEvent.MouseButton = {LEFT:0, MIDDLE:1, RIGHT:2};
goog.events.BrowserEvent.IEButtonMap = [1, 4, 2];
goog.events.BrowserEvent.prototype.target = null;
goog.events.BrowserEvent.prototype.relatedTarget = null;
goog.events.BrowserEvent.prototype.offsetX = 0;
goog.events.BrowserEvent.prototype.offsetY = 0;
goog.events.BrowserEvent.prototype.clientX = 0;
goog.events.BrowserEvent.prototype.clientY = 0;
goog.events.BrowserEvent.prototype.screenX = 0;
goog.events.BrowserEvent.prototype.screenY = 0;
goog.events.BrowserEvent.prototype.button = 0;
goog.events.BrowserEvent.prototype.keyCode = 0;
goog.events.BrowserEvent.prototype.charCode = 0;
goog.events.BrowserEvent.prototype.ctrlKey = !1;
goog.events.BrowserEvent.prototype.altKey = !1;
goog.events.BrowserEvent.prototype.shiftKey = !1;
goog.events.BrowserEvent.prototype.metaKey = !1;
goog.events.BrowserEvent.prototype.platformModifierKey = !1;
goog.events.BrowserEvent.prototype.event_ = null;
goog.events.BrowserEvent.prototype.init = function(a, b) {
  var c = this.type = a.type;
  goog.events.Event.call(this, c);
  this.target = a.target || a.srcElement;
  this.currentTarget = b;
  var d = a.relatedTarget;
  if(d) {
    goog.userAgent.GECKO && (goog.reflect.canAccessProperty(d, "nodeName") || (d = null))
  }else {
    if(c == goog.events.EventType.MOUSEOVER) {
      d = a.fromElement
    }else {
      if(c == goog.events.EventType.MOUSEOUT) {
        d = a.toElement
      }
    }
  }
  this.relatedTarget = d;
  this.offsetX = a.offsetX !== void 0 ? a.offsetX : a.layerX;
  this.offsetY = a.offsetY !== void 0 ? a.offsetY : a.layerY;
  this.clientX = a.clientX !== void 0 ? a.clientX : a.pageX;
  this.clientY = a.clientY !== void 0 ? a.clientY : a.pageY;
  this.screenX = a.screenX || 0;
  this.screenY = a.screenY || 0;
  this.button = a.button;
  this.keyCode = a.keyCode || 0;
  this.charCode = a.charCode || (c == "keypress" ? a.keyCode : 0);
  this.ctrlKey = a.ctrlKey;
  this.altKey = a.altKey;
  this.shiftKey = a.shiftKey;
  this.metaKey = a.metaKey;
  this.platformModifierKey = goog.userAgent.MAC ? a.metaKey : a.ctrlKey;
  this.state = a.state;
  this.event_ = a;
  delete this.returnValue_;
  delete this.propagationStopped_
};
goog.events.BrowserEvent.prototype.isButton = function(a) {
  return goog.events.BrowserFeature.HAS_W3C_BUTTON ? this.event_.button == a : this.type == "click" ? a == goog.events.BrowserEvent.MouseButton.LEFT : !!(this.event_.button & goog.events.BrowserEvent.IEButtonMap[a])
};
goog.events.BrowserEvent.prototype.isMouseActionButton = function() {
  return this.isButton(goog.events.BrowserEvent.MouseButton.LEFT) && !(goog.userAgent.WEBKIT && goog.userAgent.MAC && this.ctrlKey)
};
goog.events.BrowserEvent.prototype.stopPropagation = function() {
  goog.events.BrowserEvent.superClass_.stopPropagation.call(this);
  this.event_.stopPropagation ? this.event_.stopPropagation() : this.event_.cancelBubble = !0
};
goog.events.BrowserEvent.prototype.preventDefault = function() {
  goog.events.BrowserEvent.superClass_.preventDefault.call(this);
  var a = this.event_;
  if(a.preventDefault) {
    a.preventDefault()
  }else {
    if(a.returnValue = !1, goog.events.BrowserFeature.SET_KEY_CODE_TO_PREVENT_DEFAULT) {
      try {
        if(a.ctrlKey || a.keyCode >= 112 && a.keyCode <= 123) {
          a.keyCode = -1
        }
      }catch(b) {
      }
    }
  }
};
goog.events.BrowserEvent.prototype.getBrowserEvent = function() {
  return this.event_
};
goog.events.BrowserEvent.prototype.disposeInternal = function() {
  goog.events.BrowserEvent.superClass_.disposeInternal.call(this);
  this.relatedTarget = this.currentTarget = this.target = this.event_ = null
};
goog.events.EventWrapper = function() {
};
goog.events.EventWrapper.prototype.listen = function() {
};
goog.events.EventWrapper.prototype.unlisten = function() {
};
goog.events.Listener = function() {
};
goog.events.Listener.counter_ = 0;
goog.events.Listener.prototype.key = 0;
goog.events.Listener.prototype.removed = !1;
goog.events.Listener.prototype.callOnce = !1;
goog.events.Listener.prototype.init = function(a, b, c, d, e, f) {
  if(goog.isFunction(a)) {
    this.isFunctionListener_ = !0
  }else {
    if(a && a.handleEvent && goog.isFunction(a.handleEvent)) {
      this.isFunctionListener_ = !1
    }else {
      throw Error("Invalid listener argument");
    }
  }
  this.listener = a;
  this.proxy = b;
  this.src = c;
  this.type = d;
  this.capture = !!e;
  this.handler = f;
  this.callOnce = !1;
  this.key = ++goog.events.Listener.counter_;
  this.removed = !1
};
goog.events.Listener.prototype.handleEvent = function(a) {
  return this.isFunctionListener_ ? this.listener.call(this.handler || this.src, a) : this.listener.handleEvent.call(this.listener, a)
};
goog.structs = {};
goog.structs.SimplePool = function(a, b) {
  goog.Disposable.call(this);
  this.maxCount_ = b;
  this.freeQueue_ = [];
  this.createInitial_(a)
};
goog.inherits(goog.structs.SimplePool, goog.Disposable);
goog.structs.SimplePool.prototype.createObjectFn_ = null;
goog.structs.SimplePool.prototype.disposeObjectFn_ = null;
goog.structs.SimplePool.prototype.setCreateObjectFn = function(a) {
  this.createObjectFn_ = a
};
goog.structs.SimplePool.prototype.setDisposeObjectFn = function(a) {
  this.disposeObjectFn_ = a
};
goog.structs.SimplePool.prototype.getObject = function() {
  return this.freeQueue_.length ? this.freeQueue_.pop() : this.createObject()
};
goog.structs.SimplePool.prototype.releaseObject = function(a) {
  this.freeQueue_.length < this.maxCount_ ? this.freeQueue_.push(a) : this.disposeObject(a)
};
goog.structs.SimplePool.prototype.createInitial_ = function(a) {
  if(a > this.maxCount_) {
    throw Error("[goog.structs.SimplePool] Initial cannot be greater than max");
  }
  for(var b = 0;b < a;b++) {
    this.freeQueue_.push(this.createObject())
  }
};
goog.structs.SimplePool.prototype.createObject = function() {
  return this.createObjectFn_ ? this.createObjectFn_() : {}
};
goog.structs.SimplePool.prototype.disposeObject = function(a) {
  if(this.disposeObjectFn_) {
    this.disposeObjectFn_(a)
  }else {
    if(goog.isObject(a)) {
      if(goog.isFunction(a.dispose)) {
        a.dispose()
      }else {
        for(var b in a) {
          delete a[b]
        }
      }
    }
  }
};
goog.structs.SimplePool.prototype.disposeInternal = function() {
  goog.structs.SimplePool.superClass_.disposeInternal.call(this);
  for(var a = this.freeQueue_;a.length;) {
    this.disposeObject(a.pop())
  }
  delete this.freeQueue_
};
goog.userAgent.jscript = {};
goog.userAgent.jscript.ASSUME_NO_JSCRIPT = !1;
goog.userAgent.jscript.init_ = function() {
  goog.userAgent.jscript.DETECTED_HAS_JSCRIPT_ = "ScriptEngine" in goog.global && goog.global.ScriptEngine() == "JScript";
  goog.userAgent.jscript.DETECTED_VERSION_ = goog.userAgent.jscript.DETECTED_HAS_JSCRIPT_ ? goog.global.ScriptEngineMajorVersion() + "." + goog.global.ScriptEngineMinorVersion() + "." + goog.global.ScriptEngineBuildVersion() : "0"
};
goog.userAgent.jscript.ASSUME_NO_JSCRIPT || goog.userAgent.jscript.init_();
goog.userAgent.jscript.HAS_JSCRIPT = goog.userAgent.jscript.ASSUME_NO_JSCRIPT ? !1 : goog.userAgent.jscript.DETECTED_HAS_JSCRIPT_;
goog.userAgent.jscript.VERSION = goog.userAgent.jscript.ASSUME_NO_JSCRIPT ? "0" : goog.userAgent.jscript.DETECTED_VERSION_;
goog.userAgent.jscript.isVersion = function(a) {
  return goog.string.compareVersions(goog.userAgent.jscript.VERSION, a) >= 0
};
goog.events.pools = {};
goog.events.ASSUME_GOOD_GC = !1;
(function() {
  function a() {
    return{count_:0, remaining_:0}
  }
  function b() {
    return[]
  }
  function c() {
    var a = function(b) {
      b = g.call(a.src, a.key, b);
      if(!b) {
        return b
      }
    };
    return a
  }
  function d() {
    return new goog.events.Listener
  }
  function e() {
    return new goog.events.BrowserEvent
  }
  var f = !goog.events.ASSUME_GOOD_GC && goog.userAgent.jscript.HAS_JSCRIPT && !goog.userAgent.jscript.isVersion("5.7"), g;
  goog.events.pools.setProxyCallbackFunction = function(a) {
    g = a
  };
  if(f) {
    goog.events.pools.getObject = function() {
      return h.getObject()
    };
    goog.events.pools.releaseObject = function(a) {
      h.releaseObject(a)
    };
    goog.events.pools.getArray = function() {
      return i.getObject()
    };
    goog.events.pools.releaseArray = function(a) {
      i.releaseObject(a)
    };
    goog.events.pools.getProxy = function() {
      return j.getObject()
    };
    goog.events.pools.releaseProxy = function() {
      j.releaseObject(c())
    };
    goog.events.pools.getListener = function() {
      return k.getObject()
    };
    goog.events.pools.releaseListener = function(a) {
      k.releaseObject(a)
    };
    goog.events.pools.getEvent = function() {
      return m.getObject()
    };
    goog.events.pools.releaseEvent = function(a) {
      m.releaseObject(a)
    };
    var h = new goog.structs.SimplePool(0, 600);
    h.setCreateObjectFn(a);
    var i = new goog.structs.SimplePool(0, 600);
    i.setCreateObjectFn(b);
    var j = new goog.structs.SimplePool(0, 600);
    j.setCreateObjectFn(c);
    var k = new goog.structs.SimplePool(0, 600);
    k.setCreateObjectFn(d);
    var m = new goog.structs.SimplePool(0, 600);
    m.setCreateObjectFn(e)
  }else {
    goog.events.pools.getObject = a, goog.events.pools.releaseObject = goog.nullFunction, goog.events.pools.getArray = b, goog.events.pools.releaseArray = goog.nullFunction, goog.events.pools.getProxy = c, goog.events.pools.releaseProxy = goog.nullFunction, goog.events.pools.getListener = d, goog.events.pools.releaseListener = goog.nullFunction, goog.events.pools.getEvent = e, goog.events.pools.releaseEvent = goog.nullFunction
  }
})();
goog.events.listeners_ = {};
goog.events.listenerTree_ = {};
goog.events.sources_ = {};
goog.events.onString_ = "on";
goog.events.onStringMap_ = {};
goog.events.keySeparator_ = "_";
goog.events.listen = function(a, b, c, d, e) {
  if(b) {
    if(goog.isArray(b)) {
      for(var f = 0;f < b.length;f++) {
        goog.events.listen(a, b[f], c, d, e)
      }
      return null
    }else {
      var d = !!d, g = goog.events.listenerTree_;
      b in g || (g[b] = goog.events.pools.getObject());
      g = g[b];
      d in g || (g[d] = goog.events.pools.getObject(), g.count_++);
      var g = g[d], h = goog.getUid(a), i;
      g.remaining_++;
      if(g[h]) {
        i = g[h];
        for(f = 0;f < i.length;f++) {
          if(g = i[f], g.listener == c && g.handler == e) {
            if(g.removed) {
              break
            }
            return i[f].key
          }
        }
      }else {
        i = g[h] = goog.events.pools.getArray(), g.count_++
      }
      f = goog.events.pools.getProxy();
      f.src = a;
      g = goog.events.pools.getListener();
      g.init(c, f, a, b, d, e);
      c = g.key;
      f.key = c;
      i.push(g);
      goog.events.listeners_[c] = g;
      goog.events.sources_[h] || (goog.events.sources_[h] = goog.events.pools.getArray());
      goog.events.sources_[h].push(g);
      a.addEventListener ? (a == goog.global || !a.customEvent_) && a.addEventListener(b, f, d) : a.attachEvent(goog.events.getOnString_(b), f);
      return c
    }
  }else {
    throw Error("Invalid event type");
  }
};
goog.events.listenOnce = function(a, b, c, d, e) {
  if(goog.isArray(b)) {
    for(var f = 0;f < b.length;f++) {
      goog.events.listenOnce(a, b[f], c, d, e)
    }
    return null
  }
  a = goog.events.listen(a, b, c, d, e);
  goog.events.listeners_[a].callOnce = !0;
  return a
};
goog.events.listenWithWrapper = function(a, b, c, d, e) {
  b.listen(a, c, d, e)
};
goog.events.unlisten = function(a, b, c, d, e) {
  if(goog.isArray(b)) {
    for(var f = 0;f < b.length;f++) {
      goog.events.unlisten(a, b[f], c, d, e)
    }
    return null
  }
  d = !!d;
  a = goog.events.getListeners_(a, b, d);
  if(!a) {
    return!1
  }
  for(f = 0;f < a.length;f++) {
    if(a[f].listener == c && a[f].capture == d && a[f].handler == e) {
      return goog.events.unlistenByKey(a[f].key)
    }
  }
  return!1
};
goog.events.unlistenByKey = function(a) {
  if(!goog.events.listeners_[a]) {
    return!1
  }
  var b = goog.events.listeners_[a];
  if(b.removed) {
    return!1
  }
  var c = b.src, d = b.type, e = b.proxy, f = b.capture;
  c.removeEventListener ? (c == goog.global || !c.customEvent_) && c.removeEventListener(d, e, f) : c.detachEvent && c.detachEvent(goog.events.getOnString_(d), e);
  c = goog.getUid(c);
  e = goog.events.listenerTree_[d][f][c];
  if(goog.events.sources_[c]) {
    var g = goog.events.sources_[c];
    goog.array.remove(g, b);
    g.length == 0 && delete goog.events.sources_[c]
  }
  b.removed = !0;
  e.needsCleanup_ = !0;
  goog.events.cleanUp_(d, f, c, e);
  delete goog.events.listeners_[a];
  return!0
};
goog.events.unlistenWithWrapper = function(a, b, c, d, e) {
  b.unlisten(a, c, d, e)
};
goog.events.cleanUp_ = function(a, b, c, d) {
  if(!d.locked_ && d.needsCleanup_) {
    for(var e = 0, f = 0;e < d.length;e++) {
      if(d[e].removed) {
        var g = d[e].proxy;
        g.src = null;
        goog.events.pools.releaseProxy(g);
        goog.events.pools.releaseListener(d[e])
      }else {
        e != f && (d[f] = d[e]), f++
      }
    }
    d.length = f;
    d.needsCleanup_ = !1;
    f == 0 && (goog.events.pools.releaseArray(d), delete goog.events.listenerTree_[a][b][c], goog.events.listenerTree_[a][b].count_--, goog.events.listenerTree_[a][b].count_ == 0 && (goog.events.pools.releaseObject(goog.events.listenerTree_[a][b]), delete goog.events.listenerTree_[a][b], goog.events.listenerTree_[a].count_--), goog.events.listenerTree_[a].count_ == 0 && (goog.events.pools.releaseObject(goog.events.listenerTree_[a]), delete goog.events.listenerTree_[a]))
  }
};
goog.events.removeAll = function(a, b, c) {
  var d = 0, e = b == null, f = c == null, c = !!c;
  if(a == null) {
    goog.object.forEach(goog.events.sources_, function(a) {
      for(var g = a.length - 1;g >= 0;g--) {
        var h = a[g];
        if((e || b == h.type) && (f || c == h.capture)) {
          goog.events.unlistenByKey(h.key), d++
        }
      }
    })
  }else {
    if(a = goog.getUid(a), goog.events.sources_[a]) {
      for(var a = goog.events.sources_[a], g = a.length - 1;g >= 0;g--) {
        var h = a[g];
        if((e || b == h.type) && (f || c == h.capture)) {
          goog.events.unlistenByKey(h.key), d++
        }
      }
    }
  }
  return d
};
goog.events.getListeners = function(a, b, c) {
  return goog.events.getListeners_(a, b, c) || []
};
goog.events.getListeners_ = function(a, b, c) {
  var d = goog.events.listenerTree_;
  return b in d && (d = d[b], c in d && (d = d[c], a = goog.getUid(a), d[a])) ? d[a] : null
};
goog.events.getListener = function(a, b, c, d, e) {
  d = !!d;
  if(a = goog.events.getListeners_(a, b, d)) {
    for(b = 0;b < a.length;b++) {
      if(!a[b].removed && a[b].listener == c && a[b].capture == d && a[b].handler == e) {
        return a[b]
      }
    }
  }
  return null
};
goog.events.hasListener = function(a, b, c) {
  var a = goog.getUid(a), d = goog.events.sources_[a];
  if(d) {
    var e = goog.isDef(b), f = goog.isDef(c);
    return e && f ? (d = goog.events.listenerTree_[b], !!d && !!d[c] && a in d[c]) : !e && !f ? !0 : goog.array.some(d, function(a) {
      return e && a.type == b || f && a.capture == c
    })
  }
  return!1
};
goog.events.expose = function(a) {
  var b = [], c;
  for(c in a) {
    a[c] && a[c].id ? b.push(c + " = " + a[c] + " (" + a[c].id + ")") : b.push(c + " = " + a[c])
  }
  return b.join("\n")
};
goog.events.getOnString_ = function(a) {
  return a in goog.events.onStringMap_ ? goog.events.onStringMap_[a] : goog.events.onStringMap_[a] = goog.events.onString_ + a
};
goog.events.fireListeners = function(a, b, c, d) {
  var e = goog.events.listenerTree_;
  return b in e && (e = e[b], c in e) ? goog.events.fireListeners_(e[c], a, b, c, d) : !0
};
goog.events.fireListeners_ = function(a, b, c, d, e) {
  var f = 1, b = goog.getUid(b);
  if(a[b]) {
    a.remaining_--;
    a = a[b];
    a.locked_ ? a.locked_++ : a.locked_ = 1;
    try {
      for(var g = a.length, h = 0;h < g;h++) {
        var i = a[h];
        i && !i.removed && (f &= goog.events.fireListener(i, e) !== !1)
      }
    }finally {
      a.locked_--, goog.events.cleanUp_(c, d, b, a)
    }
  }
  return Boolean(f)
};
goog.events.fireListener = function(a, b) {
  var c = a.handleEvent(b);
  a.callOnce && goog.events.unlistenByKey(a.key);
  return c
};
goog.events.getTotalListenerCount = function() {
  return goog.object.getCount(goog.events.listeners_)
};
goog.events.dispatchEvent = function(a, b) {
  var c = b.type || b, d = goog.events.listenerTree_;
  if(!(c in d)) {
    return!0
  }
  if(goog.isString(b)) {
    b = new goog.events.Event(b, a)
  }else {
    if(b instanceof goog.events.Event) {
      b.target = b.target || a
    }else {
      var e = b, b = new goog.events.Event(c, a);
      goog.object.extend(b, e)
    }
  }
  var e = 1, f, d = d[c], c = !0 in d, g;
  if(c) {
    f = [];
    for(g = a;g;g = g.getParentEventTarget()) {
      f.push(g)
    }
    g = d[!0];
    g.remaining_ = g.count_;
    for(var h = f.length - 1;!b.propagationStopped_ && h >= 0 && g.remaining_;h--) {
      b.currentTarget = f[h], e &= goog.events.fireListeners_(g, f[h], b.type, !0, b) && b.returnValue_ != !1
    }
  }
  if(!1 in d) {
    if(g = d[!1], g.remaining_ = g.count_, c) {
      for(h = 0;!b.propagationStopped_ && h < f.length && g.remaining_;h++) {
        b.currentTarget = f[h], e &= goog.events.fireListeners_(g, f[h], b.type, !1, b) && b.returnValue_ != !1
      }
    }else {
      for(d = a;!b.propagationStopped_ && d && g.remaining_;d = d.getParentEventTarget()) {
        b.currentTarget = d, e &= goog.events.fireListeners_(g, d, b.type, !1, b) && b.returnValue_ != !1
      }
    }
  }
  return Boolean(e)
};
goog.events.protectBrowserEventEntryPoint = function(a) {
  goog.events.handleBrowserEvent_ = a.protectEntryPoint(goog.events.handleBrowserEvent_);
  goog.events.pools.setProxyCallbackFunction(goog.events.handleBrowserEvent_)
};
goog.events.handleBrowserEvent_ = function(a, b) {
  if(!goog.events.listeners_[a]) {
    return!0
  }
  var c = goog.events.listeners_[a], d = c.type, e = goog.events.listenerTree_;
  if(!(d in e)) {
    return!0
  }
  var e = e[d], f, g;
  if(goog.events.synthesizeEventPropagation_()) {
    f = b || goog.getObjectByName("window.event");
    var h = !0 in e, i = !1 in e;
    if(h) {
      if(goog.events.isMarkedIeEvent_(f)) {
        return!0
      }
      goog.events.markIeEvent_(f)
    }
    var j = goog.events.pools.getEvent();
    j.init(f, this);
    f = !0;
    try {
      if(h) {
        for(var k = goog.events.pools.getArray(), m = j.currentTarget;m;m = m.parentNode) {
          k.push(m)
        }
        g = e[!0];
        g.remaining_ = g.count_;
        for(var l = k.length - 1;!j.propagationStopped_ && l >= 0 && g.remaining_;l--) {
          j.currentTarget = k[l], f &= goog.events.fireListeners_(g, k[l], d, !0, j)
        }
        if(i) {
          g = e[!1];
          g.remaining_ = g.count_;
          for(l = 0;!j.propagationStopped_ && l < k.length && g.remaining_;l++) {
            j.currentTarget = k[l], f &= goog.events.fireListeners_(g, k[l], d, !1, j)
          }
        }
      }else {
        f = goog.events.fireListener(c, j)
      }
    }finally {
      if(k) {
        k.length = 0, goog.events.pools.releaseArray(k)
      }
      j.dispose();
      goog.events.pools.releaseEvent(j)
    }
    return f
  }
  d = new goog.events.BrowserEvent(b, this);
  try {
    f = goog.events.fireListener(c, d)
  }finally {
    d.dispose()
  }
  return f
};
goog.events.pools.setProxyCallbackFunction(goog.events.handleBrowserEvent_);
goog.events.markIeEvent_ = function(a) {
  var b = !1;
  if(a.keyCode == 0) {
    try {
      a.keyCode = -1;
      return
    }catch(c) {
      b = !0
    }
  }
  if(b || a.returnValue == void 0) {
    a.returnValue = !0
  }
};
goog.events.isMarkedIeEvent_ = function(a) {
  return a.keyCode < 0 || a.returnValue != void 0
};
goog.events.uniqueIdCounter_ = 0;
goog.events.getUniqueId = function(a) {
  return a + "_" + goog.events.uniqueIdCounter_++
};
goog.events.synthesizeEventPropagation_ = function() {
  if(goog.events.requiresSyntheticEventPropagation_ === void 0) {
    goog.events.requiresSyntheticEventPropagation_ = goog.userAgent.IE && !goog.global.addEventListener
  }
  return goog.events.requiresSyntheticEventPropagation_
};
goog.debug.entryPointRegistry.register(function(a) {
  goog.events.handleBrowserEvent_ = a(goog.events.handleBrowserEvent_);
  goog.events.pools.setProxyCallbackFunction(goog.events.handleBrowserEvent_)
});
goog.events.EventHandler = function(a) {
  goog.Disposable.call(this);
  this.handler_ = a;
  this.keys_ = []
};
goog.inherits(goog.events.EventHandler, goog.Disposable);
goog.events.EventHandler.typeArray_ = [];
goog.events.EventHandler.prototype.listen = function(a, b, c, d, e) {
  if(!goog.isArray(b)) {
    goog.events.EventHandler.typeArray_[0] = b, b = goog.events.EventHandler.typeArray_
  }
  for(var f = 0;f < b.length;f++) {
    this.keys_.push(goog.events.listen(a, b[f], c || this, d || !1, e || this.handler_ || this))
  }
  return this
};
goog.events.EventHandler.prototype.listenOnce = function(a, b, c, d, e) {
  if(goog.isArray(b)) {
    for(var f = 0;f < b.length;f++) {
      this.listenOnce(a, b[f], c, d, e)
    }
  }else {
    this.keys_.push(goog.events.listenOnce(a, b, c || this, d, e || this.handler_ || this))
  }
  return this
};
goog.events.EventHandler.prototype.listenWithWrapper = function(a, b, c, d, e) {
  b.listen(a, c, d, e || this.handler_, this);
  return this
};
goog.events.EventHandler.prototype.getListenerCount = function() {
  return this.keys_.length
};
goog.events.EventHandler.prototype.unlisten = function(a, b, c, d, e) {
  if(goog.isArray(b)) {
    for(var f = 0;f < b.length;f++) {
      this.unlisten(a, b[f], c, d, e)
    }
  }else {
    if(a = goog.events.getListener(a, b, c || this, d, e || this.handler_ || this)) {
      a = a.key, goog.events.unlistenByKey(a), goog.array.remove(this.keys_, a)
    }
  }
  return this
};
goog.events.EventHandler.prototype.unlistenWithWrapper = function(a, b, c, d, e) {
  b.unlisten(a, c, d, e || this.handler_, this);
  return this
};
goog.events.EventHandler.prototype.removeAll = function() {
  goog.array.forEach(this.keys_, goog.events.unlistenByKey);
  this.keys_.length = 0
};
goog.events.EventHandler.prototype.disposeInternal = function() {
  goog.events.EventHandler.superClass_.disposeInternal.call(this);
  this.removeAll()
};
goog.events.EventHandler.prototype.handleEvent = function() {
  throw Error("EventHandler.handleEvent not implemented");
};
goog.events.EventTarget = function() {
  goog.Disposable.call(this)
};
goog.inherits(goog.events.EventTarget, goog.Disposable);
goog.events.EventTarget.prototype.customEvent_ = !0;
goog.events.EventTarget.prototype.parentEventTarget_ = null;
goog.events.EventTarget.prototype.getParentEventTarget = function() {
  return this.parentEventTarget_
};
goog.events.EventTarget.prototype.setParentEventTarget = function(a) {
  this.parentEventTarget_ = a
};
goog.events.EventTarget.prototype.addEventListener = function(a, b, c, d) {
  goog.events.listen(this, a, b, c, d)
};
goog.events.EventTarget.prototype.removeEventListener = function(a, b, c, d) {
  goog.events.unlisten(this, a, b, c, d)
};
goog.events.EventTarget.prototype.dispatchEvent = function(a) {
  return goog.events.dispatchEvent(this, a)
};
goog.events.EventTarget.prototype.disposeInternal = function() {
  goog.events.EventTarget.superClass_.disposeInternal.call(this);
  goog.events.removeAll(this);
  this.parentEventTarget_ = null
};
goog.math.Box = function(a, b, c, d) {
  this.top = a;
  this.right = b;
  this.bottom = c;
  this.left = d
};
goog.math.Box.boundingBox = function(a) {
  for(var b = new goog.math.Box(arguments[0].y, arguments[0].x, arguments[0].y, arguments[0].x), c = 1;c < arguments.length;c++) {
    var d = arguments[c];
    b.top = Math.min(b.top, d.y);
    b.right = Math.max(b.right, d.x);
    b.bottom = Math.max(b.bottom, d.y);
    b.left = Math.min(b.left, d.x)
  }
  return b
};
goog.math.Box.prototype.clone = function() {
  return new goog.math.Box(this.top, this.right, this.bottom, this.left)
};
if(goog.DEBUG) {
  goog.math.Box.prototype.toString = function() {
    return"(" + this.top + "t, " + this.right + "r, " + this.bottom + "b, " + this.left + "l)"
  }
}
goog.math.Box.prototype.contains = function(a) {
  return goog.math.Box.contains(this, a)
};
goog.math.Box.prototype.expand = function(a, b, c, d) {
  goog.isObject(a) ? (this.top -= a.top, this.right += a.right, this.bottom += a.bottom, this.left -= a.left) : (this.top -= a, this.right += b, this.bottom += c, this.left -= d);
  return this
};
goog.math.Box.prototype.expandToInclude = function(a) {
  this.left = Math.min(this.left, a.left);
  this.top = Math.min(this.top, a.top);
  this.right = Math.max(this.right, a.right);
  this.bottom = Math.max(this.bottom, a.bottom)
};
goog.math.Box.equals = function(a, b) {
  return a == b ? !0 : !a || !b ? !1 : a.top == b.top && a.right == b.right && a.bottom == b.bottom && a.left == b.left
};
goog.math.Box.contains = function(a, b) {
  return!a || !b ? !1 : b instanceof goog.math.Box ? b.left >= a.left && b.right <= a.right && b.top >= a.top && b.bottom <= a.bottom : b.x >= a.left && b.x <= a.right && b.y >= a.top && b.y <= a.bottom
};
goog.math.Box.distance = function(a, b) {
  return b.x >= a.left && b.x <= a.right ? b.y >= a.top && b.y <= a.bottom ? 0 : b.y < a.top ? a.top - b.y : b.y - a.bottom : b.y >= a.top && b.y <= a.bottom ? b.x < a.left ? a.left - b.x : b.x - a.right : goog.math.Coordinate.distance(b, new goog.math.Coordinate(b.x < a.left ? a.left : a.right, b.y < a.top ? a.top : a.bottom))
};
goog.math.Box.intersects = function(a, b) {
  return a.left <= b.right && b.left <= a.right && a.top <= b.bottom && b.top <= a.bottom
};
goog.math.Box.intersectsWithPadding = function(a, b, c) {
  return a.left <= b.right + c && b.left <= a.right + c && a.top <= b.bottom + c && b.top <= a.bottom + c
};
goog.math.Rect = function(a, b, c, d) {
  this.left = a;
  this.top = b;
  this.width = c;
  this.height = d
};
goog.math.Rect.prototype.clone = function() {
  return new goog.math.Rect(this.left, this.top, this.width, this.height)
};
goog.math.Rect.prototype.toBox = function() {
  return new goog.math.Box(this.top, this.left + this.width, this.top + this.height, this.left)
};
goog.math.Rect.createFromBox = function(a) {
  return new goog.math.Rect(a.left, a.top, a.right - a.left, a.bottom - a.top)
};
if(goog.DEBUG) {
  goog.math.Rect.prototype.toString = function() {
    return"(" + this.left + ", " + this.top + " - " + this.width + "w x " + this.height + "h)"
  }
}
goog.math.Rect.equals = function(a, b) {
  return a == b ? !0 : !a || !b ? !1 : a.left == b.left && a.width == b.width && a.top == b.top && a.height == b.height
};
goog.math.Rect.prototype.intersection = function(a) {
  var b = Math.max(this.left, a.left), c = Math.min(this.left + this.width, a.left + a.width);
  if(b <= c) {
    var d = Math.max(this.top, a.top), a = Math.min(this.top + this.height, a.top + a.height);
    if(d <= a) {
      return this.left = b, this.top = d, this.width = c - b, this.height = a - d, !0
    }
  }
  return!1
};
goog.math.Rect.intersection = function(a, b) {
  var c = Math.max(a.left, b.left), d = Math.min(a.left + a.width, b.left + b.width);
  if(c <= d) {
    var e = Math.max(a.top, b.top), f = Math.min(a.top + a.height, b.top + b.height);
    if(e <= f) {
      return new goog.math.Rect(c, e, d - c, f - e)
    }
  }
  return null
};
goog.math.Rect.intersects = function(a, b) {
  return a.left <= b.left + b.width && b.left <= a.left + a.width && a.top <= b.top + b.height && b.top <= a.top + a.height
};
goog.math.Rect.prototype.intersects = function(a) {
  return goog.math.Rect.intersects(this, a)
};
goog.math.Rect.difference = function(a, b) {
  var c = goog.math.Rect.intersection(a, b);
  if(!c || !c.height || !c.width) {
    return[a.clone()]
  }
  var c = [], d = a.top, e = a.height, f = a.left + a.width, g = a.top + a.height, h = b.left + b.width, i = b.top + b.height;
  if(b.top > a.top) {
    c.push(new goog.math.Rect(a.left, a.top, a.width, b.top - a.top)), d = b.top, e -= b.top - a.top
  }
  i < g && (c.push(new goog.math.Rect(a.left, i, a.width, g - i)), e = i - d);
  b.left > a.left && c.push(new goog.math.Rect(a.left, d, b.left - a.left, e));
  h < f && c.push(new goog.math.Rect(h, d, f - h, e));
  return c
};
goog.math.Rect.prototype.difference = function(a) {
  return goog.math.Rect.difference(this, a)
};
goog.math.Rect.prototype.boundingRect = function(a) {
  var b = Math.max(this.left + this.width, a.left + a.width), c = Math.max(this.top + this.height, a.top + a.height);
  this.left = Math.min(this.left, a.left);
  this.top = Math.min(this.top, a.top);
  this.width = b - this.left;
  this.height = c - this.top
};
goog.math.Rect.boundingRect = function(a, b) {
  if(!a || !b) {
    return null
  }
  var c = a.clone();
  c.boundingRect(b);
  return c
};
goog.math.Rect.prototype.contains = function(a) {
  return a instanceof goog.math.Rect ? this.left <= a.left && this.left + this.width >= a.left + a.width && this.top <= a.top && this.top + this.height >= a.top + a.height : a.x >= this.left && a.x <= this.left + this.width && a.y >= this.top && a.y <= this.top + this.height
};
goog.math.Rect.prototype.getSize = function() {
  return new goog.math.Size(this.width, this.height)
};
goog.style = {};
goog.style.setStyle = function(a, b, c) {
  goog.isString(b) ? goog.style.setStyle_(a, c, b) : goog.object.forEach(b, goog.partial(goog.style.setStyle_, a))
};
goog.style.setStyle_ = function(a, b, c) {
  a.style[goog.string.toCamelCase(c)] = b
};
goog.style.getStyle = function(a, b) {
  return a.style[goog.string.toCamelCase(b)] || ""
};
goog.style.getComputedStyle = function(a, b) {
  var c = goog.dom.getOwnerDocument(a);
  return c.defaultView && c.defaultView.getComputedStyle && (c = c.defaultView.getComputedStyle(a, null)) ? c[b] || c.getPropertyValue(b) : ""
};
goog.style.getCascadedStyle = function(a, b) {
  return a.currentStyle ? a.currentStyle[b] : null
};
goog.style.getStyle_ = function(a, b) {
  return goog.style.getComputedStyle(a, b) || goog.style.getCascadedStyle(a, b) || a.style[b]
};
goog.style.getComputedPosition = function(a) {
  return goog.style.getStyle_(a, "position")
};
goog.style.getBackgroundColor = function(a) {
  return goog.style.getStyle_(a, "backgroundColor")
};
goog.style.getComputedOverflowX = function(a) {
  return goog.style.getStyle_(a, "overflowX")
};
goog.style.getComputedOverflowY = function(a) {
  return goog.style.getStyle_(a, "overflowY")
};
goog.style.getComputedZIndex = function(a) {
  return goog.style.getStyle_(a, "zIndex")
};
goog.style.getComputedTextAlign = function(a) {
  return goog.style.getStyle_(a, "textAlign")
};
goog.style.getComputedCursor = function(a) {
  return goog.style.getStyle_(a, "cursor")
};
goog.style.setPosition = function(a, b, c) {
  var d, e = goog.userAgent.GECKO && (goog.userAgent.MAC || goog.userAgent.X11) && goog.userAgent.isVersion("1.9");
  b instanceof goog.math.Coordinate ? (d = b.x, b = b.y) : (d = b, b = c);
  a.style.left = goog.style.getPixelStyleValue_(d, e);
  a.style.top = goog.style.getPixelStyleValue_(b, e)
};
goog.style.getPosition = function(a) {
  return new goog.math.Coordinate(a.offsetLeft, a.offsetTop)
};
goog.style.getClientViewportElement = function(a) {
  a = a ? a.nodeType == goog.dom.NodeType.DOCUMENT ? a : goog.dom.getOwnerDocument(a) : goog.dom.getDocument();
  return goog.userAgent.IE && !goog.userAgent.isDocumentMode(9) && !goog.dom.getDomHelper(a).isCss1CompatMode() ? a.body : a.documentElement
};
goog.style.getBoundingClientRect_ = function(a) {
  var b = a.getBoundingClientRect();
  if(goog.userAgent.IE) {
    a = a.ownerDocument, b.left -= a.documentElement.clientLeft + a.body.clientLeft, b.top -= a.documentElement.clientTop + a.body.clientTop
  }
  return b
};
goog.style.getOffsetParent = function(a) {
  if(goog.userAgent.IE) {
    return a.offsetParent
  }
  for(var b = goog.dom.getOwnerDocument(a), c = goog.style.getStyle_(a, "position"), d = c == "fixed" || c == "absolute", a = a.parentNode;a && a != b;a = a.parentNode) {
    if(c = goog.style.getStyle_(a, "position"), d = d && c == "static" && a != b.documentElement && a != b.body, !d && (a.scrollWidth > a.clientWidth || a.scrollHeight > a.clientHeight || c == "fixed" || c == "absolute" || c == "relative")) {
      return a
    }
  }
  return null
};
goog.style.getVisibleRectForElement = function(a) {
  for(var b = new goog.math.Box(0, Infinity, Infinity, 0), c = goog.dom.getDomHelper(a), d = c.getDocument().body, e = c.getDocumentScrollElement(), f;a = goog.style.getOffsetParent(a);) {
    if((!goog.userAgent.IE || a.clientWidth != 0) && (!goog.userAgent.WEBKIT || a.clientHeight != 0 || a != d) && (a.scrollWidth != a.clientWidth || a.scrollHeight != a.clientHeight) && goog.style.getStyle_(a, "overflow") != "visible") {
      var g = goog.style.getPageOffset(a), h = goog.style.getClientLeftTop(a);
      g.x += h.x;
      g.y += h.y;
      b.top = Math.max(b.top, g.y);
      b.right = Math.min(b.right, g.x + a.clientWidth);
      b.bottom = Math.min(b.bottom, g.y + a.clientHeight);
      b.left = Math.max(b.left, g.x);
      f = f || a != e
    }
  }
  d = e.scrollLeft;
  e = e.scrollTop;
  goog.userAgent.WEBKIT ? (b.left += d, b.top += e) : (b.left = Math.max(b.left, d), b.top = Math.max(b.top, e));
  if(!f || goog.userAgent.WEBKIT) {
    b.right += d, b.bottom += e
  }
  c = c.getViewportSize();
  b.right = Math.min(b.right, d + c.width);
  b.bottom = Math.min(b.bottom, e + c.height);
  return b.top >= 0 && b.left >= 0 && b.bottom > b.top && b.right > b.left ? b : null
};
goog.style.scrollIntoContainerView = function(a, b, c) {
  var d = goog.style.getPageOffset(a), e = goog.style.getPageOffset(b), f = goog.style.getBorderBox(b), g = d.x - e.x - f.left, d = d.y - e.y - f.top, e = b.clientWidth - a.offsetWidth, a = b.clientHeight - a.offsetHeight;
  c ? (b.scrollLeft += g - e / 2, b.scrollTop += d - a / 2) : (b.scrollLeft += Math.min(g, Math.max(g - e, 0)), b.scrollTop += Math.min(d, Math.max(d - a, 0)))
};
goog.style.getClientLeftTop = function(a) {
  if(goog.userAgent.GECKO && !goog.userAgent.isVersion("1.9")) {
    var b = parseFloat(goog.style.getComputedStyle(a, "borderLeftWidth"));
    if(goog.style.isRightToLeft(a)) {
      var c = a.offsetWidth - a.clientWidth - b - parseFloat(goog.style.getComputedStyle(a, "borderRightWidth"));
      b += c
    }
    return new goog.math.Coordinate(b, parseFloat(goog.style.getComputedStyle(a, "borderTopWidth")))
  }
  return new goog.math.Coordinate(a.clientLeft, a.clientTop)
};
goog.style.getPageOffset = function(a) {
  var b, c = goog.dom.getOwnerDocument(a), d = goog.style.getStyle_(a, "position"), e = goog.userAgent.GECKO && c.getBoxObjectFor && !a.getBoundingClientRect && d == "absolute" && (b = c.getBoxObjectFor(a)) && (b.screenX < 0 || b.screenY < 0), f = new goog.math.Coordinate(0, 0), g = goog.style.getClientViewportElement(c);
  if(a == g) {
    return f
  }
  if(a.getBoundingClientRect) {
    b = goog.style.getBoundingClientRect_(a), a = goog.dom.getDomHelper(c).getDocumentScroll(), f.x = b.left + a.x, f.y = b.top + a.y
  }else {
    if(c.getBoxObjectFor && !e) {
      b = c.getBoxObjectFor(a), a = c.getBoxObjectFor(g), f.x = b.screenX - a.screenX, f.y = b.screenY - a.screenY
    }else {
      b = a;
      do {
        f.x += b.offsetLeft;
        f.y += b.offsetTop;
        b != a && (f.x += b.clientLeft || 0, f.y += b.clientTop || 0);
        if(goog.userAgent.WEBKIT && goog.style.getComputedPosition(b) == "fixed") {
          f.x += c.body.scrollLeft;
          f.y += c.body.scrollTop;
          break
        }
        b = b.offsetParent
      }while(b && b != a);
      if(goog.userAgent.OPERA || goog.userAgent.WEBKIT && d == "absolute") {
        f.y -= c.body.offsetTop
      }
      for(b = a;(b = goog.style.getOffsetParent(b)) && b != c.body && b != g;) {
        if(f.x -= b.scrollLeft, !goog.userAgent.OPERA || b.tagName != "TR") {
          f.y -= b.scrollTop
        }
      }
    }
  }
  return f
};
goog.style.getPageOffsetLeft = function(a) {
  return goog.style.getPageOffset(a).x
};
goog.style.getPageOffsetTop = function(a) {
  return goog.style.getPageOffset(a).y
};
goog.style.getFramedPageOffset = function(a, b) {
  var c = new goog.math.Coordinate(0, 0), d = goog.dom.getWindow(goog.dom.getOwnerDocument(a)), e = a;
  do {
    var f = d == b ? goog.style.getPageOffset(e) : goog.style.getClientPosition(e);
    c.x += f.x;
    c.y += f.y
  }while(d && d != b && (e = d.frameElement) && (d = d.parent));
  return c
};
goog.style.translateRectForAnotherFrame = function(a, b, c) {
  if(b.getDocument() != c.getDocument()) {
    var d = b.getDocument().body, c = goog.style.getFramedPageOffset(d, c.getWindow()), c = goog.math.Coordinate.difference(c, goog.style.getPageOffset(d));
    goog.userAgent.IE && !b.isCss1CompatMode() && (c = goog.math.Coordinate.difference(c, b.getDocumentScroll()));
    a.left += c.x;
    a.top += c.y
  }
};
goog.style.getRelativePosition = function(a, b) {
  var c = goog.style.getClientPosition(a), d = goog.style.getClientPosition(b);
  return new goog.math.Coordinate(c.x - d.x, c.y - d.y)
};
goog.style.getClientPosition = function(a) {
  var b = new goog.math.Coordinate;
  if(a.nodeType == goog.dom.NodeType.ELEMENT) {
    if(a.getBoundingClientRect) {
      a = goog.style.getBoundingClientRect_(a), b.x = a.left, b.y = a.top
    }else {
      var c = goog.dom.getDomHelper(a).getDocumentScroll(), a = goog.style.getPageOffset(a);
      b.x = a.x - c.x;
      b.y = a.y - c.y
    }
  }else {
    var c = goog.isFunction(a.getBrowserEvent), d = a;
    a.targetTouches ? d = a.targetTouches[0] : c && a.getBrowserEvent().targetTouches && (d = a.getBrowserEvent().targetTouches[0]);
    b.x = d.clientX;
    b.y = d.clientY
  }
  return b
};
goog.style.setPageOffset = function(a, b, c) {
  var d = goog.style.getPageOffset(a);
  if(b instanceof goog.math.Coordinate) {
    c = b.y, b = b.x
  }
  goog.style.setPosition(a, a.offsetLeft + (b - d.x), a.offsetTop + (c - d.y))
};
goog.style.setSize = function(a, b, c) {
  if(b instanceof goog.math.Size) {
    c = b.height, b = b.width
  }else {
    if(c == void 0) {
      throw Error("missing height argument");
    }
  }
  goog.style.setWidth(a, b);
  goog.style.setHeight(a, c)
};
goog.style.getPixelStyleValue_ = function(a, b) {
  typeof a == "number" && (a = (b ? Math.round(a) : a) + "px");
  return a
};
goog.style.setHeight = function(a, b) {
  a.style.height = goog.style.getPixelStyleValue_(b, !0)
};
goog.style.setWidth = function(a, b) {
  a.style.width = goog.style.getPixelStyleValue_(b, !0)
};
goog.style.getSize = function(a) {
  if(goog.style.getStyle_(a, "display") != "none") {
    return goog.style.getSizeWithDisplay_(a)
  }
  var b = a.style, c = b.display, d = b.visibility, e = b.position;
  b.visibility = "hidden";
  b.position = "absolute";
  b.display = "inline";
  a = goog.style.getSizeWithDisplay_(a);
  b.display = c;
  b.position = e;
  b.visibility = d;
  return a
};
goog.style.getSizeWithDisplay_ = function(a) {
  var b = a.offsetWidth, c = a.offsetHeight, d = goog.userAgent.WEBKIT && !b && !c;
  return(!goog.isDef(b) || d) && a.getBoundingClientRect ? (a = goog.style.getBoundingClientRect_(a), new goog.math.Size(a.right - a.left, a.bottom - a.top)) : new goog.math.Size(b, c)
};
goog.style.getBounds = function(a) {
  var b = goog.style.getPageOffset(a), a = goog.style.getSize(a);
  return new goog.math.Rect(b.x, b.y, a.width, a.height)
};
goog.style.toCamelCase = function(a) {
  return goog.string.toCamelCase(String(a))
};
goog.style.toSelectorCase = function(a) {
  return goog.string.toSelectorCase(a)
};
goog.style.getOpacity = function(a) {
  var b = a.style, a = "";
  "opacity" in b ? a = b.opacity : "MozOpacity" in b ? a = b.MozOpacity : "filter" in b && (b = b.filter.match(/alpha\(opacity=([\d.]+)\)/)) && (a = String(b[1] / 100));
  return a == "" ? a : Number(a)
};
goog.style.setOpacity = function(a, b) {
  var c = a.style;
  if("opacity" in c) {
    c.opacity = b
  }else {
    if("MozOpacity" in c) {
      c.MozOpacity = b
    }else {
      if("filter" in c) {
        c.filter = b === "" ? "" : "alpha(opacity=" + b * 100 + ")"
      }
    }
  }
};
goog.style.setTransparentBackgroundImage = function(a, b) {
  var c = a.style;
  goog.userAgent.IE && !goog.userAgent.isVersion("8") ? c.filter = 'progid:DXImageTransform.Microsoft.AlphaImageLoader(src="' + b + '", sizingMethod="crop")' : (c.backgroundImage = "url(" + b + ")", c.backgroundPosition = "top left", c.backgroundRepeat = "no-repeat")
};
goog.style.clearTransparentBackgroundImage = function(a) {
  a = a.style;
  "filter" in a ? a.filter = "" : a.backgroundImage = "none"
};
goog.style.showElement = function(a, b) {
  a.style.display = b ? "" : "none"
};
goog.style.isElementShown = function(a) {
  return a.style.display != "none"
};
goog.style.installStyles = function(a, b) {
  var c = goog.dom.getDomHelper(b), d = null;
  if(goog.userAgent.IE) {
    d = c.getDocument().createStyleSheet(), goog.style.setStyles(d, a)
  }else {
    var e = c.getElementsByTagNameAndClass("head")[0];
    e || (d = c.getElementsByTagNameAndClass("body")[0], e = c.createDom("head"), d.parentNode.insertBefore(e, d));
    d = c.createDom("style");
    goog.style.setStyles(d, a);
    c.appendChild(e, d)
  }
  return d
};
goog.style.uninstallStyles = function(a) {
  goog.dom.removeNode(a.ownerNode || a.owningElement || a)
};
goog.style.setStyles = function(a, b) {
  goog.userAgent.IE ? a.cssText = b : a[goog.userAgent.WEBKIT ? "innerText" : "innerHTML"] = b
};
goog.style.setPreWrap = function(a) {
  a = a.style;
  goog.userAgent.IE && !goog.userAgent.isVersion("8") ? (a.whiteSpace = "pre", a.wordWrap = "break-word") : a.whiteSpace = goog.userAgent.GECKO ? "-moz-pre-wrap" : "pre-wrap"
};
goog.style.setInlineBlock = function(a) {
  a = a.style;
  a.position = "relative";
  goog.userAgent.IE && !goog.userAgent.isVersion("8") ? (a.zoom = "1", a.display = "inline") : a.display = goog.userAgent.GECKO ? goog.userAgent.isVersion("1.9a") ? "inline-block" : "-moz-inline-box" : "inline-block"
};
goog.style.isRightToLeft = function(a) {
  return"rtl" == goog.style.getStyle_(a, "direction")
};
goog.style.unselectableStyle_ = goog.userAgent.GECKO ? "MozUserSelect" : goog.userAgent.WEBKIT ? "WebkitUserSelect" : null;
goog.style.isUnselectable = function(a) {
  if(goog.style.unselectableStyle_) {
    return a.style[goog.style.unselectableStyle_].toLowerCase() == "none"
  }else {
    if(goog.userAgent.IE || goog.userAgent.OPERA) {
      return a.getAttribute("unselectable") == "on"
    }
  }
  return!1
};
goog.style.setUnselectable = function(a, b, c) {
  var c = !c ? a.getElementsByTagName("*") : null, d = goog.style.unselectableStyle_;
  if(d) {
    if(b = b ? "none" : "", a.style[d] = b, c) {
      for(var a = 0, e;e = c[a];a++) {
        e.style[d] = b
      }
    }
  }else {
    if(goog.userAgent.IE || goog.userAgent.OPERA) {
      if(b = b ? "on" : "", a.setAttribute("unselectable", b), c) {
        for(a = 0;e = c[a];a++) {
          e.setAttribute("unselectable", b)
        }
      }
    }
  }
};
goog.style.getBorderBoxSize = function(a) {
  return new goog.math.Size(a.offsetWidth, a.offsetHeight)
};
goog.style.setBorderBoxSize = function(a, b) {
  var c = goog.dom.getOwnerDocument(a), d = goog.dom.getDomHelper(c).isCss1CompatMode();
  if(goog.userAgent.IE && (!d || !goog.userAgent.isVersion("8"))) {
    if(c = a.style, d) {
      var d = goog.style.getPaddingBox(a), e = goog.style.getBorderBox(a);
      c.pixelWidth = b.width - e.left - d.left - d.right - e.right;
      c.pixelHeight = b.height - e.top - d.top - d.bottom - e.bottom
    }else {
      c.pixelWidth = b.width, c.pixelHeight = b.height
    }
  }else {
    goog.style.setBoxSizingSize_(a, b, "border-box")
  }
};
goog.style.getContentBoxSize = function(a) {
  var b = goog.dom.getOwnerDocument(a), c = goog.userAgent.IE && a.currentStyle;
  return c && goog.dom.getDomHelper(b).isCss1CompatMode() && c.width != "auto" && c.height != "auto" && !c.boxSizing ? (b = goog.style.getIePixelValue_(a, c.width, "width", "pixelWidth"), a = goog.style.getIePixelValue_(a, c.height, "height", "pixelHeight"), new goog.math.Size(b, a)) : (c = goog.style.getBorderBoxSize(a), b = goog.style.getPaddingBox(a), a = goog.style.getBorderBox(a), new goog.math.Size(c.width - a.left - b.left - b.right - a.right, c.height - a.top - b.top - b.bottom - a.bottom))
};
goog.style.setContentBoxSize = function(a, b) {
  var c = goog.dom.getOwnerDocument(a), d = goog.dom.getDomHelper(c).isCss1CompatMode();
  if(goog.userAgent.IE && (!d || !goog.userAgent.isVersion("8"))) {
    if(c = a.style, d) {
      c.pixelWidth = b.width, c.pixelHeight = b.height
    }else {
      var d = goog.style.getPaddingBox(a), e = goog.style.getBorderBox(a);
      c.pixelWidth = b.width + e.left + d.left + d.right + e.right;
      c.pixelHeight = b.height + e.top + d.top + d.bottom + e.bottom
    }
  }else {
    goog.style.setBoxSizingSize_(a, b, "content-box")
  }
};
goog.style.setBoxSizingSize_ = function(a, b, c) {
  a = a.style;
  goog.userAgent.GECKO ? a.MozBoxSizing = c : goog.userAgent.WEBKIT ? a.WebkitBoxSizing = c : a.boxSizing = c;
  a.width = b.width + "px";
  a.height = b.height + "px"
};
goog.style.getIePixelValue_ = function(a, b, c, d) {
  if(/^\d+px?$/.test(b)) {
    return parseInt(b, 10)
  }else {
    var e = a.style[c], f = a.runtimeStyle[c];
    a.runtimeStyle[c] = a.currentStyle[c];
    a.style[c] = b;
    b = a.style[d];
    a.style[c] = e;
    a.runtimeStyle[c] = f;
    return b
  }
};
goog.style.getIePixelDistance_ = function(a, b) {
  return goog.style.getIePixelValue_(a, goog.style.getCascadedStyle(a, b), "left", "pixelLeft")
};
goog.style.getBox_ = function(a, b) {
  if(goog.userAgent.IE) {
    var c = goog.style.getIePixelDistance_(a, b + "Left"), d = goog.style.getIePixelDistance_(a, b + "Right"), e = goog.style.getIePixelDistance_(a, b + "Top"), f = goog.style.getIePixelDistance_(a, b + "Bottom");
    return new goog.math.Box(e, d, f, c)
  }else {
    return c = goog.style.getComputedStyle(a, b + "Left"), d = goog.style.getComputedStyle(a, b + "Right"), e = goog.style.getComputedStyle(a, b + "Top"), f = goog.style.getComputedStyle(a, b + "Bottom"), new goog.math.Box(parseFloat(e), parseFloat(d), parseFloat(f), parseFloat(c))
  }
};
goog.style.getPaddingBox = function(a) {
  return goog.style.getBox_(a, "padding")
};
goog.style.getMarginBox = function(a) {
  return goog.style.getBox_(a, "margin")
};
goog.style.ieBorderWidthKeywords_ = {thin:2, medium:4, thick:6};
goog.style.getIePixelBorder_ = function(a, b) {
  if(goog.style.getCascadedStyle(a, b + "Style") == "none") {
    return 0
  }
  var c = goog.style.getCascadedStyle(a, b + "Width");
  return c in goog.style.ieBorderWidthKeywords_ ? goog.style.ieBorderWidthKeywords_[c] : goog.style.getIePixelValue_(a, c, "left", "pixelLeft")
};
goog.style.getBorderBox = function(a) {
  if(goog.userAgent.IE) {
    var b = goog.style.getIePixelBorder_(a, "borderLeft"), c = goog.style.getIePixelBorder_(a, "borderRight"), d = goog.style.getIePixelBorder_(a, "borderTop"), a = goog.style.getIePixelBorder_(a, "borderBottom");
    return new goog.math.Box(d, c, a, b)
  }else {
    return b = goog.style.getComputedStyle(a, "borderLeftWidth"), c = goog.style.getComputedStyle(a, "borderRightWidth"), d = goog.style.getComputedStyle(a, "borderTopWidth"), a = goog.style.getComputedStyle(a, "borderBottomWidth"), new goog.math.Box(parseFloat(d), parseFloat(c), parseFloat(a), parseFloat(b))
  }
};
goog.style.getFontFamily = function(a) {
  var b = goog.dom.getOwnerDocument(a), c = "";
  if(b.body.createTextRange) {
    b = b.body.createTextRange();
    b.moveToElementText(a);
    try {
      c = b.queryCommandValue("FontName")
    }catch(d) {
      c = ""
    }
  }
  c || (c = goog.style.getStyle_(a, "fontFamily"));
  a = c.split(",");
  a.length > 1 && (c = a[0]);
  return goog.string.stripQuotes(c, "\"'")
};
goog.style.lengthUnitRegex_ = /[^\d]+$/;
goog.style.getLengthUnits = function(a) {
  return(a = a.match(goog.style.lengthUnitRegex_)) && a[0] || null
};
goog.style.ABSOLUTE_CSS_LENGTH_UNITS_ = {cm:1, "in":1, mm:1, pc:1, pt:1};
goog.style.CONVERTIBLE_RELATIVE_CSS_UNITS_ = {em:1, ex:1};
goog.style.getFontSize = function(a) {
  var b = goog.style.getStyle_(a, "fontSize"), c = goog.style.getLengthUnits(b);
  if(b && "px" == c) {
    return parseInt(b, 10)
  }
  if(goog.userAgent.IE) {
    if(c in goog.style.ABSOLUTE_CSS_LENGTH_UNITS_) {
      return goog.style.getIePixelValue_(a, b, "left", "pixelLeft")
    }else {
      if(a.parentNode && a.parentNode.nodeType == goog.dom.NodeType.ELEMENT && c in goog.style.CONVERTIBLE_RELATIVE_CSS_UNITS_) {
        return a = a.parentNode, c = goog.style.getStyle_(a, "fontSize"), goog.style.getIePixelValue_(a, b == c ? "1em" : b, "left", "pixelLeft")
      }
    }
  }
  c = goog.dom.createDom("span", {style:"visibility:hidden;position:absolute;line-height:0;padding:0;margin:0;border:0;height:1em;"});
  goog.dom.appendChild(a, c);
  b = c.offsetHeight;
  goog.dom.removeNode(c);
  return b
};
goog.style.parseStyleAttribute = function(a) {
  var b = {};
  goog.array.forEach(a.split(/\s*;\s*/), function(a) {
    a = a.split(/\s*:\s*/);
    a.length == 2 && (b[goog.string.toCamelCase(a[0].toLowerCase())] = a[1])
  });
  return b
};
goog.style.toStyleAttribute = function(a) {
  var b = [];
  goog.object.forEach(a, function(a, d) {
    b.push(goog.string.toSelectorCase(d), ":", a, ";")
  });
  return b.join("")
};
goog.style.setFloat = function(a, b) {
  a.style[goog.userAgent.IE ? "styleFloat" : "cssFloat"] = b
};
goog.style.getFloat = function(a) {
  return a.style[goog.userAgent.IE ? "styleFloat" : "cssFloat"] || ""
};
goog.style.getScrollbarWidth = function() {
  var a = goog.dom.createElement("div");
  a.style.cssText = "visibility:hidden;overflow:scroll;position:absolute;top:0;width:100px;height:100px";
  goog.dom.appendChild(goog.dom.getDocument().body, a);
  var b = a.offsetWidth - a.clientWidth;
  goog.dom.removeNode(a);
  return b
};
goog.ui = {};
goog.ui.IdGenerator = function() {
};
goog.addSingletonGetter(goog.ui.IdGenerator);
goog.ui.IdGenerator.prototype.nextId_ = 0;
goog.ui.IdGenerator.prototype.getNextUniqueId = function() {
  return":" + (this.nextId_++).toString(36)
};
goog.ui.IdGenerator.instance = goog.ui.IdGenerator.getInstance();
goog.ui.Component = function(a) {
  goog.events.EventTarget.call(this);
  this.dom_ = a || goog.dom.getDomHelper();
  this.rightToLeft_ = goog.ui.Component.defaultRightToLeft_
};
goog.inherits(goog.ui.Component, goog.events.EventTarget);
goog.ui.Component.prototype.idGenerator_ = goog.ui.IdGenerator.getInstance();
goog.ui.Component.defaultRightToLeft_ = null;
goog.ui.Component.EventType = {BEFORE_SHOW:"beforeshow", SHOW:"show", HIDE:"hide", DISABLE:"disable", ENABLE:"enable", HIGHLIGHT:"highlight", UNHIGHLIGHT:"unhighlight", ACTIVATE:"activate", DEACTIVATE:"deactivate", SELECT:"select", UNSELECT:"unselect", CHECK:"check", UNCHECK:"uncheck", FOCUS:"focus", BLUR:"blur", OPEN:"open", CLOSE:"close", ENTER:"enter", LEAVE:"leave", ACTION:"action", CHANGE:"change"};
goog.ui.Component.Error = {NOT_SUPPORTED:"Method not supported", DECORATE_INVALID:"Invalid element to decorate", ALREADY_RENDERED:"Component already rendered", PARENT_UNABLE_TO_BE_SET:"Unable to set parent component", CHILD_INDEX_OUT_OF_BOUNDS:"Child component index out of bounds", NOT_OUR_CHILD:"Child is not in parent component", NOT_IN_DOCUMENT:"Operation not supported while component is not in document", STATE_INVALID:"Invalid component state"};
goog.ui.Component.State = {ALL:255, DISABLED:1, HOVER:2, ACTIVE:4, SELECTED:8, CHECKED:16, FOCUSED:32, OPENED:64};
goog.ui.Component.getStateTransitionEvent = function(a, b) {
  switch(a) {
    case goog.ui.Component.State.DISABLED:
      return b ? goog.ui.Component.EventType.DISABLE : goog.ui.Component.EventType.ENABLE;
    case goog.ui.Component.State.HOVER:
      return b ? goog.ui.Component.EventType.HIGHLIGHT : goog.ui.Component.EventType.UNHIGHLIGHT;
    case goog.ui.Component.State.ACTIVE:
      return b ? goog.ui.Component.EventType.ACTIVATE : goog.ui.Component.EventType.DEACTIVATE;
    case goog.ui.Component.State.SELECTED:
      return b ? goog.ui.Component.EventType.SELECT : goog.ui.Component.EventType.UNSELECT;
    case goog.ui.Component.State.CHECKED:
      return b ? goog.ui.Component.EventType.CHECK : goog.ui.Component.EventType.UNCHECK;
    case goog.ui.Component.State.FOCUSED:
      return b ? goog.ui.Component.EventType.FOCUS : goog.ui.Component.EventType.BLUR;
    case goog.ui.Component.State.OPENED:
      return b ? goog.ui.Component.EventType.OPEN : goog.ui.Component.EventType.CLOSE
  }
  throw Error(goog.ui.Component.Error.STATE_INVALID);
};
goog.ui.Component.setDefaultRightToLeft = function(a) {
  goog.ui.Component.defaultRightToLeft_ = a
};
goog.ui.Component.prototype.id_ = null;
goog.ui.Component.prototype.inDocument_ = !1;
goog.ui.Component.prototype.element_ = null;
goog.ui.Component.prototype.rightToLeft_ = null;
goog.ui.Component.prototype.model_ = null;
goog.ui.Component.prototype.parent_ = null;
goog.ui.Component.prototype.children_ = null;
goog.ui.Component.prototype.childIndex_ = null;
goog.ui.Component.prototype.wasDecorated_ = !1;
goog.ui.Component.prototype.getId = function() {
  return this.id_ || (this.id_ = this.idGenerator_.getNextUniqueId())
};
goog.ui.Component.prototype.setId = function(a) {
  this.parent_ && this.parent_.childIndex_ && (goog.object.remove(this.parent_.childIndex_, this.id_), goog.object.add(this.parent_.childIndex_, a, this));
  this.id_ = a
};
goog.ui.Component.prototype.getElement = function() {
  return this.element_
};
goog.ui.Component.prototype.setElementInternal = function(a) {
  this.element_ = a
};
goog.ui.Component.prototype.getElementsByClass = function(a) {
  return this.element_ ? this.dom_.getElementsByClass(a, this.element_) : []
};
goog.ui.Component.prototype.getElementByClass = function(a) {
  return this.element_ ? this.dom_.getElementByClass(a, this.element_) : null
};
goog.ui.Component.prototype.getHandler = function() {
  return this.googUiComponentHandler_ || (this.googUiComponentHandler_ = new goog.events.EventHandler(this))
};
goog.ui.Component.prototype.setParent = function(a) {
  if(this == a) {
    throw Error(goog.ui.Component.Error.PARENT_UNABLE_TO_BE_SET);
  }
  if(a && this.parent_ && this.id_ && this.parent_.getChild(this.id_) && this.parent_ != a) {
    throw Error(goog.ui.Component.Error.PARENT_UNABLE_TO_BE_SET);
  }
  this.parent_ = a;
  goog.ui.Component.superClass_.setParentEventTarget.call(this, a)
};
goog.ui.Component.prototype.getParent = function() {
  return this.parent_
};
goog.ui.Component.prototype.setParentEventTarget = function(a) {
  if(this.parent_ && this.parent_ != a) {
    throw Error(goog.ui.Component.Error.NOT_SUPPORTED);
  }
  goog.ui.Component.superClass_.setParentEventTarget.call(this, a)
};
goog.ui.Component.prototype.getDomHelper = function() {
  return this.dom_
};
goog.ui.Component.prototype.isInDocument = function() {
  return this.inDocument_
};
goog.ui.Component.prototype.createDom = function() {
  this.element_ = this.dom_.createElement("div")
};
goog.ui.Component.prototype.render = function(a) {
  this.render_(a)
};
goog.ui.Component.prototype.renderBefore = function(a) {
  this.render_(a.parentNode, a)
};
goog.ui.Component.prototype.render_ = function(a, b) {
  if(this.inDocument_) {
    throw Error(goog.ui.Component.Error.ALREADY_RENDERED);
  }
  this.element_ || this.createDom();
  a ? a.insertBefore(this.element_, b || null) : this.dom_.getDocument().body.appendChild(this.element_);
  (!this.parent_ || this.parent_.isInDocument()) && this.enterDocument()
};
goog.ui.Component.prototype.decorate = function(a) {
  if(this.inDocument_) {
    throw Error(goog.ui.Component.Error.ALREADY_RENDERED);
  }else {
    if(a && this.canDecorate(a)) {
      this.wasDecorated_ = !0;
      if(!this.dom_ || this.dom_.getDocument() != goog.dom.getOwnerDocument(a)) {
        this.dom_ = goog.dom.getDomHelper(a)
      }
      this.decorateInternal(a);
      this.enterDocument()
    }else {
      throw Error(goog.ui.Component.Error.DECORATE_INVALID);
    }
  }
};
goog.ui.Component.prototype.canDecorate = function() {
  return!0
};
goog.ui.Component.prototype.wasDecorated = function() {
  return this.wasDecorated_
};
goog.ui.Component.prototype.decorateInternal = function(a) {
  this.element_ = a
};
goog.ui.Component.prototype.enterDocument = function() {
  this.inDocument_ = !0;
  this.forEachChild(function(a) {
    !a.isInDocument() && a.getElement() && a.enterDocument()
  })
};
goog.ui.Component.prototype.exitDocument = function() {
  this.forEachChild(function(a) {
    a.isInDocument() && a.exitDocument()
  });
  this.googUiComponentHandler_ && this.googUiComponentHandler_.removeAll();
  this.inDocument_ = !1
};
goog.ui.Component.prototype.disposeInternal = function() {
  goog.ui.Component.superClass_.disposeInternal.call(this);
  this.inDocument_ && this.exitDocument();
  this.googUiComponentHandler_ && (this.googUiComponentHandler_.dispose(), delete this.googUiComponentHandler_);
  this.forEachChild(function(a) {
    a.dispose()
  });
  !this.wasDecorated_ && this.element_ && goog.dom.removeNode(this.element_);
  this.parent_ = this.model_ = this.element_ = this.childIndex_ = this.children_ = null
};
goog.ui.Component.prototype.makeId = function(a) {
  return this.getId() + "." + a
};
goog.ui.Component.prototype.makeIds = function(a) {
  var b = {}, c;
  for(c in a) {
    b[c] = this.makeId(a[c])
  }
  return b
};
goog.ui.Component.prototype.getModel = function() {
  return this.model_
};
goog.ui.Component.prototype.setModel = function(a) {
  this.model_ = a
};
goog.ui.Component.prototype.getFragmentFromId = function(a) {
  return a.substring(this.getId().length + 1)
};
goog.ui.Component.prototype.getElementByFragment = function(a) {
  if(!this.inDocument_) {
    throw Error(goog.ui.Component.Error.NOT_IN_DOCUMENT);
  }
  return this.dom_.getElement(this.makeId(a))
};
goog.ui.Component.prototype.addChild = function(a, b) {
  this.addChildAt(a, this.getChildCount(), b)
};
goog.ui.Component.prototype.addChildAt = function(a, b, c) {
  if(a.inDocument_ && (c || !this.inDocument_)) {
    throw Error(goog.ui.Component.Error.ALREADY_RENDERED);
  }
  if(b < 0 || b > this.getChildCount()) {
    throw Error(goog.ui.Component.Error.CHILD_INDEX_OUT_OF_BOUNDS);
  }
  if(!this.childIndex_ || !this.children_) {
    this.childIndex_ = {}, this.children_ = []
  }
  a.getParent() == this ? (goog.object.set(this.childIndex_, a.getId(), a), goog.array.remove(this.children_, a)) : goog.object.add(this.childIndex_, a.getId(), a);
  a.setParent(this);
  goog.array.insertAt(this.children_, a, b);
  a.inDocument_ && this.inDocument_ && a.getParent() == this ? (c = this.getContentElement(), c.insertBefore(a.getElement(), c.childNodes[b] || null)) : c ? (this.element_ || this.createDom(), b = this.getChildAt(b + 1), a.render_(this.getContentElement(), b ? b.element_ : null)) : this.inDocument_ && !a.inDocument_ && a.element_ && a.enterDocument()
};
goog.ui.Component.prototype.getContentElement = function() {
  return this.element_
};
goog.ui.Component.prototype.isRightToLeft = function() {
  if(this.rightToLeft_ == null) {
    this.rightToLeft_ = goog.style.isRightToLeft(this.inDocument_ ? this.element_ : this.dom_.getDocument().body)
  }
  return this.rightToLeft_
};
goog.ui.Component.prototype.setRightToLeft = function(a) {
  if(this.inDocument_) {
    throw Error(goog.ui.Component.Error.ALREADY_RENDERED);
  }
  this.rightToLeft_ = a
};
goog.ui.Component.prototype.hasChildren = function() {
  return!!this.children_ && this.children_.length != 0
};
goog.ui.Component.prototype.getChildCount = function() {
  return this.children_ ? this.children_.length : 0
};
goog.ui.Component.prototype.getChildIds = function() {
  var a = [];
  this.forEachChild(function(b) {
    a.push(b.getId())
  });
  return a
};
goog.ui.Component.prototype.getChild = function(a) {
  return this.childIndex_ && a ? goog.object.get(this.childIndex_, a) || null : null
};
goog.ui.Component.prototype.getChildAt = function(a) {
  return this.children_ ? this.children_[a] || null : null
};
goog.ui.Component.prototype.forEachChild = function(a, b) {
  this.children_ && goog.array.forEach(this.children_, a, b)
};
goog.ui.Component.prototype.indexOfChild = function(a) {
  return this.children_ && a ? goog.array.indexOf(this.children_, a) : -1
};
goog.ui.Component.prototype.removeChild = function(a, b) {
  if(a) {
    var c = goog.isString(a) ? a : a.getId(), a = this.getChild(c);
    c && a && (goog.object.remove(this.childIndex_, c), goog.array.remove(this.children_, a), b && (a.exitDocument(), a.element_ && goog.dom.removeNode(a.element_)), a.setParent(null))
  }
  if(!a) {
    throw Error(goog.ui.Component.Error.NOT_OUR_CHILD);
  }
  return a
};
goog.ui.Component.prototype.removeChildAt = function(a, b) {
  return this.removeChild(this.getChildAt(a), b)
};
goog.ui.Component.prototype.removeChildren = function(a) {
  for(;this.hasChildren();) {
    this.removeChildAt(0, a)
  }
};
goog.dom.a11y = {};
goog.dom.a11y.State = {ACTIVEDESCENDANT:"activedescendant", ATOMIC:"atomic", AUTOCOMPLETE:"autocomplete", BUSY:"busy", CHECKED:"checked", CONTROLS:"controls", DESCRIBEDBY:"describedby", DISABLED:"disabled", DROPEFFECT:"dropeffect", EXPANDED:"expanded", FLOWTO:"flowto", GRABBED:"grabbed", HASPOPUP:"haspopup", HIDDEN:"hidden", INVALID:"invalid", LABEL:"label", LABELLEDBY:"labelledby", LEVEL:"level", LIVE:"live", MULTILINE:"multiline", MULTISELECTABLE:"multiselectable", ORIENTATION:"orientation", OWNS:"owns", 
POSINSET:"posinset", PRESSED:"pressed", READONLY:"readonly", RELEVANT:"relevant", REQUIRED:"required", SELECTED:"selected", SETSIZE:"setsize", SORT:"sort", VALUEMAX:"valuemax", VALUEMIN:"valuemin", VALUENOW:"valuenow", VALUETEXT:"valuetext"};
goog.dom.a11y.Role = {ALERT:"alert", ALERTDIALOG:"alertdialog", APPLICATION:"application", ARTICLE:"article", BANNER:"banner", BUTTON:"button", CHECKBOX:"checkbox", COLUMNHEADER:"columnheader", COMBOBOX:"combobox", COMPLEMENTARY:"complementary", DIALOG:"dialog", DIRECTORY:"directory", DOCUMENT:"document", FORM:"form", GRID:"grid", GRIDCELL:"gridcell", GROUP:"group", HEADING:"heading", IMG:"img", LINK:"link", LIST:"list", LISTBOX:"listbox", LISTITEM:"listitem", LOG:"log", MAIN:"main", MARQUEE:"marquee", 
MATH:"math", MENU:"menu", MENUBAR:"menubar", MENU_ITEM:"menuitem", MENU_ITEM_CHECKBOX:"menuitemcheckbox", MENU_ITEM_RADIO:"menuitemradio", NAVIGATION:"navigation", NOTE:"note", OPTION:"option", PRESENTATION:"presentation", PROGRESSBAR:"progressbar", RADIO:"radio", RADIOGROUP:"radiogroup", REGION:"region", ROW:"row", ROWGROUP:"rowgroup", ROWHEADER:"rowheader", SCROLLBAR:"scrollbar", SEARCH:"search", SEPARATOR:"separator", SLIDER:"slider", SPINBUTTON:"spinbutton", STATUS:"status", TAB:"tab", TAB_LIST:"tablist", 
TAB_PANEL:"tabpanel", TEXTBOX:"textbox", TIMER:"timer", TOOLBAR:"toolbar", TOOLTIP:"tooltip", TREE:"tree", TREEGRID:"treegrid", TREEITEM:"treeitem"};
goog.dom.a11y.LivePriority = {OFF:"off", POLITE:"polite", ASSERTIVE:"assertive"};
goog.dom.a11y.setRole = function(a, b) {
  a.setAttribute("role", b);
  a.roleName = b
};
goog.dom.a11y.getRole = function(a) {
  return a.roleName || ""
};
goog.dom.a11y.setState = function(a, b, c) {
  a.setAttribute("aria-" + b, c)
};
goog.dom.a11y.getState = function(a, b) {
  var c = a.getAttribute("aria-" + b);
  return c === !0 || c === !1 ? c ? "true" : "false" : c ? String(c) : ""
};
goog.dom.a11y.getActiveDescendant = function(a) {
  var b = goog.dom.a11y.getState(a, goog.dom.a11y.State.ACTIVEDESCENDANT);
  return goog.dom.getOwnerDocument(a).getElementById(b)
};
goog.dom.a11y.setActiveDescendant = function(a, b) {
  goog.dom.a11y.setState(a, goog.dom.a11y.State.ACTIVEDESCENDANT, b ? b.id : "")
};
goog.dom.a11y.Announcer = function(a) {
  goog.Disposable.call(this);
  this.domHelper_ = a;
  this.liveRegions_ = {}
};
goog.inherits(goog.dom.a11y.Announcer, goog.Disposable);
goog.dom.a11y.Announcer.prototype.disposeInternal = function() {
  goog.object.forEach(this.liveRegions_, this.domHelper_.removeNode, this.domHelper_);
  this.domHelper_ = this.liveRegions_ = null;
  goog.dom.a11y.Announcer.superClass_.disposeInternal.call(this)
};
goog.dom.a11y.Announcer.prototype.say = function(a, b) {
  goog.dom.setTextContent(this.getLiveRegion_(b || goog.dom.a11y.LivePriority.POLITE), a)
};
goog.dom.a11y.Announcer.prototype.getLiveRegion_ = function(a) {
  if(this.liveRegions_[a]) {
    return this.liveRegions_[a]
  }
  var b;
  b = this.domHelper_.createElement("div");
  b.style.position = "absolute";
  b.style.top = "-1000px";
  goog.dom.a11y.setState(b, "live", a);
  goog.dom.a11y.setState(b, "atomic", "true");
  this.domHelper_.getDocument().body.appendChild(b);
  return this.liveRegions_[a] = b
};
goog.Timer = function(a, b) {
  goog.events.EventTarget.call(this);
  this.interval_ = a || 1;
  this.timerObject_ = b || goog.Timer.defaultTimerObject;
  this.boundTick_ = goog.bind(this.tick_, this);
  this.last_ = goog.now()
};
goog.inherits(goog.Timer, goog.events.EventTarget);
goog.Timer.MAX_TIMEOUT_ = 2147483647;
goog.Timer.prototype.enabled = !1;
goog.Timer.defaultTimerObject = goog.global.window;
goog.Timer.intervalScale = 0.8;
goog.Timer.prototype.timer_ = null;
goog.Timer.prototype.getInterval = function() {
  return this.interval_
};
goog.Timer.prototype.setInterval = function(a) {
  this.interval_ = a;
  this.timer_ && this.enabled ? (this.stop(), this.start()) : this.timer_ && this.stop()
};
goog.Timer.prototype.tick_ = function() {
  if(this.enabled) {
    var a = goog.now() - this.last_;
    if(a > 0 && a < this.interval_ * goog.Timer.intervalScale) {
      this.timer_ = this.timerObject_.setTimeout(this.boundTick_, this.interval_ - a)
    }else {
      if(this.dispatchTick(), this.enabled) {
        this.timer_ = this.timerObject_.setTimeout(this.boundTick_, this.interval_), this.last_ = goog.now()
      }
    }
  }
};
goog.Timer.prototype.dispatchTick = function() {
  this.dispatchEvent(goog.Timer.TICK)
};
goog.Timer.prototype.start = function() {
  this.enabled = !0;
  if(!this.timer_) {
    this.timer_ = this.timerObject_.setTimeout(this.boundTick_, this.interval_), this.last_ = goog.now()
  }
};
goog.Timer.prototype.stop = function() {
  this.enabled = !1;
  if(this.timer_) {
    this.timerObject_.clearTimeout(this.timer_), this.timer_ = null
  }
};
goog.Timer.prototype.disposeInternal = function() {
  goog.Timer.superClass_.disposeInternal.call(this);
  this.stop();
  delete this.timerObject_
};
goog.Timer.TICK = "tick";
goog.Timer.callOnce = function(a, b, c) {
  if(goog.isFunction(a)) {
    c && (a = goog.bind(a, c))
  }else {
    if(a && typeof a.handleEvent == "function") {
      a = goog.bind(a.handleEvent, a)
    }else {
      throw Error("Invalid listener argument");
    }
  }
  return b > goog.Timer.MAX_TIMEOUT_ ? -1 : goog.Timer.defaultTimerObject.setTimeout(a, b || 0)
};
goog.Timer.clear = function(a) {
  goog.Timer.defaultTimerObject.clearTimeout(a)
};
goog.events.KeyCodes = {MAC_ENTER:3, BACKSPACE:8, TAB:9, NUM_CENTER:12, ENTER:13, SHIFT:16, CTRL:17, ALT:18, PAUSE:19, CAPS_LOCK:20, ESC:27, SPACE:32, PAGE_UP:33, PAGE_DOWN:34, END:35, HOME:36, LEFT:37, UP:38, RIGHT:39, DOWN:40, PRINT_SCREEN:44, INSERT:45, DELETE:46, ZERO:48, ONE:49, TWO:50, THREE:51, FOUR:52, FIVE:53, SIX:54, SEVEN:55, EIGHT:56, NINE:57, QUESTION_MARK:63, A:65, B:66, C:67, D:68, E:69, F:70, G:71, H:72, I:73, J:74, K:75, L:76, M:77, N:78, O:79, P:80, Q:81, R:82, S:83, T:84, U:85, 
V:86, W:87, X:88, Y:89, Z:90, META:91, WIN_KEY_RIGHT:92, CONTEXT_MENU:93, NUM_ZERO:96, NUM_ONE:97, NUM_TWO:98, NUM_THREE:99, NUM_FOUR:100, NUM_FIVE:101, NUM_SIX:102, NUM_SEVEN:103, NUM_EIGHT:104, NUM_NINE:105, NUM_MULTIPLY:106, NUM_PLUS:107, NUM_MINUS:109, NUM_PERIOD:110, NUM_DIVISION:111, F1:112, F2:113, F3:114, F4:115, F5:116, F6:117, F7:118, F8:119, F9:120, F10:121, F11:122, F12:123, NUMLOCK:144, SEMICOLON:186, DASH:189, EQUALS:187, COMMA:188, PERIOD:190, SLASH:191, APOSTROPHE:192, SINGLE_QUOTE:222, 
OPEN_SQUARE_BRACKET:219, BACKSLASH:220, CLOSE_SQUARE_BRACKET:221, WIN_KEY:224, MAC_FF_META:224, WIN_IME:229, PHANTOM:255};
goog.events.KeyCodes.isTextModifyingKeyEvent = function(a) {
  if(a.altKey && !a.ctrlKey || a.metaKey || a.keyCode >= goog.events.KeyCodes.F1 && a.keyCode <= goog.events.KeyCodes.F12) {
    return!1
  }
  switch(a.keyCode) {
    case goog.events.KeyCodes.ALT:
    ;
    case goog.events.KeyCodes.CAPS_LOCK:
    ;
    case goog.events.KeyCodes.CONTEXT_MENU:
    ;
    case goog.events.KeyCodes.CTRL:
    ;
    case goog.events.KeyCodes.DOWN:
    ;
    case goog.events.KeyCodes.END:
    ;
    case goog.events.KeyCodes.ESC:
    ;
    case goog.events.KeyCodes.HOME:
    ;
    case goog.events.KeyCodes.INSERT:
    ;
    case goog.events.KeyCodes.LEFT:
    ;
    case goog.events.KeyCodes.MAC_FF_META:
    ;
    case goog.events.KeyCodes.META:
    ;
    case goog.events.KeyCodes.NUMLOCK:
    ;
    case goog.events.KeyCodes.NUM_CENTER:
    ;
    case goog.events.KeyCodes.PAGE_DOWN:
    ;
    case goog.events.KeyCodes.PAGE_UP:
    ;
    case goog.events.KeyCodes.PAUSE:
    ;
    case goog.events.KeyCodes.PHANTOM:
    ;
    case goog.events.KeyCodes.PRINT_SCREEN:
    ;
    case goog.events.KeyCodes.RIGHT:
    ;
    case goog.events.KeyCodes.SHIFT:
    ;
    case goog.events.KeyCodes.UP:
    ;
    case goog.events.KeyCodes.WIN_KEY:
    ;
    case goog.events.KeyCodes.WIN_KEY_RIGHT:
      return!1;
    default:
      return!0
  }
};
goog.events.KeyCodes.firesKeyPressEvent = function(a, b, c, d, e) {
  if(!goog.userAgent.IE && (!goog.userAgent.WEBKIT || !goog.userAgent.isVersion("525"))) {
    return!0
  }
  if(goog.userAgent.MAC && e) {
    return goog.events.KeyCodes.isCharacterKey(a)
  }
  if(e && !d) {
    return!1
  }
  if(!c && (b == goog.events.KeyCodes.CTRL || b == goog.events.KeyCodes.ALT)) {
    return!1
  }
  if(goog.userAgent.IE && d && b == a) {
    return!1
  }
  switch(a) {
    case goog.events.KeyCodes.ENTER:
      return!(goog.userAgent.IE && goog.userAgent.isDocumentMode(9));
    case goog.events.KeyCodes.ESC:
      return!goog.userAgent.WEBKIT
  }
  return goog.events.KeyCodes.isCharacterKey(a)
};
goog.events.KeyCodes.isCharacterKey = function(a) {
  if(a >= goog.events.KeyCodes.ZERO && a <= goog.events.KeyCodes.NINE) {
    return!0
  }
  if(a >= goog.events.KeyCodes.NUM_ZERO && a <= goog.events.KeyCodes.NUM_MULTIPLY) {
    return!0
  }
  if(a >= goog.events.KeyCodes.A && a <= goog.events.KeyCodes.Z) {
    return!0
  }
  if(goog.userAgent.WEBKIT && a == 0) {
    return!0
  }
  switch(a) {
    case goog.events.KeyCodes.SPACE:
    ;
    case goog.events.KeyCodes.QUESTION_MARK:
    ;
    case goog.events.KeyCodes.NUM_PLUS:
    ;
    case goog.events.KeyCodes.NUM_MINUS:
    ;
    case goog.events.KeyCodes.NUM_PERIOD:
    ;
    case goog.events.KeyCodes.NUM_DIVISION:
    ;
    case goog.events.KeyCodes.SEMICOLON:
    ;
    case goog.events.KeyCodes.DASH:
    ;
    case goog.events.KeyCodes.EQUALS:
    ;
    case goog.events.KeyCodes.COMMA:
    ;
    case goog.events.KeyCodes.PERIOD:
    ;
    case goog.events.KeyCodes.SLASH:
    ;
    case goog.events.KeyCodes.APOSTROPHE:
    ;
    case goog.events.KeyCodes.SINGLE_QUOTE:
    ;
    case goog.events.KeyCodes.OPEN_SQUARE_BRACKET:
    ;
    case goog.events.KeyCodes.BACKSLASH:
    ;
    case goog.events.KeyCodes.CLOSE_SQUARE_BRACKET:
      return!0;
    default:
      return!1
  }
};
goog.events.KeyHandler = function(a, b) {
  goog.events.EventTarget.call(this);
  a && this.attach(a, b)
};
goog.inherits(goog.events.KeyHandler, goog.events.EventTarget);
goog.events.KeyHandler.prototype.element_ = null;
goog.events.KeyHandler.prototype.keyPressKey_ = null;
goog.events.KeyHandler.prototype.keyDownKey_ = null;
goog.events.KeyHandler.prototype.keyUpKey_ = null;
goog.events.KeyHandler.prototype.lastKey_ = -1;
goog.events.KeyHandler.prototype.keyCode_ = -1;
goog.events.KeyHandler.EventType = {KEY:"key"};
goog.events.KeyHandler.safariKey_ = {3:goog.events.KeyCodes.ENTER, 12:goog.events.KeyCodes.NUMLOCK, 63232:goog.events.KeyCodes.UP, 63233:goog.events.KeyCodes.DOWN, 63234:goog.events.KeyCodes.LEFT, 63235:goog.events.KeyCodes.RIGHT, 63236:goog.events.KeyCodes.F1, 63237:goog.events.KeyCodes.F2, 63238:goog.events.KeyCodes.F3, 63239:goog.events.KeyCodes.F4, 63240:goog.events.KeyCodes.F5, 63241:goog.events.KeyCodes.F6, 63242:goog.events.KeyCodes.F7, 63243:goog.events.KeyCodes.F8, 63244:goog.events.KeyCodes.F9, 
63245:goog.events.KeyCodes.F10, 63246:goog.events.KeyCodes.F11, 63247:goog.events.KeyCodes.F12, 63248:goog.events.KeyCodes.PRINT_SCREEN, 63272:goog.events.KeyCodes.DELETE, 63273:goog.events.KeyCodes.HOME, 63275:goog.events.KeyCodes.END, 63276:goog.events.KeyCodes.PAGE_UP, 63277:goog.events.KeyCodes.PAGE_DOWN, 63289:goog.events.KeyCodes.NUMLOCK, 63302:goog.events.KeyCodes.INSERT};
goog.events.KeyHandler.keyIdentifier_ = {Up:goog.events.KeyCodes.UP, Down:goog.events.KeyCodes.DOWN, Left:goog.events.KeyCodes.LEFT, Right:goog.events.KeyCodes.RIGHT, Enter:goog.events.KeyCodes.ENTER, F1:goog.events.KeyCodes.F1, F2:goog.events.KeyCodes.F2, F3:goog.events.KeyCodes.F3, F4:goog.events.KeyCodes.F4, F5:goog.events.KeyCodes.F5, F6:goog.events.KeyCodes.F6, F7:goog.events.KeyCodes.F7, F8:goog.events.KeyCodes.F8, F9:goog.events.KeyCodes.F9, F10:goog.events.KeyCodes.F10, F11:goog.events.KeyCodes.F11, 
F12:goog.events.KeyCodes.F12, "U+007F":goog.events.KeyCodes.DELETE, Home:goog.events.KeyCodes.HOME, End:goog.events.KeyCodes.END, PageUp:goog.events.KeyCodes.PAGE_UP, PageDown:goog.events.KeyCodes.PAGE_DOWN, Insert:goog.events.KeyCodes.INSERT};
goog.events.KeyHandler.mozKeyCodeToKeyCodeMap_ = {61:187, 59:186};
goog.events.KeyHandler.USES_KEYDOWN_ = goog.userAgent.IE || goog.userAgent.WEBKIT && goog.userAgent.isVersion("525");
goog.events.KeyHandler.prototype.handleKeyDown_ = function(a) {
  if(goog.userAgent.WEBKIT && (this.lastKey_ == goog.events.KeyCodes.CTRL && !a.ctrlKey || this.lastKey_ == goog.events.KeyCodes.ALT && !a.altKey)) {
    this.keyCode_ = this.lastKey_ = -1
  }
  goog.events.KeyHandler.USES_KEYDOWN_ && !goog.events.KeyCodes.firesKeyPressEvent(a.keyCode, this.lastKey_, a.shiftKey, a.ctrlKey, a.altKey) ? this.handleEvent(a) : this.keyCode_ = goog.userAgent.GECKO && a.keyCode in goog.events.KeyHandler.mozKeyCodeToKeyCodeMap_ ? goog.events.KeyHandler.mozKeyCodeToKeyCodeMap_[a.keyCode] : a.keyCode
};
goog.events.KeyHandler.prototype.handleKeyup_ = function() {
  this.keyCode_ = this.lastKey_ = -1
};
goog.events.KeyHandler.prototype.handleEvent = function(a) {
  var b = a.getBrowserEvent(), c, d;
  if(goog.userAgent.IE && a.type == goog.events.EventType.KEYPRESS) {
    c = this.keyCode_, d = c != goog.events.KeyCodes.ENTER && c != goog.events.KeyCodes.ESC ? b.keyCode : 0
  }else {
    if(goog.userAgent.WEBKIT && a.type == goog.events.EventType.KEYPRESS) {
      c = this.keyCode_, d = b.charCode >= 0 && b.charCode < 63232 && goog.events.KeyCodes.isCharacterKey(c) ? b.charCode : 0
    }else {
      if(goog.userAgent.OPERA) {
        c = this.keyCode_, d = goog.events.KeyCodes.isCharacterKey(c) ? b.keyCode : 0
      }else {
        if(c = b.keyCode || this.keyCode_, d = b.charCode || 0, goog.userAgent.MAC && d == goog.events.KeyCodes.QUESTION_MARK && !c) {
          c = goog.events.KeyCodes.SLASH
        }
      }
    }
  }
  var e = c, f = b.keyIdentifier;
  c ? c >= 63232 && c in goog.events.KeyHandler.safariKey_ ? e = goog.events.KeyHandler.safariKey_[c] : c == 25 && a.shiftKey && (e = 9) : f && f in goog.events.KeyHandler.keyIdentifier_ && (e = goog.events.KeyHandler.keyIdentifier_[f]);
  a = e == this.lastKey_;
  this.lastKey_ = e;
  b = new goog.events.KeyEvent(e, d, a, b);
  try {
    this.dispatchEvent(b)
  }finally {
    b.dispose()
  }
};
goog.events.KeyHandler.prototype.getElement = function() {
  return this.element_
};
goog.events.KeyHandler.prototype.attach = function(a, b) {
  this.keyUpKey_ && this.detach();
  this.element_ = a;
  this.keyPressKey_ = goog.events.listen(this.element_, goog.events.EventType.KEYPRESS, this, b);
  this.keyDownKey_ = goog.events.listen(this.element_, goog.events.EventType.KEYDOWN, this.handleKeyDown_, b, this);
  this.keyUpKey_ = goog.events.listen(this.element_, goog.events.EventType.KEYUP, this.handleKeyup_, b, this)
};
goog.events.KeyHandler.prototype.detach = function() {
  if(this.keyPressKey_) {
    goog.events.unlistenByKey(this.keyPressKey_), goog.events.unlistenByKey(this.keyDownKey_), goog.events.unlistenByKey(this.keyUpKey_), this.keyUpKey_ = this.keyDownKey_ = this.keyPressKey_ = null
  }
  this.element_ = null;
  this.keyCode_ = this.lastKey_ = -1
};
goog.events.KeyHandler.prototype.disposeInternal = function() {
  goog.events.KeyHandler.superClass_.disposeInternal.call(this);
  this.detach()
};
goog.events.KeyEvent = function(a, b, c, d) {
  goog.events.BrowserEvent.call(this, d);
  this.type = goog.events.KeyHandler.EventType.KEY;
  this.keyCode = a;
  this.charCode = b;
  this.repeat = c
};
goog.inherits(goog.events.KeyEvent, goog.events.BrowserEvent);
goog.math.randomInt = function(a) {
  return Math.floor(Math.random() * a)
};
goog.math.uniformRandom = function(a, b) {
  return a + Math.random() * (b - a)
};
goog.math.clamp = function(a, b, c) {
  return Math.min(Math.max(a, b), c)
};
goog.math.modulo = function(a, b) {
  var c = a % b;
  return c * b < 0 ? c + b : c
};
goog.math.lerp = function(a, b, c) {
  return a + c * (b - a)
};
goog.math.nearlyEquals = function(a, b, c) {
  return Math.abs(a - b) <= (c || 1.0E-6)
};
goog.math.standardAngle = function(a) {
  return goog.math.modulo(a, 360)
};
goog.math.toRadians = function(a) {
  return a * Math.PI / 180
};
goog.math.toDegrees = function(a) {
  return a * 180 / Math.PI
};
goog.math.angleDx = function(a, b) {
  return b * Math.cos(goog.math.toRadians(a))
};
goog.math.angleDy = function(a, b) {
  return b * Math.sin(goog.math.toRadians(a))
};
goog.math.angle = function(a, b, c, d) {
  return goog.math.standardAngle(goog.math.toDegrees(Math.atan2(d - b, c - a)))
};
goog.math.angleDifference = function(a, b) {
  var c = goog.math.standardAngle(b) - goog.math.standardAngle(a);
  c > 180 ? c -= 360 : c <= -180 && (c = 360 + c);
  return c
};
goog.math.sign = function(a) {
  return a == 0 ? 0 : a < 0 ? -1 : 1
};
goog.math.longestCommonSubsequence = function(a, b, c, d) {
  for(var c = c || function(a, b) {
    return a == b
  }, d = d || function(b) {
    return a[b]
  }, e = a.length, f = b.length, g = [], h = 0;h < e + 1;h++) {
    g[h] = [], g[h][0] = 0
  }
  for(var i = 0;i < f + 1;i++) {
    g[0][i] = 0
  }
  for(h = 1;h <= e;h++) {
    for(i = 1;i <= e;i++) {
      g[h][i] = c(a[h - 1], b[i - 1]) ? g[h - 1][i - 1] + 1 : Math.max(g[h - 1][i], g[h][i - 1])
    }
  }
  for(var j = [], h = e, i = f;h > 0 && i > 0;) {
    c(a[h - 1], b[i - 1]) ? (j.unshift(d(h - 1, i - 1)), h--, i--) : g[h - 1][i] > g[h][i - 1] ? h-- : i--
  }
  return j
};
goog.math.sum = function(a) {
  return goog.array.reduce(arguments, function(a, c) {
    return a + c
  }, 0)
};
goog.math.average = function(a) {
  return goog.math.sum.apply(null, arguments) / arguments.length
};
goog.math.standardDeviation = function(a) {
  var b = arguments.length;
  if(b < 2) {
    return 0
  }
  var c = goog.math.average.apply(null, arguments), b = goog.math.sum.apply(null, goog.array.map(arguments, function(a) {
    return Math.pow(a - c, 2)
  })) / (b - 1);
  return Math.sqrt(b)
};
goog.math.isInt = function(a) {
  return isFinite(a) && a % 1 == 0
};
goog.math.isFiniteNumber = function(a) {
  return isFinite(a) && !isNaN(a)
};
goog.events.MouseWheelHandler = function(a) {
  goog.events.EventTarget.call(this);
  this.element_ = a;
  this.listenKey_ = goog.events.listen(this.element_, goog.userAgent.GECKO ? "DOMMouseScroll" : "mousewheel", this)
};
goog.inherits(goog.events.MouseWheelHandler, goog.events.EventTarget);
goog.events.MouseWheelHandler.EventType = {MOUSEWHEEL:"mousewheel"};
goog.events.MouseWheelHandler.prototype.setMaxDeltaX = function(a) {
  this.maxDeltaX_ = a
};
goog.events.MouseWheelHandler.prototype.setMaxDeltaY = function(a) {
  this.maxDeltaY_ = a
};
goog.events.MouseWheelHandler.prototype.handleEvent = function(a) {
  var b = 0, c = 0, d = 0, a = a.getBrowserEvent();
  if(a.type == "mousewheel") {
    c = 1;
    if(goog.userAgent.IE || goog.userAgent.WEBKIT && (goog.userAgent.WINDOWS || goog.userAgent.isVersion("532.0"))) {
      c = 40
    }
    d = goog.events.MouseWheelHandler.smartScale_(-a.wheelDelta, c);
    goog.isDef(a.wheelDeltaX) ? (b = goog.events.MouseWheelHandler.smartScale_(-a.wheelDeltaX, c), c = goog.events.MouseWheelHandler.smartScale_(-a.wheelDeltaY, c)) : c = d
  }else {
    d = a.detail, d > 100 ? d = 3 : d < -100 && (d = -3), goog.isDef(a.axis) && a.axis === a.HORIZONTAL_AXIS ? b = d : c = d
  }
  goog.isNumber(this.maxDeltaX_) && (b = goog.math.clamp(b, -this.maxDeltaX_, this.maxDeltaX_));
  goog.isNumber(this.maxDeltaY_) && (c = goog.math.clamp(c, -this.maxDeltaY_, this.maxDeltaY_));
  b = new goog.events.MouseWheelEvent(d, a, b, c);
  try {
    this.dispatchEvent(b)
  }finally {
    b.dispose()
  }
};
goog.events.MouseWheelHandler.smartScale_ = function(a, b) {
  return goog.userAgent.WEBKIT && goog.userAgent.MAC && a % b != 0 ? a : a / b
};
goog.events.MouseWheelHandler.prototype.disposeInternal = function() {
  goog.events.MouseWheelHandler.superClass_.disposeInternal.call(this);
  goog.events.unlistenByKey(this.listenKey_);
  delete this.listenKey_
};
goog.events.MouseWheelEvent = function(a, b, c, d) {
  goog.events.BrowserEvent.call(this, b);
  this.type = goog.events.MouseWheelHandler.EventType.MOUSEWHEEL;
  this.detail = a;
  this.deltaX = c;
  this.deltaY = d
};
goog.inherits(goog.events.MouseWheelEvent, goog.events.BrowserEvent);
goog.fx = {};
goog.fx.Transition = function() {
};
goog.fx.Transition.EventType = {PLAY:"play", BEGIN:"begin", RESUME:"resume", END:"end", STOP:"stop", FINISH:"finish", PAUSE:"pause"};
goog.fx.Animation = function(a, b, c, d) {
  goog.events.EventTarget.call(this);
  if(!goog.isArray(a) || !goog.isArray(b)) {
    throw Error("Start and end parameters must be arrays");
  }
  if(a.length != b.length) {
    throw Error("Start and end points must be the same length");
  }
  this.startPoint = a;
  this.endPoint = b;
  this.duration = c;
  this.accel_ = d;
  this.coords = []
};
goog.inherits(goog.fx.Animation, goog.events.EventTarget);
goog.fx.Animation.EventType = {PLAY:goog.fx.Transition.EventType.PLAY, BEGIN:goog.fx.Transition.EventType.BEGIN, RESUME:goog.fx.Transition.EventType.RESUME, END:goog.fx.Transition.EventType.END, STOP:goog.fx.Transition.EventType.STOP, FINISH:goog.fx.Transition.EventType.FINISH, PAUSE:goog.fx.Transition.EventType.PAUSE, ANIMATE:"animate", DESTROY:"destroy"};
goog.fx.Animation.State = {STOPPED:0, PAUSED:-1, PLAYING:1};
goog.fx.Animation.TIMEOUT = 20;
goog.fx.Animation.activeAnimations_ = {};
goog.fx.Animation.globalTimer_ = null;
goog.fx.Animation.animationWindow_ = null;
goog.fx.Animation.requestAnimationFrameFn_ = null;
goog.fx.Animation.cancelRequestAnimationFrameFn_ = null;
goog.fx.Animation.cycleAnimations_ = function() {
  goog.fx.Animation.resetTimer_();
  var a = goog.now(), b;
  for(b in goog.fx.Animation.activeAnimations_) {
    goog.fx.Animation.activeAnimations_[b].cycle(a)
  }
  goog.fx.Animation.globalTimer_ = goog.object.isEmpty(goog.fx.Animation.activeAnimations_) ? null : goog.fx.Animation.startTimer_()
};
goog.fx.Animation.resetTimer_ = function() {
  if(goog.fx.Animation.globalTimer_) {
    goog.fx.Animation.requestAnimationFrameFn_ ? goog.fx.Animation.cancelRequestAnimationFrameFn_.call(goog.fx.Animation.animationWindow_, goog.fx.Animation.globalTimer_) : goog.Timer.defaultTimerObject.clearTimeout(goog.fx.Animation.globalTimer_), goog.fx.Animation.globalTimer_ = null
  }
};
goog.fx.Animation.startTimer_ = function() {
  return goog.fx.Animation.requestAnimationFrameFn_ ? goog.fx.Animation.requestAnimationFrameFn_.call(goog.fx.Animation.animationWindow_, goog.fx.Animation.cycleAnimations_) : goog.Timer.defaultTimerObject.setTimeout(goog.fx.Animation.cycleAnimations_, goog.fx.Animation.TIMEOUT)
};
goog.fx.Animation.registerAnimation = function(a) {
  var b = goog.getUid(a);
  b in goog.fx.Animation.activeAnimations_ || (goog.fx.Animation.activeAnimations_[b] = a);
  if(!goog.fx.Animation.globalTimer_) {
    goog.fx.Animation.globalTimer_ = goog.fx.Animation.startTimer_()
  }
};
goog.fx.Animation.unregisterAnimation = function(a) {
  a = goog.getUid(a);
  delete goog.fx.Animation.activeAnimations_[a];
  goog.object.isEmpty(goog.fx.Animation.activeAnimations_) && goog.fx.Animation.resetTimer_()
};
goog.fx.Animation.setAnimationWindow = function(a) {
  (goog.fx.Animation.animationWindow_ = a) ? (goog.fx.Animation.requestAnimationFrameFn_ = a.requestAnimationFrame || a.webkitRequestAnimationFrame || a.mozRequestAnimationFrame || a.oRequestAnimationFrame || a.msRequestAnimationFrame, goog.fx.Animation.cancelRequestAnimationFrameFn_ = a.cancelRequestAnimationFrame || a.webkitCancelRequestAnimationFrame || a.mozCancelRequestAnimationFrame || a.oCancelRequestAnimationFrame || a.msCancelRequestAnimationFrame) : (goog.fx.Animation.requestAnimationFrameFn_ = 
  null, goog.fx.Animation.cancelRequestAnimationFrameFn_ = null)
};
goog.fx.Animation.prototype.state_ = goog.fx.Animation.State.STOPPED;
goog.fx.Animation.prototype.fps_ = 0;
goog.fx.Animation.prototype.progress = 0;
goog.fx.Animation.prototype.startTime = null;
goog.fx.Animation.prototype.endTime = null;
goog.fx.Animation.prototype.lastFrame = null;
goog.fx.Animation.prototype.getStateInternal = function() {
  return this.state_
};
goog.fx.Animation.prototype.play = function(a) {
  if(a || this.state_ == goog.fx.Animation.State.STOPPED) {
    this.progress = 0, this.coords = this.startPoint
  }else {
    if(this.state_ == goog.fx.Animation.State.PLAYING) {
      return!1
    }
  }
  goog.fx.Animation.unregisterAnimation(this);
  this.startTime = goog.now();
  this.state_ == goog.fx.Animation.State.PAUSED && (this.startTime -= this.duration * this.progress);
  this.endTime = this.startTime + this.duration;
  this.lastFrame = this.startTime;
  if(!this.progress) {
    this.onBegin()
  }
  this.onPlay();
  if(this.state_ == goog.fx.Animation.State.PAUSED) {
    this.onResume()
  }
  this.state_ = goog.fx.Animation.State.PLAYING;
  goog.fx.Animation.registerAnimation(this);
  this.cycle(this.startTime);
  return!0
};
goog.fx.Animation.prototype.stop = function(a) {
  goog.fx.Animation.unregisterAnimation(this);
  this.state_ = goog.fx.Animation.State.STOPPED;
  if(a) {
    this.progress = 1
  }
  this.updateCoords_(this.progress);
  this.onStop();
  this.onEnd()
};
goog.fx.Animation.prototype.pause = function() {
  if(this.state_ == goog.fx.Animation.State.PLAYING) {
    goog.fx.Animation.unregisterAnimation(this), this.state_ = goog.fx.Animation.State.PAUSED, this.onPause()
  }
};
goog.fx.Animation.prototype.disposeInternal = function() {
  this.state_ != goog.fx.Animation.State.STOPPED && this.stop(!1);
  this.onDestroy();
  goog.fx.Animation.superClass_.disposeInternal.call(this)
};
goog.fx.Animation.prototype.destroy = function() {
  this.dispose()
};
goog.fx.Animation.prototype.cycle = function(a) {
  this.progress = (a - this.startTime) / (this.endTime - this.startTime);
  if(this.progress >= 1) {
    this.progress = 1
  }
  this.fps_ = 1E3 / (a - this.lastFrame);
  this.lastFrame = a;
  this.updateCoords_(this.progress);
  if(this.progress == 1) {
    this.state_ = goog.fx.Animation.State.STOPPED, goog.fx.Animation.unregisterAnimation(this), this.onFinish(), this.onEnd()
  }else {
    if(this.state_ == goog.fx.Animation.State.PLAYING) {
      this.onAnimate()
    }
  }
};
goog.fx.Animation.prototype.updateCoords_ = function(a) {
  goog.isFunction(this.accel_) && (a = this.accel_(a));
  this.coords = Array(this.startPoint.length);
  for(var b = 0;b < this.startPoint.length;b++) {
    this.coords[b] = (this.endPoint[b] - this.startPoint[b]) * a + this.startPoint[b]
  }
};
goog.fx.Animation.prototype.onAnimate = function() {
  this.dispatchAnimationEvent_(goog.fx.Animation.EventType.ANIMATE)
};
goog.fx.Animation.prototype.onBegin = function() {
  this.dispatchAnimationEvent_(goog.fx.Transition.EventType.BEGIN)
};
goog.fx.Animation.prototype.onDestroy = function() {
  this.dispatchAnimationEvent_(goog.fx.Animation.EventType.DESTROY)
};
goog.fx.Animation.prototype.onEnd = function() {
  this.dispatchAnimationEvent_(goog.fx.Transition.EventType.END)
};
goog.fx.Animation.prototype.onFinish = function() {
  this.dispatchAnimationEvent_(goog.fx.Transition.EventType.FINISH)
};
goog.fx.Animation.prototype.onPause = function() {
  this.dispatchAnimationEvent_(goog.fx.Transition.EventType.PAUSE)
};
goog.fx.Animation.prototype.onPlay = function() {
  this.dispatchAnimationEvent_(goog.fx.Transition.EventType.PLAY)
};
goog.fx.Animation.prototype.onResume = function() {
  this.dispatchAnimationEvent_(goog.fx.Transition.EventType.RESUME)
};
goog.fx.Animation.prototype.onStop = function() {
  this.dispatchAnimationEvent_(goog.fx.Transition.EventType.STOP)
};
goog.fx.Animation.prototype.dispatchAnimationEvent_ = function(a) {
  this.dispatchEvent(new goog.fx.AnimationEvent(a, this))
};
goog.fx.AnimationEvent = function(a, b) {
  goog.events.Event.call(this, a);
  this.coords = b.coords;
  this.x = b.coords[0];
  this.y = b.coords[1];
  this.z = b.coords[2];
  this.duration = b.duration;
  this.progress = b.progress;
  this.fps = b.fps_;
  this.state = b.state_;
  this.anim = b
};
goog.inherits(goog.fx.AnimationEvent, goog.events.Event);
goog.fx.AnimationEvent.prototype.coordsAsInts = function() {
  return goog.array.map(this.coords, Math.round)
};
goog.fx.AnimationQueue = function() {
  goog.fx.Animation.call(this, [0], [0], 0);
  this.queue_ = []
};
goog.inherits(goog.fx.AnimationQueue, goog.fx.Animation);
goog.fx.AnimationQueue.prototype.onResume = function() {
  this.executeChildrenAction(function(a) {
    a.play(a.progress == 0)
  });
  goog.fx.AnimationQueue.superClass_.onResume.call(this)
};
goog.fx.AnimationQueue.prototype.onStop = function() {
  this.executeChildrenAction(function(a) {
    a.stop()
  });
  goog.fx.AnimationQueue.superClass_.onStop.call(this)
};
goog.fx.AnimationQueue.prototype.onPause = function() {
  this.executeChildrenAction(function(a) {
    a.pause()
  });
  goog.fx.AnimationQueue.superClass_.onPause.call(this)
};
goog.fx.AnimationQueue.prototype.onDestroy = function() {
  this.destroyQueueAndHandlers_();
  goog.fx.AnimationQueue.superClass_.onDestroy.call(this)
};
goog.fx.AnimationQueue.prototype.destroyQueueAndHandlers_ = function() {
  goog.array.forEach(this.queue_, function(a) {
    a.destroy()
  })
};
goog.fx.AnimationParallelQueue = function() {
  goog.fx.AnimationQueue.call(this)
};
goog.inherits(goog.fx.AnimationParallelQueue, goog.fx.AnimationQueue);
goog.fx.AnimationParallelQueue.prototype.playAll_ = function() {
  for(var a = 0;a < this.queue_.length;a++) {
    this.queue_[a].play()
  }
};
goog.fx.AnimationParallelQueue.prototype.onBegin = function() {
  this.playAll_();
  goog.fx.AnimationParallelQueue.superClass_.onBegin.call(this)
};
goog.fx.AnimationParallelQueue.prototype.executeChildrenAction = function(a) {
  goog.array.forEach(this.queue_, a)
};
goog.fx.AnimationParallelQueue.prototype.add = function(a) {
  this.queue_.push(a);
  this.duration = Math.max(this.duration, a.duration)
};
goog.fx.AnimationParallelQueue.prototype.remove = function(a) {
  if(goog.array.remove(this.queue_, a) && a.duration == this.duration) {
    this.duration = 0, goog.array.forEach(this.queue_, function(a) {
      this.duration = Math.max(a.duration, this.duration)
    }, this)
  }
};
goog.fx.AnimationSerialQueue = function() {
  goog.fx.AnimationQueue.call(this);
  this.childHandler_ = new goog.events.EventHandler(this)
};
goog.inherits(goog.fx.AnimationSerialQueue, goog.fx.AnimationQueue);
goog.fx.AnimationSerialQueue.prototype.counter_ = 0;
goog.fx.AnimationSerialQueue.prototype.onBegin = function() {
  this.playNext_();
  goog.fx.AnimationSerialQueue.superClass_.onBegin.call(this)
};
goog.fx.AnimationSerialQueue.prototype.onEnd = function() {
  this.reset_();
  goog.fx.AnimationSerialQueue.superClass_.onEnd.call(this)
};
goog.fx.AnimationSerialQueue.prototype.reset_ = function() {
  this.counter_ = 0;
  this.childHandler_.removeAll()
};
goog.fx.AnimationSerialQueue.prototype.playNext_ = function() {
  this.getStateInternal() == goog.fx.Animation.State.PAUSED && (this.reset_(), goog.array.forEach(this.queue_, function(a) {
    a.progress = 0;
    a.updateCoords_(a.progress);
    a.stop()
  }));
  this.queue_[this.counter_].play();
  this.counter_++;
  this.counter_ < this.queue_.length && this.childHandler_.listen(this.queue_[this.counter_ - 1], goog.fx.Transition.EventType.FINISH, function() {
    this.playNext_()
  })
};
goog.fx.AnimationSerialQueue.prototype.add = function(a) {
  this.queue_.push(a);
  this.duration += a.duration
};
goog.fx.AnimationSerialQueue.prototype.remove = function(a) {
  goog.array.remove(this.queue_, a) && (this.duration -= a.duration)
};
goog.fx.AnimationSerialQueue.prototype.executeChildrenAction = function(a) {
  this.counter_ > 0 && a(this.queue_[this.counter_ - 1])
};
goog.fx.AnimationSerialQueue.prototype.destroyQueueAndHandlers_ = function() {
  goog.array.forEach(this.queue_, function(a) {
    a.destroy()
  });
  this.childHandler_.dispose()
};
goog.fx.Dragger = function(a, b, c) {
  goog.events.EventTarget.call(this);
  this.target = a;
  this.handle = b || a;
  this.limits = c || new goog.math.Rect(NaN, NaN, NaN, NaN);
  this.document_ = goog.dom.getOwnerDocument(a);
  this.eventHandler_ = new goog.events.EventHandler(this);
  goog.events.listen(this.handle, [goog.events.EventType.TOUCHSTART, goog.events.EventType.MOUSEDOWN], this.startDrag, !1, this)
};
goog.inherits(goog.fx.Dragger, goog.events.EventTarget);
goog.fx.Dragger.HAS_SET_CAPTURE_ = goog.userAgent.IE || goog.userAgent.GECKO && goog.userAgent.isVersion("1.9.3");
goog.fx.Dragger.EventType = {EARLY_CANCEL:"earlycancel", START:"start", BEFOREDRAG:"beforedrag", DRAG:"drag", END:"end"};
goog.fx.Dragger.prototype.clientX = 0;
goog.fx.Dragger.prototype.clientY = 0;
goog.fx.Dragger.prototype.screenX = 0;
goog.fx.Dragger.prototype.screenY = 0;
goog.fx.Dragger.prototype.startX = 0;
goog.fx.Dragger.prototype.startY = 0;
goog.fx.Dragger.prototype.deltaX = 0;
goog.fx.Dragger.prototype.deltaY = 0;
goog.fx.Dragger.prototype.enabled_ = !0;
goog.fx.Dragger.prototype.dragging_ = !1;
goog.fx.Dragger.prototype.hysteresisDistanceSquared_ = 0;
goog.fx.Dragger.prototype.mouseDownTime_ = 0;
goog.fx.Dragger.prototype.ieDragStartCancellingOn_ = !1;
goog.fx.Dragger.prototype.getHandler = function() {
  return this.eventHandler_
};
goog.fx.Dragger.prototype.setLimits = function(a) {
  this.limits = a || new goog.math.Rect(NaN, NaN, NaN, NaN)
};
goog.fx.Dragger.prototype.setHysteresis = function(a) {
  this.hysteresisDistanceSquared_ = Math.pow(a, 2)
};
goog.fx.Dragger.prototype.getHysteresis = function() {
  return Math.sqrt(this.hysteresisDistanceSquared_)
};
goog.fx.Dragger.prototype.setScrollTarget = function(a) {
  this.scrollTarget_ = a
};
goog.fx.Dragger.prototype.setCancelIeDragStart = function(a) {
  this.ieDragStartCancellingOn_ = a
};
goog.fx.Dragger.prototype.getEnabled = function() {
  return this.enabled_
};
goog.fx.Dragger.prototype.setEnabled = function(a) {
  this.enabled_ = a
};
goog.fx.Dragger.prototype.disposeInternal = function() {
  goog.fx.Dragger.superClass_.disposeInternal.call(this);
  goog.events.unlisten(this.handle, [goog.events.EventType.TOUCHSTART, goog.events.EventType.MOUSEDOWN], this.startDrag, !1, this);
  this.eventHandler_.dispose();
  delete this.target;
  delete this.handle;
  delete this.eventHandler_
};
goog.fx.Dragger.prototype.startDrag = function(a) {
  var b = a.type == goog.events.EventType.MOUSEDOWN;
  if(this.enabled_ && !this.dragging_ && (!b || a.isMouseActionButton())) {
    this.maybeReinitTouchEvent_(a);
    if(this.hysteresisDistanceSquared_ == 0) {
      if(this.initializeDrag_(a), this.dragging_) {
        a.preventDefault()
      }else {
        return
      }
    }else {
      a.preventDefault()
    }
    this.setupDragHandlers();
    this.clientX = this.startX = a.clientX;
    this.clientY = this.startY = a.clientY;
    this.screenX = a.screenX;
    this.screenY = a.screenY;
    this.deltaX = this.target.offsetLeft;
    this.deltaY = this.target.offsetTop;
    this.pageScroll = goog.dom.getDomHelper(this.document_).getDocumentScroll();
    this.mouseDownTime_ = goog.now()
  }else {
    this.dispatchEvent(goog.fx.Dragger.EventType.EARLY_CANCEL)
  }
};
goog.fx.Dragger.prototype.setupDragHandlers = function() {
  var a = this.document_, b = a.documentElement, c = !goog.fx.Dragger.HAS_SET_CAPTURE_;
  this.eventHandler_.listen(a, [goog.events.EventType.TOUCHMOVE, goog.events.EventType.MOUSEMOVE], this.handleMove_, c);
  this.eventHandler_.listen(a, [goog.events.EventType.TOUCHEND, goog.events.EventType.MOUSEUP], this.endDrag, c);
  goog.fx.Dragger.HAS_SET_CAPTURE_ ? (b.setCapture(!1), this.eventHandler_.listen(b, goog.events.EventType.LOSECAPTURE, this.endDrag)) : this.eventHandler_.listen(goog.dom.getWindow(a), goog.events.EventType.BLUR, this.endDrag);
  goog.userAgent.IE && this.ieDragStartCancellingOn_ && this.eventHandler_.listen(a, goog.events.EventType.DRAGSTART, goog.events.Event.preventDefault);
  this.scrollTarget_ && this.eventHandler_.listen(this.scrollTarget_, goog.events.EventType.SCROLL, this.onScroll_, c)
};
goog.fx.Dragger.prototype.initializeDrag_ = function(a) {
  if(this.dispatchEvent(new goog.fx.DragEvent(goog.fx.Dragger.EventType.START, this, a.clientX, a.clientY, a)) !== !1) {
    this.dragging_ = !0
  }
};
goog.fx.Dragger.prototype.endDrag = function(a, b) {
  this.eventHandler_.removeAll();
  goog.fx.Dragger.HAS_SET_CAPTURE_ && this.document_.releaseCapture();
  var c = this.limitX(this.deltaX), d = this.limitY(this.deltaY);
  this.dragging_ ? (this.maybeReinitTouchEvent_(a), this.dragging_ = !1, this.dispatchEvent(new goog.fx.DragEvent(goog.fx.Dragger.EventType.END, this, a.clientX, a.clientY, a, c, d, b || a.type == goog.events.EventType.TOUCHCANCEL))) : this.dispatchEvent(goog.fx.Dragger.EventType.EARLY_CANCEL);
  (a.type == goog.events.EventType.TOUCHEND || a.type == goog.events.EventType.TOUCHCANCEL) && a.preventDefault()
};
goog.fx.Dragger.prototype.endDragCancel = function(a) {
  this.endDrag(a, !0)
};
goog.fx.Dragger.prototype.maybeReinitTouchEvent_ = function(a) {
  var b = a.type;
  b == goog.events.EventType.TOUCHSTART || b == goog.events.EventType.TOUCHMOVE ? a.init(a.getBrowserEvent().targetTouches[0], a.currentTarget) : (b == goog.events.EventType.TOUCHEND || b == goog.events.EventType.TOUCHCANCEL) && a.init(a.getBrowserEvent().changedTouches[0], a.currentTarget)
};
goog.fx.Dragger.prototype.handleMove_ = function(a) {
  if(this.enabled_) {
    this.maybeReinitTouchEvent_(a);
    var b = a.clientX - this.clientX, c = a.clientY - this.clientY;
    this.clientX = a.clientX;
    this.clientY = a.clientY;
    this.screenX = a.screenX;
    this.screenY = a.screenY;
    if(!this.dragging_) {
      var d = this.startX - this.clientX, e = this.startY - this.clientY;
      if(d * d + e * e > this.hysteresisDistanceSquared_ && (this.initializeDrag_(a), !this.dragging_)) {
        this.endDrag(a);
        return
      }
    }
    c = this.calculatePosition_(b, c);
    b = c.x;
    c = c.y;
    this.dragging_ && this.dispatchEvent(new goog.fx.DragEvent(goog.fx.Dragger.EventType.BEFOREDRAG, this, a.clientX, a.clientY, a, b, c)) !== !1 && (this.doDrag(a, b, c, !1), a.preventDefault())
  }
};
goog.fx.Dragger.prototype.calculatePosition_ = function(a, b) {
  var c = goog.dom.getDomHelper(this.document_).getDocumentScroll();
  a += c.x - this.pageScroll.x;
  b += c.y - this.pageScroll.y;
  this.pageScroll = c;
  this.deltaX += a;
  this.deltaY += b;
  var c = this.limitX(this.deltaX), d = this.limitY(this.deltaY);
  return new goog.math.Coordinate(c, d)
};
goog.fx.Dragger.prototype.onScroll_ = function(a) {
  var b = this.calculatePosition_(0, 0);
  a.clientX = this.clientX;
  a.clientY = this.clientY;
  this.doDrag(a, b.x, b.y, !0)
};
goog.fx.Dragger.prototype.doDrag = function(a, b, c) {
  this.defaultAction(b, c);
  this.dispatchEvent(new goog.fx.DragEvent(goog.fx.Dragger.EventType.DRAG, this, a.clientX, a.clientY, a, b, c))
};
goog.fx.Dragger.prototype.limitX = function(a) {
  var b = this.limits, c = !isNaN(b.left) ? b.left : null, b = !isNaN(b.width) ? b.width : 0;
  return Math.min(c != null ? c + b : Infinity, Math.max(c != null ? c : -Infinity, a))
};
goog.fx.Dragger.prototype.limitY = function(a) {
  var b = this.limits, c = !isNaN(b.top) ? b.top : null, b = !isNaN(b.height) ? b.height : 0;
  return Math.min(c != null ? c + b : Infinity, Math.max(c != null ? c : -Infinity, a))
};
goog.fx.Dragger.prototype.defaultAction = function(a, b) {
  this.target.style.left = a + "px";
  this.target.style.top = b + "px"
};
goog.fx.DragEvent = function(a, b, c, d, e, f, g, h) {
  goog.events.Event.call(this, a);
  this.clientX = c;
  this.clientY = d;
  this.browserEvent = e;
  this.left = goog.isDef(f) ? f : b.deltaX;
  this.top = goog.isDef(g) ? g : b.deltaY;
  this.dragger = b;
  this.dragCanceled = !!h
};
goog.inherits(goog.fx.DragEvent, goog.events.Event);
goog.color = {};
goog.color.names = {aliceblue:"#f0f8ff", antiquewhite:"#faebd7", aqua:"#00ffff", aquamarine:"#7fffd4", azure:"#f0ffff", beige:"#f5f5dc", bisque:"#ffe4c4", black:"#000000", blanchedalmond:"#ffebcd", blue:"#0000ff", blueviolet:"#8a2be2", brown:"#a52a2a", burlywood:"#deb887", cadetblue:"#5f9ea0", chartreuse:"#7fff00", chocolate:"#d2691e", coral:"#ff7f50", cornflowerblue:"#6495ed", cornsilk:"#fff8dc", crimson:"#dc143c", cyan:"#00ffff", darkblue:"#00008b", darkcyan:"#008b8b", darkgoldenrod:"#b8860b", 
darkgray:"#a9a9a9", darkgreen:"#006400", darkgrey:"#a9a9a9", darkkhaki:"#bdb76b", darkmagenta:"#8b008b", darkolivegreen:"#556b2f", darkorange:"#ff8c00", darkorchid:"#9932cc", darkred:"#8b0000", darksalmon:"#e9967a", darkseagreen:"#8fbc8f", darkslateblue:"#483d8b", darkslategray:"#2f4f4f", darkslategrey:"#2f4f4f", darkturquoise:"#00ced1", darkviolet:"#9400d3", deeppink:"#ff1493", deepskyblue:"#00bfff", dimgray:"#696969", dimgrey:"#696969", dodgerblue:"#1e90ff", firebrick:"#b22222", floralwhite:"#fffaf0", 
forestgreen:"#228b22", fuchsia:"#ff00ff", gainsboro:"#dcdcdc", ghostwhite:"#f8f8ff", gold:"#ffd700", goldenrod:"#daa520", gray:"#808080", green:"#008000", greenyellow:"#adff2f", grey:"#808080", honeydew:"#f0fff0", hotpink:"#ff69b4", indianred:"#cd5c5c", indigo:"#4b0082", ivory:"#fffff0", khaki:"#f0e68c", lavender:"#e6e6fa", lavenderblush:"#fff0f5", lawngreen:"#7cfc00", lemonchiffon:"#fffacd", lightblue:"#add8e6", lightcoral:"#f08080", lightcyan:"#e0ffff", lightgoldenrodyellow:"#fafad2", lightgray:"#d3d3d3", 
lightgreen:"#90ee90", lightgrey:"#d3d3d3", lightpink:"#ffb6c1", lightsalmon:"#ffa07a", lightseagreen:"#20b2aa", lightskyblue:"#87cefa", lightslategray:"#778899", lightslategrey:"#778899", lightsteelblue:"#b0c4de", lightyellow:"#ffffe0", lime:"#00ff00", limegreen:"#32cd32", linen:"#faf0e6", magenta:"#ff00ff", maroon:"#800000", mediumaquamarine:"#66cdaa", mediumblue:"#0000cd", mediumorchid:"#ba55d3", mediumpurple:"#9370d8", mediumseagreen:"#3cb371", mediumslateblue:"#7b68ee", mediumspringgreen:"#00fa9a", 
mediumturquoise:"#48d1cc", mediumvioletred:"#c71585", midnightblue:"#191970", mintcream:"#f5fffa", mistyrose:"#ffe4e1", moccasin:"#ffe4b5", navajowhite:"#ffdead", navy:"#000080", oldlace:"#fdf5e6", olive:"#808000", olivedrab:"#6b8e23", orange:"#ffa500", orangered:"#ff4500", orchid:"#da70d6", palegoldenrod:"#eee8aa", palegreen:"#98fb98", paleturquoise:"#afeeee", palevioletred:"#d87093", papayawhip:"#ffefd5", peachpuff:"#ffdab9", peru:"#cd853f", pink:"#ffc0cb", plum:"#dda0dd", powderblue:"#b0e0e6", 
purple:"#800080", red:"#ff0000", rosybrown:"#bc8f8f", royalblue:"#4169e1", saddlebrown:"#8b4513", salmon:"#fa8072", sandybrown:"#f4a460", seagreen:"#2e8b57", seashell:"#fff5ee", sienna:"#a0522d", silver:"#c0c0c0", skyblue:"#87ceeb", slateblue:"#6a5acd", slategray:"#708090", slategrey:"#708090", snow:"#fffafa", springgreen:"#00ff7f", steelblue:"#4682b4", tan:"#d2b48c", teal:"#008080", thistle:"#d8bfd8", tomato:"#ff6347", turquoise:"#40e0d0", violet:"#ee82ee", wheat:"#f5deb3", white:"#ffffff", whitesmoke:"#f5f5f5", 
yellow:"#ffff00", yellowgreen:"#9acd32"};
goog.color.parse = function(a) {
  var b = {}, a = String(a), c = goog.color.prependPoundIfNecessary_(a);
  if(goog.color.isValidHexColor_(c)) {
    return b.hex = goog.color.normalizeHex(c), b.type = "hex", b
  }else {
    if(c = goog.color.isValidRgbColor_(a), c.length) {
      return b.hex = goog.color.rgbArrayToHex(c), b.type = "rgb", b
    }else {
      if(goog.color.names && (c = goog.color.names[a.toLowerCase()])) {
        return b.hex = c, b.type = "named", b
      }
    }
  }
  throw Error(a + " is not a valid color string");
};
goog.color.parseRgb = function(a) {
  var b = goog.color.isValidRgbColor_(a);
  if(!b.length) {
    throw Error(a + " is not a valid RGB color");
  }
  return b
};
goog.color.hexToRgbStyle = function(a) {
  return goog.color.rgbStyle_(goog.color.hexToRgb(a))
};
goog.color.hexTripletRe_ = /#(.)(.)(.)/;
goog.color.normalizeHex = function(a) {
  if(!goog.color.isValidHexColor_(a)) {
    throw Error("'" + a + "' is not a valid hex color");
  }
  a.length == 4 && (a = a.replace(goog.color.hexTripletRe_, "#$1$1$2$2$3$3"));
  return a.toLowerCase()
};
goog.color.hexToRgb = function(a) {
  var a = goog.color.normalizeHex(a), b = parseInt(a.substr(1, 2), 16), c = parseInt(a.substr(3, 2), 16), a = parseInt(a.substr(5, 2), 16);
  return[b, c, a]
};
goog.color.rgbToHex = function(a, b, c) {
  a = Number(a);
  b = Number(b);
  c = Number(c);
  if(isNaN(a) || a < 0 || a > 255 || isNaN(b) || b < 0 || b > 255 || isNaN(c) || c < 0 || c > 255) {
    throw Error('"(' + a + "," + b + "," + c + '") is not a valid RGB color');
  }
  a = goog.color.prependZeroIfNecessary_(a.toString(16));
  b = goog.color.prependZeroIfNecessary_(b.toString(16));
  c = goog.color.prependZeroIfNecessary_(c.toString(16));
  return"#" + a + b + c
};
goog.color.rgbArrayToHex = function(a) {
  return goog.color.rgbToHex(a[0], a[1], a[2])
};
goog.color.rgbToHsl = function(a, b, c) {
  a /= 255;
  b /= 255;
  c /= 255;
  var d = Math.max(a, b, c), e = Math.min(a, b, c), f = 0, g = 0, h = 0.5 * (d + e);
  d != e && (d == a ? f = 60 * (b - c) / (d - e) : d == b ? f = 60 * (c - a) / (d - e) + 120 : d == c && (f = 60 * (a - b) / (d - e) + 240), g = 0 < h && h <= 0.5 ? (d - e) / (2 * h) : (d - e) / (2 - 2 * h));
  return[Math.round(f + 360) % 360, g, h]
};
goog.color.rgbArrayToHsl = function(a) {
  return goog.color.rgbToHsl(a[0], a[1], a[2])
};
goog.color.hueToRgb_ = function(a, b, c) {
  c < 0 ? c += 1 : c > 1 && (c -= 1);
  if(6 * c < 1) {
    return a + (b - a) * 6 * c
  }else {
    if(2 * c < 1) {
      return b
    }else {
      if(3 * c < 2) {
        return a + (b - a) * (2 / 3 - c) * 6
      }
    }
  }
  return a
};
goog.color.hslToRgb = function(a, b, c) {
  var d = 0, e = 0, f = 0;
  a /= 360;
  if(b == 0) {
    d = e = f = c * 255
  }else {
    var g = f = 0, g = c < 0.5 ? c * (1 + b) : c + b - b * c, f = 2 * c - g, d = 255 * goog.color.hueToRgb_(f, g, a + 1 / 3), e = 255 * goog.color.hueToRgb_(f, g, a), f = 255 * goog.color.hueToRgb_(f, g, a - 1 / 3)
  }
  return[Math.round(d), Math.round(e), Math.round(f)]
};
goog.color.hslArrayToRgb = function(a) {
  return goog.color.hslToRgb(a[0], a[1], a[2])
};
goog.color.validHexColorRe_ = /^#(?:[0-9a-f]{3}){1,2}$/i;
goog.color.isValidHexColor_ = function(a) {
  return goog.color.validHexColorRe_.test(a)
};
goog.color.normalizedHexColorRe_ = /^#[0-9a-f]{6}$/;
goog.color.isNormalizedHexColor_ = function(a) {
  return goog.color.normalizedHexColorRe_.test(a)
};
goog.color.rgbColorRe_ = /^(?:rgb)?\((0|[1-9]\d{0,2}),\s?(0|[1-9]\d{0,2}),\s?(0|[1-9]\d{0,2})\)$/i;
goog.color.isValidRgbColor_ = function(a) {
  var b = a.match(goog.color.rgbColorRe_);
  if(b) {
    var a = Number(b[1]), c = Number(b[2]), b = Number(b[3]);
    if(a >= 0 && a <= 255 && c >= 0 && c <= 255 && b >= 0 && b <= 255) {
      return[a, c, b]
    }
  }
  return[]
};
goog.color.prependZeroIfNecessary_ = function(a) {
  return a.length == 1 ? "0" + a : a
};
goog.color.prependPoundIfNecessary_ = function(a) {
  return a.charAt(0) == "#" ? a : "#" + a
};
goog.color.rgbStyle_ = function(a) {
  return"rgb(" + a.join(",") + ")"
};
goog.color.hsvToRgb = function(a, b, c) {
  var d = 0, e = 0, f = 0;
  if(b == 0) {
    f = e = d = c
  }else {
    var g = Math.floor(a / 60), h = a / 60 - g, a = c * (1 - b), i = c * (1 - b * h), b = c * (1 - b * (1 - h));
    switch(g) {
      case 1:
        d = i;
        e = c;
        f = a;
        break;
      case 2:
        d = a;
        e = c;
        f = b;
        break;
      case 3:
        d = a;
        e = i;
        f = c;
        break;
      case 4:
        d = b;
        e = a;
        f = c;
        break;
      case 5:
        d = c;
        e = a;
        f = i;
        break;
      case 6:
      ;
      case 0:
        d = c, e = b, f = a
    }
  }
  return[Math.floor(d), Math.floor(e), Math.floor(f)]
};
goog.color.rgbToHsv = function(a, b, c) {
  var d = Math.max(Math.max(a, b), c), e = Math.min(Math.min(a, b), c);
  if(e == d) {
    e = a = 0
  }else {
    var f = d - e, e = f / d, a = a == d ? (b - c) / f : b == d ? 2 + (c - a) / f : 4 + (a - b) / f;
    a *= 60;
    a < 0 && (a += 360);
    a > 360 && (a -= 360)
  }
  return[a, e, d]
};
goog.color.rgbArrayToHsv = function(a) {
  return goog.color.rgbToHsv(a[0], a[1], a[2])
};
goog.color.hsvArrayToRgb = function(a) {
  return goog.color.hsvToRgb(a[0], a[1], a[2])
};
goog.color.hexToHsl = function(a) {
  a = goog.color.hexToRgb(a);
  return goog.color.rgbToHsl(a[0], a[1], a[2])
};
goog.color.hslToHex = function(a, b, c) {
  return goog.color.rgbArrayToHex(goog.color.hslToRgb(a, b, c))
};
goog.color.hslArrayToHex = function(a) {
  return goog.color.rgbArrayToHex(goog.color.hslToRgb(a[0], a[1], a[2]))
};
goog.color.hexToHsv = function(a) {
  return goog.color.rgbArrayToHsv(goog.color.hexToRgb(a))
};
goog.color.hsvToHex = function(a, b, c) {
  return goog.color.rgbArrayToHex(goog.color.hsvToRgb(a, b, c))
};
goog.color.hsvArrayToHex = function(a) {
  return goog.color.hsvToHex(a[0], a[1], a[2])
};
goog.color.hslDistance = function(a, b) {
  var c, d;
  c = a[2] <= 0.5 ? a[1] * a[2] : a[1] * (1 - a[2]);
  d = b[2] <= 0.5 ? b[1] * b[2] : b[1] * (1 - b[2]);
  return(a[2] - b[2]) * (a[2] - b[2]) + c * c + d * d - 2 * c * d * Math.cos((a[0] / 360 - b[0] / 360) * 2 * Math.PI)
};
goog.color.blend = function(a, b, c) {
  c = goog.math.clamp(c, 0, 1);
  return[Math.round(c * a[0] + (1 - c) * b[0]), Math.round(c * a[1] + (1 - c) * b[1]), Math.round(c * a[2] + (1 - c) * b[2])]
};
goog.color.darken = function(a, b) {
  return goog.color.blend([0, 0, 0], a, b)
};
goog.color.lighten = function(a, b) {
  return goog.color.blend([255, 255, 255], a, b)
};
goog.color.highContrast = function(a, b) {
  for(var c = [], d = 0;d < b.length;d++) {
    c.push({color:b[d], diff:goog.color.yiqBrightnessDiff_(b[d], a) + goog.color.colorDiff_(b[d], a)})
  }
  c.sort(function(a, b) {
    return b.diff - a.diff
  });
  return c[0].color
};
goog.color.yiqBrightness_ = function(a) {
  return Math.round((a[0] * 299 + a[1] * 587 + a[2] * 114) / 1E3)
};
goog.color.yiqBrightnessDiff_ = function(a, b) {
  return Math.abs(goog.color.yiqBrightness_(a) - goog.color.yiqBrightness_(b))
};
goog.color.colorDiff_ = function(a, b) {
  return Math.abs(a[0] - b[0]) + Math.abs(a[1] - b[1]) + Math.abs(a[2] - b[2])
};
goog.fx.dom = {};
goog.fx.dom.PredefinedEffect = function(a, b, c, d, e) {
  goog.fx.Animation.call(this, b, c, d, e);
  this.element = a
};
goog.inherits(goog.fx.dom.PredefinedEffect, goog.fx.Animation);
goog.fx.dom.PredefinedEffect.prototype.updateStyle = goog.nullFunction;
goog.fx.dom.PredefinedEffect.prototype.onAnimate = function() {
  this.updateStyle();
  goog.fx.dom.PredefinedEffect.superClass_.onAnimate.call(this)
};
goog.fx.dom.PredefinedEffect.prototype.onEnd = function() {
  this.updateStyle();
  goog.fx.dom.PredefinedEffect.superClass_.onEnd.call(this)
};
goog.fx.dom.PredefinedEffect.prototype.onBegin = function() {
  this.updateStyle();
  goog.fx.dom.PredefinedEffect.superClass_.onBegin.call(this)
};
goog.fx.dom.Slide = function(a, b, c, d, e) {
  if(b.length != 2 || c.length != 2) {
    throw Error("Start and end points must be 2D");
  }
  goog.fx.dom.PredefinedEffect.apply(this, arguments)
};
goog.inherits(goog.fx.dom.Slide, goog.fx.dom.PredefinedEffect);
goog.fx.dom.Slide.prototype.updateStyle = function() {
  this.element.style.left = Math.round(this.coords[0]) + "px";
  this.element.style.top = Math.round(this.coords[1]) + "px"
};
goog.fx.dom.SlideFrom = function(a, b, c, d) {
  goog.fx.dom.Slide.call(this, a, [a.offsetLeft, a.offsetTop], b, c, d)
};
goog.inherits(goog.fx.dom.SlideFrom, goog.fx.dom.Slide);
goog.fx.dom.SlideFrom.prototype.onBegin = function() {
  this.startPoint = [this.element.offsetLeft, this.element.offsetTop];
  goog.fx.dom.SlideFrom.superClass_.onBegin.call(this)
};
goog.fx.dom.Swipe = function(a, b, c, d, e) {
  if(b.length != 2 || c.length != 2) {
    throw Error("Start and end points must be 2D");
  }
  goog.fx.dom.PredefinedEffect.apply(this, arguments);
  this.maxWidth_ = Math.max(this.endPoint[0], this.startPoint[0]);
  this.maxHeight_ = Math.max(this.endPoint[1], this.startPoint[1])
};
goog.inherits(goog.fx.dom.Swipe, goog.fx.dom.PredefinedEffect);
goog.fx.dom.Swipe.prototype.updateStyle = function() {
  var a = this.coords[0], b = this.coords[1];
  this.clip_(Math.round(a), Math.round(b), this.maxWidth_, this.maxHeight_);
  this.element.style.width = Math.round(a) + "px";
  this.element.style.marginLeft = Math.round(a) - this.maxWidth_ + "px";
  this.element.style.marginTop = Math.round(b) - this.maxHeight_ + "px"
};
goog.fx.dom.Swipe.prototype.clip_ = function(a, b, c, d) {
  this.element.style.clip = "rect(" + (d - b) + "px " + c + "px " + d + "px " + (c - a) + "px)"
};
goog.fx.dom.Scroll = function(a, b, c, d, e) {
  if(b.length != 2 || c.length != 2) {
    throw Error("Start and end points must be 2D");
  }
  goog.fx.dom.PredefinedEffect.apply(this, arguments)
};
goog.inherits(goog.fx.dom.Scroll, goog.fx.dom.PredefinedEffect);
goog.fx.dom.Scroll.prototype.updateStyle = function() {
  this.element.scrollLeft = Math.round(this.coords[0]);
  this.element.scrollTop = Math.round(this.coords[1])
};
goog.fx.dom.Resize = function(a, b, c, d, e) {
  if(b.length != 2 || c.length != 2) {
    throw Error("Start and end points must be 2D");
  }
  goog.fx.dom.PredefinedEffect.apply(this, arguments)
};
goog.inherits(goog.fx.dom.Resize, goog.fx.dom.PredefinedEffect);
goog.fx.dom.Resize.prototype.updateStyle = function() {
  this.element.style.width = Math.round(this.coords[0]) + "px";
  this.element.style.height = Math.round(this.coords[1]) + "px"
};
goog.fx.dom.ResizeWidth = function(a, b, c, d, e) {
  goog.fx.dom.PredefinedEffect.call(this, a, [b], [c], d, e)
};
goog.inherits(goog.fx.dom.ResizeWidth, goog.fx.dom.PredefinedEffect);
goog.fx.dom.ResizeWidth.prototype.updateStyle = function() {
  this.element.style.width = Math.round(this.coords[0]) + "px"
};
goog.fx.dom.ResizeHeight = function(a, b, c, d, e) {
  goog.fx.dom.PredefinedEffect.call(this, a, [b], [c], d, e)
};
goog.inherits(goog.fx.dom.ResizeHeight, goog.fx.dom.PredefinedEffect);
goog.fx.dom.ResizeHeight.prototype.updateStyle = function() {
  this.element.style.height = Math.round(this.coords[0]) + "px"
};
goog.fx.dom.Fade = function(a, b, c, d, e) {
  goog.isNumber(b) && (b = [b]);
  goog.isNumber(c) && (c = [c]);
  goog.fx.dom.PredefinedEffect.call(this, a, b, c, d, e);
  if(b.length != 1 || c.length != 1) {
    throw Error("Start and end points must be 1D");
  }
};
goog.inherits(goog.fx.dom.Fade, goog.fx.dom.PredefinedEffect);
goog.fx.dom.Fade.prototype.updateStyle = function() {
  goog.style.setOpacity(this.element, this.coords[0])
};
goog.fx.dom.Fade.prototype.show = function() {
  this.element.style.display = ""
};
goog.fx.dom.Fade.prototype.hide = function() {
  this.element.style.display = "none"
};
goog.fx.dom.FadeOut = function(a, b, c) {
  goog.fx.dom.Fade.call(this, a, 1, 0, b, c)
};
goog.inherits(goog.fx.dom.FadeOut, goog.fx.dom.Fade);
goog.fx.dom.FadeIn = function(a, b, c) {
  goog.fx.dom.Fade.call(this, a, 0, 1, b, c)
};
goog.inherits(goog.fx.dom.FadeIn, goog.fx.dom.Fade);
goog.fx.dom.FadeOutAndHide = function(a, b, c) {
  goog.fx.dom.Fade.call(this, a, 1, 0, b, c)
};
goog.inherits(goog.fx.dom.FadeOutAndHide, goog.fx.dom.Fade);
goog.fx.dom.FadeOutAndHide.prototype.onBegin = function() {
  this.show();
  goog.fx.dom.FadeOutAndHide.superClass_.onBegin.call(this)
};
goog.fx.dom.FadeOutAndHide.prototype.onEnd = function() {
  this.hide();
  goog.fx.dom.FadeOutAndHide.superClass_.onEnd.call(this)
};
goog.fx.dom.FadeInAndShow = function(a, b, c) {
  goog.fx.dom.Fade.call(this, a, 0, 1, b, c)
};
goog.inherits(goog.fx.dom.FadeInAndShow, goog.fx.dom.Fade);
goog.fx.dom.FadeInAndShow.prototype.onBegin = function() {
  this.show();
  goog.fx.dom.FadeInAndShow.superClass_.onBegin.call(this)
};
goog.fx.dom.BgColorTransform = function(a, b, c, d, e) {
  if(b.length != 3 || c.length != 3) {
    throw Error("Start and end points must be 3D");
  }
  goog.fx.dom.PredefinedEffect.apply(this, arguments)
};
goog.inherits(goog.fx.dom.BgColorTransform, goog.fx.dom.PredefinedEffect);
goog.fx.dom.BgColorTransform.prototype.setColor = function() {
  for(var a = [], b = 0;b < this.coords.length;b++) {
    a[b] = Math.round(this.coords[b])
  }
  this.element.style.backgroundColor = "rgb(" + a.join(",") + ")"
};
goog.fx.dom.BgColorTransform.prototype.updateStyle = function() {
  this.setColor()
};
goog.fx.dom.bgColorFadeIn = function(a, b, c, d) {
  function e() {
    a.style.backgroundColor = f
  }
  var f = a.style.backgroundColor || "", g = goog.style.getBackgroundColor(a), g = g && g != "transparent" && g != "rgba(0, 0, 0, 0)" ? goog.color.hexToRgb(goog.color.parse(g).hex) : [255, 255, 255], b = new goog.fx.dom.BgColorTransform(a, b, g, c);
  d ? d.listen(b, goog.fx.Transition.EventType.END, e) : goog.events.listen(b, goog.fx.Transition.EventType.END, e);
  b.play()
};
goog.fx.dom.ColorTransform = function(a, b, c, d, e) {
  if(b.length != 3 || c.length != 3) {
    throw Error("Start and end points must be 3D");
  }
  goog.fx.dom.PredefinedEffect.apply(this, arguments)
};
goog.inherits(goog.fx.dom.ColorTransform, goog.fx.dom.PredefinedEffect);
goog.fx.dom.ColorTransform.prototype.updateStyle = function() {
  for(var a = [], b = 0;b < this.coords.length;b++) {
    a[b] = Math.round(this.coords[b])
  }
  this.element.style.color = "rgb(" + a.join(",") + ")"
};
goog.ui.RangeModel = function() {
  goog.events.EventTarget.call(this)
};
goog.inherits(goog.ui.RangeModel, goog.events.EventTarget);
goog.ui.RangeModel.prototype.value_ = 0;
goog.ui.RangeModel.prototype.minimum_ = 0;
goog.ui.RangeModel.prototype.maximum_ = 100;
goog.ui.RangeModel.prototype.extent_ = 0;
goog.ui.RangeModel.prototype.step_ = 1;
goog.ui.RangeModel.prototype.isChanging_ = !1;
goog.ui.RangeModel.prototype.mute_ = !1;
goog.ui.RangeModel.prototype.setMute = function(a) {
  this.mute_ = a
};
goog.ui.RangeModel.prototype.setValue = function(a) {
  a = this.roundToStepWithMin(a);
  if(this.value_ != a) {
    this.value_ = a + this.extent_ > this.maximum_ ? this.maximum_ - this.extent_ : a < this.minimum_ ? this.minimum_ : a, !this.isChanging_ && !this.mute_ && this.dispatchEvent(goog.ui.Component.EventType.CHANGE)
  }
};
goog.ui.RangeModel.prototype.getValue = function() {
  return this.roundToStepWithMin(this.value_)
};
goog.ui.RangeModel.prototype.setExtent = function(a) {
  a = this.roundToStepWithMin(a);
  if(this.extent_ != a) {
    this.extent_ = a < 0 ? 0 : this.value_ + a > this.maximum_ ? this.maximum_ - this.value_ : a, !this.isChanging_ && !this.mute_ && this.dispatchEvent(goog.ui.Component.EventType.CHANGE)
  }
};
goog.ui.RangeModel.prototype.getExtent = function() {
  return this.roundToStep(this.extent_)
};
goog.ui.RangeModel.prototype.setMinimum = function(a) {
  if(this.minimum_ != a) {
    var b = this.isChanging_;
    this.isChanging_ = !0;
    this.minimum_ = a;
    if(a + this.extent_ > this.maximum_) {
      this.extent_ = this.maximum_ - this.minimum_
    }
    a > this.value_ && this.setValue(a);
    if(a > this.maximum_) {
      this.extent_ = 0, this.setMaximum(a), this.setValue(a)
    }
    this.isChanging_ = b;
    !this.isChanging_ && !this.mute_ && this.dispatchEvent(goog.ui.Component.EventType.CHANGE)
  }
};
goog.ui.RangeModel.prototype.getMinimum = function() {
  return this.roundToStepWithMin(this.minimum_)
};
goog.ui.RangeModel.prototype.setMaximum = function(a) {
  a = this.roundToStepWithMin(a);
  if(this.maximum_ != a) {
    var b = this.isChanging_;
    this.isChanging_ = !0;
    this.maximum_ = a;
    a < this.value_ + this.extent_ && this.setValue(a - this.extent_);
    if(a < this.minimum_) {
      this.extent_ = 0, this.setMinimum(a), this.setValue(this.maximum_)
    }
    if(a < this.minimum_ + this.extent_) {
      this.extent_ = this.maximum_ - this.minimum_
    }
    this.isChanging_ = b;
    !this.isChanging_ && !this.mute_ && this.dispatchEvent(goog.ui.Component.EventType.CHANGE)
  }
};
goog.ui.RangeModel.prototype.getMaximum = function() {
  return this.roundToStepWithMin(this.maximum_)
};
goog.ui.RangeModel.prototype.getStep = function() {
  return this.step_
};
goog.ui.RangeModel.prototype.setStep = function(a) {
  if(this.step_ != a) {
    this.step_ = a, a = this.isChanging_, this.isChanging_ = !0, this.setMaximum(this.getMaximum()), this.setExtent(this.getExtent()), this.setValue(this.getValue()), this.isChanging_ = a, !this.isChanging_ && !this.mute_ && this.dispatchEvent(goog.ui.Component.EventType.CHANGE)
  }
};
goog.ui.RangeModel.prototype.roundToStepWithMin = function(a) {
  return this.step_ == null ? a : this.minimum_ + Math.round((a - this.minimum_) / this.step_) * this.step_
};
goog.ui.RangeModel.prototype.roundToStep = function(a) {
  return this.step_ == null ? a : Math.round(a / this.step_) * this.step_
};
goog.ui.SliderBase = function(a) {
  goog.ui.Component.call(this, a);
  this.rangeModel = new goog.ui.RangeModel;
  goog.events.listen(this.rangeModel, goog.ui.Component.EventType.CHANGE, this.handleRangeModelChange, !1, this)
};
goog.inherits(goog.ui.SliderBase, goog.ui.Component);
goog.ui.SliderBase.Orientation = {VERTICAL:"vertical", HORIZONTAL:"horizontal"};
goog.ui.SliderBase.prototype.orientation_ = goog.ui.SliderBase.Orientation.HORIZONTAL;
goog.ui.SliderBase.MOUSE_DOWN_INCREMENT_INTERVAL_ = 200;
goog.ui.SliderBase.ANIMATION_INTERVAL_ = 100;
goog.ui.SliderBase.prototype.isAnimating_ = !1;
goog.ui.SliderBase.prototype.moveToPointEnabled_ = !1;
goog.ui.SliderBase.prototype.blockIncrement_ = 10;
goog.ui.SliderBase.prototype.minExtent_ = 0;
goog.ui.SliderBase.prototype.createDom = function() {
  goog.ui.SliderBase.superClass_.createDom.call(this);
  this.decorateInternal(this.getDomHelper().createDom("div", this.getCssClass(this.orientation_)))
};
goog.ui.SliderBase.prototype.decorateInternal = function(a) {
  goog.ui.SliderBase.superClass_.decorateInternal.call(this, a);
  goog.dom.classes.add(a, this.getCssClass(this.orientation_));
  this.createThumbs();
  this.setAriaRoles()
};
goog.ui.SliderBase.prototype.enterDocument = function() {
  goog.ui.SliderBase.superClass_.enterDocument.call(this);
  this.valueDragger_ = new goog.fx.Dragger(this.valueThumb);
  this.extentDragger_ = new goog.fx.Dragger(this.extentThumb);
  this.valueDragger_.defaultAction = this.extentDragger_.defaultAction = goog.nullFunction;
  this.keyHandler_ = new goog.events.KeyHandler(this.getElement());
  this.mouseWheelHandler_ = new goog.events.MouseWheelHandler(this.getElement());
  this.getHandler().listen(this.valueDragger_, goog.fx.Dragger.EventType.BEFOREDRAG, this.handleBeforeDrag_).listen(this.extentDragger_, goog.fx.Dragger.EventType.BEFOREDRAG, this.handleBeforeDrag_).listen(this.keyHandler_, goog.events.KeyHandler.EventType.KEY, this.handleKeyDown_).listen(this.getElement(), goog.events.EventType.MOUSEDOWN, this.handleMouseDown_).listen(this.mouseWheelHandler_, goog.events.MouseWheelHandler.EventType.MOUSEWHEEL, this.handleMouseWheel_);
  this.getElement().tabIndex = 0;
  this.updateUi_()
};
goog.ui.SliderBase.prototype.exitDocument = function() {
  goog.ui.SliderBase.superClass_.exitDocument.call(this);
  goog.disposeAll(this.valueDragger_, this.extentDragger_, this.keyHandler_, this.mouseWheelHandler_)
};
goog.ui.SliderBase.prototype.handleBeforeDrag_ = function(a) {
  var b = a.dragger == this.valueDragger_ ? this.valueThumb : this.extentThumb, c;
  this.orientation_ == goog.ui.SliderBase.Orientation.VERTICAL ? (c = this.getElement().clientHeight - b.offsetHeight, c = (c - a.top) / c * (this.getMaximum() - this.getMinimum()) + this.getMinimum()) : (c = this.getElement().clientWidth - b.offsetWidth, c = a.left / c * (this.getMaximum() - this.getMinimum()) + this.getMinimum());
  c = a.dragger == this.valueDragger_ ? Math.min(Math.max(c, this.getMinimum()), this.getValue() + this.getExtent()) : Math.min(Math.max(c, this.getValue()), this.getMaximum());
  this.setThumbPosition_(b, c)
};
goog.ui.SliderBase.prototype.handleKeyDown_ = function(a) {
  var b = !0;
  switch(a.keyCode) {
    case goog.events.KeyCodes.HOME:
      this.animatedSetValue(this.getMinimum());
      break;
    case goog.events.KeyCodes.END:
      this.animatedSetValue(this.getMaximum());
      break;
    case goog.events.KeyCodes.PAGE_UP:
      this.moveThumbs(this.getBlockIncrement());
      break;
    case goog.events.KeyCodes.PAGE_DOWN:
      this.moveThumbs(-this.getBlockIncrement());
      break;
    case goog.events.KeyCodes.LEFT:
    ;
    case goog.events.KeyCodes.DOWN:
      this.moveThumbs(a.shiftKey ? -this.getBlockIncrement() : -this.getUnitIncrement());
      break;
    case goog.events.KeyCodes.RIGHT:
    ;
    case goog.events.KeyCodes.UP:
      this.moveThumbs(a.shiftKey ? this.getBlockIncrement() : this.getUnitIncrement());
      break;
    default:
      b = !1
  }
  b && a.preventDefault()
};
goog.ui.SliderBase.prototype.handleMouseDown_ = function(a) {
  this.getElement().focus && this.getElement().focus();
  var b = a.target;
  !goog.dom.contains(this.valueThumb, b) && !goog.dom.contains(this.extentThumb, b) && (this.moveToPointEnabled_ ? this.animatedSetValue(this.getValueFromMousePosition_(a)) : this.startBlockIncrementing_(a))
};
goog.ui.SliderBase.prototype.handleMouseWheel_ = function(a) {
  this.moveThumbs((a.detail > 0 ? -1 : 1) * this.getUnitIncrement());
  a.preventDefault()
};
goog.ui.SliderBase.prototype.startBlockIncrementing_ = function(a) {
  this.storeMousePos_(a);
  this.thumbToMove_ = this.getClosestThumb_(this.getValueFromMousePosition_(a));
  this.incrementing_ = this.orientation_ == goog.ui.SliderBase.Orientation.VERTICAL ? this.lastMousePosition_ < this.thumbToMove_.offsetTop : this.lastMousePosition_ > this.thumbToMove_.offsetLeft + this.thumbToMove_.offsetWidth;
  a = goog.dom.getOwnerDocument(this.getElement());
  this.getHandler().listen(a, goog.events.EventType.MOUSEUP, this.handleMouseUp_, !0).listen(this.getElement(), goog.events.EventType.MOUSEMOVE, this.storeMousePos_);
  if(!this.incTimer_) {
    this.incTimer_ = new goog.Timer(goog.ui.SliderBase.MOUSE_DOWN_INCREMENT_INTERVAL_), this.getHandler().listen(this.incTimer_, goog.Timer.TICK, this.handleTimerTick_)
  }
  this.handleTimerTick_();
  this.incTimer_.start()
};
goog.ui.SliderBase.prototype.handleTimerTick_ = function() {
  var a;
  if(this.orientation_ == goog.ui.SliderBase.Orientation.VERTICAL) {
    var b = this.lastMousePosition_, c = this.thumbToMove_.offsetTop;
    this.incrementing_ ? b < c && (a = this.getThumbPosition_(this.thumbToMove_) + this.getBlockIncrement()) : b > c + this.thumbToMove_.offsetHeight && (a = this.getThumbPosition_(this.thumbToMove_) - this.getBlockIncrement())
  }else {
    b = this.lastMousePosition_, c = this.thumbToMove_.offsetLeft, this.incrementing_ ? b > c + this.thumbToMove_.offsetWidth && (a = this.getThumbPosition_(this.thumbToMove_) + this.getBlockIncrement()) : b < c && (a = this.getThumbPosition_(this.thumbToMove_) - this.getBlockIncrement())
  }
  goog.isDef(a) && this.setThumbPosition_(this.thumbToMove_, a)
};
goog.ui.SliderBase.prototype.handleMouseUp_ = function() {
  this.incTimer_ && this.incTimer_.stop();
  var a = goog.dom.getOwnerDocument(this.getElement());
  this.getHandler().unlisten(a, goog.events.EventType.MOUSEUP, this.handleMouseUp_, !0).unlisten(this.getElement(), goog.events.EventType.MOUSEMOVE, this.storeMousePos_)
};
goog.ui.SliderBase.prototype.getRelativeMousePos_ = function(a) {
  a = goog.style.getRelativePosition(a, this.getElement());
  return this.orientation_ == goog.ui.SliderBase.Orientation.VERTICAL ? a.y : a.x
};
goog.ui.SliderBase.prototype.storeMousePos_ = function(a) {
  this.lastMousePosition_ = this.getRelativeMousePos_(a)
};
goog.ui.SliderBase.prototype.getValueFromMousePosition_ = function(a) {
  var b = this.getMinimum(), c = this.getMaximum();
  if(this.orientation_ == goog.ui.SliderBase.Orientation.VERTICAL) {
    var d = this.valueThumb.offsetHeight, e = this.getElement().clientHeight - d, a = this.getRelativeMousePos_(a) - d / 2;
    return(c - b) * (e - a) / e + b
  }else {
    return d = this.valueThumb.offsetWidth, e = this.getElement().clientWidth - d, a = this.getRelativeMousePos_(a) - d / 2, (c - b) * a / e + b
  }
};
goog.ui.SliderBase.prototype.getThumbPosition_ = function(a) {
  if(a == this.valueThumb) {
    return this.rangeModel.getValue()
  }else {
    if(a == this.extentThumb) {
      return this.rangeModel.getValue() + this.rangeModel.getExtent()
    }else {
      throw Error("Illegal thumb element. Neither minThumb nor maxThumb");
    }
  }
};
goog.ui.SliderBase.prototype.moveThumbs = function(a) {
  var b = this.getThumbPosition_(this.valueThumb) + a, a = this.getThumbPosition_(this.extentThumb) + a, b = goog.math.clamp(b, this.getMinimum(), this.getMaximum() - this.minExtent_), a = goog.math.clamp(a, this.getMinimum() + this.minExtent_, this.getMaximum());
  this.setValueAndExtent(b, a - b)
};
goog.ui.SliderBase.prototype.setThumbPosition_ = function(a, b) {
  var c = null;
  a == this.extentThumb && b <= this.rangeModel.getMaximum() && b >= this.rangeModel.getValue() + this.minExtent_ && (c = b - this.rangeModel.getValue());
  var d = c || this.rangeModel.getExtent();
  a == this.valueThumb && b >= this.getMinimum() && b <= this.rangeModel.getValue() + d - this.minExtent_ && (d -= b - this.rangeModel.getValue(), this.rangeModel.roundToStepWithMin(b) + this.rangeModel.roundToStepWithMin(d) == this.rangeModel.roundToStepWithMin(b + d) && (this.setValueAndExtent(b, d), c = null));
  c != null && this.rangeModel.setExtent(c)
};
goog.ui.SliderBase.prototype.setValueAndExtent = function(a, b) {
  this.getMinimum() <= a && a <= this.getMaximum() - b && this.minExtent_ <= b && b <= this.getMaximum() - a && !(a == this.getValue() && b == this.getExtent()) && (this.rangeModel.setMute(!0), this.rangeModel.setExtent(0), this.rangeModel.setValue(a), this.rangeModel.setExtent(b), this.rangeModel.setMute(!1), this.updateUi_(), this.dispatchEvent(goog.ui.Component.EventType.CHANGE))
};
goog.ui.SliderBase.prototype.getMinimum = function() {
  return this.rangeModel.getMinimum()
};
goog.ui.SliderBase.prototype.setMinimum = function(a) {
  this.rangeModel.setMinimum(a)
};
goog.ui.SliderBase.prototype.getMaximum = function() {
  return this.rangeModel.getMaximum()
};
goog.ui.SliderBase.prototype.setMaximum = function(a) {
  this.rangeModel.setMaximum(a)
};
goog.ui.SliderBase.prototype.getValueThumb = function() {
  return this.valueThumb
};
goog.ui.SliderBase.prototype.getExtentThumb = function() {
  return this.extentThumb
};
goog.ui.SliderBase.prototype.getClosestThumb_ = function(a) {
  return a <= this.rangeModel.getValue() + this.rangeModel.getExtent() / 2 ? this.valueThumb : this.extentThumb
};
goog.ui.SliderBase.prototype.handleRangeModelChange = function() {
  this.updateUi_();
  this.updateAriaStates();
  this.dispatchEvent(goog.ui.Component.EventType.CHANGE)
};
goog.ui.SliderBase.prototype.updateUi_ = function() {
  if(this.valueThumb && !this.isAnimating_) {
    var a = this.getThumbCoordinateForValue_(this.getThumbPosition_(this.valueThumb)), b = this.getThumbCoordinateForValue_(this.getThumbPosition_(this.extentThumb));
    if(this.orientation_ == goog.ui.SliderBase.Orientation.VERTICAL) {
      if(this.valueThumb.style.top = a.y + "px", this.extentThumb.style.top = b.y + "px", this.rangeHighlight) {
        a = this.calculateRangeHighlightPositioning_(b.y, a.y, this.valueThumb.offsetHeight), this.rangeHighlight.style.top = a.offset + "px", this.rangeHighlight.style.height = a.size + "px"
      }
    }else {
      if(this.valueThumb.style.left = a.x + "px", this.extentThumb.style.left = b.x + "px", this.rangeHighlight) {
        a = this.calculateRangeHighlightPositioning_(a.x, b.x, this.valueThumb.offsetWidth), this.rangeHighlight.style.left = a.offset + "px", this.rangeHighlight.style.width = a.size + "px"
      }
    }
  }
};
goog.ui.SliderBase.prototype.calculateRangeHighlightPositioning_ = function(a, b, c) {
  var d = Math.ceil(c / 2);
  return{offset:a + d, size:b - a + c - 2 * d}
};
goog.ui.SliderBase.prototype.getThumbCoordinateForValue_ = function(a) {
  var b = new goog.math.Coordinate;
  if(this.valueThumb) {
    var c = this.getMinimum(), d = this.getMaximum(), a = a == c && c == d ? 0 : (a - c) / (d - c);
    this.orientation_ == goog.ui.SliderBase.Orientation.VERTICAL ? (c = this.valueThumb.offsetHeight, c = this.getElement().clientHeight - c, a = Math.round(a * c), b.y = c - a) : (c = this.getElement().clientWidth - this.valueThumb.offsetWidth, a = Math.round(a * c), b.x = a)
  }
  return b
};
goog.ui.SliderBase.prototype.animatedSetValue = function(a) {
  a = Math.min(this.getMaximum(), Math.max(a, this.getMinimum()));
  this.isAnimating_ && this.currentAnimation_.stop(!0);
  var b = new goog.fx.AnimationParallelQueue, c = this.getClosestThumb_(a), d = this.getThumbCoordinateForValue_(a);
  b.add(new goog.fx.dom.SlideFrom(c, this.orientation_ == goog.ui.SliderBase.Orientation.VERTICAL ? [c.offsetLeft, d.y] : [d.x, c.offsetTop], goog.ui.SliderBase.ANIMATION_INTERVAL_));
  this.rangeHighlight && this.addRangeHighlightAnimations_(c, d, b);
  this.currentAnimation_ = b;
  this.getHandler().listen(b, goog.fx.Transition.EventType.END, this.endAnimation_);
  this.isAnimating_ = !0;
  this.setThumbPosition_(c, a);
  b.play(!1)
};
goog.ui.SliderBase.prototype.addRangeHighlightAnimations_ = function(a, b, c) {
  var d = this.getThumbCoordinateForValue_(this.rangeModel.getValue()), e = this.getThumbCoordinateForValue_(this.rangeModel.getValue() + this.rangeModel.getExtent());
  a == this.valueThumb ? d = b : e = b;
  this.orientation_ == goog.ui.SliderBase.Orientation.VERTICAL ? (a = this.calculateRangeHighlightPositioning_(e.y, d.y, this.valueThumb.offsetHeight), c.add(new goog.fx.dom.SlideFrom(this.rangeHighlight, [this.rangeHighlight.offsetLeft, a.offset], goog.ui.SliderBase.ANIMATION_INTERVAL_)), c.add(new goog.fx.dom.ResizeHeight(this.rangeHighlight, this.rangeHighlight.offsetHeight, a.size, goog.ui.SliderBase.ANIMATION_INTERVAL_))) : (a = this.calculateRangeHighlightPositioning_(d.x, e.x, this.valueThumb.offsetWidth), 
  c.add(new goog.fx.dom.SlideFrom(this.rangeHighlight, [a.offset, this.rangeHighlight.offsetTop], goog.ui.SliderBase.ANIMATION_INTERVAL_)), c.add(new goog.fx.dom.ResizeWidth(this.rangeHighlight, this.rangeHighlight.offsetWidth, a.size, goog.ui.SliderBase.ANIMATION_INTERVAL_)))
};
goog.ui.SliderBase.prototype.endAnimation_ = function() {
  this.isAnimating_ = !1
};
goog.ui.SliderBase.prototype.setOrientation = function(a) {
  if(this.orientation_ != a) {
    var b = this.getCssClass(this.orientation_), c = this.getCssClass(a);
    this.orientation_ = a;
    if(this.getElement()) {
      goog.dom.classes.swap(this.getElement(), b, c);
      this.valueThumb.style.left = this.valueThumb.style.top = "";
      this.extentThumb.style.left = this.extentThumb.style.top = "";
      if(this.rangeHighlight) {
        this.rangeHighlight.style.left = this.rangeHighlight.style.top = "", this.rangeHighlight.style.width = this.rangeHighlight.style.height = ""
      }
      this.updateUi_()
    }
  }
};
goog.ui.SliderBase.prototype.getOrientation = function() {
  return this.orientation_
};
goog.ui.SliderBase.prototype.disposeInternal = function() {
  goog.ui.SliderBase.superClass_.disposeInternal.call(this);
  this.incTimer_ && this.incTimer_.dispose();
  delete this.incTimer_;
  this.currentAnimation_ && this.currentAnimation_.dispose();
  delete this.currentAnimation_;
  delete this.valueThumb;
  delete this.extentThumb;
  this.rangeHighlight && delete this.rangeHighlight;
  this.rangeModel.dispose();
  delete this.rangeModel;
  this.keyHandler_ && (this.keyHandler_.dispose(), delete this.keyHandler_);
  this.mouseWheelHandler_ && (this.mouseWheelHandler_.dispose(), delete this.mouseWheelHandler_);
  this.valueDragger_ && (this.valueDragger_.dispose(), delete this.valueDragger_);
  this.extentDragger_ && (this.extentDragger_.dispose(), delete this.extentDragger_)
};
goog.ui.SliderBase.prototype.getBlockIncrement = function() {
  return this.blockIncrement_
};
goog.ui.SliderBase.prototype.setBlockIncrement = function(a) {
  this.blockIncrement_ = a
};
goog.ui.SliderBase.prototype.setMinExtent = function(a) {
  this.minExtent_ = a
};
goog.ui.SliderBase.prototype.unitIncrement_ = 1;
goog.ui.SliderBase.prototype.getUnitIncrement = function() {
  return this.unitIncrement_
};
goog.ui.SliderBase.prototype.setUnitIncrement = function(a) {
  this.unitIncrement_ = a
};
goog.ui.SliderBase.prototype.getStep = function() {
  return this.rangeModel.getStep()
};
goog.ui.SliderBase.prototype.setStep = function(a) {
  this.rangeModel.setStep(a)
};
goog.ui.SliderBase.prototype.getMoveToPointEnabled = function() {
  return this.moveToPointEnabled_
};
goog.ui.SliderBase.prototype.setMoveToPointEnabled = function(a) {
  this.moveToPointEnabled_ = a
};
goog.ui.SliderBase.prototype.getValue = function() {
  return this.rangeModel.getValue()
};
goog.ui.SliderBase.prototype.setValue = function(a) {
  this.setThumbPosition_(this.valueThumb, a)
};
goog.ui.SliderBase.prototype.getExtent = function() {
  return this.rangeModel.getExtent()
};
goog.ui.SliderBase.prototype.setExtent = function(a) {
  this.setThumbPosition_(this.extentThumb, this.rangeModel.getValue() + a)
};
goog.ui.SliderBase.prototype.setVisible = function(a) {
  goog.style.showElement(this.getElement(), a);
  a && this.updateUi_()
};
goog.ui.SliderBase.prototype.setAriaRoles = function() {
  goog.dom.a11y.setRole(this.getElement(), goog.dom.a11y.Role.SLIDER);
  this.updateAriaStates()
};
goog.ui.SliderBase.prototype.updateAriaStates = function() {
  var a = this.getElement();
  a && (goog.dom.a11y.setState(a, goog.dom.a11y.State.VALUEMIN, this.getMinimum()), goog.dom.a11y.setState(a, goog.dom.a11y.State.VALUEMAX, this.getMaximum()), goog.dom.a11y.setState(a, goog.dom.a11y.State.VALUENOW, this.getValue()))
};
goog.ui.Slider = function(a) {
  goog.ui.SliderBase.call(this, a);
  this.rangeModel.setExtent(0)
};
goog.inherits(goog.ui.Slider, goog.ui.SliderBase);
goog.ui.Slider.Orientation = goog.ui.SliderBase.Orientation;
goog.ui.Slider.CSS_CLASS_PREFIX = "goog-slider";
goog.ui.Slider.THUMB_CSS_CLASS = goog.ui.Slider.CSS_CLASS_PREFIX + "-thumb";
goog.ui.Slider.prototype.getCssClass = function(a) {
  return a == goog.ui.SliderBase.Orientation.VERTICAL ? goog.ui.Slider.CSS_CLASS_PREFIX + "-vertical" : goog.ui.Slider.CSS_CLASS_PREFIX + "-horizontal"
};
goog.ui.Slider.prototype.createThumbs = function() {
  var a = this.getElement(), b = goog.dom.getElementsByTagNameAndClass(null, goog.ui.Slider.THUMB_CSS_CLASS, a)[0];
  b || (b = this.createThumb_(), a.appendChild(b));
  this.valueThumb = this.extentThumb = b
};
goog.ui.Slider.prototype.createThumb_ = function() {
  var a = this.getDomHelper().createDom("div", goog.ui.Slider.THUMB_CSS_CLASS);
  goog.dom.a11y.setRole(a, goog.dom.a11y.Role.BUTTON);
  return a
};
goog.math.Vec2 = function(a, b) {
  this.x = a;
  this.y = b
};
goog.inherits(goog.math.Vec2, goog.math.Coordinate);
goog.math.Vec2.randomUnit = function() {
  var a = Math.random() * Math.PI * 2;
  return new goog.math.Vec2(Math.cos(a), Math.sin(a))
};
goog.math.Vec2.random = function() {
  var a = Math.sqrt(Math.random()), b = Math.random() * Math.PI * 2;
  return new goog.math.Vec2(Math.cos(b) * a, Math.sin(b) * a)
};
goog.math.Vec2.fromCoordinate = function(a) {
  return new goog.math.Vec2(a.x, a.y)
};
goog.math.Vec2.prototype.clone = function() {
  return new goog.math.Vec2(this.x, this.y)
};
goog.math.Vec2.prototype.magnitude = function() {
  return Math.sqrt(this.x * this.x + this.y * this.y)
};
goog.math.Vec2.prototype.squaredMagnitude = function() {
  return this.x * this.x + this.y * this.y
};
goog.math.Vec2.prototype.scale = function(a) {
  this.x *= a;
  this.y *= a;
  return this
};
goog.math.Vec2.prototype.invert = function() {
  this.x = -this.x;
  this.y = -this.y;
  return this
};
goog.math.Vec2.prototype.normalize = function() {
  return this.scale(1 / this.magnitude())
};
goog.math.Vec2.prototype.add = function(a) {
  this.x += a.x;
  this.y += a.y;
  return this
};
goog.math.Vec2.prototype.subtract = function(a) {
  this.x -= a.x;
  this.y -= a.y;
  return this
};
goog.math.Vec2.prototype.equals = function(a) {
  return this == a || !!a && this.x == a.x && this.y == a.y
};
goog.math.Vec2.distance = goog.math.Coordinate.distance;
goog.math.Vec2.squaredDistance = goog.math.Coordinate.squaredDistance;
goog.math.Vec2.equals = goog.math.Coordinate.equals;
goog.math.Vec2.sum = function(a, b) {
  return new goog.math.Vec2(a.x + b.x, a.y + b.y)
};
goog.math.Vec2.difference = function(a, b) {
  return new goog.math.Vec2(a.x - b.x, a.y - b.y)
};
goog.math.Vec2.dot = function(a, b) {
  return a.x * b.x + a.y * b.y
};
goog.math.Vec2.lerp = function(a, b, c) {
  return new goog.math.Vec2(goog.math.lerp(a.x, b.x, c), goog.math.lerp(a.y, b.y, c))
};
