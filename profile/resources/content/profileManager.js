var selected    = null;
var currProfile = "";
var profile = Components.classes["component://netscape/profile/manager"].createInstance();
profile = profile.QueryInterface(Components.interfaces.nsIProfile);             
//dump("profile = " + profile + "\n"); 

function openCreateProfile()
{
	// Need to call CreateNewProfile xuls
	window.openDialog('chrome://profile/content/createProfileWizard.xul', 'CPW', 'chrome');
}

function CreateProfile()
{
	this.location.href = "chrome://profile/content/profileManager.xul";
}

function RenameProfile(w)
{
	if (!selected)
	{
		//dump("Select a profile to rename.\n");
		return;
	}

	var newName = w.document.getElementById("NewName").value;
	var oldName = selected.getAttribute("rowName");
	var migrate = selected.getAttribute("rowMigrate");

	if (migrate == "true")
	{
		//dump("Migrate this profile before renaming it.\n");
		return;
	}

	//dump("RenameProfile : " + oldName + " to " + newName + "\n");
	try {
		profile.renameProfile(oldName, newName);
        }
	catch (ex) {
		alert("Sorry, failed to rename profile.");
	}
	//this.location.replace(this.location);
	this.location.href = "chrome://profile/content/profileManager.xul";
}

function DeleteProfile(deleteFilesFlag)
{
	if (!selected)
	{
		//dump("Select a profile to delete.\n");
		return;
	}

	var migrate = selected.getAttribute("rowMigrate");

	var name = selected.getAttribute("rowName");
	//dump("Delete '" + name + "'\n");
	try {
		profile.deleteProfile(name, deleteFilesFlag);
	}
	catch (ex) {
		alert("Sorry, failed to delete profile.");
	}
	//this.location.replace(this.location);
	//this.location.href = this.location;
	this.location.href = "chrome://profile/content/profileManager.xul";
}

function StartCommunicator()
{
	//dump("************Inside Start Communicator prof\n");
	if (!selected)
	{
		//dump("Select a profile to migrate.\n");
		return;
	}

	var migrate = selected.getAttribute("rowMigrate");
	var name = selected.getAttribute("rowName");

	if (migrate == "true")
	{
		profile.migrateProfile(name);
	}

	//dump("************name: "+name+"\n");
	profile.startCommunicator(name);
	ExitApp();
}

function ExitApp()
{
	// Need to call this to stop the event loop
        var appShell = Components.classes['component://netscape/appshell/appShellService'].getService();
        appShell = appShell.QueryInterface( Components.interfaces.nsIAppShellService);
        appShell.Quit();
}

function showSelection(node)
{
	//dump("************** In showSelection routine!!!!!!!!! \n");
	// (see tree's onclick definition)
	// Tree events originate in the smallest clickable object which is the cell.  The object 
	// originating the event is available as event.target.  We want the cell's row, so we go 
	// one further and get event.target.parentNode.
	selected = node;
	var num = node.getAttribute("rowNum");
	//dump("num: "+num+"\n");

	var name = node.getAttribute("rowName");
	//dump("name: "+name+"\n");

	//dump("Selected " + num + " : " + name + "\n");
}

function addTreeItem(num, name, migrate)
{
  //dump("Adding element " + num + " : " + name + "\n");
  var body = document.getElementById("theTreeBody");

  var newitem = document.createElement('treeitem');
  var newrow = document.createElement('treerow');
  newrow.setAttribute("rowNum", num);
  newrow.setAttribute("rowName", name);
  newrow.setAttribute("rowMigrate", migrate);

  var elem = document.createElement('treecell');

  // Hack in a differentation for migration
  if (migrate)
	  var text = document.createTextNode('Migrate');
  else
	  var text = document.createTextNode('');

  elem.appendChild(text);
  newrow.appendChild(elem);

  var elem = document.createElement('treecell');
  var text = document.createTextNode(name);
  elem.appendChild(text);
  newrow.appendChild(elem);

  newitem.appendChild(newrow);
  body.appendChild(newitem);
}

function loadElements()
{
	//dump("****************hacked onload handler adds elements to tree widget\n");
	var profileList = "";

	profileList = profile.getProfileList();	

	//dump("Got profile list of '" + profileList + "'\n");
	profileList = profileList.split(",");
	try {
		currProfile = profile.currentProfile;
	}
	catch (ex) {
		if (profileList != "") {
			currProfile = profileList;
		}
	}

	for (var i=0; i < profileList.length; i++)
	{
		var pvals = profileList[i].split(" - ");
		addTreeItem(i, pvals[0], (pvals[1] == "migrate"));
	}
}

function openRename()
{
	if (!selected) {
		//dump("Select a profile to rename.\n");
	}
	else
	{
		var migrate = selected.getAttribute("rowMigrate");
		if (migrate == "true") {
			//dump("Migrate the profile before renaming it.\n");
		}
		else
			var win = window.openDialog('chrome://profile/content/renameProfile.xul', 'Renamer', 'chrome');
	}
}


function ConfirmDelete() 
{
	if (!selected)
	{
		//dump("Select a profile to delete.\n");
		return;
	}

	var migrate = selected.getAttribute("rowMigrate");
	var name = selected.getAttribute("rowName");

	if (migrate == "true")
	{
		//dump("Migrate this profile before deleting it.\n");
		return;
	}

    var win = window.openDialog('chrome://profile/content/deleteProfile.xul', 'Deleter', 'chrome');
    return win;
}


function ConfirmMigrateAll() 
{
    var win = window.openDialog('chrome://profile/content/profileManagerMigrateAll.xul', 'MigrateAll', 'chrome');
    return win;
}


// -------------------------------------------- begin Hack for OnLoad handling
setTimeout('loadElements()', 0);
// -------------------------------------------- end   Hack for OnLoad handling
