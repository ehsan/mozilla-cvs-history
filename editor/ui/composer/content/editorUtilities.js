/*
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation. Portions created by Netscape are
 * Copyright (C) 1998-1999 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 *   Pete Collins
 *   Brian King
 *   Daniel Glazman <glazman@netscape.com>
 */
/*
  if we ever need to use a different string bundle, use srGetStrBundle
  by including
  <script type="application/x-javascript" src="chrome://global/content/strres.js"/>
  e.g.:
  var bundle = srGetStrBundle("chrome://global/locale/filepicker.properties");
*/

/**** NAMESPACES ****/
const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";

// Each editor window must include this file
// Variables  shared by all dialogs:
var editorShell;

// Object to attach commonly-used widgets (all dialogs should use this)
var gDialog = {};

// Bummer! Can't get at enums from nsIDocumentEncoder.h
// http://lxr.mozilla.org/seamonkey/source/content/base/public/nsIDocumentEncoder.h#111
var gStringBundle;
var gIOService;
var gPrefsService;
var gPrefsBranch;
var gFilePickerDirectory;

var gOS = "";
const gWin = "Win";
const gUNIX = "UNIX";
const gMac = "Mac";

/************* Message dialogs ***************/

function AlertWithTitle(title, message)
{
  var promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService();
  promptService = promptService.QueryInterface(Components.interfaces.nsIPromptService);

  if (promptService)
  {
    if (!title)
      title = GetString("Alert");

    // "window" is the calling dialog window
    promptService.alert(window, title, message);
  }
}

// Optional: Caller may supply text to substitue for "Ok" and/or "Cancel"
function ConfirmWithTitle(title, message, okButtonText, cancelButtonText)
{
  var promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService();
  promptService = promptService.QueryInterface(Components.interfaces.nsIPromptService);

  if (promptService)
  {
    var result = {value:0};
    var okFlag = okButtonText ? promptService.BUTTON_TITLE_IS_STRING : promptService.BUTTON_TITLE_OK;
    var cancelFlag = cancelButtonText ? promptService.BUTTON_TITLE_IS_STRING : promptService.BUTTON_TITLE_CANCEL;

    promptService.confirmEx(window, title, message,
                            (okFlag * promptService.BUTTON_POS_0) +
                            (cancelFlag * promptService.BUTTON_POS_1),
                            okButtonText, cancelButtonText, null, null, {value:0}, result);
    return (result.value == 0);      
  }
  return false;
}

/************* String Utilities ***************/

function GetString(name)
{
  if (editorShell)
  {
    try {
      return editorShell.GetString(name);
    } catch (e) {}
  }
  else
  {
    // Non-editors (like prefs) may use these methods
    if (!gStringBundle)
    {
      gStringBundle = srGetStrBundle("chrome://editor/locale/editor.properties");
      if (!gStringBundle)
        return null;
    }
    try {
      return gStringBundle.GetStringFromName(name);
    } catch (e) {}
  }
  return null;
}

function TrimStringLeft(string)
{
  if(!string) return "";
  return string.replace(/^\s+/, "");
}

function TrimStringRight(string)
{
  if (!string) return "";
  return string.replace(/\s+$/, '');
}

// Remove whitespace from both ends of a string
function TrimString(string)
{
  if (!string) return "";
  return string.replace(/(^\s+)|(\s+$)/g, '')
}

function IsWhitespace(string)
{
  return /^\s/.test(string);
}

function TruncateStringAtWordEnd(string, maxLength, addEllipses)
{
  // Return empty if string is null, undefined, or the empty string
  if (!string)
    return "";

  // We assume they probably don't want whitespace at the beginning
  string = string.replace(/^\s+/, '');
  if (string.length <= maxLength)
    return string;

  // We need to truncate the string to maxLength or fewer chars
  if (addEllipses)
    maxLength -= 3;
  string = string.replace(RegExp("(.{0," + maxLength + "})\\s.*"), "$1")

  if (string.length > maxLength)
    string = string.slice(0, maxLength);

  if (addEllipses)
    string += "...";
  return string;
}

