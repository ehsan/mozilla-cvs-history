/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/**
 * In this file we clean up the address books created during the tests.
 */

var abs = [ kPABData.fileName,
            kCABData.fileName ];

try {
  // Get the top-level directory
  var rdf = Components.classes["@mozilla.org/rdf/rdf-service;1"]
                      .getService(Components.interfaces.nsIRDFService);

  var childNodes = rdf.GetResource("moz-abdirectory:///")
                      .QueryInterface(Components.interfaces.nsIAbDirectory)
                      .childNodes;

  // and iterate through it, shutting down MDB databases where appropriate
  while (childNodes.hasMoreElements()) {
    var ab = childNodes.getNext();
    if (ab instanceof Components.interfaces.nsIAbDirectory) {
      var database = ab.database;
      if (database)
        database.forceClosed();
    }
  }

  // Now remove the directory
  if (profileDir.exists())
    profileDir.remove(true);
}
catch (e) {
  throw "FAILED to clean up AB tests: " + e;
}
