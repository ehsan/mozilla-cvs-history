//GLOBALS

//pref service and pref string
var prefInt;  

//locale bundles
var regionsBundle;
var languagesBundle;
var acceptedBundle;

//dictionary of all supported locales
var availLanguageDict;

//XUL tree handles
var available_languages;
var available_languages_treeroot;

//XUL tree handles
var active_languages;
var active_languages_treeroot;

//XUL window pref window interface object
var prefwindow_proxy_object;

function GetBundles()
{
  if (!regionsBundle)    regionsBundle   = srGetStrBundle("chrome://global/locale/regionNames.properties"); 
  if (!languagesBundle)  languagesBundle = srGetStrBundle("chrome://global/locale/languageNames.properties"); 
  if (!acceptedBundle)   acceptedBundle  = srGetStrBundle("resource:/res/language.properties"); 
}


function Init()
{
  dump("********** Init()\n");    

  try {
    GetBundles()
	  ReadAvailableLanguages();
  }

  catch(ex) {
    dump("*** Couldn't get string bundles\n");
  }

  if (!window.arguments) {
    
    try {
      //base window
      active_languages              = document.getElementById('active_languages'); 
      active_languages_treeroot     = document.getElementById('active_languages_root'); 
      prefwindow_proxy_object       = document.getElementById('intlAcceptLanguages');
    } //try

    catch(ex) {
      dump("*** Couldn't get XUL element handles\n");
    } //catch

    try {
      parent.initPanel('chrome://communicator/content/pref/pref-languages2.xul');
    }

    catch(ex) {

      dump("*** Couldn't initialize pref panel\n");
      //pref service backup

    } //catch


      //get pref service as backup
	    try
	    {
        if (!prefInt) {
		      
          prefInt = Components.classes["component://netscape/preferences"];

		      if (prefInt) {
			      prefInt = prefInt.getService();
			      prefInt = prefInt.QueryInterface(Components.interfaces.nsIPref);
  		      prefwindow_proxy_object.value = prefInt.CopyCharPref("intl.accept_languages");
            if (!prefwindow_proxy_object.value) prefwindow_proxy_object.value = "en";
		      } //if
        } //if
      } //try

	    catch(ex)
	    {
		    dump("failed to get pref service!\n");
		    prefInt = null;
	    }



    try {
      dump("*** Language PrefString: " + prefwindow_proxy_object.value + "\n");
    } //try

    catch(ex) {
      dump("*** Pref object doesn't exist\n");
    } //catch

  	LoadActiveLanguages();

  } else {

    try {

      //add language popup
      available_languages 			    = document.getElementById('available_languages'); 
      available_languages_treeroot  = document.getElementById('available_languages_root');
      active_languages		          = window.opener.document.getElementById('active_languages'); 
      active_languages_treeroot     = window.opener.document.getElementById('active_languages_root'); 
      prefwindow_proxy_object       = window.opener.document.getElementById('intlAcceptLanguages');

    } //try

    catch(ex) {
      dump("*** Couldn't get XUL element handles\n");
    } //catch

	  LoadAvailableLanguages();

  }
}


function AddLanguage() 
{
    dump("********** AddLanguage()\n");    
    window.openDialog("chrome://communicator/content/pref/pref-languages-add.xul","","modal=yes,chrome,resizable=no", "addlangwindow");
    UpdatePrefString();

}


