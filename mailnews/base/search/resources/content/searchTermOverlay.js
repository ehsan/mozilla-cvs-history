/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *   Alec Flett <alecf@netscape.com>
 */

var gTotalSearchTerms=0;
var gSearchRowContainer;
var gSearchTerms = new Array;
var gSearchRemovedTerms = new Array;
var gSearchScope;
var gSearchLessButton;
var gSearchBooleanRadiogroup;

//
function searchTermContainer() {}

searchTermContainer.prototype = {
    internalSearchTerm : '',
    internalBooleanAnd : '',

    // this.searchTerm: the actual nsIMsgSearchTerm object
    get searchTerm() { return this.internalSearchTerm; },
    set searchTerm(val) {
        this.internalSearchTerm = val;

        var term = val;
        // val is a nsIMsgSearchTerm
        var searchAttribute=this.searchattribute;
        var searchOperator=this.searchoperator;
        var searchValue=this.searchvalue;

        // now reflect all attributes of the searchterm into the widgets
        if (searchAttribute) searchAttribute.value = term.attrib;
        if (searchOperator) searchOperator.value = val.op;
        if (searchValue) searchValue.value = term.value;

        this.booleanAnd = val.booleanAnd;
        return val;
    },

    // searchscope - just forward to the searchattribute
    get searchScope() {
        if (this.searchattribute)
          return this.searchattribute.searchScope;
        return undefined;
    },
    set searchScope(val) {
        var searchAttribute = this.searchattribute;
        if (searchAttribute) searchAttribute.searchScope=val;
        return val;
    },

    saveId: function (element, slot) {
        this[slot] = element.id;
    },

    getElement: function (slot) {
        return document.getElementById(this[slot]);
    },

    // three well-defined properties:
    // searchattribute, searchoperator, searchvalue
    // the trick going on here is that we're storing the Element's Id,
    // not the element itself, because the XBL object may change out
    // from underneath us
    get searchattribute() { return this.getElement("internalSearchAttributeId"); },
    set searchattribute(val) {
        this.saveId(val, "internalSearchAttributeId");
        return val;
    },
    get searchoperator() { return this.getElement("internalSearchOperatorId"); },
    set searchoperator(val) {
        this.saveId(val, "internalSearchOperatorId");
        return val;
    },
    get searchvalue() { return this.getElement("internalSearchValueId"); },
    set searchvalue(val) {
        this.saveId(val, "internalSearchValueId");
        return val;
    },

    booleanNodes: null,
    stringBundle: document.getElementById("bundle_search"),
    get booleanAnd() { return this.internalBooleanAnd; },
    set booleanAnd(val) {
        // whenever you set this, all nodes in booleanNodes
        // are updated to reflect the string

        if (this.internalBooleanAnd == val) return val;
        this.internalBooleanAnd = val;

        var booleanNodes = this.booleanNodes;
        if (!booleanNodes) return val;

        var stringBundle = this.stringBundle;
        var andString = val ? "And" : "Or";
        for (var i=0; i<booleanNodes.length; i++) {
            try {
                var staticString =
                    stringBundle.getString("search" + andString + i);
                if (staticString && staticString.length>0)
                    booleanNodes[i].setAttribute("label", staticString);
            } catch (ex) { /* no error, means string not found */}
        }
        return val;
    },

    save: function () {
        var searchTerm = this.searchTerm;
        searchTerm.attrib = this.searchattribute.value;
        searchTerm.op = this.searchoperator.value;
        if (this.searchvalue.value)
          this.searchvalue.save();
        else
          this.searchvalue.saveTo(searchTerm.value);
        searchTerm.value = this.searchvalue.value;
        searchTerm.booleanAnd = this.booleanAnd;
    },
    // if you have a search term element with no search term
    saveTo: function(searchTerm) {
        this.internalSearchTerm = searchTerm;
        this.save();
    }
}

var nsIMsgSearchTerm = Components.interfaces.nsIMsgSearchTerm;