// Replace all whitespace characters with supplied character
// E.g.: Use charReplace = " ", to "unwrap" the string by removing line-end chars
//       Use charReplace = "_" when you don't want spaces (like in a URL)
function ReplaceWhitespace(string, charReplace)
{
  return string.replace(/(^\s+)|(\s+$)/g,'').replace(/\s+/g,charReplace)
}

// Replace whitespace with "_" and allow only HTML CDATA
//   characters: "a"-"z","A"-"Z","0"-"9", "_", ":", "-", ".",
//   and characters above ASCII 127
function ConvertToCDATAString(string)
{
  return string.replace(/\s+/g,"_").replace(/[^a-zA-Z0-9_\.\-\:\u0080-\uFFFF]+/g,'');
}

function GetSelectionAsText()
{
  return editorShell.GetContentsAs("text/plain", 1); // OutputSelectionOnly
}


/************* Element enbabling/disabling ***************/

// this function takes an elementID and a flag
// if the element can be found by ID, then it is either enabled (by removing "disabled" attr)
// or disabled (setAttribute) as specified in the "doEnable" parameter
function SetElementEnabledById(elementID, doEnable)
{
  SetElementEnabled(document.getElementById(elementID), doEnable);
}

function SetElementEnabled(element, doEnable)
{
  if ( element )
  {
    if ( doEnable )
      element.removeAttribute("disabled");
    else
      element.setAttribute("disabled", "true");
  }
  else
  {
    dump("Element  not found in SetElementEnabled\n");
  }
}

function SetElementHidden(element, hide)
{
  if (element)
  {
    if (hide)
      element.setAttribute("hidden", true);
    else
      element.removeAttribute("hidden");
  }
}

function DisableItem(id, disable)
{
  var item = document.getElementById(id);
  if (item)
  {
    if (disable != (item.getAttribute("disabled") == "true"))
      item.setAttribute("disabled", disable ? "true" : "");
  }
}


/************* Services / Prefs ***************/

function GetIOService()
{
  if (gIOService)
    return gIOService;

  gIOService = Components.classes["@mozilla.org/network/io-service;1"]
               .getService(Components.interfaces.nsIIOService);

  return gIOService;
}

function GetPrefsService()
{
  if (gPrefsService)
    return gPrefsService;

  try {
    gPrefsService = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);
  }
  catch(ex) {
    dump("failed to get prefs service!\n");
  }

  return gPrefsService;
}

function GetPrefs()
{
  if (gPrefsBranch)
    return gPrefsBranch;

  try {
    var prefService = GetPrefsService();
    if (prefService)
      gPrefsBranch = prefService.getBranch(null);

    if (gPrefsBranch)
      return gPrefsBranch;
    else
      dump("failed to get root prefs!\n");
  }
  catch(ex) {
    dump("failed to get root prefs!\n");
  }
  return null;
}

function SetUnicharPref(aPrefName, aPrefValue)
{
  var prefs = GetPrefs();
  if (prefs)
  {
    try {
      var str = Components.classes["@mozilla.org/supports-wstring;1"]
                          .createInstance(Components.interfaces.nsISupportsWString);
      str.data = aPrefValue;
      prefs.setComplexValue(aPrefName, Components.interfaces.nsISupportsWString, str);
    }
    catch(e) {}
  }
}

function GetUnicharPref(aPrefName, aDefVal)
{
  var prefs = GetPrefs();
  if (prefs)
  {
    try {
      return prefs.getComplexValue(aPrefName, Components.interfaces.nsISupportsWString).data;
    }
    catch(e) {}
  }
  return "";
}

// Set initial directory for a filepicker from URLs saved in prefs
function SetFilePickerDirectory(filePicker, fileType)
{
  if (filePicker)
  {
    try {
      var prefBranch = GetPrefs();
      if (prefBranch)
      {
        // Save current directory so we can reset it in SaveFilePickerDirectory
        gFilePickerDirectory = filePicker.displayDirectory;

        var location = prefBranch.getComplexValue("editor.lastFileLocation."+fileType, Components.interfaces.nsILocalFile);
        if (location)
          filePicker.displayDirectory = location;
      }
    }
    catch(e) {}
  }
}