function ReadAvailableLanguages()
{

    availLanguageDict		= new Array();
    var visible = new String();
    var str = new String();
    var i =0;

    acceptedBundleEnum = acceptedBundle.getSimpleEnumeration(); 

    while (acceptedBundleEnum.HasMoreElements()) { 			

       //progress through the bundle 
       curItem = acceptedBundleEnum.GetNext(); 

       //"unpack" the item, nsIPropertyElement is now partially scriptable 
       curItem = curItem.QueryInterface(Components.interfaces.nsIPropertyElement); 

       //dump string name (key) 
       var stringName = curItem.getKey(); 
       stringNameProperty = stringName.split('.');

       if (stringNameProperty[1] == 'accept') {

          //dump the UI string (value) 
           visible   = curItem.getValue(); 

          //if (visible == 'true') {

             str = stringNameProperty[0];
             stringLangRegion = stringNameProperty[0].split('-');
                     
             if (stringLangRegion[0]) {

                 try {   
                    var tit = languagesBundle.GetStringFromName(stringLangRegion[0]);
                 }
          
                 catch (ex) {
                    dump("No language string for:" + stringLangRegion[1] + "\n");
                 }


                 if (stringLangRegion[1]) {
                 
                   try {   
                    var tit = tit + "/" + regionsBundle.GetStringFromName(stringLangRegion[1]);
                   }
               
                   catch (ex) {
                      dump("No region string for:" + stringLangRegion[1] + "\n");
                   }

                 } //if region

                 tit = tit + "  [" + str + "]";

             } //if language
				    
  		       if (str) if (tit) {
                
				        availLanguageDict[i] = new Array(2);
				        availLanguageDict[i][0]	= tit;	
				        availLanguageDict[i][1]	= str;
				        availLanguageDict[i][2]	= visible;
                i++;

				        if (tit) {}
				        else dump('Not label for :' + str + ', ' + tit+'\n');
			        
             } // if str&tit
            //} //if visible
			    } //if accepted
		    } //while

		availLanguageDict.sort();
}


function LoadAvailableLanguages()
{

  dump("Loading available languages!\n");
  
		if (availLanguageDict) for (i = 0; i < availLanguageDict.length; i++) {

  		if (availLanguageDict[i][2] == 'true') { 	

        AddTreeItem(document, available_languages_treeroot, availLanguageDict[i][1], availLanguageDict[i][0]);
   
      } //if

		} //for
}


function LoadActiveLanguages()
{

  //dump("Loading: " + prefwindow_proxy_object.value + "!\n");
  
  try {
	  arrayOfPrefs = prefwindow_proxy_object.value.split(', ');
  } 
  
  catch (ex) {
	  dump("failed to split the preference string!\n");
  }
	
		if (arrayOfPrefs) for (i = 0; i < arrayOfPrefs.length; i++) {

			str = arrayOfPrefs[i];
      tit = GetLanguageTitle(str);
      
			if (str) if (tit) {

        AddTreeItem(document, active_languages_treeroot, str, tit);

			} //if 
		} //for
}


function LangAlreadyActive(langId)
{
  try {
    if (prefwindow_proxy_object.value.indexOf(langId) != -1)
      return true;
    else
      return false;
  }

  catch(ex){
     return false;
  }
}


function AddAvailableLanguage()
{
  var Languagename = new String();
	var Languageid = new String();


  //selected languages
  for (var nodeIndex=0; nodeIndex < available_languages.selectedItems.length; nodeIndex++) {
  
    var selItem =  available_languages.selectedItems[nodeIndex];
    var selRow  =  selItem.firstChild;
    var selCell =  selRow.firstChild;

	  Languagename		= selCell.getAttribute('value');
	  Languageid	    = selCell.getAttribute('id');
	  already_active	= false;	
    
    if (!LangAlreadyActive(Languageid)) {

      AddTreeItem(window.opener.document, active_languages_treeroot, Languageid, Languagename);

	  }//if

  } //loop selected languages


  //user-defined languages
  var otherField = document.getElementById( "languages.other" );
  
  if (otherField.value) {

    dump("Other field: " + otherField.value + "\n");
	  
    Languageid	    = otherField.value;
    Languageid      = Languageid.toLowerCase();
	  Languagename		= GetLanguageTitle(Languageid);
	  already_active	= false;	
  
    if (!LangAlreadyActive(Languageid)) {

      AddTreeItem(window.opener.document, active_languages_treeroot, Languageid, Languagename);

	  }//if
  }

  available_languages.clearItemSelection();
  UpdatePrefString();
  window.close();

} //AddAvailableLanguage


