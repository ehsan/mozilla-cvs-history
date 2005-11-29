/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et: */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Darin Fisher <darin@meer.net>
 *  Boris Zbarsky <bzbarsky@mit.edu>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// This file contains common code that is loaded with each test file.
const I                    = Components.interfaces;
const C                    = Components.classes;
const nsIEventQueueService = I.nsIEventQueueService;
const nsIEventQueue        = I.nsIEventQueue;
const nsILocalFile         = I.nsILocalFile;
const nsIProperties        = I.nsIProperties;
const nsIFileInputStream   = I.nsIFileInputStream;
const nsIInputStream       = I.nsIInputStream;

const nsIDOMParser         = I.nsIDOMParser;
const nsIDOMSerializer     = I.nsIDOMSerializer;
const nsIDOMDocument       = I.nsIDOMDocument;
const nsIDOMElement        = I.nsIDOMElement;
const nsIDOMNode           = I.nsIDOMNode;
const nsIDOMCharacterData  = I.nsIDOMCharacterData;
const nsIDOMAttr           = I.nsIDOMAttr;
const nsIDOMProcessingInstruction = I.nsIDOMProcessingInstruction;

var _eqs;
var _quit = false;
var _fail = false;
var _running_event_loop = false;
var _tests_pending = 0;

function _TimerCallback(expr) {
  this._expr = expr;
}
_TimerCallback.prototype = {
  _expr: "",
  QueryInterface: function(iid) {
    if (iid.Equals(Components.interfaces.nsITimerCallback) ||
        iid.Equals(Components.interfaces.nsISupports))
      return this;
    throw Components.results.NS_ERROR_NO_INTERFACE;
  },
  notify: function(timer) {
    eval(this._expr);  
  }
};

function do_timeout(delay, expr) {
  var timer = Components.classes["@mozilla.org/timer;1"]
                        .createInstance(Components.interfaces.nsITimer);
  timer.initWithCallback(new _TimerCallback(expr), delay, timer.TYPE_ONE_SHOT);
}

function do_main() {
  if (_quit)
    return;

  dump("*** running event loop\n");
  var eq = _eqs.getSpecialEventQueue(_eqs.CURRENT_THREAD_EVENT_QUEUE);

  _running_event_loop = true;
  eq.eventLoop();  // unblocked via interrupt from do_quit()
  _running_event_loop = false;

  // process any remaining events before exiting
  eq.processPendingEvents();
  eq.stopAcceptingEvents();
  eq.processPendingEvents();
}

function do_quit() {
  dump("*** exiting\n");

  _quit = true;

  if (_running_event_loop) {
    // interrupt the current thread to make eventLoop return.
    var thr = Components.classes["@mozilla.org/thread;1"]
                        .createInstance(Components.interfaces.nsIThread);
    thr.currentThread.interrupt();
  }
}

function do_throw(text) {
  _fail = true;
  do_quit();
  dump("*** CHECK FAILED: " + text + "\n");
  var frame = Components.stack;
  while (frame != null) {
    dump(frame + "\n");
    frame = frame.caller;
  }
  throw Components.results.NS_ERROR_ABORT;
}

function do_check_neq(_left, _right) {
  if (_left == _right)
    do_throw(_left + " != " + _right);
}

function do_check_eq(_left, _right) {
  if (_left != _right)
    do_throw(_left + " == " + _right);
}

function do_test_pending() {
  dump("*** test pending\n");
  _tests_pending++;
}

function do_test_finished() {
  dump("*** test finished\n");
  if (--_tests_pending == 0)
    do_quit();
}

function DOMParser() {
  return C["@mozilla.org/xmlextras/domparser;1"].createInstance(nsIDOMParser);
}