// Save the directory of the selected file to prefs
function SaveFilePickerDirectory(filePicker, fileType)
{
  if (filePicker && filePicker.file)
  {
    try {
      var prefBranch = GetPrefs();

      var fileDir;
      if (filePicker.file.parent)
        fileDir = filePicker.file.parent.QueryInterface(Components.interfaces.nsILocalFile);

      if (prefBranch)
       prefBranch.setComplexValue("editor.lastFileLocation."+fileType, Components.interfaces.nsILocalFile, fileDir);
    
      var prefsService = GetPrefsService();
        prefsService.savePrefFile(null);
    } catch (e) {}
  }

  // Restore the directory used before SetFilePickerDirectory was called;
  // This reduces interference with Browser and other module directory defaults
  if (gFilePickerDirectory)
    filePicker.displayDirectory = gFilePickerDirectory;

  gFilePickerDirectory = null;
}

function GetDefaultBrowserColors()
{
  var prefs = GetPrefs();
  var colors = new Object();
  var useSysColors = false;
  colors.TextColor = 0;
  colors.BackgroundColor = 0;
  try { useSysColors = prefs.getBoolPref("browser.display.use_system_colors"); } catch (e) {}

  if (!useSysColors)
  {
    try { colors.TextColor = prefs.getCharPref("browser.display.foreground_color"); } catch (e) {}

    try { colors.BackgroundColor = prefs.getCharPref("browser.display.background_color"); } catch (e) {}
  }
  // Use OS colors for text and background if explicitly asked or pref is not set
  if (!colors.TextColor)
    colors.TextColor = "windowtext";

  if (!colors.BackgroundColor)
    colors.BackgroundColor = "window";

  colors.LinkColor = prefs.getCharPref("browser.anchor_color");
  colors.VisitedLinkColor = prefs.getCharPref("browser.visited_color");

  return colors;
}

/************* URL handling ***************/

function TextIsURI(selectedText)
{
  if (selectedText)
  {
    var text = selectedText.toLowerCase();
    return text.match(/^http:\/\/|^https:\/\/|^file:\/\/|^ftp:\/\/|\
                      ^about:|^mailto:|^news:|^snews:|^telnet:|\
                      ^ldap:|^ldaps:|^gopher:|^finger:|^javascript:/);
  }
  return false;
}

function IsUrlAboutBlank(urlString)
{
  return (urlString == "about:blank");
}