function RemoveActiveLanguage()
{

  var nextNode = null;
  var numSelected = active_languages.selectedItems.length;
  var deleted_all = false;

  while (active_languages.selectedItems.length > 0) {
    
	var selectedNode = active_languages.selectedItems[0];
    nextNode = selectedNode.nextSibling;
    
	if (!nextNode) 
	  
    if (selectedNode.previousSibling) 
		nextNode = selectedNode.previousSibling;
    
    var row  =  selectedNode.firstChild;
    var cell =  row.firstChild;

  	row.removeChild(cell);
	  selectedNode.removeChild(row);
    active_languages_treeroot.removeChild(selectedNode);

   } //while
  
  if (nextNode) {
    active_languages.selectItem(nextNode)
  } else {
    //active_languages.clearItemSelection();
  }

  UpdatePrefString();

} //RemoveActiveLanguage


function GetLanguageTitle(id) 
{
	
		if (availLanguageDict) for (j = 0; j < availLanguageDict.length; j++) {

			if ( availLanguageDict[j][1] == id) {	
				//title = 
				dump("found title for:" + id + " ==> " + availLanguageDict[j][0] + "\n");
				return availLanguageDict[j][0];
			}	
		}
		return '';
}


function AddTreeItem(doc, treeRoot, langID, langTitle)
{
	try {  //let's beef up our error handling for languages without label / title

			// Create a treerow for the new Language
			var item = doc.createElement('treeitem');
			var row  = doc.createElement('treerow');
			var cell = doc.createElement('treecell');

			// Copy over the attributes
			cell.setAttribute('value', langTitle);
			cell.setAttribute('id', langID);

			// Add it to the active languages tree
			item.appendChild(row);
			row.appendChild(cell);

			treeRoot.appendChild(item);
			dump("*** Added tree item: " + langTitle + "\n");

	} //try

	catch (ex) {
		dump("*** Failed to add item: " + langTitle + "\n");
	} //catch 

}


function UpdatePrefString()
{
  var num_languages = 0;

  dump("*** UpdatePrefString()\n");

  for (var item = active_languages_treeroot.firstChild; item != null; item = item.nextSibling) {

    row  =  item.firstChild;
    cell =  row.firstChild;
    languageid = cell.getAttribute('id');

	  if (languageid.length > 1) {
	  
          num_languages++;

		  //separate >1 languages by commas
		  if (num_languages > 1) {
			  prefwindow_proxy_object.value = prefwindow_proxy_object.value + "," + " " + languageid;
		  } else {
			  prefwindow_proxy_object.value = languageid;
		  } //if
	  } //if
  }//for

  dump("*** Pref string set to: " + prefwindow_proxy_object.value + "\n");

}

function Save()
{

  // Iterate through the 'active languages  tree to collect the languages
  // that the user has chosen. 

  dump('*** Save()\n');

  var row           = null;
  var cell          = null;
  var languageid    = new String();

  UpdatePrefString();

	//Save Prefs
  try
	{

    if (!prefInt) {
		
        prefInt = Components.classes["component://netscape/preferences"];

		    if (prefInt) {
			    prefInt = prefInt.getService();
			    prefInt = prefInt.QueryInterface(Components.interfaces.nsIPref);
		    }
    }

		if (prefInt)
		{
			prefInt.SetCharPref("intl.accept_languages", prefwindow_proxy_object.value);
			dump('*** saved pref: ' + prefwindow_proxy_object.value + '.\n');
		}
 		window.close();
	}

	catch(ex)
	{
		dump("*** Couldn't save!\n");
		window.close();
	}

} //Save


function MoveUp() {

  if (active_languages.selectedItems.length == 1) {
    var selected = active_languages.selectedItems[0];
    var before = selected.previousSibling
    if (before) {
      before.parentNode.insertBefore(selected, before);
      active_languages.selectItem(selected);
      active_languages.ensureElementIsVisible(selected);
    }
  }

  UpdatePrefString();

} //MoveUp

  
function MoveDown() {

  if (active_languages.selectedItems.length == 1) {
    var selected = active_languages.selectedItems[0];
    if (selected.nextSibling) {
      if (selected.nextSibling.nextSibling) {
        selected.parentNode.insertBefore(selected, selected.nextSibling.nextSibling);
      }
      else {
        selected.parentNode.appendChild(selected);
      }
      active_languages.selectItem(selected);
    }
  }

  UpdatePrefString();

} //MoveDown
