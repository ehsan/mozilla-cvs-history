
var xpi = new Object(); 
var numxpi = 0; 
var numstatus = 0; 

function showprops(obj) { 
  var props = ""; 
  for (i in obj) { 
     props += i+":"+obj[i]+"\n  "; 
  } 
  alert(props); 
} 

function statusCallback(url,status) { 
    for (i in xpi) { 
        if ( url.indexOf(xpi[i]) != -1 ) { 
            xpi[i] = status; 
            numstatus++; 
            break; 
        } 
    } 

    // if we've gotten all results then display them 
    if (numstatus == numxpi) 
    { 
        var text; 
        var restart = false; 
        dlg = window.open("","resultWindow"); 
        dlg.document.write("<head><title>XPInstall Results</title></head>"); 
        dlg.document.write("<body><h1>XPInstall Results</h1>"); 
        for (i in xpi) 
        { 
            text = "    "+i+": "; 
            switch (status) { 
              case 999: 
                 restart = true;     // fall-through 
              case 0: 
                 text += "Successful"; 
                 break; 
              default: 
                 text += "Error encountered -- "+status; 
                 break; 
            } 
            text += "<br>"; 
            dlg.document.write(text); 
        } 
        if (restart) { 
            dlg.document.write("<p>Some files were in use, you must restart to complete the installation"); 
        } 

        dlg.document.write("</body>"); 
        dlg.document.close(); 
    } 
} 

function launchwindows() 
{ 

    xpi["PSM"] = "psm_1.2_win32.xpi"; 
    numxpi ++; 

     // showprops(xpi); 
    InstallTrigger.install(xpi,statusCallback); 
} 

function launchlinux() 
{ 
    xpi["PSM"] = "psm_1.2_linux.xpi"; 
    numxpi ++; 

     // showprops(xpi); 
    InstallTrigger.install(xpi,statusCallback); 
} 