function MakeRelativeUrl(url)
{
  var inputUrl = TrimString(url);
  if (!inputUrl)
    return inputUrl;

  // Get the filespec relative to current document's location
  // NOTE: Can't do this if file isn't saved yet!
  var docUrl = GetDocumentBaseUrl();
  var docScheme = GetScheme(docUrl);

  // Can't relativize if no doc scheme (page hasn't been saved)
  if (!docScheme)
    return inputUrl;

  var urlScheme = GetScheme(inputUrl);

  // Do nothing if not the same scheme or url is already relativized
  if (docScheme != urlScheme)
    return inputUrl;

  var IOService = GetIOService();
  if (!IOService)
    return inputUrl;

  // Host must be the same
  var docHost = GetHost(docUrl);
  var urlHost = GetHost(inputUrl);
  if (docHost != urlHost)
    return inputUrl;


  // Get just the file path part of the urls
  var docPath = IOService.extractUrlPart(docUrl, IOService.url_Path, {start:0}, {end:0}); 
  var urlPath = IOService.extractUrlPart(inputUrl, IOService.url_Path, {start:0}, {end:0});

  // We only return "urlPath", so we can convert
  //  the entire docPath for case-insensitive comparisons
  var os = GetOS();
  var doCaseInsensitive = (docScheme == "file" && os == gWin);
  if (doCaseInsensitive)
    docPath = docPath.toLowerCase();

  // Get document filename before we start chopping up the docPath
  var docFilename = GetFilename(docPath);

  // Both url and doc paths now begin with "/"
  // Look for shared dirs starting after that
  urlPath = urlPath.slice(1);
  docPath = docPath.slice(1);

  var firstDirTest = true;
  var nextDocSlash = 0;
  var done = false;

  // Remove all matching subdirs common to both doc and input urls
  do {
    nextDocSlash = docPath.indexOf("\/");
    var nextUrlSlash = urlPath.indexOf("\/");

    if (nextUrlSlash == -1)
    {
      // We're done matching and all dirs in url
      // what's left is the filename
      done = true;

      // Remove filename for named anchors in the same file
      if (nextDocSlash == -1 && docFilename)
      { 
        var anchorIndex = urlPath.indexOf("#");
        if (anchorIndex > 0)
        {
          var urlFilename = doCaseInsensitive ? urlPath.toLowerCase() : urlPath;
        
          if (urlFilename.indexOf(docFilename) == 0)
            urlPath = urlPath.slice(anchorIndex);
        }
      }
    }
    else if (nextDocSlash >= 0)
    {
      // Test for matching subdir
      var docDir = docPath.slice(0, nextDocSlash);
      var urlDir = urlPath.slice(0, nextUrlSlash);
      if (doCaseInsensitive)
        urlDir = urlDir.toLowerCase();

      if (urlDir == docDir)
      {

        // Remove matching dir+"/" from each path
        //  and continue to next dir
        docPath = docPath.slice(nextDocSlash+1);
        urlPath = urlPath.slice(nextUrlSlash+1);
      }
      else
      {
        // No match, we're done
        done = true;

        // Be sure we are on the same local drive or volume 
        //   (the first "dir" in the path) because we can't 
        //   relativize to different drives/volumes.
        // UNIX doesn't have volumes, so we must not do this else
        //  the first directory will be misinterpreted as a volume name
        if (firstDirTest && docScheme == "file" && os != gUNIX)
          return inputUrl;
      }
    }
    else  // No more doc dirs left, we're done
      done = true;

    firstDirTest = false;
  }
  while (!done);

  // Add "../" for each dir left in docPath
  while (nextDocSlash > 0)
  {
    urlPath = "../" + urlPath;
    nextDocSlash = docPath.indexOf("\/", nextDocSlash+1);
  }
  return urlPath;
}

function MakeAbsoluteUrl(url)
{
  var resultUrl = TrimString(url);
  if (!resultUrl)
    return resultUrl;

  // Check if URL is already absolute, i.e., it has a scheme
  var urlScheme = GetScheme(resultUrl);

  if (urlScheme)
    return resultUrl;

  var docUrl = GetDocumentBaseUrl();
  var docScheme = GetScheme(docUrl);

  // Can't relativize if no doc scheme (page hasn't been saved)
  if (!docScheme)
    return resultUrl;

  var  IOService = GetIOService();
  if (!IOService)
    return resultUrl;
  
  // Make a URI object to use its "resolve" method
  var absoluteUrl = resultUrl;
  var docUri = IOService.newURI(docUrl, null, null);

  try {
    absoluteUrl = docUri.resolve(resultUrl);
    // This is deprecated and buggy! 
    // If used, we must make it a path for the parent directory (remove filename)
    //absoluteUrl = IOService.resolveRelativePath(resultUrl, docUrl);
  } catch (e) {}

  return absoluteUrl;
}

// Get the HREF of the page's <base> tag or the document location
// returns empty string if no base href and document hasn't been saved yet
function GetDocumentBaseUrl()
{
  if (window.editorShell)
  {
    var docUrl;

    // if document supplies a <base> tag, use that URL instead 
    var baseList = editorShell.editorDocument.getElementsByTagName("base");
    if (baseList)
    {
      var base = baseList.item(0);
      if (base)
        docUrl = base.getAttribute("href");
    }
    if (!docUrl)
      docUrl = GetDocumentUrl();

    if (!IsUrlAboutBlank(docUrl))
      return docUrl;
  }
  return "";
}

function GetDocumentUrl()
{
  if (editorShell && editorShell.editorDocument)
  {
    try {
      var aDOMHTMLDoc = editorShell.editorDocument.QueryInterface(Components.interfaces.nsIDOMHTMLDocument);
      return aDOMHTMLDoc.URL;
    }
    catch (e) {}
  }
  return "";
}