function initializeSearchWidgets() {
    gSearchBooleanRadiogroup = document.getElementById("booleanAndGroup");
    gSearchRowContainer = document.getElementById("searchTermList");
    gSearchLessButton = document.getElementById("less");
    if (!gSearchLessButton)
        dump("I couldn't find less button!");
}

function initializeBooleanWidgets() {

    var booleanAnd = true;
    // get the boolean value from the first term
    var firstTerm = gSearchTerms[0];
    if (firstTerm)
        booleanAnd = firstTerm.booleanAnd;

    // target radio items have value="and" or value="or"
    targetValue = "or";
    if (booleanAnd) targetValue = "and";

    targetElement = gSearchBooleanRadiogroup.getElementsByAttribute("value", targetValue)[0];

    gSearchBooleanRadiogroup.selectedItem = targetElement;
}

function initializeSearchRows(scope, searchTerms)
{
    gTotalSearchTerms = searchTerms.Count();
    for (var i=0; i<gTotalSearchTerms; i++) {
        var searchTerm = searchTerms.QueryElementAt(i, nsIMsgSearchTerm);
        createSearchRow(i, scope, searchTerm);
    }
    initializeBooleanWidgets();
}

function onMore(event)
{
    if(gTotalSearchTerms==1)
  gSearchLessButton .removeAttribute("disabled", "false");
    createSearchRow(gTotalSearchTerms++, gSearchScope, null);
}

function onLess(event)
{
    if (gTotalSearchTerms>1)
        removeSearchRow(--gTotalSearchTerms);
    if (gTotalSearchTerms==1)
        gSearchLessButton .setAttribute("disabled", "true");
}

// set scope on all visible searhattribute tags
function setSearchScope(scope) {
    gSearchScope = scope;
    for (var i=0; i<gSearchTerms.length; i++) {
        gSearchTerms[i].searchattribute.searchScope = scope;
    }
}

function booleanChanged(event) {
    // when boolean changes, we have to update all the attributes on the
    // search terms

    var newBoolValue =
        (event.target.getAttribute("value") == "and") ? true : false;
    for (var i=0; i<gSearchTerms.length; i++) {
        var searchTerm = gSearchTerms[i];
        searchTerm.booleanAnd = newBoolValue;
    }
}


function createSearchRow(index, scope, searchTerm)
{
    var searchAttr = document.createElement("searchattribute");
    var searchOp = document.createElement("searchoperator");
    var searchVal = document.createElement("searchvalue");

    // now set up ids:
    searchAttr.id = "searchAttr" + index;
    searchOp.id  = "searchOp" + index;
    searchVal.id = "searchVal" + index;

    searchAttr.setAttribute("for", searchOp.id + "," + searchVal.id);

    var rowdata = new Array(null, searchAttr,
                            null, searchOp,
                            null, searchVal,
                            null);
    var searchrow = constructRow(rowdata);

    searchrow.id = "searchRow" + index;

    var searchTermObj = new searchTermContainer;
    // is this necessary?
    //searchTermElement.id = "searchTerm" + index;
    gSearchTerms[gSearchTerms.length] = searchTermObj;

    searchTermObj.searchattribute = searchAttr;
    searchTermObj.searchoperator = searchOp;
    searchTermObj.searchvalue = searchVal;

    // now invalidate the newly created items because they've been inserted
    // into the document, and XBL bindings will be inserted in their place
    //searchAttr = searchOp = searchVal = undefined;

    // and/or string handling:
    // this is scary - basically we want to take every other
    // treecell, (note the i+=2) which will be a text label,
    // and set the searchTermObj's
    // booleanNodes to that
    var stringNodes = new Array;
    var treecells = searchrow.firstChild.childNodes;
    var j=0;
    for (var i=0; i<treecells.length; i+=2) {
        stringNodes[j++] = treecells[i];
    }
    searchTermObj.booleanNodes = stringNodes;

    gSearchRowContainer.appendChild(searchrow);

    searchTermObj.searchScope = scope;
    // the search term will initialize the searchTerm element, including
    // .booleanAnd
    if (searchTerm) {
        searchTermObj.searchTerm = searchTerm;
    }

    // here, we don't have a searchTerm, so it's probably a new element -
    // we'll initialize the .booleanAnd from the existing setting in
    // the UI
    else
        searchTermObj.booleanAnd = getBooleanAnd();

}

