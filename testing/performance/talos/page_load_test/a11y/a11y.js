const nsIAccessible = Components.interfaces.nsIAccessible;
const nsIAccessNode = Components.interfaces.nsIAccessNode;
const nsIDOMNode = Components.interfaces.nsIDOMNode;

gAccRetrieval = 0;

function initAccessibility()
{
  netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect");
  if (!gAccRetrieval) {
    var retrieval = Components.classes["@mozilla.org/accessibleRetrieval;1"];
    if (retrieval) { // fails if build lacks accessibility module
      gAccRetrieval =
      Components.classes["@mozilla.org/accessibleRetrieval;1"]
                .getService(Components.interfaces.nsIAccessibleRetrieval);
    }
  }
  return gAccRetrieval;
}

function getAccessible(aAccOrElmOrID, aInterfaces)
{
  if (!aAccOrElmOrID) {
    return null;
  }

  var elm = null;

  if (aAccOrElmOrID instanceof nsIAccessible) {
    aAccOrElmOrID.QueryInterface(nsIAccessNode);
    elm = aAccOrElmOrID.DOMNode;

  } else if (aAccOrElmOrID instanceof nsIDOMNode) {
    elm = aAccOrElmOrID;

  } else {
    elm = document.getElementById(aAccOrElmOrID);
  }

  var acc = (aAccOrElmOrID instanceof nsIAccessible) ? aAccOrElmOrID : null;
  if (!acc) {
    try {
      acc = gAccRetrieval.getAccessibleFor(elm);
    } catch (e) {
    }
  }

  if (!aInterfaces) {
    return acc;
  }

  if (aInterfaces instanceof Array) {
    for (var index = 0; index < aInterfaces.length; index++) {
      try {
        acc.QueryInterface(aInterfaces[index]);
      } catch (e) {
      }
    }
    return acc;
  }
  
  try {
    acc.QueryInterface(aInterfaces);
  } catch (e) {
  }
  
  return acc;
}

// Walk accessible tree of the given identifier to ensure tree creation
function ensureAccessibleTree(aAccOrElmOrID)
{
  var acc = getAccessible(aAccOrElmOrID);
  if (!acc) {
    return;
  }

  var child = acc.firstChild;
  while (child) {
    ensureAccessibleTree(child);
    try {
      child = child.nextSibling;
    } catch (e) {
      child = null;
    }
  }
}