// Extract the scheme (e.g., 'file', 'http') from a URL string
function GetScheme(url)
{
  var resultUrl = TrimString(url);
  // Unsaved document URL has no acceptable scheme yet
  if (!resultUrl || IsUrlAboutBlank(resultUrl))
    return "";

  if (/^internal-gopher-/.test(resultUrl))
    return "internal-gopher-";

  var IOService = GetIOService();
  if (!IOService)
    return "";

  var scheme = "";
  try {
    // This fails if there's no scheme
    scheme = IOService.extractScheme(resultUrl, {schemeStartPos:0}, {schemeEndPos:0});
  } catch (e) {}

  return scheme ? scheme.toLowerCase() : "";
}

function GetHost(url)
{
  if (!url)
    return "";

  var IOService = GetIOService();
  if (!IOService)
    return "";

  var host = "";
  try {
    host = IOService.extractUrlPart(url, IOService.url_Host, {start:0}, {end:0}); 
   } catch (e) {}

  return host;
}

function GetUsername(url)
{
  if (!url)
    return "";

  var IOService = GetIOService();
  if (!IOService)
    return "";

  var username = "";
  try {
    username = IOService.extractUrlPart(url, IOService.url_Username, {start:0}, {end:0});
  } catch (e) {}

  return username;
}

function GetFilename(url)
{
  if (!url || IsUrlAboutBlank(url))
    return "";

  var IOService = GetIOService();
  if (!IOService)
    return "";

  var filename;

  try {
    filename = IOService.extractUrlPart(url, IOService.url_FileBaseName, {start:0}, {end:0});
    if (filename)
    {
      var ext = IOService.extractUrlPart(url, IOService.url_FileExtension, {start:0}, {end:0});
      if (ext)
        filename += "."+ext;
    }
  } catch (e) {}

  return filename ? filename : "";
}

// Return the url with username and password (preHost) removed
function StripUsernamePassword(url)
{
  if (url)
  {
    try {
      uri = Components.classes["@mozilla.org/network/standard-url;1"].createInstance(Components.interfaces.nsIURI);
      uri.spec = url;
      return StripUsernamePasswordFromURI(uri);
    } catch (e) {}    
  }
  return url;
}

function StripUsernamePasswordFromURI(uri)
{
  var url = "";
  if (uri)
  {
    try {
      url = uri.spec;
      var userPass = uri.userPass;
      if (userPass)
      {
        start = url.indexOf(userPass);
        url = url.slice(0, start) + url.slice(start+userPass.length+1);
      }
    } catch (e) {}    
  }
  return url;
}

function GetOS()
{
  if (gOS)
    return gOS;

  var platform = navigator.platform.toLowerCase();

  if (platform.indexOf("win") != -1)
    gOS = gWin;
  else if (platform.indexOf("mac") != -1)
    gOS = gMac;
  else if (platform.indexOf("unix") != -1 || platform.indexOf("linux") != -1 || platform.indexOf("sun") != -1)
    gOS = gUNIX;
  else
    gOS = "";
  // Add other tests?

  return gOS;
}

function ConvertRGBColorIntoHEXColor(color)
{
  if (color.search( /rgb.*/ ) != -1)
  {
    var res = color.match( /rgb\((\d*),(\d*),(\d*)\)/ );
    var r = Number(RegExp.$1).toString(16);
    if (r.length == 1) r = "0"+r;
    var g = Number(RegExp.$2).toString(16);
    if (g.length == 1) g = "0"+g;
    var b = Number(RegExp.$3).toString(16);
    if (b.length == 1) b = "0"+b;
    return "#"+r+g+b;
  }
  else
  {
    return color;
  }
}

/************* CSS ***************/

function GetHTMLOrCSSStyleValue(element, attrName, cssPropertyName)
{
  var prefs = GetPrefs();
  var IsCSSPrefChecked = prefs.getBoolPref("editor.use_css");
  var value;
  if (IsCSSPrefChecked && editorShell.editorType == "html")
  {
    value = element.style.getPropertyValue(cssPropertyName);
    if (value == "") {
      value = element.getAttribute(attrName);
    }
  }
  else
  {
    value = element.getAttribute(attrName);
  }
  if (value == null) {
    value = "";
  }
  return value;
}