// creates a <treerow> using the array treeCellChildren as
// the children of each treecell
function constructRow(treeCellChildren)
{
    var treeitem = document.createElement("treeitem");
    var row = document.createElement("treerow");
    for (var i = 0; i<treeCellChildren.length; i++) {
      var treecell = document.createElement("treecell");

      // it's ok to have empty cells
      if (treeCellChildren[i]) {
          treecell.setAttribute("allowevents", "true");
          treeCellChildren[i].setAttribute("flex", "1");
          treecell.appendChild(treeCellChildren[i]);
      }
      row.appendChild(treecell);
    }
    treeitem.appendChild(row);
    return treeitem;
}

function removeSearchRow(index)
{
    var searchTermObj = gSearchTerms[index];
    if (!searchTermObj) {
        return;
    }

    // need to remove row from tree, so walk upwards from the
    // searchattribute to find the first <treeitem>
    var treeItemRow = searchTermObj.searchattribute;
    while (treeItemRow) {
        if (treeItemRow.localName == "treeitem") break;
        treeItemRow = treeItemRow.parentNode;
    }

    if (!treeItemRow) {
        dump("Error: couldn't find parent treeitem!\n");
        return;
    }


    if (searchTermObj.searchTerm) {
        gSearchRemovedTerms[gSearchRemovedTerms.length] = searchTermObj.searchTerm;
    } else {
        //dump("That wasn't real. ignoring \n");
    }

    treeItemRow.parentNode.removeChild(treeItemRow);
    // remove it from the list of terms - XXX this does it?
    // remove the last element
    gSearchTerms.length--;
}

function getBooleanAnd()
{
    if (gSearchBooleanRadiogroup.selectedItem)
        return (gSearchBooleanRadiogroup.selectedItem.getAttribute("value") == "and") ? true : false;

    // default to false
    return false;
}

// save the search terms from the UI back to the actual search terms
// searchTerms: nsISupportsArray of terms
// termOwner:   object which can contain and create the terms
//              (will be unnecessary if we just make terms creatable
//               via XPCOM)
function saveSearchTerms(searchTerms, termOwner)
{
    var i;
    for (i = 0; i<gSearchTerms.length; i++) {
        try {
            var searchTerm = gSearchTerms[i].searchTerm;
            if (searchTerm)
                gSearchTerms[i].save();
            else {
                // need to create a new searchTerm, and somehow save it to that
                searchTerm = termOwner.createTerm();
                gSearchTerms[i].saveTo(searchTerm);
                termOwner.appendTerm(searchTerm);
            }
        } catch (ex) {
            dump("** Error saving element " + i + ": " + ex + "\n");
        }
    }

    // now remove the queued elements
    for (i=0; i<gSearchRemovedTerms.length; i++) {
        // this is so nasty, we have to iterate through
        // because GetIndexOf is acting funny
        var searchTermSupports =
            gSearchRemovedTerms[i].QueryInterface(Components.interfaces.nsISupports);
        searchTerms.RemoveElement(searchTermSupports);
    }

}

function onReset(event)
{
    while (gTotalSearchTerms>0)
        removeSearchRow(--gTotalSearchTerms);
    onMore(event);
}

function convertPRTimeToString(tm)
{
  var time = new Date();
  // PRTime is in microseconds, Javascript time is in seconds
  // so divide by 1000 when converting
  time.setTime(tm / 1000);
  
  return convertDateToString(time);
}

function convertDateToString(time)
{
  var dateStr = time.getMonth() + 1;
  dateStr += "/";
  dateStr += time.getDate();
  dateStr += "/";
  dateStr += 1900 + time.getYear();
  return dateStr;
}

function convertStringToPRTime(str)
{
  var time = new Date();
  time.setTime(Date.parse(str));
  // Javascript time is in seconds, PRTime is in microseconds
  // so multiply by 1000 when converting
  return (time.getTime() * 1000);
}
