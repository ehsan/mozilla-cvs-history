var gSubscribetree = null;
var gCurrentServer = null;
var okCallback = null;

function SubscribeOnLoad()
{
	dump("SubscribeOnLoad()\n");

    gSubscribetree = document.getElementById('subscribetree');
    gCurrentServer = document.getElementById('currentserver');

	doSetOKCancel(subscribeOK,subscribeCancel);

	// look in arguments[0] for parameters
	if (window.arguments && window.arguments[0]) {
		if ( window.arguments[0].title ) {
			top.window.title = window.arguments[0].title;
		}
		
		if ( window.arguments[0].okCallback ) {
			top.okCallback = window.arguments[0].okCallback;
		}
	}
	
	// pre select the folderPicker, based on what they selected in the folder pane
	if (window.arguments[0].preselectedURI) {
		var folder = GetMsgFolderFromUri(window.arguments[0].preselectedURI);
		var server = folder.server;
		
		gSubscribetree.setAttribute('ref','urn:' + server.hostName);
		gCurrentServer.value = server.hostName;	// use gServer.prettyName?

		dump("for each child of news://" + server.hostName + " set subscribed to true in the datasource\n");

		var folders = folder.GetSubFolders();
		
		if (folders) {
			try {
				while (true) {
					var i = folders.currentItem();
					var f = i.QueryInterface(Components.interfaces.nsIMsgFolder);
					dump(f.URI+ "\n");
					folders.next();
				}
			}
			catch (ex) {
				dump("no more subfolders\n");
			}
		}
	}
}

function subscribeOK()
{
	dump("in subscribeOK()\n")
	if (top.okCallback) {
		// we stored the uri as the ref, now get it back
    	var tree = document.getElementById('subscribetree');
		//var uri = tree.getAttribute('ref');
		//top.okCallback(uri);
	}
	return true;
}

function subscribeCancel()
{
	dump("in subscribeCancel()\n")
	return true;
}

function SubscribeButtonClicked()
{
	dump("subscribe button clicked\n");
}

function UnsubscribeButtonClicked()
{
	dump("unsubscribe button clicked\n");
}

function SubscribeOnClick(event)
{
	dump("subscribe tree clicked\n");
	dump(event + "\n");
}