function ParseFile(file) {
  if (typeof(file) == "string") {
    var dirServ = C["@mozilla.org/file/directory_service;1"]
                   .getService(nsIProperties);
    var dummy = {};
    var fileObj = dirServ.get("CurProcD", nsILocalFile);
    fileObj.append("content_unit_tests");
    fileObj.append(file);
    file = fileObj;
  }

  do_check_eq(file instanceof nsILocalFile, true);

  fileStr = C["@mozilla.org/network/file-input-stream;1"]
             .createInstance(nsIFileInputStream);
  // Init for readonly reading
  fileStr.init(file,  0x01, 0400, nsIFileInputStream.CLOSE_ON_EOF);
  return ParseXML(fileStr);
}

function ParseXML(data) {
  if (typeof(data) == "string") {
    return DOMParser().parseFromString(data, "application/xml");
  }

  do_check_eq(data instanceof nsIInputStream, true);
  
  return DOMParser().parseFromStream(data, "UTF-8", data.available(),
                                     "application/xml");
}

function DOMSerializer() {
  return C["@mozilla.org/xmlextras/xmlserializer;1"]
          .createInstance(nsIDOMSerializer);
}

function SerializeXML(node) {
  return DOMSerializer().serializeToString(node);
}

function roundtrip(obj) {
  if (typeof(obj) == "string") {
    return SerializeXML(ParseXML(obj));
  }

  do_check_eq(obj instanceof nsIDOMNode, true);
  return ParseXML(SerializeXML(obj));
}

function do_compare_attrs(e1, e2) {
  const xmlns = "http://www.w3.org/2000/xmlns/";

  var a1 = e1.attributes;
  var a2 = e2.attributes;
  for (var i = 0; i < a1.length; ++i) {
    var att = a1.item(i);
    // Don't test for namespace decls, since those can just sorta be
    // scattered about
    if (att.namespaceURI != xmlns) {
      var att2 = a2.getNamedItemNS(att.namespaceURI, att.localName);
      if (!att2) {
        do_throw("Missing attribute with namespaceURI '" + att.namespaceURI +
                 "' and localName '" + att.localName + "'");
      }
      do_check_eq(att.QueryInterface(nsIDOMAttr).value, 
                  att2.QueryInterface(nsIDOMAttr).value);
    }
  }
}

function do_check_equiv(dom1, dom2) {
  do_check_eq(dom1.nodeType, dom2.nodeType);
  // There's no classinfo around, so we'll need to do some QIing to
  // make sure the right interfaces are flattened as needed.
  switch (dom1.nodeType) {
  case nsIDOMNode.PROCESSING_INSTRUCTION_NODE:
    do_check_eq(dom1.QueryInterface(nsIDOMProcessingInstruction).target, 
                dom2.QueryInterface(nsIDOMProcessingInstruction).target);
    do_check_eq(dom1.data, dom2.data);
  case nsIDOMNode.TEXT_NODE:
  case nsIDOMNode.CDATA_SECTION_NODE:
  case nsIDOMNode.COMMENT_NODE:
    do_check_eq(dom1.QueryInterface(nsIDOMCharacterData).data,
                dom2.QueryInterface(nsIDOMCharacterData).data);
    break;
  case nsIDOMNode.ELEMENT_NODE:
    do_check_eq(dom1.namespaceURI, dom2.namespaceURI);
    do_check_eq(dom1.localName, dom2.localName);
    // Compare attrs in both directions -- do_compare_attrs does a
    // subset check.
    do_compare_attrs(dom1, dom2);
    do_compare_attrs(dom2, dom1);
    // Fall through
  case nsIDOMNode.DOCUMENT_NODE:
    do_check_eq(dom1.childNodes.length, dom2.childNodes.length);
    for (var i = 0; i < dom1.childNodes.length; ++i) {
      do_check_equiv(dom1.childNodes.item(i), dom2.childNodes.item(i));
    }
    break;
  }
}

function do_check_serialize(dom) {
  do_check_equiv(dom, roundtrip(dom));
}

// setup the main thread event queue
_eqs = Components.classes["@mozilla.org/event-queue-service;1"]
                 .getService(nsIEventQueueService);
_eqs.createMonitoredThreadEventQueue();
